
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

	private:
		static SPA::CUCriticalSection m_cs;
		static Persistent<Function> constructor;
		SPA::ClientSide::CClientSocket *m_socket;
	};
}
