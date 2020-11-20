
#include "stdafx.h"
#include "njsocket.h"
#include "njqueue.h"
#include "njclientqueue.h"
#include "njpush.h"
#include "njcert.h"

namespace NJA {

    SPA::CUCriticalSection NJSocket::m_cs;
    Persistent<Function> NJSocket::constructor;

    NJSocket::NJSocket(CClientSocket *socket) : m_socket(socket) {
    }

    NJSocket::~NJSocket() {
        Release();
    }

    bool NJSocket::IsValid(Isolate* isolate) {
        if (!m_socket) {
            ThrowException(isolate, "Socket handle disposed");
            return false;
        }
        return true;
    }

    void NJSocket::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, u"CSocket", 7));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Dispose);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Close", Close);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Shutdown", Shutdown);
        NODE_SET_PROTOTYPE_METHOD(tpl, "IgnoreLastRequest", IgnoreLastRequest);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Cancel", Cancel);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Echo", DoEcho);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SetSvrZip", TurnOnZipAtSvr);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SetSvrZiplevel", SetZipLevelAtSvr);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "setZip", setZip);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getZip", getZip);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setZipLevel", setZipLevel);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getZipLevel", getZipLevel);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setConnTimeout", setConnTimeout);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getConnTimeout", getConnTimeout);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setRecvTimeout", setRecvTimeout);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getRecvTimeout", getRecvTimeout);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setAutoConn", setAutoConn);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getAutoConn", getAutoConn);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSendable", getSendable);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getCert", getCert);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getConnState", getConnState);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getRouteeCount", getRouteeCount);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isRouting", isRouting);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getReqsQueued", getRequestsInQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getCurrReqId", getCurrReqId);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getCurrSvsId", getCurrSvsId);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getServerPingTime", getServerPingTime);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getEM", getEncryptionMethod);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getError", getError);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isConnected", getConnected);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getConnContext", getConnContext);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isRandom", isRandom);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBytesInSendingBuffer", getBytesInSendingBuffer);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBytesInRecvBuffer", getBytesInRecvBuffer);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBytesBatched", getBytesBatched);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBytesReceived", getBytesReceived);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBytesSent", getBytesSent);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getUserId", getUserId);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getPeerOs", getPeerOs);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getPeerAddr", getPeerAddr);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getPush", getPush);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getQueue", getQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getPoolId", getPoolId);
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, u"CSocket", 7), tpl->GetFunction(ctx).ToLocalChecked());
    }

    Local<Object> NJSocket::New(Isolate* isolate, CClientSocket *ash, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)ash;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJSocket::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
#ifdef BOOL_ISOLATE
                //bool setCb = args[0]->BooleanValue(isolate);
