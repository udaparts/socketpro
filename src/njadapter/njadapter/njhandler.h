#pragma once

namespace NJA {
	class NJHandler : public node::ObjectWrap {
	public:
		NJHandler(CAsyncServiceHandler *ash);
		NJHandler(const NJHandler &h)=delete;
		~NJHandler();

	public:
		NJHandler& operator=(const NJHandler &h)=delete;
		static void Init(Local<Object> exports);
		bool IsValid(Isolate* isolate);

	private:
		static void New(const FunctionCallbackInfo<Value>& args);
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

	private:
		static Persistent<Function> constructor;
		CAsyncServiceHandler *m_ash;
	};
}
