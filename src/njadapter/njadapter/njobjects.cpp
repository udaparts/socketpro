#include "stdafx.h"
#include "njobjects.h"

namespace NJA {
	using v8::Context;

	Persistent<Function> NJAObjects::constructor;

	NJAObjects::NJAObjects(double value) : value_(value) {
		uv_loop_t *main_loop = uv_default_loop();
		/*
		uv_async_t context;
		memset(&context, 0, sizeof(uv_async_t));

		int err = uv_async_init(main_loop, &context, nullptr);
		*/
		main_loop = nullptr;
	}

	NJAObjects::~NJAObjects() {
		
	}

	void NJAObjects::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "NJAObjects"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Prototype
		NODE_SET_PROTOTYPE_METHOD(tpl, "plusOne", PlusOne);
		
		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "NJAObjects"), tpl->GetFunction());
	}

	void NJAObjects::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		
		if (args.IsConstructCall()) {
			// Invoked as constructor: `new NJAObjects(...)`
			double value = args[0]->IsNumber() ? args[0]->NumberValue() : 0;
			NJAObjects* obj = new NJAObjects(value);
			obj->Wrap(args.This());
			Local<Number> num = Number::New(isolate, 1.24);
			args.This()->Set(String::NewFromUtf8(isolate, "msg"), num);
			args.GetReturnValue().Set(args.This());
		}
		else {
			// Invoked as plain function `NJAObjects(...)`, turn into construct call.
			Local<Value> argv[] = { args[0] };
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 1, argv).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJAObjects::PlusOne(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		NJAObjects* obj = ObjectWrap::Unwrap<NJAObjects>(args.Holder());
		obj->value_ += 1;

		args.GetReturnValue().Set(Number::New(isolate, obj->value_));
	}

	void NJAObjects::At_Exit(void *arg) {

	}
}