#else
                //bool setCb = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                NJSocket *obj = new NJSocket((CClientSocket*) ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CSocket()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJSocket::Release() {
        SPA::CAutoLock al(m_cs);
        if (m_socket) {
            m_socket = nullptr;
        }
    }

    void NJSocket::Dispose(const FunctionCallbackInfo<Value>& args) {
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        obj->Release();
    }

    void NJSocket::Close(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            obj->m_socket->Close();
        }
    }

    void NJSocket::Shutdown(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::tagShutdownType st = SPA::tagShutdownType::stBoth;
            auto p = args[0];
            if (p->IsInt32()) {
                SPA::INT64 data = p->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                if (data < 0 || data > (int) SPA::tagShutdownType::stBoth) {
                    ThrowException(isolate, INTEGER_EXPECTED);
                    return;
                }
                st = (SPA::tagShutdownType)data;
            }
            obj->m_socket->Shutdown(st);
        }
    }

    void NJSocket::setZip(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool zip = false;
            auto p = args[0];
            if (p->IsBoolean() || p->IsUint32()) {
#ifdef BOOL_ISOLATE
                zip = p->BooleanValue(isolate);
#else
                zip = p->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, BOOLEAN_EXPECTED);
                return;
            }
            obj->m_socket->SetZip(zip);
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->GetZip()));
        }
    }

    void NJSocket::getZip(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->GetZip()));
        }
    }

    void NJSocket::setZipLevel(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            tagZipLevel zip = tagZipLevel::zlDefault;
            auto p = args[0];
            if (p->IsInt32()) {
                SPA::UINT64 data = p->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                if (data < 0 || data > (int) tagZipLevel::zlBestCompression) {
                    ThrowException(isolate, "Bad zip level value");
                    return;
                }
                zip = (tagZipLevel) data;
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, INTEGER_EXPECTED);
                return;
            }
            obj->m_socket->SetZipLevel(zip);
            args.GetReturnValue().Set(Int32::New(isolate, (int) obj->m_socket->GetZipLevel()));
        }
    }

    void NJSocket::getZipLevel(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, (int) obj->m_socket->GetZipLevel()));
        }
    }

    void NJSocket::setConnTimeout(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int timeout = DEFAULT_CONN_TIMEOUT;
            auto p = args[0];
            if (p->IsUint32()) {
                timeout = p->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, "An unsigned int value expected");
                return;
            }
            obj->m_socket->SetConnTimeout(timeout);
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetConnTimeout()));
        }
    }

    void NJSocket::getConnTimeout(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetConnTimeout()));
        }
    }

    void NJSocket::setRecvTimeout(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int timeout = DEFAULT_RECV_TIMEOUT;
            auto p = args[0];
            if (p->IsUint32()) {
                timeout = p->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, "An unsigned int valaue expected");
                return;
            }
            obj->m_socket->SetRecvTimeout(timeout);
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetRecvTimeout()));
        }
    }

    void NJSocket::getRecvTimeout(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetRecvTimeout()));
        }
    }

    void NJSocket::setAutoConn(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ac = false;
            auto p = args[0];
            if (p->IsBoolean() || p->IsUint32()) {
#ifdef BOOL_ISOLATE
                ac = p->BooleanValue(isolate);
#else
                ac = p->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, BOOLEAN_EXPECTED);
                return;
            }
            obj->m_socket->SetAutoConn(ac);
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->GetAutoConn()));
        }
    }

    void NJSocket::getAutoConn(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->GetAutoConn()));
        }
    }

    void NJSocket::getSendable(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->Sendable()));
        }
    }

    void NJSocket::getCert(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            if (obj->m_socket->GetUCert()) {
                args.GetReturnValue().Set(NJCert::New(isolate, obj->m_socket->GetUCert(), true));
            } else {
                args.GetReturnValue().SetNull();
            }
        }
    }

    void NJSocket::getPush(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(NJPush::New(isolate, &obj->m_socket->GetPush(), true));
        }
    }

    void NJSocket::getQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(NJClientQueue::New(isolate, &obj->m_socket->GetClientQueue(), true));
        }
    }

    void NJSocket::getConnState(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, (int) obj->m_socket->GetConnectionState()));
        }
    }

    void NJSocket::IgnoreLastRequest(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (!p->IsUint32()) {
                ThrowException(isolate, "A request id expected for the 1st input");
                return;
            }
            unsigned int reqId = p->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            if (reqId > 0xffff || reqId <= (unsigned short) SPA::tagBaseRequestID::idReservedTwo) {
                ThrowException(isolate, "An unsigned short request id expected");
                return;
            }
            bool ok = obj->m_socket->IgnoreLastRequest((unsigned short) reqId);
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJSocket::getRouteeCount(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetRouteeCount()));
        }
    }

    void NJSocket::isRouting(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->IsRouting()));
        }
    }

    void NJSocket::getRequestsInQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetCountOfRequestsInQueue()));
        }
    }

    void NJSocket::getCurrReqId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Uint32::New(isolate, obj->m_socket->GetCurrentRequestID()));
        }
    }

    void NJSocket::getCurrSvsId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetCurrentServiceID()));
        }
    }

    void NJSocket::getServerPingTime(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetServerPingTime()));
        }
    }

    void NJSocket::getEncryptionMethod(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, (int) obj->m_socket->GetEncryptionMethod()));
        }
    }

    void NJSocket::getError(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto ctx = isolate->GetCurrentContext();
            Local<Object> errObj = Object::New(isolate);
            errObj->Set(ctx, ToStr(isolate, u"ec", 2), Int32::New(isolate, obj->m_socket->GetErrorCode()));
            errObj->Set(ctx, ToStr(isolate, u"em", 2), ToStr(isolate, obj->m_socket->GetErrorMsg().c_str()));
            args.GetReturnValue().Set(errObj);
        }
    }

    void NJSocket::getConnected(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->IsConnected()));
        }
    }

    void NJSocket::getConnContext(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto ctx = isolate->GetCurrentContext();
            auto cc = obj->m_socket->GetConnectionContext();
            Local<Object> objCC = Object::New(isolate);
            objCC->Set(ctx, ToStr(isolate, u"Host", 4), ToStr(isolate, cc.Host.c_str()));
            objCC->Set(ctx, ToStr(isolate, u"Port", 4), Number::New(isolate, cc.Port));
#ifdef WIN32_64
            objCC->Set(ctx, ToStr(isolate, u"User", 4), ToStr(isolate, (const UTF16*) cc.UserId.c_str(), cc.UserId.size()));
#else
            auto s = Utilities::ToUTF16(cc.UserId.c_str(), cc.UserId.size());
            objCC->Set(ctx, ToStr(isolate, u"User", 4), ToStr(isolate, s.c_str(), s.size()));
#endif
            objCC->Set(ctx, ToStr(isolate, u"Pwd", 3), Null(isolate)); //no password returned
            objCC->Set(ctx, ToStr(isolate, u"EM", 2), Number::New(isolate, (int) cc.EncrytionMethod));
            objCC->Set(ctx, ToStr(isolate, u"Zip", 3), Boolean::New(isolate, cc.Zip));
            objCC->Set(ctx, ToStr(isolate, u"V6", 2), Boolean::New(isolate, cc.V6));
            objCC->Set(ctx, ToStr(isolate, u"AnyData", 7), From(isolate, cc.AnyData));
            args.GetReturnValue().Set(objCC);
        }
    }

    void NJSocket::isRandom(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->IsRandom()));
        }
    }

    void NJSocket::getBytesInSendingBuffer(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetBytesInSendingBuffer()));
        }
    }

    void NJSocket::getBytesInRecvBuffer(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetBytesInReceivingBuffer()));
        }
    }

    void NJSocket::getBytesBatched(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, obj->m_socket->GetBytesBatched()));
        }
    }

    void NJSocket::getBytesReceived(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, (double) obj->m_socket->GetBytesReceived()));
        }
    }

    void NJSocket::getBytesSent(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Number::New(isolate, (double) obj->m_socket->GetBytesSent()));
        }
    }

    void NJSocket::getUserId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto uid = obj->m_socket->GetUID();
