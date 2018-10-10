#include "stdafx.h"
#include "njobjects.h"
#include "../../../include/sqlite/usqlite.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/odbc/uodbc.h"
#include "../../../include/rdbcache.h"
#include "../../../include/masterpool.h"
#include "njcache.h"

namespace NJA {
    using v8::Context;

    Persistent<Function> NJSocketPool::constructor;

    NJSocketPool::NJSocketPool(const wchar_t* defaultDb, unsigned int id, bool slave)
    : SvsId(id), m_errSSL(0) {
        std::wstring dfltDb(defaultDb ? defaultDb : L"");
        switch (id) {
            case SPA::Mysql::sidMysql:
            case SPA::Odbc::sidOdbc:
            case SPA::Sqlite::sidSqlite:
                if (dfltDb.size()) {
                    Db = new CSQLMasterPool<false, CNjDb>(dfltDb.c_str(), SPA::ClientSide::DEFAULT_RECV_TIMEOUT, id);
                    m_defaultDb = dfltDb;
                } else if (slave) {
                    Db = new CSQLMasterPool<false, CNjDb>::CSlavePool(dfltDb.c_str(), SPA::ClientSide::DEFAULT_RECV_TIMEOUT, id);
                } else {
                    Db = new CSocketPool<CNjDb>(true, SPA::ClientSide::DEFAULT_RECV_TIMEOUT, SPA::ClientSide::DEFAULT_CONN_TIMEOUT, id);
                }
                Db->DoSslServerAuthentication = [this](CSocketPool<CNjDb> *pool, SPA::ClientSide::CClientSocket * cs)->bool {
                    return this->DoAuthentication(cs->GetUCert());
                };
                Db->SocketPoolEvent = [this](CSocketPool<CNjDb> *pool, tagSocketPoolEvent spe, CNjDb * handler) {
                    this->SendPoolEvent(spe, handler);
                };
                break;
            case SPA::Queue::sidQueue:
                assert(!slave);
                Queue = new CSocketPool<CAQueue>(true, SPA::ClientSide::DEFAULT_RECV_TIMEOUT, SPA::ClientSide::DEFAULT_CONN_TIMEOUT);
                Queue->DoSslServerAuthentication = [this](CSocketPool<CAQueue> *pool, SPA::ClientSide::CClientSocket * cs)->bool {
                    return this->DoAuthentication(cs->GetUCert());
                };
                Queue->SocketPoolEvent = [this](CSocketPool<CAQueue> *pool, tagSocketPoolEvent spe, CAQueue * handler) {
                    this->SendPoolEvent(spe, handler);
                };
                break;
            case SPA::SFile::sidFile:
                assert(!slave);
                File = new CSocketPool<CSFile>(true, SPA::ClientSide::DEFAULT_RECV_TIMEOUT, SPA::ClientSide::DEFAULT_CONN_TIMEOUT);
                File->DoSslServerAuthentication = [this](CSocketPool<CSFile> *pool, SPA::ClientSide::CClientSocket * cs)->bool {
                    return this->DoAuthentication(cs->GetUCert());
                };
                File->SocketPoolEvent = [this](CSocketPool<CSFile> *pool, tagSocketPoolEvent spe, CSFile * handler) {
                    this->SendPoolEvent(spe, handler);
                };
                break;
            default:
                if (dfltDb.size()) {
                    Handler = new CMasterPool<false, CAsyncHandler>(dfltDb.c_str(), SPA::ClientSide::DEFAULT_RECV_TIMEOUT, id);
                    m_defaultDb = dfltDb;
                } else if (slave) {
                    Handler = new CMasterPool<false, CAsyncHandler>::CSlavePool(dfltDb.c_str(), SPA::ClientSide::DEFAULT_RECV_TIMEOUT, id);
                } else {
                    Handler = new CSocketPool<CAsyncHandler>(true, SPA::ClientSide::DEFAULT_RECV_TIMEOUT, SPA::ClientSide::DEFAULT_CONN_TIMEOUT, id);
                }
                Handler->DoSslServerAuthentication = [this](CSocketPool<CAsyncHandler> *pool, SPA::ClientSide::CClientSocket * cs)->bool {
                    return this->DoAuthentication(cs->GetUCert());
                };
                Handler->SocketPoolEvent = [this](CSocketPool<CAsyncHandler> *pool, tagSocketPoolEvent spe, CAsyncHandler * handler) {
                    this->SendPoolEvent(spe, handler);
                };
                break;
        }
        ::memset(&m_asyncType, 0, sizeof (m_asyncType));
        m_asyncType.data = this;
        ::memset(&m_csType, 0, sizeof (m_csType));
        m_csType.data = this;
        int fail = uv_async_init(uv_default_loop(), &m_asyncType, async_cb);
        assert(!fail);
        fail = uv_async_init(uv_default_loop(), &m_csType, async_cs_cb);
        assert(!fail);
    }

