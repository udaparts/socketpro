
#include "stdafx.h"
#include "njasyncqueue.h"
#include "njqueue.h"

namespace NJA {

    Persistent<Function> NJAsyncQueue::constructor;

    NJAsyncQueue::NJAsyncQueue(CAQueue *aq) : NJHandlerRoot(aq), m_aq(aq), m_qBatch(nullptr) {

    }

    NJAsyncQueue::~NJAsyncQueue() {
        Release();
    }

    bool NJAsyncQueue::IsValid(Isolate* isolate) {
        if (!m_aq) {
            ThrowException(isolate, "Async client queue handler disposed");
            return false;
        }
        return NJHandlerRoot::IsValid(isolate);
    }

    void NJAsyncQueue::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, "CAsyncQueue"));
        tpl->InstanceTemplate()->SetInternalFieldCount(3);

        NJHandlerRoot::Init(exports, tpl);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "GetKeys", GetKeys);
        NODE_SET_PROTOTYPE_METHOD(tpl, "StartTrans", StartQueueTrans);
        NODE_SET_PROTOTYPE_METHOD(tpl, "EndTrans", EndQueueTrans);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Close", CloseQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Flush", FlushQueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Dequeue", Dequeue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Enqueue", Enqueue);
        NODE_SET_PROTOTYPE_METHOD(tpl, "EnqueueBatch", EnqueueBatch);
        NODE_SET_PROTOTYPE_METHOD(tpl, "BatchMessage", BatchMessage);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDeqBatchSize", getDequeueBatchSize);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getEnqNotified", getEnqueueNotified);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setResultReturned", setResultReturned);

        constructor.Reset(isolate, tpl->GetFunction());
        exports->Set(ToStr(isolate, "CAsyncQueue"), tpl->GetFunction());
    }

    Local<Object> NJAsyncQueue::New(Isolate* isolate, CAQueue *ash, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)ash;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJAsyncQueue::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
                bool setCb = args[0]->BooleanValue();
                SPA::INT64 ptr = args[2]->IntegerValue();
                NJAsyncQueue *obj = new NJAsyncQueue((CAQueue*) ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CAsyncQueue()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJAsyncQueue::Release() {
        if (m_qBatch) {
            CScopeUQueue::Unlock(m_qBatch);
        }
        {
            SPA::CAutoLock al(m_cs);
            if (m_aq) {
                m_aq = nullptr;
            }
        }
        NJHandlerRoot::Release();
    }

    void NJAsyncQueue::setResultReturned(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (p->IsFunction()) {
                obj->m_aq->SetRR(isolate, p);
            } else if (p->IsNullOrUndefined()) {
                obj->m_aq->SetRR(isolate, p);
            } else {
                ThrowException(isolate, "A callback expected for tracking request returned result");
            }
        }
    }

    void NJAsyncQueue::getDequeueBatchSize(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int size = obj->m_aq->GetDequeueBatchSize();
            args.GetReturnValue().Set(Number::New(isolate, size));
        }
    }

    void NJAsyncQueue::getEnqueueNotified(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool notify = obj->m_aq->GetEnqueueNotified();
            args.GetReturnValue().Set(Boolean::New(isolate, notify));
        }
    }

    void NJAsyncQueue::GetKeys(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            Local<Value> argv[] = {args[0], args[1]};
            SPA::UINT64 index = obj->m_aq->GetKeys(isolate, 2, argv);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    std::string NJAsyncQueue::GetKey(Isolate* isolate, Local<Value> jsKey) {
        if (!jsKey->IsString()) {
            ThrowException(isolate, "A valid key string required to find a queue file at server side");
            return "";
        }
        String::Utf8Value str(jsKey);
        std::string s(*str);
        if (!s.size()) {
            ThrowException(isolate, "A valid key string required to find a queue file at server side");
        }
        return s;
    }

    void NJAsyncQueue::StartQueueTrans(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string key = GetKey(isolate, args[0]);
            if (!key.size())
                return;
            Local<Value> argv[] = {args[1], args[2]};
            SPA::UINT64 index = obj->m_aq->StartQueueTrans(isolate, 2, argv, key.c_str());
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJAsyncQueue::EndQueueTrans(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            bool rollback = false;
            auto p0 = args[0];
            if (p0->IsBoolean())
                rollback = p0->BooleanValue();
            else if (!p0->IsNullOrUndefined()) {
                ThrowException(isolate, "Boolean value expected for rollback");
                return;
            }
            Local<Value> argv[] = {args[1], args[2]};
            SPA::UINT64 index = obj->m_aq->EndQueueTrans(isolate, 2, argv, rollback);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJAsyncQueue::CloseQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string key = GetKey(isolate, args[0]);
            if (!key.size())
                return;
            Local<Value> argv[] = {args[1], args[2]};
            auto p = args[3];
            bool perm = false;
            if (p->IsBoolean())
                perm = p->BooleanValue();
            else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "Boolean value expected for permanent delete");
                return;
            }
            SPA::UINT64 index = obj->m_aq->CloseQueue(isolate, 2, argv, key.c_str(), perm);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJAsyncQueue::FlushQueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string key = GetKey(isolate, args[0]);
            if (!key.size())
                return;
            Local<Value> argv[] = {args[1], args[2]};
            auto p = args[3];
            int option = 0;
            if (p->IsInt32())
                option = p->Int32Value();
            else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "Integer value expected for flush option");
                return;
            }
            if (option < 0 || option > SPA::oDiskCommitted) {
                ThrowException(isolate, "Bad option value");
                return;
            }
            SPA::UINT64 index = obj->m_aq->FlushQueue(isolate, 2, argv, key.c_str(), (SPA::tagOptimistic)option);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJAsyncQueue::Dequeue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string key = GetKey(isolate, args[0]);
            if (!key.size())
                return;
            Local<Value> argv[] = {args[1], args[2]};
            auto p = args[3];
            unsigned int timeout = 0;
            if (p->IsUint32())
                timeout = p->Uint32Value();
            else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "Unsigned int value expected for dequeue timeout in millsecond");
                return;
            }
            SPA::UINT64 index = obj->m_aq->Dequeue(isolate, 2, argv, key.c_str(), timeout);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJAsyncQueue::Enqueue(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string key = GetKey(isolate, args[0]);
            if (!key.size())
                return;
            auto p = args[1];
            unsigned int reqId = 0;
            if (p->IsUint32())
                reqId = p->Uint32Value();
            else {
                ThrowException(isolate, "Unsigned short value expected for message request id");
                return;
            }
            int pos = 0;
            p = args[2];
            NJQueue *njq = nullptr;
            unsigned int size = 0;
            const unsigned char *buffer = nullptr;
            if (p->IsObject()) {
                auto qObj = p->ToObject();
                if (NJQueue::IsUQueue(qObj)) {
                    njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                    SPA::CUQueue *q = njq->get();
                    if (q) {
                        buffer = q->GetBuffer();
                        size = q->GetSize();
                    }
                    ++pos;
                }
            }
            Local<Value> argv[] = {args[2 + pos], args[3 + pos]};
            SPA::UINT64 index = obj->m_aq->Enqueue(isolate, 2, argv, key.c_str(), (unsigned short) reqId, buffer, size);
            if (njq)
                njq->Release();
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJAsyncQueue::BatchMessage(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p0 = args[0];
            if (!p0->IsUint32()) {
                ThrowException(isolate, "A valid request id expected for the 1st input");
                return;
            }
            unsigned int reqId = p0->Uint32Value();
            if (reqId > 0xffff || reqId <= SPA::tagBaseRequestID::idReservedTwo) {
                ThrowException(isolate, "A valid unsigned short request id expected");
                return;
            }

            if (reqId <= Queue::idEnqueueBatch || reqId == Queue::idBatchSizeNotified) {
                ThrowException(isolate, "Cannot use reserved message request ids");
                return;
            }

            if (!obj->m_qBatch) {
                obj->m_qBatch = CScopeUQueue::Lock();
            }
            auto p1 = args[1];
            if (p1->IsNullOrUndefined()) {
                CAsyncQueue::BatchMessage(reqId, (const unsigned char*) nullptr, 0, *obj->m_qBatch);
                return;
            } else if (p1->IsObject()) {
                auto qObj = p1->ToObject();
                if (NJQueue::IsUQueue(qObj)) {
                    NJQueue *njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                    SPA::CUQueue *q = njq->get();
                    if (q) {
                        CAsyncQueue::BatchMessage(reqId, q->GetBuffer(), q->GetSize(), *obj->m_qBatch);
                    } else {
                        CAsyncQueue::BatchMessage(reqId, (const unsigned char*) nullptr, 0, *obj->m_qBatch);
                    }
                    njq->Release();
                    return;
                }
            }
            ThrowException(isolate, "A CUQueue instance, null or undefined value expected");
        }
    }

    void NJAsyncQueue::EnqueueBatch(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::string key = GetKey(isolate, args[0]);
            if (!key.size())
                return;
            if (!obj->m_qBatch || !obj->m_qBatch->GetSize()) {
                ThrowException(isolate, "No messages batched yet");
                CScopeUQueue::Unlock(obj->m_qBatch);
                return;
            }
            Local<Value> argv[] = {args[1], args[2]};
            SPA::UINT64 index = obj->m_aq->EnqueueBatch(isolate, 2, argv, key.c_str(), obj->m_qBatch->GetBuffer(), obj->m_qBatch->GetSize());
            CScopeUQueue::Unlock(obj->m_qBatch);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }
}
