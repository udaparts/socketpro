
#include "stdafx.h"
#include "njasyncqueue.h"

namespace NJA {

	Persistent<Function> NJAsyncQueue::constructor;

	NJAsyncQueue::NJAsyncQueue(CAQueue *aq) : NJHandlerRoot(aq), m_aq(aq) {

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

		NODE_SET_PROTOTYPE_METHOD(tpl, "getDeqBatchSize", getDequeueBatchSize);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getEnqNotified", getEnqueueNotified);
		NODE_SET_PROTOTYPE_METHOD(tpl, "GetKeys", GetKeys);
		NODE_SET_PROTOTYPE_METHOD(tpl, "StartTrans", StartQueueTrans);
		NODE_SET_PROTOTYPE_METHOD(tpl, "EndTrans", EndQueueTrans);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Close", CloseQueue);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Flush", FlushQueue);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Dequeue", Dequeue);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Enqueue", Enqueue);
		NODE_SET_PROTOTYPE_METHOD(tpl, "EnqueueBatch", EnqueueBatch);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CAsyncQueue"), tpl->GetFunction());
	}

	Local<Object> NJAsyncQueue::New(Isolate* isolate, CAQueue *ash, bool setCb) {
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
				NJAsyncQueue *obj = new NJAsyncQueue((CAQueue*)ptr);
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

	void NJAsyncQueue::GetKeys(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			Local<Value> argv[] = { args[0], args[1] };
			SPA::UINT64 index = obj->m_aq->GetKeys(isolate, 2, argv);
			if (index) {
				args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
			}
		}
	}

	std::string NJAsyncQueue::GetKey(Isolate* isolate, Local<Value> jsKey) {
		if (!jsKey->IsString()) {
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A valid key string required to find a queue file at server side")));
			return "";
		}
		String::Utf8Value str(jsKey);
		std::string s(*str);
		if (!s.size()) {
			isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A valid key string required to find a queue file at server side")));
		}
		return s;
	}

	void NJAsyncQueue::StartQueueTrans(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			std::string key = GetKey(isolate, args[0]);
			if (!key.size())
				return;
			Local<Value> argv[] = { args[1], args[2] };
			SPA::UINT64 index = obj->m_aq->StartQueueTrans(isolate, 2, argv, key.c_str());
			if (index) {
				args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
			}
		}
	}

	void NJAsyncQueue::EndQueueTrans(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool rollback = false;
			auto p0 = args[0];
			if (p0->IsBoolean())
				rollback = p0->BooleanValue();
			else if (!p0->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Boolean value expected for rollback")));
				return;
			}
			Local<Value> argv[] = { args[1], args[2] };
			SPA::UINT64 index = obj->m_aq->EndQueueTrans(isolate, 2, argv, rollback);
			if (index) {
				args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
			}
		}
	}

	void NJAsyncQueue::CloseQueue(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			std::string key = GetKey(isolate, args[0]);
			if (!key.size())
				return;
			Local<Value> argv[] = { args[1], args[2] };
			auto p = args[3];
			bool perm = false;
			if (p->IsBoolean())
				perm = p->BooleanValue();
			else if (!p->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Boolean value expected for permanent delete")));
				return;
			}
			SPA::UINT64 index = obj->m_aq->CloseQueue(isolate, 2, argv, key.c_str(), perm);
			if (index) {
				args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
			}
		}
	}

	void NJAsyncQueue::FlushQueue(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			std::string key = GetKey(isolate, args[0]);
			if (!key.size())
				return;
			Local<Value> argv[] = { args[1], args[2] };
			auto p = args[3];
			int option = 0;
			if (p->IsInt32())
				option = p->Int32Value();
			else if (!p->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Integer value expected for flush option")));
				return;
			}
			if (option < 0 || option > SPA::oDiskCommitted) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Bad option value")));
				return;
			}
			SPA::UINT64 index = obj->m_aq->FlushQueue(isolate, 2, argv, key.c_str(), (SPA::tagOptimistic)option);
			if (index) {
				args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
			}
		}
	}

	void NJAsyncQueue::Dequeue(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJAsyncQueue* obj = ObjectWrap::Unwrap<NJAsyncQueue>(args.Holder());
		if (obj->IsValid(isolate)) {
			std::string key = GetKey(isolate, args[0]);
			if (!key.size())
				return;
			Local<Value> argv[] = { args[1], args[2] };
			auto p = args[3];
			unsigned int timeout = 0;
			if (p->IsUint32())
				timeout = p->Uint32Value();
			else if (!p->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Unsigned int value expected for dequeue timeout in millsecond")));
				return;
			}
			SPA::UINT64 index = obj->m_aq->Dequeue(isolate, 2, argv, key.c_str(), timeout);
			if (index) {
				args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
			}
		}
	}

	void NJAsyncQueue::Enqueue(const FunctionCallbackInfo<Value>& args) {

	}

	void NJAsyncQueue::EnqueueBatch(const FunctionCallbackInfo<Value>& args) {

	}
}
