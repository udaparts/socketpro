#pragma once

namespace NJA {

    class NJHandlerRoot : public node::ObjectWrap {
    public:
        NJHandlerRoot(SPA::ClientSide::CAsyncServiceHandler *ash);
        NJHandlerRoot(const NJHandlerRoot &r) = delete;
        ~NJHandlerRoot();

    public:
        NJHandlerRoot& operator=(const NJHandlerRoot &r) = delete;
        static bool IsHandler(Local<Object> obj);

    protected:
        virtual bool IsValid(Isolate* isolate);
        virtual void Release();

        static void Init(Local<Object> exports, Local<v8::FunctionTemplate> &tpl);

        static void getSvsId(const FunctionCallbackInfo<Value>& args);
        static void AbortBatching(const FunctionCallbackInfo<Value>& args);
        static void AbortDequeuedMessage(const FunctionCallbackInfo<Value>& args);
        static void CleanCallbacks(const FunctionCallbackInfo<Value>& args);
        static void CommitBatching(const FunctionCallbackInfo<Value>& args);
        static void getRequestsQueued(const FunctionCallbackInfo<Value>& args);
        static void IsBatching(const FunctionCallbackInfo<Value>& args);
        static void IsDequeuedMessageAborted(const FunctionCallbackInfo<Value>& args);
        static void IsDequeuedResult(const FunctionCallbackInfo<Value>& args);
        static void IsRouteeResult(const FunctionCallbackInfo<Value>& args);
        static void StartBatching(const FunctionCallbackInfo<Value>& args);
        static void SendRequest(const FunctionCallbackInfo<Value>& args);
        static void getSocket(const FunctionCallbackInfo<Value>& args);
        static void Dispose(const FunctionCallbackInfo<Value>& args);
        static void IsSame(const FunctionCallbackInfo<Value>& args);

    protected:
        static SPA::CUCriticalSection m_cs;

    private:
        SPA::ClientSide::CAsyncServiceHandler *m_ash;
        static Persistent<v8::FunctionTemplate> m_tpl;
    };
}