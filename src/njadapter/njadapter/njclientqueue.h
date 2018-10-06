#pragma once


namespace NJA {

    class NJClientQueue : public node::ObjectWrap {
    public:
        NJClientQueue(SPA::ClientSide::IClientQueue *cq);
        NJClientQueue(const NJClientQueue &cq) = delete;

        ~NJClientQueue();

    public:
        NJClientQueue& operator=(const NJClientQueue &cq) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::ClientSide::IClientQueue *cq, bool setCb);

    private:
        static const SPA::INT64 SECRECT_NUM = 0x7fabb0ffe4a5;
        static void New(const FunctionCallbackInfo<Value>& args);
        static void StartQueue(const FunctionCallbackInfo<Value>& args);
        static void StopQueue(const FunctionCallbackInfo<Value>& args);
        static void AbortJob(const FunctionCallbackInfo<Value>& args);
        static void StartJob(const FunctionCallbackInfo<Value>& args);
        static void EndJob(const FunctionCallbackInfo<Value>& args);
        static void CancelQueuedRequests(const FunctionCallbackInfo<Value>& args);
        static void AppendTo(const FunctionCallbackInfo<Value>& args);
        static void EnsureAppending(const FunctionCallbackInfo<Value>& args);
        static void RemoveByTTL(const FunctionCallbackInfo<Value>& args);
        static void Reset(const FunctionCallbackInfo<Value>& args);
        static void getMessagesInDequeuing(const FunctionCallbackInfo<Value>& args);
        static void getMessageCount(const FunctionCallbackInfo<Value>& args);
        static void getQueueSize(const FunctionCallbackInfo<Value>& args);
        static void getAvailable(const FunctionCallbackInfo<Value>& args);
        static void getSecure(const FunctionCallbackInfo<Value>& args);
        static void getQueueFileName(const FunctionCallbackInfo<Value>& args);
        static void getQueueName(const FunctionCallbackInfo<Value>& args);
        static void getDequeueEnabled(const FunctionCallbackInfo<Value>& args);
        static void getJobSize(const FunctionCallbackInfo<Value>& args);
        static void getLastIndex(const FunctionCallbackInfo<Value>& args);
        static void getDequeueShared(const FunctionCallbackInfo<Value>& args);
        static void getTTL(const FunctionCallbackInfo<Value>& args);
        static void getQueueStatus(const FunctionCallbackInfo<Value>& args);
        static void getLastMessageTime(const FunctionCallbackInfo<Value>& args);
        static void setRoutingQueueIndex(const FunctionCallbackInfo<Value>& args);
        static void getRoutingQueueIndex(const FunctionCallbackInfo<Value>& args);
        static void getOptimistic(const FunctionCallbackInfo<Value>& args);
        static void setOptimistic(const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
        SPA::ClientSide::IClientQueue *m_cq;
    };
}
