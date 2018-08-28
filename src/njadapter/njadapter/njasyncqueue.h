
#pragma once


#include "../../../include/aqhandler.h"
#include "njhandlerroot.h"

namespace NJA {
	class NJAsyncQueue : public NJHandlerRoot {
	public:
		NJAsyncQueue(SPA::ClientSide::CAsyncQueue *aq);
		NJAsyncQueue(const NJAsyncQueue &h) = delete;
		~NJAsyncQueue();

	public:
		NJAsyncQueue& operator=(const NJAsyncQueue &h) = delete;
		static void Init(Local<Object> exports);
		static Local<Object> New(Isolate* isolate, SPA::ClientSide::CAsyncQueue *ash, bool setCb);

	private:
		void Release();
		bool IsValid(Isolate* isolate);

		static const SPA::INT64 SECRECT_NUM = 0x7fa1b4ff23a5;
		static void New(const FunctionCallbackInfo<Value>& args);
		static void getDequeueBatchSize(const FunctionCallbackInfo<Value>& args);
		static void getEnqueueNotified(const FunctionCallbackInfo<Value>& args);

	private:
		static Persistent<Function> constructor;
		SPA::ClientSide::CAsyncQueue *m_aq;
	};
}
