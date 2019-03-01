#include "stdafx.h"
#include "basehandler.h"
#include "phpbuffer.h"
#include "phpsocket.h"
#include "phppush.h"
#include "phpclientqueue.h"

namespace PA
{

    CPhpBaseHandler::CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler *h, unsigned int poolId) : m_locked(locked), m_h(h), m_PoolId(poolId) {
        assert(m_h);
    }

    CPhpBaseHandler::~CPhpBaseHandler() {
        Unlock();
    }

    void CPhpBaseHandler::__destruct() {
    }

    Php::Value CPhpBaseHandler::Unlock() {
        if (m_locked) {
            SPA::ClientSide::ClientCoreLoader.UnlockASocket(m_PoolId, m_h->GetAttachedClientSocket()->GetHandle());
            m_locked = false;
        }
        return true;
    }

    Php::Value CPhpBaseHandler::CleanCallbacks(Php::Parameters & params) {
        return (int64_t) m_h->CleanCallbacks();
    }

    SPA::ClientSide::CAsyncServiceHandler::DDiscarded CPhpBaseHandler::SetAbortCallback(const Php::Value& phpCanceled, unsigned short reqId, bool sync) {
        if (phpCanceled.isNull()) {
        } else if (!phpCanceled.isCallable()) {
            throw Php::Exception("A callback required for request aborting event");
        }
        m_rrs = rrsOk;
        CPVPointer callback;
        if (phpCanceled.isCallable()) {
            callback.reset(new Php::Value(phpCanceled));
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [reqId, sync, callback, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
            if (callback) {
                SPA::CScopeUQueue sb;
                PACallback cb;
                cb.CallbackType = ctDiscarded;
                sb << reqId << canceled;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
            if (sync) {
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_rrs = canceled ? rrsCanceled : rrsClosed;
                this->m_cvPhp.notify_all();
            }
        };
        return discarded;
    }

    void CPhpBaseHandler::ReqSyncEnd(bool ok, std::unique_lock<std::mutex> &lk, unsigned int timeout) {
        //Unlock();
        PopCallbacks();
        if (!ok) {
            throw Php::Exception(PA::PHP_SOCKET_CLOSED);
        }
        auto status = m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
        PopCallbacks();
        if (status == std::cv_status::timeout) {
            m_rrs = rrsTimeout;
        }
        switch (m_rrs) {
            case rrsServerException:
                throw Php::Exception(PHP_SERVER_EXCEPTION);
            case rrsCanceled:
                throw Php::Exception(PHP_REQUEST_CANCELED);
            case rrsClosed:
                throw Php::Exception(PHP_SOCKET_CLOSED);
            case rrsTimeout:
                throw Php::Exception(PHP_REQUEST_TIMEOUT);
            default:
                break;
        }
    }

    Php::Value CPhpBaseHandler::SendRequest(Php::Parameters & params) {
        int64_t id = params[0].numericValue();
        if (id <= SPA::idReservedTwo || id > 0xffff) {
            throw Php::Exception("Bad request id");
        }
        unsigned short reqId = (unsigned short) id;
        unsigned int timeout = (~0);
        bool sync = false;
        Php::Value phpRh = params[2];
        if (phpRh.isNumeric()) {
            sync = true;
            timeout = (unsigned int) phpRh.numericValue();
        } else if (phpRh.isBool()) {
            sync = phpRh.boolValue();
        } else if (phpRh.isNull()) {
        } else if (!phpRh.isCallable()) {
            throw Php::Exception("A callback required for returning result");
        }
        std::shared_ptr<CPhpBuffer> buffer;
        if (sync) {
            buffer.reset(new CPhpBuffer);
        }
        CPVPointer callback;
        if (phpRh.isCallable()) {
            callback.reset(new Php::Value(phpRh));
        }
        SPA::ClientSide::ResultHandler rh = [buffer, callback, this](SPA::ClientSide::CAsyncResult & ar) {
            SPA::ClientSide::PAsyncServiceHandler ash = ar.AsyncServiceHandler;
            if (buffer) {
                buffer->Swap(&ar.UQueue);
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << ar.RequestId;
                sb->Push(ar.UQueue.GetBuffer(), ar.UQueue.GetSize());
                ar.UQueue.SetSize(0);
                PACallback cb;
                cb.CallbackType = ctSendRequest;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            } else {
                ar.UQueue.SetSize(0);
            }
        };
        size_t args = params.size();
        Php::Value phpCanceled;
        if (args > 3) {
            phpCanceled = params[3];
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, reqId, sync);
        Php::Value phpEx;
        if (args > 4) {
            phpEx = params[4];
            if (phpEx.isNull()) {
            } else if (!phpEx.isCallable()) {
                throw Php::Exception("A callback required for server exception");
            }
        }
        CPVPointer callbackEx;
        if (phpEx.isCallable()) {
            callbackEx.reset(new Php::Value(phpEx));
        }
        SPA::ClientSide::CAsyncServiceHandler::DServerException se = [callbackEx, sync, this](SPA::ClientSide::CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char *errWhere, unsigned int errCode) {
            if (callbackEx) {
                SPA::CScopeUQueue sb;
                sb << reqId << errMsg << errWhere << errCode;
                PACallback cb;
                cb.CallbackType = ctServerException;
                cb.Res = sb.Detach();
                cb.CallBack = callbackEx;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
            if (sync) {
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_rrs = rrsServerException;
                this->m_cvPhp.notify_all();
            }
        };
        unsigned int bytes = 0;
        const unsigned char *pBuffer = nullptr;
        Php::Value v;
        Php::Value &q = params[1];
        if (q.instanceOf(SPA_NS + PHP_BUFFER)) {
            v = q.call(PHP_POPBYTES);
            pBuffer = (const unsigned char*) v.rawValue();
            bytes = (unsigned int) v.length();
        } else if (!q.isNull()) {
            throw Php::Exception("An instance of CUQueue or null required for request sending data");
        }
        if (sync) {
            {
                std::unique_lock<std::mutex> lk(m_mPhp);
                ReqSyncEnd(m_h->SendRequest(reqId, pBuffer, bytes, rh, discarded, se), lk, timeout);
            }
            CPhpBuffer *p = new CPhpBuffer;
            p->Swap(buffer.get());
            return Php::Object((SPA_NS + PHP_BUFFER).c_str(), p);
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            PopCallbacks();
        }
        return m_h->SendRequest(reqId, pBuffer, bytes, rh, discarded, se);
    }

    void CPhpBaseHandler::PopCallbacks() {
        assert(!m_mPhp.try_lock());
        for (auto &cb : m_vCallback) {
            auto &callback = *cb.CallBack;
            unsigned short reqId;
            switch (cb.CallbackType) {
                case ctSendRequest:
                {
                    CPhpBuffer *buff = new CPhpBuffer;
                    *cb.Res >> reqId;
                    buff->EnsureBuffer(cb.Res);
                    Php::Object q((SPA_NS + PHP_BUFFER).c_str(), buff);
                    callback(q, reqId);
                    cb.Res = nullptr;
                }
                    break;
                case ctDiscarded:
                {
                    bool canceled;
                    *cb.Res >> reqId >> canceled;
                    SPA::CScopeUQueue::Unlock(cb.Res);
                    callback(canceled, reqId);
                }
                    break;
                case ctServerException:
                {
                    std::wstring errMsg;
                    std::string errWhere;
                    unsigned int errCode;
                    *cb.Res >> reqId >> errMsg >> errWhere >> errCode;
                    SPA::CScopeUQueue::Unlock(cb.Res);
                    Php::Value v;
                    v.set(PHP_ERR_CODE, (int64_t) errCode);
                    v.set(PHP_ERR_MSG, SPA::Utilities::ToUTF8(errMsg));
                    callback(v, reqId);
                }
                default:
                    PopTopCallbacks(cb);
                    break;
            }
            if (cb.Res) {
                SPA::CScopeUQueue::Unlock(cb.Res);
            }
        }
        m_vCallback.clear();
    }

    void CPhpBaseHandler::__construct(Php::Parameters & params) {
    }

    Php::Value CPhpBaseHandler::WaitAll(Php::Parameters & params) {
        unsigned int timeout = (~0);
        if (params.size() && params[0].isNumeric()) {
            timeout = (unsigned int) params[0].numericValue();
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            PopCallbacks();
        }
        bool ok = m_h->WaitAll(timeout);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            PopCallbacks();
        }
        return ok;
    }

    Php::Value CPhpBaseHandler::StartBatching() {
        return m_h->StartBatching();
    }

    Php::Value CPhpBaseHandler::CommitBatching() {
        return m_h->CommitBatching();
    }

    Php::Value CPhpBaseHandler::AbortBatching() {
        return m_h->AbortBatching();
    }

    void CPhpBaseHandler::RegisterInto(Php::Class<CPhpBaseHandler> &h, Php::Namespace & cs) {
        h.method<&CPhpBaseHandler::__construct>(PHP_CONSTRUCT, Php::Private);
        h.method<&CPhpBaseHandler::SendRequest>(PHP_SENDREQUEST,{
            Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
            Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null),
            Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
        });
        h.method<&CPhpBaseHandler::WaitAll>(PHP_WAITALL);
        h.method<&CPhpBaseHandler::StartBatching>(PHP_STARTBATCHING);
        h.method<&CPhpBaseHandler::AbortBatching>(PHP_ABORTBATCHING);
        h.method<&CPhpBaseHandler::CommitBatching>(PHP_COMMITBATCHING);
        h.method<&CPhpBaseHandler::Unlock>(PHP_UNLOCK);
        h.method<&CPhpBaseHandler::CleanCallbacks>(PHP_CLEAN_CALLBACKS);
        cs.add(h);
    }

    unsigned int CPhpBaseHandler::GetPoolId() const {
        return m_PoolId;
    }

    int CPhpBaseHandler::__compare(const CPhpBaseHandler & pbh) const {
        if (!m_h || !pbh.m_h) {
            return 1;
        }
        return (m_h == pbh.m_h) ? 0 : 1;
    }

    Php::Value CPhpBaseHandler::__get(const Php::Value & name) {
        if (name == "Socket" || name == "ClientSocket" || name == "AttachedClientSocket") {
            return Php::Object((SPA_CS_NS + PHP_SOCKET).c_str(), new CPhpSocket(m_h->GetAttachedClientSocket()));
        } else if (name == "Push" || name == "Chat" || name == "Publisher") {
            return Php::Object((SPA_CS_NS + PHP_PUSH).c_str(), new CPhpPush(m_h->GetAttachedClientSocket()->GetPush()));
        } else if (name == "Queue" || name == "ClientQueue") {
            return Php::Object((SPA_CS_NS + PHP_CLIENTQUEUE).c_str(), new CPhpClientQueue(m_h->GetAttachedClientSocket()->GetClientQueue()));
        } else if (name == "Locked") {
            return m_locked;
        } else if (name == "SvsId" || name == "SvsID") {
            return (int64_t) m_h->GetSvsID();
        } else if (name == "Batching") {
            return m_h->IsBatching();
        } else if (name == "RouteeRequest") {
            return m_h->IsRouteeRequest();
        } else if (name == "DequeuedResult") {
            return m_h->IsDequeuedResult();
        } else if (name == "DequeuedMessageAborted") {
            return m_h->IsDequeuedMessageAborted();
        }
        return Php::Base::__get(name);
    }

} //namespace PA
