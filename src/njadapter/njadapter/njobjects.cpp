#include "stdafx.h"
#include "njobjects.h"

namespace NJA {
	using v8::Context;

	Persistent<Function> MyObject::constructor;

	MyObject::MyObject(double value) : value_(value) {
		uv_loop_t *main_loop = uv_default_loop();
		/*
		uv_async_t context;
		memset(&context, 0, sizeof(uv_async_t));

		int err = uv_async_init(main_loop, &context, nullptr);
		*/
		main_loop = nullptr;
	}

	MyObject::~MyObject() {
		
	}

	void MyObject::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "MyObject"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Prototype
		NODE_SET_PROTOTYPE_METHOD(tpl, "plusOne", PlusOne);
		
		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "MyObject"), tpl->GetFunction());
	}

	void MyObject::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		
		if (args.IsConstructCall()) {
			// Invoked as constructor: `new MyObject(...)`
			double value = args[0]->IsNumber() ? args[0]->NumberValue() : 0;
			MyObject* obj = new MyObject(value);
			obj->Wrap(args.This());
			Local<Number> num = Number::New(isolate, 1.24);
			args.This()->Set(String::NewFromUtf8(isolate, "msg"), num);
			args.GetReturnValue().Set(args.This());
		}
		else {
			// Invoked as plain function `MyObject(...)`, turn into construct call.
			Local<Value> argv[] = { args[0] };
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 1, argv).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void MyObject::PlusOne(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		MyObject* obj = ObjectWrap::Unwrap<MyObject>(args.Holder());
		obj->value_ += 1;

		args.GetReturnValue().Set(Number::New(isolate, obj->value_));
	}

	void MyObject::At_Exit(void *arg) {

	}
}