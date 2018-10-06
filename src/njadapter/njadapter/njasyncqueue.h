
#pragma once

#include "aqueue.h"
#include "njhandlerroot.h"

namespace NJA {

    class NJAsyncQueue : public NJHandlerRoot {
    public:
        NJAsyncQueue(CAQueue *aq);
        NJAsyncQueue(const NJAsyncQueue &h) = delete;
        ~NJAsyncQueue();

    public:
        NJAsyncQueue& operator=(const NJAsyncQueue &h) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, CAQueue *ash, bool setCb);

    private:
        void Release();
        bool IsValid(Isolate* isolate);

        static const SPA::INT64 SECRECT_NUM = 0x7fa1b4ff23a5;
        static void New(const FunctionCallbackInfo<Value>& args);
        static void getDequeueBatchSize(const FunctionCallbackInfo<Value>& args);
        static void getEnqueueNotified(const FunctionCallbackInfo<Value>& args);

        static void GetKeys(const FunctionCallbackInfo<Value>& args);
        static void StartQueueTrans(const FunctionCallbackInfo<Value>& args);
        static void EndQueueTrans(const FunctionCallbackInfo<Value>& args);
        static void CloseQueue(const FunctionCallbackInfo<Value>& args);
        static void FlushQueue(const FunctionCallbackInfo<Value>& args);
        static void Dequeue(const FunctionCallbackInfo<Value>& args);
        static void Enqueue(const FunctionCallbackInfo<Value>& args);
        static void EnqueueBatch(const FunctionCallbackInfo<Value>& args);
        static void BatchMessage(const FunctionCallbackInfo<Value>& args);
        static std::string GetKey(Isolate* isolate, Local<Value> jsKey);
        static void setResultReturned(const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
        CAQueue *m_aq;
        SPA::CUQueue *m_qBatch;
    };
}
