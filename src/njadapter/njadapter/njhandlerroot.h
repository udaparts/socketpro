#pragma once

namespace NJA {
	class NJHandlerRoot : public node::ObjectWrap {
	public:
		NJHandlerRoot(CAsyncServiceHandler *ash);
		~NJHandlerRoot();

	protected:
		bool IsValid(Isolate* isolate);
		void Release();
		void SetCb();

		static void Init(Local<Object> exports, Local<FunctionTemplate> &tpl);
		static void req_cb(uv_async_t* handle);

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
		static void Dispose(const FunctionCallbackInfo<Value>& args);

	protected:
		static SPA::CUCriticalSection m_cs;

	private:
		CAsyncServiceHandler *m_ash;
		std::deque<ReqCb> m_deqReqCb; //protected by m_cs
		uv_async_t m_typeReq; //protected by m_cs
	};
}