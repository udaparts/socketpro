
#include "stdafx.h"
#include "aqueue.h"
#include "njasyncqueue.h"
#include "njqueue.h"

namespace NJA {

    CAQueue::CAQueue(SPA::ClientSide::CClientSocket *cs)
    : CAsyncQueue(cs) {
        ::memset(&m_qType, 0, sizeof (m_qType));
        m_qType.data = this;
        int fail = uv_async_init(uv_default_loop(), &m_qType, queue_cb);
        assert(!fail);
    }

    CAQueue::~CAQueue() {
        CAutoLock al(m_csJQ);
        uv_close((uv_handle_t*) & m_qType, nullptr);
    }

    SPA::ClientSide::CAsyncServiceHandler::DDiscarded CAQueue::Get(Isolate* isolate, Local<Value> abort, bool &bad) {
        bad = false;
        DDiscarded dd;
        if (abort->IsFunction()) {
            std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(abort)));
            dd = [func](CAsyncServiceHandler *aq, bool canceled) {
                QueueCb qcb(tagQueueEvent::qeDiscarded);
                qcb.Func = func;
                PAQueue ash = (PAQueue) aq;
                *qcb.Buffer << ash << canceled;
                CAutoLock al(ash->m_csJQ);
                ash->m_deqQCb.push_back(std::move(qcb));
                int fail = uv_async_send(&ash->m_qType);
                assert(!fail);
            };
        } else if (!IsNullOrUndefined(abort)) {
            ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
            bad = true;
        }
        return dd;
    }

    SPA::ClientSide::CAsyncServiceHandler::DServerException CAQueue::GetSE(Isolate* isolate, Local<Value> se, bool& bad) {
        bad = false;
        SPA::ClientSide::CAsyncServiceHandler::DServerException dSe;
        if (se->IsFunction()) {
            std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(se)));
            dSe = [func](CAsyncServiceHandler* aq, unsigned short requestId, const wchar_t* errMessage, const char* errWhere, unsigned int errCode) {
                QueueCb qcb(tagQueueEvent::qeException);
                qcb.Func = func;
                PAQueue ash = (PAQueue) aq;
                *qcb.Buffer << ash << requestId << errMessage << errWhere << errCode;
                CAutoLock al(ash->m_csJQ);
                ash->m_deqQCb.push_back(std::move(qcb));
                int fail = uv_async_send(&ash->m_qType);
                assert(!fail);
            };
        } else if (!IsNullOrUndefined(se)) {
            ThrowException(isolate, "A callback expected for tracking exception from server");
            bad = true;
        }
        return dSe;
    }

    void CAQueue::queue_cb(uv_async_t* handle) {
        CAQueue* obj = (CAQueue*) handle->data; //sender
        assert(obj);
        if (!obj) return;
        Isolate* isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate); //required for Node 4.x
        auto ctx = isolate->GetCurrentContext();
        {
            obj->m_csJQ.lock();
            while (obj->m_deqQCb.size()) {
                QueueCb cb(std::move(obj->m_deqQCb.front()));
                obj->m_deqQCb.pop_front();
                obj->m_csJQ.unlock();
                PAQueue processor;
                *cb.Buffer >> processor;
                assert(processor);
                Local<Function> func = Local<Function>::New(isolate, *cb.Func);
                switch (cb.EventType) {
                    case tagQueueEvent::qeException:
                        if (!func.IsEmpty()) {
                            unsigned short reqId;
                            SPA::CDBString errMsg;
                            std::string errWhere;
                            int errCode;
                            *cb.Buffer >> reqId >> errMsg >> errWhere >> errCode;
                            assert(!cb.Buffer->GetSize());
                            Local<String> jsMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                            Local<String> jsWhere = ToStr(isolate, errWhere.c_str());
                            Local<Value> jsCode = Number::New(isolate, errCode);
                            Local<Value> argv[] = {jsCode, jsMsg, jsWhere, Number::New(isolate, reqId)};
                            func->Call(isolate->GetCurrentContext(), Null(isolate), 4, argv);
                        }
                        break;
                    case tagQueueEvent::qeDiscarded:
                    {
                        bool canceled;
                        *cb.Buffer >> canceled;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> argv[] = {Boolean::New(isolate, canceled)};
                        func->Call(ctx, Null(isolate), 1, argv);
                    }
                        break;
                    case tagQueueEvent::qeGetKeys:
                    {
                        unsigned int size;
                        *cb.Buffer >> size;
                        unsigned int index = 0;
                        Local<Array> jsKeys = Array::New(isolate, (int) size);
                        while (cb.Buffer->GetSize()) {
                            std::string s;
                            *cb.Buffer >> s;
                            auto str = ToStr(isolate, s.c_str());
                            jsKeys->Set(ctx, index, str);
                            ++index;
                        }
                        assert(index == size);
                        Local<Value> argv[] = {jsKeys};
                        func->Call(ctx, Null(isolate), 1, argv);
                    }
                        break;
                    case tagQueueEvent::qeEnqueueBatch:
                    case tagQueueEvent::qeEnqueue:
                    {
                        SPA::UINT64 indexMessage;
                        *cb.Buffer >> indexMessage;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> im = Number::New(isolate, (double) indexMessage);
                        Local<Value> argv[] = {im};
                        func->Call(ctx, Null(isolate), 1, argv);
                    }
                        break;
                    case tagQueueEvent::qeCloseQueue:
                    case tagQueueEvent::qeEndQueueTrans:
                    case tagQueueEvent::qeStartQueueTrans:
                    {
                        int errCode;
                        *cb.Buffer >> errCode;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> jsCode = Int32::New(isolate, errCode);
                        Local<Value> argv[] = {jsCode};
                        func->Call(ctx, Null(isolate), 1, argv);
                    }
                        break;
                    case tagQueueEvent::qeFlushQueue:
                    {
                        SPA::UINT64 messageCount, fileSize;
                        *cb.Buffer >> messageCount >> fileSize;
                        assert(!cb.Buffer->GetSize());
                        //Local<Object> njQ = NJAsyncQueue::New(isolate, processor, true);
                        Local<Value> mc = Number::New(isolate, (double) messageCount);
                        Local<Value> fs = Number::New(isolate, (double) fileSize);
                        Local<Value> argv[] = {mc, fs};
                        func->Call(ctx, Null(isolate), 2, argv);
                    }
                        break;
                    case tagQueueEvent::qeDequeue:
                    {
                        SPA::UINT64 messageCount, fileSize;
                        unsigned int messagesDequeuedInBatch, bytesDequeuedInBatch;
                        *cb.Buffer >> messageCount >> fileSize >> messagesDequeuedInBatch >> bytesDequeuedInBatch;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> mc = Number::New(isolate, (double) messageCount);
                        Local<Value> fs = Number::New(isolate, (double) fileSize);
                        Local<Value> mdib = Uint32::New(isolate, messagesDequeuedInBatch);
                        Local<Value> bdib = Uint32::New(isolate, bytesDequeuedInBatch);
                        Local<Value> argv[] = {mc, fs, mdib, bdib};
                        func->Call(ctx, Null(isolate), 4, argv);
                    }
                        break;
                    case tagQueueEvent::qeResultReturned:
                    {
                        unsigned short reqId;
                        *cb.Buffer >> reqId;
                        Local<Object> q = NJQueue::New(isolate, cb.Buffer);
                        Local<Value> jsReqid = Uint32::New(isolate, reqId);
                        //Local<Object> njQ = NJAsyncQueue::New(isolate, processor, true);
                        Local<Value> argv[] = {jsReqid, q};
                        func->Call(ctx, Null(isolate), 2, argv);
                        auto obj = node::ObjectWrap::Unwrap<NJQueue>(q);
                        obj->Release();
                    }
                        break;
                    default:
                        assert(false); //shouldn't come here
                        break;
                }
                CScopeUQueue::Unlock(cb.Buffer);
                obj->m_csJQ.lock();
            }
            obj->m_csJQ.unlock();
        }
        isolate->RunMicrotasks();
    }

    void CAQueue::SetRR(Isolate* isolate, Local<Value> rr) {
        if (rr->IsFunction()) {
            CAutoLock al(m_csJQ);
            m_rr.reset(new CNJFunc(isolate, Local<Function>::Cast(rr)));
        } else if (IsNullOrUndefined(rr)) {
            CAutoLock al(m_csJQ);
            m_rr.reset();
        }
    }

    void CAQueue::OnResultReturned(unsigned short reqId, CUQueue &mc) {
        if (reqId > Queue::idEnqueueBatch && reqId != Queue::idBatchSizeNotified) {
            CAutoLock al(m_csJQ);
            if (m_rr) {
                QueueCb qcb(tagQueueEvent::qeResultReturned);
                PAQueue ash = this;
                *qcb.Buffer << ash << reqId;
                qcb.Buffer->Push(mc.GetBuffer(), mc.GetSize());
                qcb.Func = m_rr;
                mc.SetSize(0);
                ash->m_deqQCb.push_back(std::move(qcb));
                int fail = uv_async_send(&ash->m_qType);
                assert(!fail);
            }
        } else {
            CAsyncQueue::OnResultReturned(reqId, mc);
        }
    }

    SPA::UINT64 CAQueue::GetKeys(Isolate* isolate, int args, Local<Value> *argv) {
        SPA::UINT64 index = GetCallIndex();
        DGetKeys gk;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                gk = [func](CAsyncQueue *aq, std::vector<std::string>& v) {
                    QueueCb qcb(tagQueueEvent::qeGetKeys);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << (unsigned int) v.size();
                    for (auto it = v.begin(), end = v.end(); it != end; ++it) {
                        *qcb.Buffer << *it;
                    }
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for GetKeys end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::GetKeys(gk, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::StartQueueTrans(Isolate* isolate, int args, Local<Value> *argv, const char *key) {
        SPA::UINT64 index = GetCallIndex();
        DQueueTrans qt;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                qt = [func](CAsyncQueue *aq, int errCode) {
                    QueueCb qcb(tagQueueEvent::qeStartQueueTrans);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << errCode;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for StartTrans end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::StartQueueTrans(key, qt, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::EndQueueTrans(Isolate* isolate, int args, Local<Value> *argv, bool rollback) {
        SPA::UINT64 index = GetCallIndex();
        DQueueTrans qt;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                qt = [func](CAsyncQueue *aq, int errCode) {
                    QueueCb qcb(tagQueueEvent::qeEndQueueTrans);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << errCode;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for EndTrans end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::EndQueueTrans(rollback, qt, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::CloseQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, bool permanent) {
        SPA::UINT64 index = GetCallIndex();
        DClose c;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                c = [func](CAsyncQueue *aq, int errCode) {
                    QueueCb qcb(tagQueueEvent::qeCloseQueue);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << errCode;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for Close end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::CloseQueue(key, c, permanent, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::FlushQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, tagOptimistic option) {
        SPA::UINT64 index = GetCallIndex();
        DFlush f;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                f = [func](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize) {
                    QueueCb qcb(tagQueueEvent::qeFlushQueue);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << messageCount << fileSize;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for Flush end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::FlushQueue(key, f, option, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::Dequeue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned int timeout) {
        SPA::UINT64 index = GetCallIndex();
        DDequeue d;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                d = [func](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messagesDequeuedInBatch, unsigned int bytesDequeuedInBatch) {
                    QueueCb qcb(tagQueueEvent::qeDequeue);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << messageCount << fileSize << messagesDequeuedInBatch << bytesDequeuedInBatch;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for Dequeue end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::Dequeue(key, d, timeout, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::Enqueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned short idMessage, const unsigned char *pBuffer, unsigned int size) {
        if (idMessage <= Queue::idEnqueueBatch || idMessage == Queue::idBatchSizeNotified) {
            ThrowException(isolate, "Bad message request id");
            return 0;
        }
        SPA::UINT64 index = GetCallIndex();
        DEnqueue e;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                e = [func](CAsyncQueue *aq, SPA::UINT64 indexMessage) {
                    QueueCb qcb(tagQueueEvent::qeEnqueue);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << indexMessage;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for Enqueue end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::Enqueue(key, idMessage, pBuffer, size, e, dd, se) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::EnqueueBatch(Isolate* isolate, int args, Local<Value> *argv, const char *key, const unsigned char *pBuffer, unsigned int size) {
        SPA::UINT64 index = GetCallIndex();
        DEnqueue e;
        DDiscarded dd;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                e = [func](CAsyncQueue *aq, SPA::UINT64 indexMessage) {
                    QueueCb qcb(tagQueueEvent::qeEnqueueBatch);
                    qcb.Func = func;
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << indexMessage;
                    CAutoLock al(ash->m_csJQ);
                    ash->m_deqQCb.push_back(std::move(qcb));
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for EnqueueBatch end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        if (args > 2) {
            bool bad;
            se = GetSE(isolate, argv[2], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::EnqueueBatch(key, pBuffer, size, e, dd, se) ? index : INVALID_NUMBER;
    }
}
