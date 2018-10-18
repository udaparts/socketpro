
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
        tpl->SetClassName(ToStr(isolate, "CSocket"));
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

        constructor.Reset(isolate, tpl->GetFunction());
        exports->Set(ToStr(isolate, "CSocket"), tpl->GetFunction());
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
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
                bool setCb = args[0]->BooleanValue();
                SPA::INT64 ptr = args[2]->IntegerValue();
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
        Isolate* isolate = args.GetIsolate();
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
            SPA::tagShutdownType st = stBoth;
            auto p = args[0];
            if (p->IsInt32()) {
                SPA::INT64 data = p->IntegerValue();
                if (data < 0 || data > SPA::stBoth) {
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
            if (p->IsBoolean())
                zip = p->BooleanValue();
            else if (!p->IsNullOrUndefined()) {
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
            tagZipLevel zip = zlDefault;
            auto p = args[0];
            if (p->IsInt32()) {
                SPA::UINT64 data = p->IntegerValue();
                if (data < 0 || data > zlBestCompression) {
                    ThrowException(isolate, "Bad zip level value");
                    return;
                }
                zip = (tagZipLevel) data;
            } else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, INTEGER_EXPECTED);
                return;
            }
            obj->m_socket->SetZipLevel(zip);
            args.GetReturnValue().Set(Int32::New(isolate, obj->m_socket->GetZipLevel()));
        }
    }

    void NJSocket::getZipLevel(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, obj->m_socket->GetZipLevel()));
        }
    }

    void NJSocket::setConnTimeout(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int timeout = DEFAULT_CONN_TIMEOUT;
            auto p = args[0];
            if (p->IsUint32()) {
                timeout = p->Uint32Value();
            } else if (!p->IsNullOrUndefined()) {
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
                timeout = p->Uint32Value();
            } else if (!p->IsNullOrUndefined()) {
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
            if (p->IsBoolean())
                ac = p->BooleanValue();
            else if (!p->IsNullOrUndefined()) {
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
            args.GetReturnValue().Set(Int32::New(isolate, obj->m_socket->GetConnectionState()));
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
            unsigned int reqId = p->Uint32Value();
            if (reqId > 0xffff || reqId <= SPA::tagBaseRequestID::idReservedTwo) {
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
            args.GetReturnValue().Set(Int32::New(isolate, obj->m_socket->GetEncryptionMethod()));
        }
    }

    void NJSocket::getError(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            Local<Object> errObj = Object::New(isolate);
            errObj->Set(ToStr(isolate, "ec"), Int32::New(isolate, obj->m_socket->GetErrorCode()));
            errObj->Set(ToStr(isolate, "em"), ToStr(isolate, obj->m_socket->GetErrorMsg().c_str()));
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
            bool ok;
            auto cc = obj->m_socket->GetConnectionContext();
            Local<Object> objCC = Object::New(isolate);
            ok = objCC->Set(ToStr(isolate, "Host"), ToStr(isolate, cc.Host.c_str()));
            ok = objCC->Set(ToStr(isolate, "Port"), Number::New(isolate, cc.Port));
            ok = objCC->Set(ToStr(isolate, "User"), ToStr(isolate, cc.UserId.c_str()));
            ok = objCC->Set(ToStr(isolate, "Pwd"), Null(isolate)); //no password returned
            ok = objCC->Set(ToStr(isolate, "EM"), Number::New(isolate, cc.EncrytionMethod));
            ok = objCC->Set(ToStr(isolate, "Zip"), Boolean::New(isolate, cc.Zip));
            ok = objCC->Set(ToStr(isolate, "V6"), Boolean::New(isolate, cc.V6));
            ok = objCC->Set(ToStr(isolate, "AnyData"), From(isolate, cc.AnyData));
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
            args.GetReturnValue().Set(ToStr(isolate, uid.c_str()));
        }
    }

    void NJSocket::getPeerOs(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, obj->m_socket->GetPeerOs()));
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
            if (p->IsBoolean())
                zip = p->BooleanValue();
            else if (!p->IsNullOrUndefined()) {
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
            int zl = SPA::zlDefault;
            auto p = args[0];
            if (p->IsInt32()) {
                zl = p->Int32Value();
            } else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, INTEGER_EXPECTED);
                return;
            }
            if (zl > SPA::zlBestCompression) {
                ThrowException(isolate, "Bad zip level value");
                return;
            }
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_socket->SetZipLevelAtSvr((SPA::tagZipLevel)zl)));
        }
    }
}
