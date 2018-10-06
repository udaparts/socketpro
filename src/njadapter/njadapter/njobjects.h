#ifndef __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__
#define __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__

#include "../../../include/generalcache.h"
#include <node.h>
#include <node_object_wrap.h>

#include "njhandler.h"
#include "njqueue.h"
#include "njfile.h"
#include "njasyncqueue.h"
#include "njsqlite.h"
#include "njsocket.h"

namespace NJA {

    typedef SPA::ClientSide::CCachedBaseHandler<0> CAsyncHandler;

    enum tagSocketEvent {
        seAllProcessed = 0,
        seResultReturned,
        seBaseRequestProcessed,
        seServerException,
        seChatEnter,
        seChatExit,
        sePublish,
        sePublishEx,
        sePostUserMessage,
        sePostUserMessageEx,
    };

    struct SocketEvent {
        tagSocketEvent Se;
        SPA::CUQueue *QData;
    };

    class NJSocketPool : public node::ObjectWrap {
    public:
        static void Init(Local<Object> exports);
        bool IsValid(Isolate* isolate);

    private:
        NJSocketPool(const wchar_t* defaultDb, unsigned int id, bool slave);
        ~NJSocketPool();

        NJSocketPool(const NJSocketPool &obj) = delete;
        NJSocketPool& operator=(const NJSocketPool &obj) = delete;

        struct PoolEvent {
            SPA::ClientSide::tagSocketPoolEvent Spe;
            SPA::ClientSide::PAsyncServiceHandler Handler;
        };

        bool DoAuthentication(SPA::IUcert *cert);
        void SendPoolEvent(SPA::ClientSide::tagSocketPoolEvent spe, SPA::ClientSide::PAsyncServiceHandler handler);
        void Release();

        static void New(const FunctionCallbackInfo<Value>& args);
        static Persistent<Function> constructor;

        static void Dispose(const FunctionCallbackInfo<Value>& args);
        static void DisconnectAll(const FunctionCallbackInfo<Value>& args);

        static void getAsyncHandlers(const FunctionCallbackInfo<Value>& args);
        static void getAvg(const FunctionCallbackInfo<Value>& args);
        static void getConnectedSockets(const FunctionCallbackInfo<Value>& args);
        static void getDisconnectedSockets(const FunctionCallbackInfo<Value>& args);
        static void getIdleSockets(const FunctionCallbackInfo<Value>& args);
        static void getLockedSockets(const FunctionCallbackInfo<Value>& args);
        static void getPoolId(const FunctionCallbackInfo<Value>& args);
        static void getSvsId(const FunctionCallbackInfo<Value>& args);
        static void getError(const FunctionCallbackInfo<Value>& args);
        static void getQueueAutoMerge(const FunctionCallbackInfo<Value>& args);
        static void setQueueAutoMerge(const FunctionCallbackInfo<Value>& args);
        static void getQueueName(const FunctionCallbackInfo<Value>& args);
        static void setQueueName(const FunctionCallbackInfo<Value>& args);
        static void getQueues(const FunctionCallbackInfo<Value>& args);
        static void getSockets(const FunctionCallbackInfo<Value>& args);
        static void getSocketsPerThread(const FunctionCallbackInfo<Value>& args);
        static void getStarted(const FunctionCallbackInfo<Value>& args);
        static void getThreadsCreated(const FunctionCallbackInfo<Value>& args);
        static void getCache(const FunctionCallbackInfo<Value>& args);

        static void Lock(const FunctionCallbackInfo<Value>& args);
        static void Seek(const FunctionCallbackInfo<Value>& args);
        static void SeekByQueue(const FunctionCallbackInfo<Value>& args);
        static void ShutdownPool(const FunctionCallbackInfo<Value>& args);
        static void StartSocketPool(const FunctionCallbackInfo<Value>& args);
        static void Unlock(const FunctionCallbackInfo<Value>& args);
        static void newSlave(const FunctionCallbackInfo<Value>& args);

        static void setPoolEvent(const FunctionCallbackInfo<Value>& args);

        static bool To(Isolate* isolate, const Local<Object>& obj, SPA::ClientSide::CConnectionContext &cc);

        static void setResultReturned(const FunctionCallbackInfo<Value>& args);
        static void setAllProcessed(const FunctionCallbackInfo<Value>& args);
        static void setServerException(const FunctionCallbackInfo<Value>& args);
        static void setBaseRequestProcessed(const FunctionCallbackInfo<Value>& args);
        static void setPush(const FunctionCallbackInfo<Value>& args);

        static void async_cs_cb(uv_async_t* handle); //socket events
        static void async_cb(uv_async_t* handle); //pool events

    private:
        unsigned int SvsId;

        union {
            SPA::ClientSide::CSocketPool<CAsyncHandler> *Handler;
            SPA::ClientSide::CSocketPool<CNjDb> *Db;
            SPA::ClientSide::CSocketPool<CSFile> *File;
            SPA::ClientSide::CSocketPool<CAQueue> *Queue;
        };
        SPA::CUCriticalSection m_cs;
        Persistent<Function> m_rr; //OnResultReturned protected by m_cs
        Persistent<Function> m_brp; //OnBaseRequestProcessed by m_cs
        Persistent<Function> m_se; //OnServerException by m_cs
        Persistent<Function> m_ap; //OnAllRequestsProcessed by m_cs
        Persistent<Function> m_push; //OnAllRequestsProcessed by m_cs
        Persistent<Function> m_evPool; //OnSocketPoolEvent by m_cs
        uv_async_t m_asyncType;
        std::deque<PoolEvent> m_deqPoolEvent; //Protected by m_cs;

        uv_async_t m_csType;
        std::deque<SocketEvent> m_deqSocketEvent; //Protected by m_cs;

        int m_errSSL;
        std::string m_errMsg;
        std::wstring m_defaultDb;
        friend class NJHandler;
        friend class SPA::ClientSide::CClientSocket;
    };
}

#endif
