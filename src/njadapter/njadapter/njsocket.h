
#pragma once

namespace NJA {

    class NJSocket : public node::ObjectWrap {
    public:
        NJSocket(SPA::ClientSide::CClientSocket *socket);
        NJSocket(const NJSocket &h) = delete;
        ~NJSocket();

    public:
        NJSocket& operator=(const NJSocket &h) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::ClientSide::CClientSocket *ash, bool setCb);

    private:
        void Release();
        bool IsValid(Isolate* isolate);

        static const SPA::INT64 SECRECT_NUM = 0x7fa1d4ff2c45;
        static void New(const FunctionCallbackInfo<Value>& args);
        static void Dispose(const FunctionCallbackInfo<Value>& args);
        static void Close(const FunctionCallbackInfo<Value>& args);
        static void Shutdown(const FunctionCallbackInfo<Value>& args);
        static void setZip(const FunctionCallbackInfo<Value>& args);
        static void getZip(const FunctionCallbackInfo<Value>& args);
        static void setZipLevel(const FunctionCallbackInfo<Value>& args);
        static void getZipLevel(const FunctionCallbackInfo<Value>& args);
        static void setConnTimeout(const FunctionCallbackInfo<Value>& args);
        static void getConnTimeout(const FunctionCallbackInfo<Value>& args);
        static void setRecvTimeout(const FunctionCallbackInfo<Value>& args);
        static void getRecvTimeout(const FunctionCallbackInfo<Value>& args);
        static void setAutoConn(const FunctionCallbackInfo<Value>& args);
        static void getAutoConn(const FunctionCallbackInfo<Value>& args);
        static void getSendable(const FunctionCallbackInfo<Value>& args);
        static void getCert(const FunctionCallbackInfo<Value>& args);
        static void getConnState(const FunctionCallbackInfo<Value>& args);
        static void IgnoreLastRequest(const FunctionCallbackInfo<Value>& args);
        static void getRouteeCount(const FunctionCallbackInfo<Value>& args);
        static void isRouting(const FunctionCallbackInfo<Value>& args);
        static void getRequestsInQueue(const FunctionCallbackInfo<Value>& args);
        static void getCurrReqId(const FunctionCallbackInfo<Value>& args);
        static void getCurrSvsId(const FunctionCallbackInfo<Value>& args);
        static void getServerPingTime(const FunctionCallbackInfo<Value>& args);
        static void getEncryptionMethod(const FunctionCallbackInfo<Value>& args);
        static void getError(const FunctionCallbackInfo<Value>& args);
        static void getConnected(const FunctionCallbackInfo<Value>& args);
        static void getConnContext(const FunctionCallbackInfo<Value>& args);
        static void isRandom(const FunctionCallbackInfo<Value>& args);
        static void getBytesInSendingBuffer(const FunctionCallbackInfo<Value>& args);
        static void getBytesInRecvBuffer(const FunctionCallbackInfo<Value>& args);
        static void getBytesBatched(const FunctionCallbackInfo<Value>& args);
        static void getBytesReceived(const FunctionCallbackInfo<Value>& args);
        static void getBytesSent(const FunctionCallbackInfo<Value>& args);
        static void getUserId(const FunctionCallbackInfo<Value>& args);
        static void getPeerOs(const FunctionCallbackInfo<Value>& args);
        static void getPeerAddr(const FunctionCallbackInfo<Value>& args);
        static void Cancel(const FunctionCallbackInfo<Value>& args);
        static void DoEcho(const FunctionCallbackInfo<Value>& args);
        static void TurnOnZipAtSvr(const FunctionCallbackInfo<Value>& args);
        static void SetZipLevelAtSvr(const FunctionCallbackInfo<Value>& args);
        static void getPush(const FunctionCallbackInfo<Value>& args);
        static void getQueue(const FunctionCallbackInfo<Value>& args);

    private:
        static SPA::CUCriticalSection m_cs;
        static Persistent<Function> constructor;
        SPA::ClientSide::CClientSocket *m_socket;
    };
}
