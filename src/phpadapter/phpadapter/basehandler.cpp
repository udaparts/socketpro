#include "stdafx.h"
#include "basehandler.h"
#include "phpbuffer.h"
#include "phpsocket.h"
#include "phppush.h"
#include "phpclientqueue.h"

namespace PA
{

    CPhpBaseHandler::CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler * h) : m_locked(locked), m_h(h) {
        assert(m_h);
    }

    CPhpBaseHandler::~CPhpBaseHandler() {
        Unlock();
    }

    void CPhpBaseHandler::__destruct() {
    }

    Php::Value CPhpBaseHandler::Unlock() {
        if (m_locked) {
            SPA::ClientSide::ClientCoreLoader.UnlockASocket(m_h->GetSocket()->GetPoolId(), m_h->GetSocket()->GetHandle());
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
        m_rrs = tagRequestReturnStatus::rrsOk;
        CPVPointer callback;
        if (phpCanceled.isCallable()) {
            callback.reset(new Php::Value(phpCanceled));
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [reqId, sync, callback, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
            this->m_rrs = canceled ? tagRequestReturnStatus::rrsCanceled : tagRequestReturnStatus::rrsClosed;
            if (callback) {
                SPA::CScopeUQueue sb;
                PACallback cb;
                cb.CallbackType = enumCallbackType::ctDiscarded;
                sb << reqId << canceled;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
            if (sync) {
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            }
        };
        return discarded;
    }

    void CPhpBaseHandler::ReqSyncEnd(bool ok, std::unique_lock<std::mutex> &lk, unsigned int timeout) {
        Unlock();
        PopCallbacks();
        if (!ok) {
            throw Php::Exception(PA::PHP_SOCKET_CLOSED + m_h->GetSocket()->GetErrorMsg());
        }
        auto status = m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
        PopCallbacks();
        if (status == std::cv_status::timeout) {
            m_rrs = tagRequestReturnStatus::rrsTimeout;
        }
        switch (m_rrs) {
            case tagRequestReturnStatus::rrsServerException:
                throw Php::Exception(PHP_SERVER_EXCEPTION);
            case tagRequestReturnStatus::rrsCanceled:
                throw Php::Exception(PHP_REQUEST_CANCELED);
            case tagRequestReturnStatus::rrsClosed:
                throw Php::Exception(PA::PHP_SOCKET_CLOSED + m_h->GetSocket()->GetErrorMsg());
            case tagRequestReturnStatus::rrsTimeout:
                throw Php::Exception(PHP_REQUEST_TIMEOUT);
            default:
                break;
        }
    }

    tagRequestReturnStatus CPhpBaseHandler::GetRRS() const {
        return m_rrs;
    }

    Php::Value CPhpBaseHandler::SendRequest(Php::Parameters & params) {
        int64_t id = params[0].numericValue();
        if (id <= (unsigned short) SPA::tagBaseRequestID::idReservedTwo || id > 0xffff) {
            throw Php::Exception("Request id must be larger than 0x2001 but less than 0x10000");
        }
        unsigned short reqId = (unsigned short) id;
        unsigned int timeout = (~0);
        bool sync = false;
        size_t args = params.size();
        SPA::ClientSide::DResultHandler rh;
        std::shared_ptr<CPhpBuffer> buffer;
        if (args > 2) {
            const Php::Value &phpRh = params[2];
            if (phpRh.isNumeric()) {
                sync = true;
                timeout = (unsigned int) phpRh.numericValue();
            } else if (phpRh.isBool()) {
                sync = phpRh.boolValue();
            } else if (phpRh.isNull()) {
            } else if (!phpRh.isCallable()) {
                throw Php::Exception("A callback required for returning result");
            }
            if (sync) {
                buffer.reset(new CPhpBuffer);
            }
            CPVPointer callback;
            if (phpRh.isCallable()) {
                callback.reset(new Php::Value(phpRh));
            }
            rh = [buffer, callback, this](SPA::ClientSide::CAsyncResult & ar) {
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
                    cb.CallbackType = enumCallbackType::ctSendRequest;
                    cb.Res = sb.Detach();
                    cb.CallBack = callback;
                    std::unique_lock<std::mutex> lk(this->m_mPhp);
                    this->m_vCallback.push_back(cb);
                } else {
                    ar.UQueue.SetSize(0);
                }
            };
        }
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
            this->m_rrs = tagRequestReturnStatus::rrsServerException;
            if (callbackEx) {
                SPA::CScopeUQueue sb;
                sb << reqId << errMsg << errWhere << errCode;
                PACallback cb;
                cb.CallbackType = enumCallbackType::ctServerException;
                cb.Res = sb.Detach();
                cb.CallBack = callbackEx;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
            if (sync) {
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            }
        };
        unsigned int bytes = 0;
        const unsigned char *pBuffer = nullptr;
        Php::Value v;
        const Php::Value &q = params[1];
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
                case enumCallbackType::ctSendRequest:
                {
                    CPhpBuffer *buff = new CPhpBuffer;
                    *cb.Res >> reqId;
                    buff->EnsureBuffer(cb.Res);
                    Php::Object q((SPA_NS + PHP_BUFFER).c_str(), buff);
                    callback(q, reqId);
                    cb.Res = nullptr;
                }
                    break;
                case enumCallbackType::ctDiscarded:
                {
                    bool canceled;
                    *cb.Res >> reqId >> canceled;
                    SPA::CScopeUQueue::Unlock(cb.Res);
                    callback(canceled, reqId);
                }
                    break;
                case enumCallbackType::ctServerException:
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

    Php::Value CPhpBaseHandler::CommitBatching(Php::Parameters & params) {
        bool server_batching = false;
        if (params.size()) {
            server_batching = params[0].boolValue();
        }
        return m_h->CommitBatching(server_batching);
    }

    Php::Value CPhpBaseHandler::Interrupt(Php::Parameters & params) {
        SPA::UINT64 options = 0;
        if (params.size() && params[0].isNumeric()) {
            options = (SPA::UINT64)params[0].numericValue();
        }
        return m_h->Interrupt(options);
    }

    Php::Value CPhpBaseHandler::AbortBatching() {
        return m_h->AbortBatching();
    }

    void CPhpBaseHandler::RegisterInto(Php::Class<CPhpBaseHandler> &h, Php::Namespace & cs) {
        h.method<&CPhpBaseHandler::__construct>(PHP_CONSTRUCT, Php::Private);
        h.method<&CPhpBaseHandler::SendRequest>(PHP_SENDREQUEST,{
            Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
            Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null)
        });
        h.method<&CPhpBaseHandler::WaitAll>(PHP_WAITALL);
        h.method<&CPhpBaseHandler::StartBatching>(PHP_STARTBATCHING);
        h.method<&CPhpBaseHandler::AbortBatching>(PHP_ABORTBATCHING);
        h.method<&CPhpBaseHandler::CommitBatching>(PHP_COMMITBATCHING,{Php::ByVal("serverBatching", Php::Type::Bool)});
        h.method<&CPhpBaseHandler::Interrupt>("Interrupt",{Php::ByVal("options", Php::Type::Numeric)});
        h.method<&CPhpBaseHandler::Unlock>(PHP_UNLOCK);
        h.method<&CPhpBaseHandler::CleanCallbacks>(PHP_CLEAN_CALLBACKS);
        cs.add(h);
    }

    unsigned int CPhpBaseHandler::GetPoolId() const {
        return m_h->GetSocket()->GetPoolId();
    }

    int CPhpBaseHandler::__compare(const CPhpBaseHandler & pbh) const {
        if (!m_h || !pbh.m_h) {
            return 1;
        }
        return (m_h == pbh.m_h) ? 0 : 1;
    }

    Php::Value CPhpBaseHandler::__get(const Php::Value & name) {
        if (name == "Socket" || name == "ClientSocket" || name == "AttachedClientSocket") {
            return Php::Object((SPA_CS_NS + PHP_SOCKET).c_str(), new CPhpSocket(m_h->GetSocket()));
        } else if (name == "Push" || name == "Chat" || name == "Publisher") {
            return Php::Object((SPA_CS_NS + PHP_PUSH).c_str(), new CPhpPush(m_h->GetSocket()->GetPush()));
        } else if (name == "Queue" || name == "ClientQueue") {
            return Php::Object((SPA_CS_NS + PHP_CLIENTQUEUE).c_str(), new CPhpClientQueue(m_h->GetSocket()->GetClientQueue()));
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
