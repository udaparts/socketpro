#ifndef __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__
#define __SOCKETPRO_NODEJS_ADAPTER_NJOBJECTS_H__


namespace NJA {

	class NJSocketPool : public node::ObjectWrap {
	public:
		static void Init(Local<Object> exports);

	private:
		NJSocketPool();
		~NJSocketPool();

		NJSocketPool(const NJSocketPool &obj) = delete;
		NJSocketPool& operator=(const NJSocketPool &obj) = delete;

		static void New(const FunctionCallbackInfo<Value>& args);
		static Persistent<Function> constructor;

		static void DisconnectAll(const FunctionCallbackInfo<Value>& args);

		static void getAsyncHandlers(const FunctionCallbackInfo<Value>& args);
		static void getAvg(const FunctionCallbackInfo<Value>& args);
		static void getConenctedSockets(const FunctionCallbackInfo<Value>& args);
		static void getDisconnectedSockets(const FunctionCallbackInfo<Value>& args);
		static void getIdleSockets(const FunctionCallbackInfo<Value>& args);
		static void getLockedSockets(const FunctionCallbackInfo<Value>& args);
		static void getPoolId(const FunctionCallbackInfo<Value>& args);

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

	private:
		unsigned int m_svsId;
		void *m_pPool;
	};
}

#endif
