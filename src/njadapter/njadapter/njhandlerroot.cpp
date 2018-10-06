#include "stdafx.h"
#include "njhandlerroot.h"
#include "njqueue.h"
#include "njsocket.h"

namespace NJA {

    SPA::CUCriticalSection NJHandlerRoot::m_cs;
    Persistent<v8::FunctionTemplate> NJHandlerRoot::m_tpl;

    NJHandlerRoot::NJHandlerRoot(CAsyncServiceHandler *ash) : m_ash(ash) {
        assert(ash);
    }

    NJHandlerRoot::~NJHandlerRoot() {
        Release();
    }

    bool NJHandlerRoot::IsHandler(Local<Object> obj) {
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope handleScope(isolate); //required for Node 4.x or later
        Local<FunctionTemplate> cb = Local<FunctionTemplate>::New(isolate, m_tpl);
        return cb->HasInstance(obj);
    }

    bool NJHandlerRoot::IsValid(Isolate* isolate) {
        if (!m_ash) {
            ThrowException(isolate, "Async handler disposed");
            return false;
        }
        return true;
    }

    void NJHandlerRoot::Init(Local<Object> exports, Local<FunctionTemplate> &tpl) {

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Dispose);
        NODE_SET_PROTOTYPE_METHOD(tpl, "StartBatching", StartBatching);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SendRequest", SendRequest);
        NODE_SET_PROTOTYPE_METHOD(tpl, "AbortBatching", AbortBatching);
        NODE_SET_PROTOTYPE_METHOD(tpl, "AbortDequeuedMessage", AbortDequeuedMessage);
        NODE_SET_PROTOTYPE_METHOD(tpl, "CleanCallbacks", CleanCallbacks);
        NODE_SET_PROTOTYPE_METHOD(tpl, "CommitBatching", CommitBatching);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSvsId", getSvsId);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getRequestsQueued", getRequestsQueued);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isBatching", IsBatching);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isDequeuedMessageAborted", IsDequeuedMessageAborted);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isDequeuedResult", IsDequeuedResult);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isRouteeResult", IsRouteeResult);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSocket", getSocket);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isSame", IsSame);

        m_tpl.Reset(exports->GetIsolate(), tpl);
    }

    void NJHandlerRoot::Release() {
        SPA::CAutoLock al(m_cs);
        if (m_ash) {
            m_ash = nullptr;
        }
    }

    void NJHandlerRoot::IsSame(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        auto p0 = args[0];
        if (p0->IsObject()) {
            auto objH = p0->ToObject();
            if (IsHandler(objH)) {
                NJHandlerRoot* p = ObjectWrap::Unwrap<NJHandlerRoot>(objH);
                args.GetReturnValue().Set(Boolean::New(isolate, p->m_ash == obj->m_ash));
                return;
            }
        }
        args.GetReturnValue().Set(Boolean::New(isolate, false));
    }

    void NJHandlerRoot::getSvsId(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->m_ash->GetSvsID();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJHandlerRoot::CommitBatching(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool server_commit = false;
            auto p = args[0];
            if (p->IsBoolean())
                server_commit = p->BooleanValue();
            else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, BOOLEAN_EXPECTED);
                return;
            }
            bool ok = obj->m_ash->CommitBatching(server_commit);
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::getRequestsQueued(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->m_ash->GetRequestsQueued();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }

    void NJHandlerRoot::AbortBatching(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->m_ash->AbortBatching();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::StartBatching(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->m_ash->StartBatching();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::getSocket(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            Local<Object> njSocket = NJSocket::New(isolate, obj->m_ash->GetAttachedClientSocket(), true);
            args.GetReturnValue().Set(njSocket);
        }
    }

    void NJHandlerRoot::SendRequest(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p0 = args[0];
            if (!p0->IsUint32()) {
                ThrowException(isolate, "A request id expected for the 1st input");
                return;
            }
            unsigned int reqId = p0->Uint32Value();
            if (reqId > 0xffff || reqId <= SPA::tagBaseRequestID::idReservedTwo) {
                ThrowException(isolate, "An unsigned short request id expected");
                return;
            }
            NJQueue *njq = nullptr;
            const unsigned char *buffer = nullptr;
            unsigned int bytes = 0;
            int index = 1;
            auto p1 = args[1];
            if (p1->IsObject()) {
                auto qObj = p1->ToObject();
                if (NJQueue::IsUQueue(qObj)) {
                    njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                    SPA::CUQueue *q = njq->get();
                    if (q) {
                        buffer = q->GetBuffer();
                        bytes = q->GetSize();
                    }
                    ++index;
                }
            }
            Local<Value> argv[3];
            argv[0] = args[index];
            ++index;
            argv[1] = args[index];
            ++index;
            argv[2] = args[index];
            auto res = obj->m_ash->SendRequest(isolate, 3, argv, reqId, buffer, bytes);
            if (njq)
                njq->Release();
            args.GetReturnValue().Set(Boolean::New(isolate, (res && res != INVALID_NUMBER)));
        }
    }

    void NJHandlerRoot::Dispose(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        obj->Release();
    }

    void NJHandlerRoot::IsBatching(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->m_ash->IsBatching();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::IsDequeuedMessageAborted(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->m_ash->IsDequeuedMessageAborted();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::IsDequeuedResult(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->m_ash->IsDequeuedResult();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::IsRouteeResult(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool ok = obj->m_ash->IsRouteeRequest();
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        }
    }

    void NJHandlerRoot::AbortDequeuedMessage(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            obj->m_ash->AbortDequeuedMessage();
        }
    }

    void NJHandlerRoot::CleanCallbacks(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->m_ash->CleanCallbacks();
            args.GetReturnValue().Set(Uint32::New(isolate, data));
        }
    }
}
