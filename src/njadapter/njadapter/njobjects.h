#ifndef __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__
#define __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__

#include "asynchandler.h"
#include "../../../include/async_odbc.h"
#include "../../../include/aqhandler.h"
#include "../../../include/streamingfile.h"

namespace NJA {

	class NJSocketPool : public node::ObjectWrap {
	public:
		static void Init(Local<Object> exports);

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
		bool IsValid(Isolate* isolate);

		static void New(const FunctionCallbackInfo<Value>& args);
		static Persistent<Function> constructor;

		static void Dispose(const FunctionCallbackInfo<Value>& args);
		static void DisconnectAll(const FunctionCallbackInfo<Value>& args);

		static void getAsyncHandlers(const FunctionCallbackInfo<Value>& args);
		static void getAvg(const FunctionCallbackInfo<Value>& args);
		static void getConenctedSockets(const FunctionCallbackInfo<Value>& args);
		static void getDisconnectedSockets(const FunctionCallbackInfo<Value>& args);
		static void getIdleSockets(const FunctionCallbackInfo<Value>& args);
		static void getLockedSockets(const FunctionCallbackInfo<Value>& args);
		static void getPoolId(const FunctionCallbackInfo<Value>& args);
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

	private:
		unsigned int SvsId;
		union {
			CSocketPool<CAsyncHandler> *Handler; //
			CSocketPool<CAsyncDBHandler<0>> *Db; //SQL streaming
			CSocketPool<COdbc> *Odbc; //ODBC streaming
			CSocketPool<CStreamingFile> *File; //File streaming
			CSocketPool<CAsyncQueue> *Queue; //Persistent queue
		};
		Isolate* m_isolate;
		Local<Function> m_rr;
		Local<Function> m_mt;
		Local<Function> m_brp;
		Local<Function> m_se;
		Local<Function> m_ap;
		Local<Function> m_evPool;
		uv_async_t m_asyncType;
		CUCriticalSection m_cs;
		std::deque<PoolEvent> m_deqPoolEvent;
		std::deque<CClientSocket*> m_deqSocket;
		int m_errSSL;
		std::string m_errMsg;
		std::wstring m_defaultDb;
	};
}

#endif