    NJSocketPool::~NJSocketPool() {
        Release();
    }

    void NJSocketPool::SendPoolEvent(tagSocketPoolEvent spe, SPA::ClientSide::PAsyncServiceHandler handler) {
        switch (spe) {
            case SPA::ClientSide::speUSocketCreated:
                handler->GetAttachedClientSocket()->m_asyncType = &m_csType;
                break;
            default:
                break;
        }
        SPA::CAutoLock al(m_cs);
        if (m_evPool.IsEmpty())
            return;
        PoolEvent pe;
        pe.Spe = spe;
        pe.Handler = handler;
        m_deqPoolEvent.push_back(pe);
        int fail = uv_async_send(&m_asyncType);
        assert(!fail);
    }

    bool NJSocketPool::DoAuthentication(IUcert *cert) {
        {
            SPA::CAutoLock al(m_cs);
            if (!cert->Validity) {
                m_errMsg = "Certificate not valid";
                m_errSSL = -1;
                return false;
            }
            m_errMsg = cert->Verify(&m_errSSL);
            bool ok = (m_errSSL == 0);
            if (ok) {
                return true;
            }
        }
        SPA::CAutoLock al(g_cs);
        if (cert->PKSize && g_KeyAllowed.GetSize() == cert->PKSize) {
            return (::memcmp(g_KeyAllowed.GetBuffer(), cert->PublicKey, cert->PKSize) == 0);
        }
        return false;
    }

    bool NJSocketPool::IsValid(Isolate* isolate) {
        if (!Handler) {
            ThrowException(isolate, "Socket Pool object already disposed");
            return false;
        }
        return true;
    }

    void NJSocketPool::Release() {
        SPA::CAutoLock al(m_cs);
        if (Handler) {
            switch (SvsId) {
                case SPA::Sqlite::sidSqlite:
                case SPA::Mysql::sidMysql:
                case SPA::Odbc::sidOdbc:
                    delete Db;
                    break;
                case SPA::Queue::sidQueue:
                    delete Queue;
                    break;
                case SPA::SFile::sidFile:
                    delete File;
                    break;
                default:
                    delete Handler;
                    break;
            }
            Handler = nullptr;
            uv_close((uv_handle_t*) & m_asyncType, nullptr);
            uv_close((uv_handle_t*) & m_csType, nullptr);
        }
    }

