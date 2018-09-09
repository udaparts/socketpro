
#include "stdafx.h"
#include "njodbc.h"

namespace NJA {

	Persistent<Function> NJOdbc::constructor;

	NJOdbc::NJOdbc(COdbc *odbc) : NJHandlerRoot(odbc), m_db(odbc) {

	}

	NJOdbc::~NJOdbc() {
		Release();
	}

	bool NJOdbc::IsValid(Isolate* isolate) {
		if (!m_db) {
			ThrowException(isolate, "Async ODBC handler disposed");
			return false;
		}
		return NJHandlerRoot::IsValid(isolate);
	}

	void NJOdbc::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(ToStr(isolate, "COdbc"));
		tpl->InstanceTemplate()->SetInternalFieldCount(3);

		NJHandlerRoot::Init(exports, tpl);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(ToStr(isolate, "COdbc"), tpl->GetFunction());
	}

	Local<Object> NJOdbc::New(Isolate* isolate, COdbc *ash, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)ash;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJOdbc::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJOdbc *obj = new NJOdbc((COdbc*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `COdbc()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJOdbc::Release() {
		{
			SPA::CAutoLock al(m_cs);
			if (m_db) {
				m_db = nullptr;
			}
		}
		NJHandlerRoot::Release();
	}
}
