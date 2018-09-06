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

	private:
		static Persistent<Function> constructor;
		SPA::ClientSide::IClientQueue *m_cq;
	};
}