    void NJSocketPool::getCache(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            switch (obj->SvsId) {
                case SPA::Mysql::sidMysql:
                case SPA::Odbc::sidOdbc:
                case SPA::Sqlite::sidSqlite:
                    if (obj->m_defaultDb.size()) {
                        auto pool = (CSQLMasterPool<false, CNjDb>*) obj->Db;
                        args.GetReturnValue().Set(NJCache::New(isolate, &pool->Cache, true));
                        return;
                    }
                    break;
                case SPA::Queue::sidQueue:
                case SPA::SFile::sidFile:
                    break;
                default:
                    if (obj->m_defaultDb.size()) {
                        auto pool = (CMasterPool<false, CAsyncHandler>*) obj->Handler;
                        args.GetReturnValue().Set(NJCache::New(isolate, &pool->Cache, true));
                        return;
                    }
                    break;
            }
            ThrowException(isolate, "Real-time updateable cache not available");
        }
    }

    void NJSocketPool::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, "CSocketPool"));
        tpl->InstanceTemplate()->SetInternalFieldCount(15);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Dispose);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Seek", Seek);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SeekByQueue", SeekByQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Shutdown", ShutdownPool);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Start", StartSocketPool);
        NODE_SET_PROTOTYPE_METHOD(tpl, "NewSlave", newSlave);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getHandlers", getAsyncHandlers);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getConnectedSockets", getConnectedSockets);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getClosedSockets", getDisconnectedSockets);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getPoolId", getPoolId);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSvsId", getSvsId);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getError", getError);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getAutoMerge", getQueueAutoMerge);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setAutoMerge", setQueueAutoMerge);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getQueueName", getQueueName);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setQueueName", setQueueName);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getQueues", getQueues);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSockets", getSockets);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getTotalSockets", getSocketsPerThread);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getStarted", getStarted);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getCache", getCache);

        NODE_SET_PROTOTYPE_METHOD(tpl, "setPoolEvent", setPoolEvent);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setReturned", setResultReturned);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setAllProcessed", setAllProcessed);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setPush", setPush);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setBaseReqProcessed", setBaseRequestProcessed);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setServerException", setServerException);

        //NODE_SET_PROTOTYPE_METHOD(tpl, "getThreads", getThreadsCreated);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "getIdleSockets", getIdleSockets);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "getAvg", getAvg);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "getLockedSockets", getLockedSockets);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "Lock", Lock);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "Unlock", Unlock);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "CloseAll", DisconnectAll);

        constructor.Reset(isolate, tpl->GetFunction());
        exports->Set(ToStr(isolate, "CSocketPool"), tpl->GetFunction());
    }

    void NJSocketPool::newSlave(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            if (!obj->m_defaultDb.size()) {
                ThrowException(isolate, "Cannot create a slave pool from a non-master pool");
                return;
            }
            std::wstring defaultDb(obj->m_defaultDb);
            auto p = args[0];
            if (p->IsString()) {
                std::wstring s = ToStr(p);
                if (s.size()) {
                    defaultDb = s;
                }
            } else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "A default database name string expected");
                return;
            }
            Local<Value> argv[] = {Number::New(isolate, obj->SvsId), ToStr(isolate, defaultDb.c_str(), defaultDb.size()), Boolean::New(isolate, true)};
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 3, argv).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJSocketPool::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            unsigned int svsId = 0;
            if (args[0]->IsUint32()) {
                svsId = args[0]->Uint32Value();
            }
            if (svsId < SPA::sidChat || (svsId > SPA::sidODBC && svsId <= SPA::sidReserved)) {
                ThrowException(isolate, "A valid unsigned int required for service id");
                return;
            }
            if (svsId == sidHTTP) {
                ThrowException(isolate, "No support to HTTP/websocket at client side");
                return;
            }
            std::wstring db;
            if (args[1]->IsString()) {
                String::Value str(args[1]);
                unsigned int len = (unsigned int) str.length();
                if (len)
                    db.assign(*str, *str + len);
            }
            bool slave = false;
            if (args[2]->IsBoolean()) {
                slave = args[2]->BooleanValue();
            } else if (!args[2]->IsNullOrUndefined()) {
                ThrowException(isolate, "Must be a boolean value for slave pool");
                return;
            }
            if (db.size() || slave) {
                switch (svsId) {
                    case sidFile:
                        ThrowException(isolate, "File streaming doesn't support master-slave pool");
                        return;
                    case sidChat:
                        ThrowException(isolate, "Persistent queue doesn't support master-slave pool");
                        return;
                    default:
                        break;
                }
            }

            // Invoked as constructor: `new NJSocketPool(...)`
            NJSocketPool* obj = new NJSocketPool(db.c_str(), svsId, slave);
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            // Invoked as plain function `NJSocketPool(...)`, turn into construct call.
            Local<Value> argv[] = {args[0], args[1]};
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 2, argv).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJSocketPool::async_cb(uv_async_t* handle) {
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope handleScope(isolate); //required for Node 4.x
        NJSocketPool* obj = (NJSocketPool*) handle->data;
        assert(obj);
        if (!obj) return;
        {
            SPA::CAutoLock al(obj->m_cs);
            while (obj->m_deqPoolEvent.size()) {
                const PoolEvent &pe = obj->m_deqPoolEvent.front();
                if (!obj->m_evPool.IsEmpty()) {
                    Local<Value> argv[2];
                    argv[0] = Int32::New(isolate, pe.Spe);
                    auto handler = pe.Handler;
                    if (handler) {
                        unsigned int svsId = handler->GetSvsID();
                        switch (svsId) {
                            case SPA::Queue::sidQueue:
                                argv[1] = NJAsyncQueue::New(isolate, (NJA::CAQueue*)handler, true);
                                break;
                            case SPA::Mysql::sidMysql:
                            case SPA::Odbc::sidOdbc:
                            case SPA::Sqlite::sidSqlite:
                                argv[1] = NJSqlite::New(isolate, (NJA::CNjDb*)handler, true);
                                break;
                            case SPA::SFile::sidFile:
                                argv[1] = NJFile::New(isolate, (NJA::CSFile*)handler, true);
                                break;
                            default:
                                argv[1] = NJHandler::New(isolate, handler, true);
                                break;
                        }
                    } else {
                        argv[1] = Null(isolate);
                    }
                    Local<Function> cb = Local<Function>::New(isolate, obj->m_evPool);
                    cb->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                }
                obj->m_deqPoolEvent.pop_front();
            }
        }
        //isolate->RunMicrotasks();
    }

    void NJSocketPool::async_cs_cb(uv_async_t* handle) {
        unsigned short reqId;
        NJSocketPool* obj = (NJSocketPool*) handle->data;
        assert(obj);
        if (!obj) return;
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope handleScope(isolate); //required for Node 4.x or later
        {
            SPA::CAutoLock al(obj->m_cs);
            while (obj->m_deqSocketEvent.size()) {
                SocketEvent se = obj->m_deqSocketEvent.front();
                SPA::ClientSide::PAsyncServiceHandler ash = nullptr;
                *se.QData >> ash >> reqId;
                assert(ash);
                assert(reqId);
                Local<Object> njAsh;
                unsigned int sid = ash->GetSvsID();
                switch (sid) {
                    case SPA::Odbc::sidOdbc:
                    case SPA::Mysql::sidMysql:
                    case SPA::Sqlite::sidSqlite:
                        njAsh = NJSqlite::New(isolate, (CNjDb*) ash, true);
                        break;
                    case SPA::Queue::sidQueue:
                        njAsh = NJAsyncQueue::New(isolate, (CAQueue*) ash, true);
                        break;
                    case SPA::SFile::sidFile:
                        njAsh = NJFile::New(isolate, (CSFile*) ash, true);
                        break;
                    default:
                        njAsh = NJHandler::New(isolate, ash, true);
                        break;
                }
                Local<Value> jsReqId = Uint32::New(isolate, reqId);
                if (ash) {
                    sid = ash->GetSvsID();
                    switch (se.Se) {
                        case seChatEnter:
                            if (!obj->m_push.IsEmpty()) {
                                Local<String> jsName = ToStr(isolate, "Subscribe");
                                auto sender = ToMessageSender(isolate, *se.QData);
                                CComVariant vt;
                                *se.QData >> vt;
                                assert(!se.QData->GetSize());
                                auto groups = From(isolate, vt);
                                Local<Value> argv[] = {jsName, groups, sender, njAsh};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_push);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 4, argv);
                            }
                            break;
                        case seChatExit:
                            if (!obj->m_push.IsEmpty()) {
                                Local<String> jsName = ToStr(isolate, "Unsubscribe");
                                auto sender = ToMessageSender(isolate, *se.QData);
                                CComVariant vt;
                                *se.QData >> vt;
                                assert(!se.QData->GetSize());
                                auto groups = From(isolate, vt);
                                Local<Value> argv[] = {jsName, groups, sender, njAsh};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_push);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 4, argv);
                            }
                            break;
                        case sePublish:
                            if (!obj->m_push.IsEmpty()) {
                                Local<String> jsName = ToStr(isolate, "Publish");
                                auto sender = ToMessageSender(isolate, *se.QData);
                                CComVariant vt;
                                *se.QData >> vt;
                                auto groups = From(isolate, vt);
                                vt.Clear();
                                *se.QData >> vt;
                                assert(!se.QData->GetSize());
                                auto msg = From(isolate, vt);
                                Local<Value> argv[] = {jsName, groups, msg, sender, njAsh};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_push);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 5, argv);
                            }
                            break;
                        case sePublishEx:
                            if (!obj->m_push.IsEmpty()) {
                                Local<String> jsName = ToStr(isolate, "PublishEx");
                                auto sender = ToMessageSender(isolate, *se.QData);
                                CComVariant vt;
                                *se.QData >> vt;
                                auto groups = From(isolate, vt);
                                unsigned int len = se.QData->GetSize();
                                auto bytes = node::Buffer::New(isolate, (char*) se.QData->GetBuffer(), len).ToLocalChecked();
                                se.QData->SetSize(0);
                                Local<Value> argv[] = {jsName, groups, bytes, sender, njAsh};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_push);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 5, argv);
                            }
                            break;
                        case sePostUserMessage:
                            if (!obj->m_push.IsEmpty()) {
                                Local<String> jsName = ToStr(isolate, "SendMessage");
                                auto sender = ToMessageSender(isolate, *se.QData);
                                CComVariant vt;
                                *se.QData >> vt;
                                auto msg = From(isolate, vt);
                                assert(!se.QData->GetSize());
                                Local<Value> argv[] = {jsName, msg, sender, njAsh};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_push);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 4, argv);
                            }
                            break;
                        case sePostUserMessageEx:
                            if (!obj->m_push.IsEmpty()) {
                                Local<String> jsName = ToStr(isolate, "SendMessageEx");
                                auto sender = ToMessageSender(isolate, *se.QData);
                                unsigned int len = se.QData->GetSize();
                                auto bytes = node::Buffer::New(isolate, (char*) se.QData->GetBuffer(), len).ToLocalChecked();
                                se.QData->SetSize(0);
                                Local<Value> argv[] = {jsName, bytes, sender, njAsh};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_push);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 4, argv);
                            }
                            break;
                        case seAllProcessed:
                            if (!obj->m_ap.IsEmpty()) {
                                Local<Value> argv[] = {njAsh, jsReqId};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_ap);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                            }
                            break;
                        case seBaseRequestProcessed:
                            if (!obj->m_brp.IsEmpty()) {
                                Local<Value> argv[] = {njAsh, jsReqId};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_brp);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                            }
                            break;
                        case seResultReturned:
                            if (!obj->m_rr.IsEmpty()) {
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_rr);
                                Local<Value> argv[3];
                                argv[0] = njAsh;
                                argv[1] = jsReqId;
                                Local<Object> q;
                                if (se.QData->GetSize()) {
                                    q = NJQueue::New(isolate, se.QData);
                                } else {
                                    SPA::PUQueue buffer = nullptr;
                                    q = NJQueue::New(isolate, buffer);
                                }
                                argv[2] = q;
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                                auto objQueue = ObjectWrap::Unwrap<NJQueue>(q);
                                objQueue->Release();
                            }
                            break;
                        case seServerException:
                            if (!obj->m_se.IsEmpty()) {
                                std::wstring errMsg;
                                std::string errWhere;
                                unsigned int errCode = 0;
                                *se.QData >> errMsg >> errWhere >> errCode;
                                Local<String> jsMsg = ToStr(isolate, errMsg.c_str());
                                Local<String> jsWhere = ToStr(isolate, errWhere.c_str());
                                Local<Value> jsCode = Number::New(isolate, errCode);
                                Local<Value> argv[] = {njAsh, jsReqId, jsMsg, jsCode, jsWhere};
                                Local<Function> cb = Local<Function>::New(isolate, obj->m_se);
                                cb->Call(isolate->GetCurrentContext(), Null(isolate), 5, argv);
                            }
                            break;
                        default:
                            break;
                    }
                }
                CScopeUQueue::Unlock(se.QData);
                obj->m_deqSocketEvent.pop_front();
            }
        }
        //isolate->RunMicrotasks();
    }

    void NJSocketPool::Dispose(const FunctionCallbackInfo<Value>& args) {
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        obj->Release();
    }

    void NJSocketPool::DisconnectAll(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->Handler->DisconnectAll();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJSocketPool::getAsyncHandlers(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int index = 0;
            Local<Array> v = Array::New(isolate);
            switch (obj->SvsId) {
                case SPA::Queue::sidQueue:
                {
                    auto handlers = obj->Queue->GetAsyncHandlers();
                    for (auto it = handlers.begin(), end = handlers.end(); it != end; ++it, ++index) {
                        v->Set(index, NJAsyncQueue::New(isolate, it->get(), true));
                    }
                }
                    break;
                case SPA::Mysql::sidMysql:
                case SPA::Odbc::sidOdbc:
                case SPA::Sqlite::sidSqlite:
                {
                    auto handlers = obj->Db->GetAsyncHandlers();
                    for (auto it = handlers.begin(), end = handlers.end(); it != end; ++it, ++index) {
                        v->Set(index, NJSqlite::New(isolate, it->get(), true));
                    }
                }
                    break;
                    break;
                case SPA::SFile::sidFile:
                {
                    auto handlers = obj->File->GetAsyncHandlers();
                    for (auto it = handlers.begin(), end = handlers.end(); it != end; ++it, ++index) {
                        v->Set(index, NJFile::New(isolate, it->get(), true));
                    }
                }
                    break;
                default:
                {
                    auto handlers = obj->Handler->GetAsyncHandlers();
                    for (auto it = handlers.begin(), end = handlers.end(); it != end; ++it, ++index) {
                        v->Set(index, NJHandler::New(isolate, it->get(), true));
                    }
                }
                    break;
            }
            args.GetReturnValue().Set(v);
        }
    }

    void NJSocketPool::getAvg(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->Handler->IsAvg();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJSocketPool::getConnectedSockets(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetConnectedSockets();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getDisconnectedSockets(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetDisconnectedSockets();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getIdleSockets(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetIdleSockets();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getLockedSockets(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetLockedSockets();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getSvsId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        unsigned int data = obj->SvsId;
        args.GetReturnValue().Set(Uint32::New(isolate, data));
    }

    void NJSocketPool::getPoolId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetPoolId();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getError(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        Local<Object> errObj = Object::New(isolate);
        obj->m_cs.lock();
        errObj->Set(ToStr(isolate, "errCode"), Int32::New(isolate, obj->m_errSSL));
        errObj->Set(ToStr(isolate, "errMsg"), ToStr(isolate, obj->m_errMsg.c_str()));
        obj->m_cs.unlock();
        args.GetReturnValue().Set(errObj);
    }

    void NJSocketPool::getQueueAutoMerge(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->Handler->GetQueueAutoMerge();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJSocketPool::setQueueAutoMerge(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsBoolean()) {
                bool b = p->BooleanValue();
                obj->Handler->SetQueueAutoMerge(b);
            } else {
                ThrowException(isolate, BOOLEAN_EXPECTED);
            }
        }
    }

    void NJSocketPool::getQueueName(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string queueName = obj->Handler->GetQueueName();
            args.GetReturnValue().Set(ToStr(isolate, queueName.c_str()));
        }
    }

    void NJSocketPool::setQueueName(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsString()) {
                String::Utf8Value str(p);
                obj->Handler->SetQueueName(*str);
            } else {
                ThrowException(isolate, BOOLEAN_EXPECTED);
            }
        }
    }

    void NJSocketPool::getQueues(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetQueues();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getSockets(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            Local<Array> v = Array::New(isolate);
            auto sockets = obj->Handler->GetSockets();
            unsigned int index = 0;
            for (auto it = sockets.begin(), end = sockets.end(); it != end; ++it, ++index) {
                auto s = it->get();
                v->Set(index, NJSocket::New(isolate, s, true));
            }
            args.GetReturnValue().Set(v);
        }
    }

    void NJSocketPool::getSocketsPerThread(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetSocketsPerThread();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::getStarted(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->Handler->IsStarted();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJSocketPool::getThreadsCreated(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->Handler->GetThreadsCreated();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJSocketPool::Lock(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int timeout = (~0);
            if (args[0]->IsNumber()) {
                timeout = args[0]->Uint32Value();
            }
            switch (obj->SvsId) {
                case SPA::Queue::sidQueue:
                {
                    auto p = obj->Queue->Lock(timeout);
                    args.GetReturnValue().Set(NJAsyncQueue::New(isolate, p.get(), true));
                }
                    break;
                case SPA::Mysql::sidMysql:
                case SPA::Odbc::sidOdbc:
                case SPA::Sqlite::sidSqlite:
                {
                    auto p = obj->Db->Lock(timeout);
                    args.GetReturnValue().Set(NJSqlite::New(isolate, p.get(), true));
                }
                    break;
                case SPA::SFile::sidFile:
                {
                    auto p = obj->File->Lock(timeout);
                    args.GetReturnValue().Set(NJFile::New(isolate, p.get(), true));
                }
                    break;
                default:
                {
                    auto p = obj->Handler->Lock(timeout);
                    args.GetReturnValue().Set(NJHandler::New(isolate, p.get(), true));
                }
                    break;
            }
        }
    }

    void NJSocketPool::Seek(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            switch (obj->SvsId) {
                case SPA::Queue::sidQueue:
                {
                    auto p = obj->Queue->Seek();
                    if (p)
                        args.GetReturnValue().Set(NJAsyncQueue::New(isolate, p.get(), true));
                    else
                        args.GetReturnValue().SetNull();
                }
                    break;
                case SPA::Mysql::sidMysql:
                case SPA::Odbc::sidOdbc:
                case SPA::Sqlite::sidSqlite:
                {
                    auto p = obj->Db->Seek();
                    if (p)
                        args.GetReturnValue().Set(NJSqlite::New(isolate, p.get(), true));
                    else
                        args.GetReturnValue().SetNull();
                }
                    break;
                case SPA::SFile::sidFile:
                {
                    auto p = obj->File->Seek();
                    if (p)
                        args.GetReturnValue().Set(NJFile::New(isolate, p.get(), true));
                    else
                        args.GetReturnValue().SetNull();
                }
                    break;
                default:
                {
                    auto p = obj->Handler->Seek();
                    if (p)
                        args.GetReturnValue().Set(NJHandler::New(isolate, p.get(), true));
                    else
                        args.GetReturnValue().SetNull();
                }
                    break;
            }
        }
    }

    void NJSocketPool::SeekByQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsNullOrUndefined()) {
                switch (obj->SvsId) {
                    case SPA::Queue::sidQueue:
                    {
                        auto p = obj->Queue->SeekByQueue();
                        if (p)
                            args.GetReturnValue().Set(NJAsyncQueue::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                    case SPA::Odbc::sidOdbc:
                    case SPA::Mysql::sidMysql:
                    case SPA::Sqlite::sidSqlite:
                    {
                        auto p = obj->Db->SeekByQueue();
                        if (p)
                            args.GetReturnValue().Set(NJSqlite::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                    case SPA::SFile::sidFile:
                    {
                        auto p = obj->File->SeekByQueue();
                        if (p)
                            args.GetReturnValue().Set(NJFile::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                    default:
                    {
                        auto p = obj->Handler->SeekByQueue();
                        if (p)
                            args.GetReturnValue().Set(NJHandler::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                }
            } else if (p->IsString()) {
                std::string qname;
                String::Utf8Value str(p);
                qname = *str;
                switch (obj->SvsId) {
                    case SPA::Queue::sidQueue:
                    {
                        auto p = obj->Queue->SeekByQueue(qname);
                        if (p)
                            args.GetReturnValue().Set(NJAsyncQueue::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                    case SPA::Mysql::sidMysql:
                    case SPA::Odbc::sidOdbc:
                    case SPA::Sqlite::sidSqlite:
                    {
                        auto p = obj->Db->SeekByQueue(qname);
                        if (p)
                            args.GetReturnValue().Set(NJSqlite::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                    case SPA::SFile::sidFile:
                    {
                        auto p = obj->File->SeekByQueue(qname);
                        if (p)
                            args.GetReturnValue().Set(NJFile::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                    default:
                    {
                        auto p = obj->Handler->SeekByQueue(qname);
                        if (p)
                            args.GetReturnValue().Set(NJHandler::New(isolate, p.get(), true));
                        else
                            args.GetReturnValue().SetNull();
                    }
                        break;
                }
            } else {
                ThrowException(isolate, "A string, undefined or null expected");
            }
        }
    }

    void NJSocketPool::ShutdownPool(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            obj->Handler->ShutdownPool();
        }
    }

    bool NJSocketPool::To(Isolate* isolate, const Local<Object> &obj, SPA::ClientSide::CConnectionContext &cc) {
        Local<Array> props = obj->GetPropertyNames();
        if (props->Length() != 8) {
            ThrowException(isolate, "Invalid connection context");
            return false;
        }

        auto v = obj->Get(ToStr(isolate, "Host"));
        if (!v->IsString()) {
            ThrowException(isolate, "Invalid host string");
            return false;
        }
        String::Utf8Value host(v);
        cc.Host = *host;

        v = obj->Get(ToStr(isolate, "Port"));
        if (!v->IsUint32()) {
            ThrowException(isolate, "Invalid port number");
            return false;
        }
        cc.Port = v->Uint32Value();

        v = obj->Get(ToStr(isolate, "User"));
        if (!v->IsString()) {
            ThrowException(isolate, "Invalid user id string");
            return false;
        }
        String::Utf8Value uid(v);
        cc.UserId = Utilities::ToWide(*uid);

        v = obj->Get(ToStr(isolate, "Pwd"));
        if (!v->IsString()) {
            ThrowException(isolate, "Invalid password string");
            return false;
        }
        String::Utf8Value pwd(v);
        cc.Password = Utilities::ToWide(*pwd);
        unsigned int em = 0;
        v = obj->Get(ToStr(isolate, "EM"));
        if (v->IsUint32()) {
            em = v->Uint32Value();
        } else if (!v->IsNullOrUndefined()) {
            ThrowException(isolate, "An integer for encryption method expected");
            return false;
        }
        if (em > TLSv1) {
            ThrowException(isolate, "Invalid encryption method");
            return false;
        }
        cc.EncrytionMethod = (SPA::tagEncryptionMethod)em;

        v = obj->Get(ToStr(isolate, "Zip"));
        if (v->IsBoolean()) {
            cc.Zip = v->BooleanValue();
        } else if (!v->IsNullOrUndefined()) {
            ThrowException(isolate, "Boolean value expected for Zip");
            return false;
        }

        v = obj->Get(ToStr(isolate, "V6"));
        if (v->IsBoolean()) {
            cc.V6 = v->BooleanValue();
        } else if (!v->IsNullOrUndefined()) {
            ThrowException(isolate, "Boolean value expected for V6");
            return false;
        }

        v = obj->Get(ToStr(isolate, "AnyData"));
        if (!From(v, "", cc.AnyData)) {
            ThrowException(isolate, "Invalid data for AnyData");
            return false;
        }
        return true;
    }

    void NJSocketPool::StartSocketPool(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (!obj->IsValid(isolate))
            return;
        auto p1 = args[1];
        if (!p1->IsUint32()) {
            ThrowException(isolate, "An unsigned int number expected for client sockets");
            return;
        }
        unsigned int sessions = p1->Uint32Value();
        if (!sessions) {
            ThrowException(isolate, "The number of client sockets cannot be zero");
            return;
        }
        if (obj->Handler->IsStarted()) {
            ThrowException(isolate, "The socket pool already started");
            return;
        }
        std::vector<SPA::ClientSide::CConnectionContext> vCC;
        auto p0 = args[0];
        if (p0->IsArray()) {
            Local<Array> jsArr = Local<Array>::Cast(p0);
            unsigned int count = jsArr->Length();
            for (unsigned int n = 0; n < count; ++n) {
                auto v = jsArr->Get(n);
                if (!v->IsObject()) {
                    ThrowException(isolate, "Invalid connection context found");
                    return;
                }
                Local<Object> obj = jsArr->Get(n)->ToObject();
                SPA::ClientSide::CConnectionContext cc;
                if (!To(isolate, obj, cc)) {
                    return;
                }
                vCC.push_back(cc);
            }
        } else if (p0->IsObject()) {
            Local<Object> obj = p0->ToObject();
            SPA::ClientSide::CConnectionContext cc;
            if (!To(isolate, obj, cc)) {
                return;
            }
            vCC.push_back(cc);
        } else {
            ThrowException(isolate, "One or an array of connection contexts expected");
            return;
        }
        unsigned int remain = sessions % vCC.size();
        sessions = (unsigned int) ((sessions / vCC.size()) * vCC.size() + (remain ? vCC.size() : 0));
        unsigned int repeats = (unsigned int) (sessions / vCC.size() - 1);
        std::vector<SPA::ClientSide::CConnectionContext> vCCs = vCC;
        for (unsigned int n = 0; n < repeats; ++n) {
            for (auto it = vCC.begin(), end = vCC.end(); it != end; ++it) {
                vCCs.push_back(*it);
            }
        }
        obj->m_errSSL = 0;
        obj->m_errMsg.clear();
        typedef CConnectionContext* PCConnectionContext;
        PCConnectionContext ppCCs[] = {vCCs.data()};
        bool ok = obj->Handler->StartSocketPool(ppCCs, 1, sessions, true, SPA::tagThreadApartment::taNone);
        obj->m_cs.lock();
        if (!ok && !obj->m_errSSL) {
            auto cs = obj->Handler->GetSockets()[0];
            obj->m_errSSL = cs->GetErrorCode();
            obj->m_errMsg = cs->GetErrorMsg();
        } else if (obj->m_errSSL) {
            ok = false;
        }
        obj->m_cs.unlock();
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void NJSocketPool::Unlock(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {

        }
    }

    void NJSocketPool::setPoolEvent(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_evPool.Reset(isolate, Local<Function>::Cast(p));
            } else if (p->IsNullOrUndefined()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_evPool.Empty();
            } else {
                ThrowException(isolate, "A callback expected for tracking pool event");
            }
        }
    }

    void NJSocketPool::setResultReturned(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_rr.Reset(isolate, Local<Function>::Cast(p));
            } else if (p->IsNullOrUndefined()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_rr.Empty();
            } else {
                ThrowException(isolate, "A callback expected for tracking request returned result");
            }
        }
    }

    void NJSocketPool::setAllProcessed(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_ap.Reset(isolate, Local<Function>::Cast(p));
            } else if (p->IsNullOrUndefined()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_ap.Empty();
            } else {
                ThrowException(isolate, "A callback expected for tracking event that all requests are processed");
            }
        }
    }

    void NJSocketPool::setPush(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_push.Reset(isolate, Local<Function>::Cast(p));
            } else if (p->IsNullOrUndefined()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_push.Empty();
            } else {
                ThrowException(isolate, "A callback expected for tracking online message from server");
            }
        }
    }

    void NJSocketPool::setServerException(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_se.Reset(isolate, Local<Function>::Cast(p));
            } else if (p->IsNullOrUndefined()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_se.Empty();
            } else {
                ThrowException(isolate, "A callback expected for tracking exception from server");
            }
        }
    }

    void NJSocketPool::setBaseRequestProcessed(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocketPool* obj = ObjectWrap::Unwrap<NJSocketPool>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_brp.Reset(isolate, Local<Function>::Cast(p));
            } else if (p->IsNullOrUndefined()) {
                SPA::CAutoLock al(obj->m_cs);
                obj->m_brp.Empty();
            } else {
                ThrowException(isolate, "A callback expected for tracking the event of base request processed");
            }
        }
    }
}
