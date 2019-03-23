#include "stdafx.h"
#include "phpqueue.h"
#include "phpbuffer.h"

namespace PA
{
    const char *CPhpQueue::PHP_QUEUE_KEY = "key";
    const char *CPhpQueue::PHP_QUEUE_MESSAGES = "messages";
    const char *CPhpQueue::PHP_QUEUE_FILESIZE = "fileSize";
    const char *CPhpQueue::PHP_QUEUE_MESSAGES_DEQUEUED = "messagesDequeued";
    const char *CPhpQueue::PHP_QUEUE_BYTES_DEQUEUED = "bytesDequeued";

    CPhpQueue::CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked) : CPhpBaseHandler(locked, aq, poolId), m_aq(aq), m_pBuff(new CPhpBuffer) {
    }

    Php::Value CPhpQueue::__get(const Php::Value & name) {
        if (name == "AutoNotified") {
            return m_aq->GetEnqueueNotified();
        } else if (name == "DequeueBatchSize") {
            return (int64_t) m_aq->GetDequeueBatchSize();
        }
        return CPhpBaseHandler::__get(name);
    }

    Php::Value CPhpQueue::CloseQueue(Php::Parameters & params) {
        unsigned int timeout;
        std::string key = GetKey(params[0]);
        std::shared_ptr<int> pErrCode;
        CAsyncQueue::DQueueTrans qt;
        size_t args = params.size();
        if (args > 1) {
            qt = SetQueueTransCallback(SPA::Queue::idClose, params[1], pErrCode, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idClose, pErrCode ? true : false);
        bool permanent = false;
        if (args > 3) {
            permanent = params[3].boolValue();
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pErrCode) {
                ReqSyncEnd(m_aq->CloseQueue(key.c_str(), qt, permanent, discarded), lk, timeout);
                return *pErrCode;
            }
            PopCallbacks();
        }
        return m_aq->CloseQueue(key.c_str(), qt, permanent, discarded);
    }

    Php::Value CPhpQueue::EnqueueBatch(Php::Parameters & params) {
        unsigned int timeout;
        if (!m_pBuff->GetBuffer()->GetSize()) {
            throw Php::Exception("No message batched yet");
        }
        std::string key = GetKey(params[0]);
        std::shared_ptr<SPA::INT64> pIndex;
        CAsyncQueue::DEnqueue c;
        size_t args = params.size();
        if (args > 1) {
            c = SetEnqueueResCallback(SPA::Queue::idEnqueueBatch, params[1], pIndex, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idEnqueueBatch, pIndex ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pIndex) {
                ReqSyncEnd(m_aq->EnqueueBatch(key.c_str(), m_pBuff->GetBuffer()->GetBuffer(), m_pBuff->GetBuffer()->GetSize(), c, discarded), lk, timeout);
                return *pIndex;
            }
            PopCallbacks();
        }
        return m_aq->EnqueueBatch(key.c_str(), m_pBuff->GetBuffer()->GetBuffer(), m_pBuff->GetBuffer()->GetSize(), c, discarded);
    }

    Php::Value CPhpQueue::ToDeqValue(SPA::CUQueue * q) {
        SPA::INT64 messages = 0, fileSize = 0;
        unsigned int messagesDequeued = 0, bytesDequeued = 0;
        if (q->GetSize() >= 24) {
            *q >> messages >> fileSize >> messagesDequeued >> bytesDequeued;
        } else {
            q->SetSize(0);
        }
        Php::Value v;
        v.set(PHP_QUEUE_MESSAGES, messages);
        v.set(PHP_QUEUE_FILESIZE, fileSize);
        v.set(PHP_QUEUE_MESSAGES_DEQUEUED, (int64_t) messagesDequeued);
        v.set(PHP_QUEUE_BYTES_DEQUEUED, (int64_t) bytesDequeued);
        return v;
    }

    Php::Value CPhpQueue::Unlock() {
        throw Php::Exception("Server queue doesn't support the method Unlock");
    }

    Php::Value CPhpQueue::Dequeue(Php::Parameters & params) {
        std::string key = GetKey(params[0]);
        CPVPointer callback(new Php::Value(params[1]));
        m_aq->ResultReturned = [callback, this](SPA::ClientSide::CAsyncServiceHandler *ash, unsigned short reqId, SPA::CUQueue & q) -> bool {
            if (reqId >= SPA::Queue::idEnqueue && reqId <= SPA::Queue::idEnqueueBatch) {
                return false;
            } else if (reqId == SPA::Queue::idBatchSizeNotified) {
                return false;
            }
            SPA::CScopeUQueue sb;
            sb << reqId;
            sb->Push(q.GetBuffer(), q.GetSize());
            PACallback cb;
            cb.CallbackType = ctDequeueResult;
            cb.Res = sb.Detach();
            cb.CallBack = callback;
            std::unique_lock<std::mutex> lk(this->m_mPhp);
            this->m_vCallback.push_back(cb);
            return true;
        };
        CQPointer pF(SPA::CScopeUQueue::Lock(), [](SPA::CUQueue * q) {
            SPA::CScopeUQueue::Unlock(q);
        });
        CAsyncQueue::DDequeue f = [pF, this](CAsyncQueue *aq, SPA::UINT64 messages, SPA::UINT64 fileSize, unsigned int messagesDequeued, unsigned int bytesDequeued) {
            *pF << messages << fileSize << messagesDequeued << bytesDequeued;
        };
        size_t args = params.size();
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idDequeue, true);
        unsigned int to = 0;
        if (args > 3) {
            int64_t o = params[3].numericValue();
            if (o < 0) {
                throw Php::Exception("Bad value for Dequeue timeout");
            }
            to = (unsigned int) o;
        }
        bool ok = m_aq->Dequeue(key.c_str(), f, to, discarded);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            PopCallbacks();
        }
        if (!ok) {
            m_aq->ResultReturned = nullptr;
            throw Php::Exception(PA::PHP_SOCKET_CLOSED + m_aq->GetAttachedClientSocket()->GetErrorMsg());
        }
        ok = m_aq->WaitAll();
        m_aq->ResultReturned = nullptr;
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            PopCallbacks();
        }
        if (!ok) {
            tagRequestReturnStatus rrs = GetRRS();
            switch (rrs) {
                case rrsCanceled:
                    throw Php::Exception(PHP_REQUEST_CANCELED);
                default:
                    throw Php::Exception(PA::PHP_SOCKET_CLOSED + m_aq->GetAttachedClientSocket()->GetErrorMsg());
            }
        }
        return ToDeqValue(pF.get());
    }

    Php::Value CPhpQueue::ToFlushValue(SPA::CUQueue * q) {
        SPA::INT64 messages, fileSize;
        *q >> messages >> fileSize;
        Php::Value v;
        v.set(PHP_QUEUE_MESSAGES, messages);
        v.set(PHP_QUEUE_FILESIZE, fileSize);
        return v;
    }

    Php::Value CPhpQueue::FlushQueue(Php::Parameters & params) {
        std::string key = GetKey(params[0]);
        unsigned int timeout = (~0);
        bool sync = false;
        CPVPointer callback;
        size_t args = params.size();
        if (args > 1) {
            const Php::Value& phpF = params[1];
            if (phpF.isNumeric()) {
                timeout = (unsigned int) phpF.numericValue();
                sync = true;
            } else if (phpF.isBool()) {
                sync = phpF.boolValue();
            } else if (phpF.isNull()) {
            } else if (!phpF.isCallable()) {
                throw Php::Exception("A callback required for Flush final result");
            }
            if (phpF.isCallable()) {
                callback.reset(new Php::Value(phpF));
            }
        }
        CQPointer pF;
        if (sync) {
            pF.reset(SPA::CScopeUQueue::Lock(), [](SPA::CUQueue * q) {
                SPA::CScopeUQueue::Unlock(q);
            });
        }
        CAsyncQueue::DFlush f = [callback, pF, this](CAsyncQueue *aq, SPA::UINT64 messages, SPA::UINT64 fileSize) {
            if (pF) {
                *pF << messages << fileSize;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << (unsigned short) SPA::Queue::idFlush << messages << fileSize;
                PACallback cb;
                cb.CallbackType = ctQueueFlush;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
        };
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idFlush, sync);
        SPA::tagOptimistic option = SPA::oMemoryCached;
        if (args > 3) {
            if (params[3].isNumeric()) {
                int64_t o = params[3].numericValue();
                if (o < 0 || o > SPA::oDiskCommitted) {
                    throw Php::Exception("Bad value for memory queue flush status");
                }
                option = (SPA::tagOptimistic)o;
            } else {
                throw Php::Exception("An integer value required for memory queue flush status");
            }
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (sync) {
                ReqSyncEnd(m_aq->FlushQueue(key.c_str(), f, option, discarded), lk, timeout);
                return ToFlushValue(pF.get());
            }
            PopCallbacks();
        }
        return m_aq->FlushQueue(key.c_str(), f, option, discarded);
    }

    Php::Value CPhpQueue::StartQueueTrans(Php::Parameters & params) {
        unsigned int timeout;
        std::string key = GetKey(params[0]);
        std::shared_ptr<int> pErrCode;
        CAsyncQueue::DQueueTrans qt;
        size_t args = params.size();
        if (args > 1) {
            qt = SetQueueTransCallback(SPA::Queue::idStartTrans, params[1], pErrCode, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idStartTrans, pErrCode ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pErrCode) {
                ReqSyncEnd(m_aq->StartQueueTrans(key.c_str(), qt, discarded), lk, timeout);
                return *pErrCode;
            }
            PopCallbacks();
        }
        return m_aq->StartQueueTrans(key.c_str(), qt, discarded);
    }

    CAsyncQueue::DQueueTrans CPhpQueue::SetQueueTransCallback(unsigned short reqId, const Php::Value& phpTrans, std::shared_ptr<int> &pErrCode, unsigned int &timeout) {
        timeout = (~0);
        bool sync = false;
        if (phpTrans.isNumeric()) {
            timeout = (unsigned int) phpTrans.numericValue();
            sync = true;
        } else if (phpTrans.isBool()) {
            sync = phpTrans.boolValue();
        } else if (phpTrans.isNull()) {
        } else if (!phpTrans.isCallable()) {
            throw Php::Exception("A callback required for queue transaction final result");
        }
        if (sync) {
            pErrCode.reset(new int);
        } else {
            pErrCode.reset();
        }
        CPVPointer callback;
        if (phpTrans.isCallable()) {
            callback.reset(new Php::Value(phpTrans));
        }
        CAsyncQueue::DQueueTrans qt = [reqId, callback, pErrCode, this](CAsyncQueue *aq, int errCode) {
            if (pErrCode) {
                *pErrCode = errCode;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << reqId << errCode;
                PACallback cb;
                cb.CallbackType = ctQueueTrans;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
        };
        return qt;
    }

    Php::Value CPhpQueue::EndQueueTrans(Php::Parameters & params) {
        unsigned int timeout;
        bool rollback = params[0].boolValue();
        std::shared_ptr<int> pErrCode;
        CAsyncQueue::DQueueTrans qt;
        size_t args = params.size();
        if (args > 1) {
            qt = SetQueueTransCallback(SPA::Queue::idEndTrans, params[1], pErrCode, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idEndTrans, pErrCode ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pErrCode) {
                ReqSyncEnd(m_aq->EndQueueTrans(rollback, qt, discarded), lk, timeout);
                return *pErrCode;
            }
            PopCallbacks();
        }
        return m_aq->EndQueueTrans(rollback, qt, discarded);
    }

    Php::Value CPhpQueue::GetKeys(Php::Parameters & params) {
        unsigned int timeout = (~0);
        bool sync = false;
        const Php::Value& phpGK = params[0];
        if (phpGK.isNumeric()) {
            timeout = (unsigned int) phpGK.numericValue();
            sync = true;
        } else if (phpGK.isBool()) {
            sync = phpGK.boolValue();
        } else if (phpGK.isNull()) {
        } else if (!phpGK.isCallable()) {
            throw Php::Exception("A callback required for GetKeys final result");
        }
        std::shared_ptr<std::vector < std::string>> pV;
        if (sync) {
            pV.reset(new std::vector<std::string>);
        }
        CPVPointer callback;
        if (phpGK.isCallable()) {
            callback.reset(new Php::Value(phpGK));
        }
        CAsyncQueue::DGetKeys gk = [callback, pV, this](CAsyncQueue *aq, const std::vector<std::string> &v) {
            if (pV) {
                *pV = v;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << (unsigned short) SPA::Queue::idGetKeys;
                for (auto &s : v) {
                    sb << s;
                }
                PACallback cb;
                cb.CallbackType = ctQueueGetKeys;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
        };
        size_t args = params.size();
        Php::Value phpCanceled;
        if (args > 1) {
            phpCanceled = params[1];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idGetKeys, sync);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (sync) {
                ReqSyncEnd(m_aq->GetKeys(gk, discarded), lk, timeout);
                return *pV;
            }
            PopCallbacks();
        }
        return m_aq->GetKeys(gk, discarded);
    }

    CAsyncQueue::DEnqueue CPhpQueue::SetEnqueueResCallback(unsigned short reqId, const Php::Value& phpF, std::shared_ptr<SPA::INT64> &pF, unsigned int &timeout) {
        timeout = (~0);
        bool sync = false;
        if (phpF.isNumeric()) {
            timeout = (unsigned int) phpF.numericValue();
            sync = true;
        } else if (phpF.isBool()) {
            sync = phpF.boolValue();
        } else if (phpF.isNull()) {
        } else if (!phpF.isCallable()) {
            throw Php::Exception("A callback required for Enqueue final result");
        }
        if (sync) {
            pF.reset(new SPA::INT64);
        } else {
            pF.reset();
        }
        CPVPointer callback;
        if (phpF.isCallable()) {
            callback.reset(new Php::Value(phpF));
        }
        CAsyncQueue::DEnqueue f = [reqId, callback, pF, this](CAsyncQueue *aq, SPA::UINT64 index) {
            if (pF) {
                *pF = (int64_t) index;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << reqId << index;
                PACallback cb;
                cb.CallbackType = ctEnqueueRes;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
        };
        return f;
    }

    std::string CPhpQueue::GetKey(const Php::Value & v) {
        std::string key = v.stringValue();
        Trim(key);
        if (!key.size()) {
            throw Php::Exception("Message queue key cannot be empty");
        }
        return key;
    }

    Php::Value CPhpQueue::Enqueue(Php::Parameters & params) {
        std::string key = GetKey(params[0]);
        int64_t idMsg = params[1].numericValue();
        if (idMsg <= SPA::idReservedTwo || idMsg > 0xffff) {
            throw Php::Exception("Bad message request Id");
        }
        unsigned int bytes = 0;
        const unsigned char *pBuffer = nullptr;
        Php::Value v;
        const Php::Value &q = params[2];
        if (q.instanceOf(SPA_NS + PHP_BUFFER)) {
            v = q.call(PHP_POPBYTES);
            pBuffer = (const unsigned char*) v.rawValue();
            bytes = (unsigned int) v.length();
        } else if (!q.isNull()) {
            throw Php::Exception("An instance of CUQueue or null required for Enqueue");
        }
        unsigned int timeout;
        std::shared_ptr<SPA::INT64> pF;
        CAsyncQueue::DEnqueue f;
        size_t args = params.size();
        if (args > 3) {
            f = SetEnqueueResCallback(SPA::Queue::idEnqueue, params[3], pF, timeout);
        }
        Php::Value phpCanceled;
        if (args > 4) {
            phpCanceled = params[4];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::Queue::idEnqueue, pF ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pF) {
                ReqSyncEnd(m_aq->Enqueue(key.c_str(), (unsigned short) idMsg, pBuffer, bytes, f, discarded), lk, timeout);
                return *pF;
            }
            PopCallbacks();
        }
        return m_aq->Enqueue(key.c_str(), (unsigned short) idMsg, pBuffer, bytes, f, discarded);
    }

    void CPhpQueue::BatchMessage(Php::Parameters & params) {
        int64_t idMsg = params[0].numericValue();
        if (idMsg <= SPA::idReservedTwo || idMsg > 0xffff) {
            throw Php::Exception("Bad message request Id");
        }
        m_pBuff->EnsureBuffer();
        unsigned int bytes = 0;
        const unsigned char *pBuffer = nullptr;
        Php::Value v;
        const Php::Value &q = params[1];
        if (q.instanceOf(SPA_NS + PHP_BUFFER)) {
            v = q.call(PHP_POPBYTES);
            pBuffer = (const unsigned char*) v.rawValue();
            bytes = (unsigned int) v.length();
        } else if (!q.isNull()) {
            throw Php::Exception("An instance of CUQueue or null required for Batch or BatchMessage");
        }
        CAsyncQueue::BatchMessage((unsigned short) idMsg, pBuffer, bytes, *m_pBuff->GetBuffer());
    }

    void CPhpQueue::RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace & cs) {
        Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
        handler.extends(base);

        handler.property("idEnqueue", SPA::Queue::idEnqueue, Php::Const);
        handler.property("idDequeue", SPA::Queue::idDequeue, Php::Const);
        handler.property("idStartTrans", SPA::Queue::idStartTrans, Php::Const);
        handler.property("idEndTrans", SPA::Queue::idEndTrans, Php::Const);
        handler.property("idFlush", SPA::Queue::idFlush, Php::Const);
        handler.property("idClose", SPA::Queue::idClose, Php::Const);
        handler.property("idGetKeys", SPA::Queue::idGetKeys, Php::Const);
        handler.property("idEnqueueBatch", SPA::Queue::idEnqueueBatch, Php::Const);
        handler.property("idBatchSizeNotified", SPA::Queue::idBatchSizeNotified, Php::Const);

        handler.property("OK", SPA::Queue::QUEUE_OK, Php::Const);
        handler.property("TRANS_ALREADY_STARTED", SPA::Queue::QUEUE_TRANS_ALREADY_STARTED, Php::Const);
        handler.property("TRANS_STARTING_FAILED", SPA::Queue::QUEUE_TRANS_STARTING_FAILED, Php::Const);
        handler.property("TRANS_NOT_STARTED_YET", SPA::Queue::QUEUE_TRANS_NOT_STARTED_YET, Php::Const);
        handler.property("TRANS_COMMITTING_FAILED", SPA::Queue::QUEUE_TRANS_COMMITTING_FAILED, Php::Const);
        handler.property("DEQUEUING", SPA::Queue::QUEUE_DEQUEUING, Php::Const);
        handler.property("OTHER_WORKING_WITH_SAME_QUEUE", SPA::Queue::QUEUE_OTHER_WORKING_WITH_SAME_QUEUE, Php::Const);
        handler.property("CLOSE_FAILED", SPA::Queue::QUEUE_CLOSE_FAILED, Php::Const);
        handler.property("ENQUEUING_FAILED", SPA::Queue::QUEUE_ENQUEUING_FAILED, Php::Const);

        handler.method<&CPhpQueue::Enqueue>("Enqueue",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
            Php::ByVal("idMessage", Php::Type::Numeric),
            Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null)
        });
        handler.method<&CPhpQueue::CloseQueue>("Close",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String)
        });
        handler.method<&CPhpQueue::CloseQueue>("CloseQueue",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String)
        });
        handler.method<&CPhpQueue::GetKeys>("GetKeys",{
            Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
        });
        handler.method<&CPhpQueue::StartQueueTrans>("StartTrans",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String)
        });
        handler.method<&CPhpQueue::EndQueueTrans>("EndTrans",{
            Php::ByVal("rollback", Php::Type::Bool)
        });
        handler.method<&CPhpQueue::FlushQueue>("Flush",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String)
        });
        handler.method<&CPhpQueue::Dequeue>("Dequeue",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String),
            Php::ByVal("rr", Php::Type::Callable),
            Php::ByVal("canceled", Php::Type::Callable, false),
            Php::ByVal("deqTimeout", Php::Type::Numeric, false)
        });
        handler.method<&CPhpQueue::BatchMessage>("Batch",{
            Php::ByVal("idMsg", Php::Type::Numeric),
            Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null)
        });
        handler.method<&CPhpQueue::BatchMessage>("BatchMessage",{
            Php::ByVal("idMsg", Php::Type::Numeric),
            Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null)
        });
        handler.method<&CPhpQueue::EnqueueBatch>("EnqueueBatch",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String)
        });
        handler.method<&CPhpQueue::EnqueueBatch>("BatchEnqueue",{
            Php::ByVal(PHP_QUEUE_KEY, Php::Type::String)
        });
        cs.add(handler);
    }

    void CPhpQueue::PopTopCallbacks(PACallback & cb) {
        unsigned short reqId;
        auto &callback = *cb.CallBack;
        switch (cb.CallbackType) {
            case ctQueueTrans:
            {
                int errCode;
                *cb.Res >> reqId >> errCode;
                callback(errCode, reqId);
            }
                break;
            case ctEnqueueRes:
            {
                SPA::INT64 index;
                *cb.Res >> reqId >> index;
                callback(index, reqId);
            }
                break;
            case ctQueueFlush:
                *cb.Res >> reqId;
                callback(ToFlushValue(cb.Res), reqId);
            case ctQueueGetKeys:
            {
                std::vector<std::string> v;
                *cb.Res >> reqId;
                while (cb.Res->GetSize()) {
                    std::string s;
                    *cb.Res >> s;
                    v.push_back(std::move(s));
                }
                callback(v, reqId);
            }
                break;
            case ctDequeue:
                *cb.Res >> reqId;
                callback(ToDeqValue(cb.Res), reqId);
                break;
            case ctDequeueResult:
            {
                *cb.Res >> reqId;
                Php::Object buff((SPA_NS + PHP_BUFFER).c_str(), new CPhpBuffer(cb.Res));
                callback(buff, reqId);
            }
                break;
            default:
                assert(false); //shouldn't come here
                break;
        }
    }
}
