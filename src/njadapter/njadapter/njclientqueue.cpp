#include "stdafx.h"
#include "njclientqueue.h"

namespace NJA {

	Persistent<Function> NJClientQueue::constructor;

	NJClientQueue::NJClientQueue(SPA::ClientSide::IClientQueue *cq) : m_cq(cq) {
	}

	NJClientQueue::~NJClientQueue() {
	}

	void NJClientQueue::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CClientQueue"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CClientQueue"), tpl->GetFunction());
	}

	Local<Object> NJClientQueue::New(Isolate* isolate, SPA::ClientSide::IClientQueue *cq, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)cq;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJClientQueue::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJClientQueue *obj = new NJClientQueue((SPA::ClientSide::IClientQueue*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `CClientQueue()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}
}