#ifdef WIN64
            args.GetReturnValue().Set(ToStr(isolate, uid.c_str()));
#else
            args.GetReturnValue().Set(ToStr(isolate, Utilities::ToUTF8(uid).c_str()));
#endif
        }
    }

    void NJSocket::getPoolId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto id = obj->m_socket->GetPoolId();
            args.GetReturnValue().Set(Number::New(isolate, (double) id));
        }
    }

    void NJSocket::getPeerOs(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, (int) obj->m_socket->GetPeerOs()));
        }
    }

    void NJSocket::getPeerAddr(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int port = 0;
            args.GetReturnValue().Set(ToStr(isolate, obj->m_socket->GetPeerName(&port).c_str()));
        }
    }

    void NJSocket::Cancel(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->Cancel()));
        }
    }

    void NJSocket::DoEcho(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->DoEcho()));
        }
    }

    void NJSocket::TurnOnZipAtSvr(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool zip = false;
            auto p = args[0];
            if (p->IsBoolean() || p->IsUint32()) {
#ifdef BOOL_ISOLATE
                zip = p->BooleanValue(isolate);
#else
                zip = p->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, BOOLEAN_EXPECTED);
                return;
            }
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->TurnOnZipAtSvr(zip)));
        }
    }

    void NJSocket::SetZipLevelAtSvr(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            int zl = (int) tagZipLevel::zlDefault;
            auto p = args[0];
            if (p->IsInt32()) {
                zl = p->Int32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, INTEGER_EXPECTED);
                return;
            }
            if (zl > (int) tagZipLevel::zlBestCompression) {
                ThrowException(isolate, "Bad zip level value");
                return;
            }
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->SetZipLevelAtSvr((tagZipLevel) zl)));
        }
    }
}
