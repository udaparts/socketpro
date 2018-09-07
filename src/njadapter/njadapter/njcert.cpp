#include "stdafx.h"
#include "njcert.h"

namespace NJA {

	Persistent<Function> NJCert::constructor;

	NJCert::NJCert(SPA::IUcert *c) : m_c(c) {
	}

	NJCert::~NJCert() {
	}

	void NJCert::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(ToStr(isolate, "CCert"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(ToStr(isolate, "CCert"), tpl->GetFunction());
	}

	Local<Object> NJCert::New(Isolate* isolate, SPA::IUcert *c, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)c;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJCert::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJCert *obj = new NJCert((SPA::IUcert*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `CCert()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}
}
