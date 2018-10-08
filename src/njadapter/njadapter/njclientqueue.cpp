#include "stdafx.h"
#include "njclientqueue.h"

namespace NJA {

    Persistent<Function> NJClientQueue::constructor;

    NJClientQueue::NJClientQueue(SPA::ClientSide::IClientQueue *cq) : m_cq(cq) {
    }

    NJClientQueue::~NJClientQueue() {
    }

    void NJClientQueue::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, "CClientQueue"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "StartQueue", StartQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "StopQueue", StopQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "AbortJob", AbortJob);
        NODE_SET_PROTOTYPE_METHOD(tpl, "StartJob", StartJob);
        NODE_SET_PROTOTYPE_METHOD(tpl, "EndJob", EndJob);
        NODE_SET_PROTOTYPE_METHOD(tpl, "RemoveByTTL", RemoveByTTL);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Reset", Reset);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getMessagesInDequeuing", getMessagesInDequeuing);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getMessageCount", getMessageCount);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSize", getQueueSize);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getAvailable", getAvailable);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSecure", getSecure);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getFileName", getQueueFileName);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getName", getQueueName);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getEnabled", getDequeueEnabled);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getJobSize", getJobSize);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getLastIndex", getLastIndex);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getShared", getDequeueShared);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getTTL", getTTL);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getStatus", getQueueStatus);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getLastMsgTime", getLastMessageTime);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setRoutingIndex", setRoutingQueueIndex);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getRoutingIndex", getRoutingQueueIndex);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getOptimistic", getOptimistic);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setOptimistic", setOptimistic);

        //Followings are not implemented because you will not use them
        //NODE_SET_PROTOTYPE_METHOD(tpl, "CancelQueuedRequests", CancelQueuedRequests);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "AppendTo", AppendTo);
        //NODE_SET_PROTOTYPE_METHOD(tpl, "EnsureAppending", EnsureAppending);

        constructor.Reset(isolate, tpl->GetFunction());
        exports->Set(ToStr(isolate, "CClientQueue"), tpl->GetFunction());
    }

    Local<Object> NJClientQueue::New(Isolate* isolate, SPA::ClientSide::IClientQueue *cq, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)cq;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJClientQueue::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
                bool setCb = args[0]->BooleanValue();
                SPA::INT64 ptr = args[2]->IntegerValue();
                NJClientQueue *obj = new NJClientQueue((SPA::ClientSide::IClientQueue*)ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CClientQueue()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJClientQueue::StartQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto p = args[0];
        std::string qName;
        if (p->IsString()) {
            qName = ToAStr(p);
        }
        if (!qName.size()) {
            ThrowException(isolate, "A non-empty string expected for client queue name");
            return;
        }
        bool secure = (ClientCoreLoader.GetEncryptionMethod(obj->m_cq->GetHandle()) != NoEncryption);
        p = args[1];
        if (p->IsBoolean()) {
            secure = p->BooleanValue();
        } else if (!p->IsNullOrUndefined()) {
            ThrowException(isolate, "A boolean expected for client queue security");
            return;
        }
        p = args[2];
        unsigned int ttl = 24 * 3600;
        if (p->IsUint32()) {
            ttl = p->Uint32Value();
        } else if (!p->IsNullOrUndefined()) {
            ThrowException(isolate, "An unsigned int value expected for message time-to-live");
            return;
        }
        bool ok = obj->m_cq->StartQueue(qName.c_str(), ttl, secure, false);
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void NJClientQueue::StopQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto p = args[0];
        bool permanent = false;
        if (p->IsBoolean()) {
            permanent = p->BooleanValue();
        } else if (!p->IsNullOrUndefined()) {
            ThrowException(isolate, BOOLEAN_EXPECTED);
            return;
        }
        obj->m_cq->StopQueue(permanent);
    }

    void NJClientQueue::getMessagesInDequeuing(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto count = obj->m_cq->GetMessagesInDequeuing();
        args.GetReturnValue().Set(Number::New(isolate, count));
    }

    void NJClientQueue::getMessageCount(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto count = obj->m_cq->GetMessageCount();
        args.GetReturnValue().Set(Number::New(isolate, (double) count));
    }

    void NJClientQueue::getQueueSize(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto size = obj->m_cq->GetQueueSize();
        args.GetReturnValue().Set(Number::New(isolate, (double) size));
    }

    void NJClientQueue::getAvailable(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto available = obj->m_cq->IsAvailable();
        args.GetReturnValue().Set(Boolean::New(isolate, available));
    }

    void NJClientQueue::getSecure(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto secure = obj->m_cq->IsSecure();
        args.GetReturnValue().Set(Boolean::New(isolate, secure));
    }

    void NJClientQueue::getQueueFileName(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto file = ToStr(isolate, obj->m_cq->GetQueueFileName());
        args.GetReturnValue().Set(file);
    }

    void NJClientQueue::getQueueName(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto qn = ToStr(isolate, obj->m_cq->GetQueueName());
        args.GetReturnValue().Set(qn);
    }

    void NJClientQueue::getDequeueEnabled(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto enabled = obj->m_cq->IsDequeueEnabled();
        args.GetReturnValue().Set(Boolean::New(isolate, enabled));
    }

    void NJClientQueue::AbortJob(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        bool ok = obj->m_cq->AbortJob();
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void NJClientQueue::StartJob(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        bool ok = obj->m_cq->StartJob();
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void NJClientQueue::EndJob(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        bool ok = obj->m_cq->EndJob();
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void NJClientQueue::getJobSize(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto size = obj->m_cq->GetJobSize();
        args.GetReturnValue().Set(Number::New(isolate, (double) size));
    }

    void NJClientQueue::getLastIndex(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto index = obj->m_cq->GetLastIndex();
        args.GetReturnValue().Set(Number::New(isolate, (double) index));
    }

    void NJClientQueue::getDequeueShared(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto shared = obj->m_cq->IsDequeueShared();
        args.GetReturnValue().Set(Boolean::New(isolate, shared));
    }

    void NJClientQueue::getTTL(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto ttl = obj->m_cq->GetTTL();
        args.GetReturnValue().Set(Number::New(isolate, ttl));
    }

    void NJClientQueue::RemoveByTTL(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto index = obj->m_cq->RemoveByTTL();
        args.GetReturnValue().Set(Number::New(isolate, (double) index));
    }

    void NJClientQueue::getQueueStatus(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        args.GetReturnValue().Set(Int32::New(isolate, obj->m_cq->GetQueueOpenStatus()));
    }

    void NJClientQueue::Reset(const FunctionCallbackInfo<Value>& args) {
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        obj->m_cq->Reset();
    }

    void NJClientQueue::getLastMessageTime(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        SPA::UINT64 milliseconds = obj->m_cq->GetLastMessageTime();
        milliseconds += time_offset((time_t) milliseconds);
        milliseconds *= 1000;
        args.GetReturnValue().Set(Date::New(isolate, (double) milliseconds));
    }

    void NJClientQueue::setRoutingQueueIndex(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        bool enabled = false;
        auto p = args[0];
        if (p->IsBoolean()) {
            enabled = p->BooleanValue();
        } else if (!p->IsNullOrUndefined()) {
            ThrowException(isolate, BOOLEAN_EXPECTED);
            return;
        }
        obj->m_cq->EnableRoutingQueueIndex(enabled);
        args.GetReturnValue().Set(Boolean::New(isolate, obj->m_cq->IsRoutingQueueIndexEnabled()));
    }

    void NJClientQueue::getRoutingQueueIndex(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        args.GetReturnValue().Set(Boolean::New(isolate, obj->m_cq->IsRoutingQueueIndexEnabled()));
    }

    void NJClientQueue::getOptimistic(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        args.GetReturnValue().Set(Int32::New(isolate, obj->m_cq->GetOptimistic()));
    }

    void NJClientQueue::setOptimistic(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJClientQueue* obj = ObjectWrap::Unwrap<NJClientQueue>(args.Holder());
        auto p = args[0];
        if (!p->IsInt32()) {
            ThrowException(isolate, "An integer value expected for optimistic value");
            return;
        }
        int n = p->Int32Value();
        if (n < oMemoryCached || n > oDiskCommitted) {
            ThrowException(isolate, "A valid value expected for optimistic value");
            return;
        }
        obj->m_cq->SetOptimistic((tagOptimistic) n);
        args.GetReturnValue().Set(Int32::New(isolate, obj->m_cq->GetOptimistic()));
    }

    //Followings not implemented because you will not use them

    void NJClientQueue::AppendTo(const FunctionCallbackInfo<Value>& args) {

    }

    void NJClientQueue::EnsureAppending(const FunctionCallbackInfo<Value>& args) {

    }

    void NJClientQueue::CancelQueuedRequests(const FunctionCallbackInfo<Value>& args) {

    }
}
