
#include "stdafx.h"
#include "njasyncqueue.h"

namespace NJA {

	Persistent<Function> NJAsyncQueue::constructor;

	NJAsyncQueue::NJAsyncQueue(CAsyncQueue *aq) : NJHandlerRoot(aq), m_aq(aq) {

	}

	NJAsyncQueue::~NJAsyncQueue() {
		Release();
	}

	bool NJAsyncQueue::IsValid(Isolate* isolate) {
		if (!m_aq) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Async client queue handler disposed")));
			return false;
		}
		return NJHandlerRoot::IsValid(isolate);
	}

	void NJAsyncQueue::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CAsyncQueue"));
		tpl->InstanceTemplate()->SetInternalFieldCount(3);

		NJHandlerRoot::Init(exports, tpl);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CAsyncQueue"), tpl->GetFunction());
	}

	Local<Object> NJAsyncQueue::New(Isolate* isolate, CAsyncQueue *ash, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)ash;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJAsyncQueue::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJAsyncQueue *obj = new NJAsyncQueue((CAsyncQueue*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `CAsyncQueue()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJAsyncQueue::Release() {
		{
			SPA::CAutoLock al(m_cs);
			if (m_aq) {
				m_aq = nullptr;
			}
		}
		NJHandlerRoot::Release();
	}

	void NJAsyncQueue::getDequeueBatchSize(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int size = obj->m_aq->GetDequeueBatchSize();
			args.GetReturnValue().Set(Number::New(isolate, size));
		}
	}

	void NJAsyncQueue::getEnqueueNotified(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool notify = obj->m_aq->GetEnqueueNotified();
			args.GetReturnValue().Set(Boolean::New(isolate, notify));
		}
	}
}
