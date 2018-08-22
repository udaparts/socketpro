#ifndef __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__
#define __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__

#include "asynchandler.h"

namespace NJA {

	class NJSocketPool : public node::ObjectWrap {
	public:
		static void Init(Local<Object> exports);
		bool IsValid(Isolate* isolate);

	private:
		NJSocketPool(const wchar_t* defaultDb, unsigned int id, bool autoConn = true, unsigned int recvTimeout = DEFAULT_RECV_TIMEOUT, unsigned int connTimeout = DEFAULT_CONN_TIMEOUT);
		~NJSocketPool();

		NJSocketPool(const NJSocketPool &obj) = delete;
		NJSocketPool& operator=(const NJSocketPool &obj) = delete;

		struct PoolEvent {
			tagSocketPoolEvent Spe;
			CAsyncHandler *Handler;
		};

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
		static void getErrCode(const FunctionCallbackInfo<Value>& args);
		static void getErrMsg(const FunctionCallbackInfo<Value>& args);

		static void getQueueAutoMerge(const FunctionCallbackInfo<Value>& args);
		static void setQueueAutoMerge(const FunctionCallbackInfo<Value>& args);
		static void getQueueName(const FunctionCallbackInfo<Value>& args);
		static void setQueueName(const FunctionCallbackInfo<Value>& args);
		static void getQueues(const FunctionCallbackInfo<Value>& args);
		static void getSockets(const FunctionCallbackInfo<Value>& args);
		static void getSocketsPerThread(const FunctionCallbackInfo<Value>& args);
		static void getStarted(const FunctionCallbackInfo<Value>& args);
		static void getThreadsCreated(const FunctionCallbackInfo<Value>& args);

		static void Lock(const FunctionCallbackInfo<Value>& args);
		static void Seek(const FunctionCallbackInfo<Value>& args);
		static void SeekByQueue(const FunctionCallbackInfo<Value>& args);
		static void ShutdownPool(const FunctionCallbackInfo<Value>& args);
		static void StartSocketPool(const FunctionCallbackInfo<Value>& args);
		static void Unlock(const FunctionCallbackInfo<Value>& args);

		static void setPoolEvent(const FunctionCallbackInfo<Value>& args);
		static void async_cb(uv_async_t* handle);
		static bool To(Isolate* isolate, const Local<Object>& obj, SPA::ClientSide::CConnectionContext &cc);


	private:
		unsigned int SvsId;
		union {
			CSocketPool<CAsyncHandler> *Handler;
			CSocketPool<CSqlite> *Sqlite;
			CSocketPool<CMySQL> *Mysql;
			CSocketPool<COdbc> *Odbc;
			CSocketPool<CStreamingFile> *File; //File streaming
			CSocketPool<CAsyncQueue> *Queue; //Persistent queue
		};
		CUCriticalSection m_cs;
		Persistent<Function> m_rr; //OnResultReturned protected by m_cs
		Persistent<Function> m_brp; //OnBaseRequestProcessed by m_cs
		Persistent<Function> m_se; //OnServerException by m_cs
		Persistent<Function> m_ap; //OnAllRequestsProcessed by m_cs
		Persistent<Function> m_evPool; //OnSocketPoolEvent by m_cs
		uv_async_t m_asyncType;
		std::deque<PoolEvent> m_deqPoolEvent;
		int m_errSSL;
		std::string m_errMsg;
		std::wstring m_defaultDb;
		friend class NJHandler;
	};
}

#endif
