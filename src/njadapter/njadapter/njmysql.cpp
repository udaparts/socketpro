
#include "stdafx.h"
#include "njmysql.h"

namespace NJA {

	Persistent<Function> NJMysql::constructor;

	NJMysql::NJMysql(CMysql *mysql) : NJHandlerRoot(mysql), m_mysql(mysql) {

	}

	NJMysql::~NJMysql() {
		Release();
	}

	bool NJMysql::IsValid(Isolate* isolate) {
		if (!m_mysql) {
			isolate->ThrowException(Exception::TypeError(ToStr(isolate, "Async MySQL handler disposed")));
			return false;
		}
		return NJHandlerRoot::IsValid(isolate);
	}

	void NJMysql::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(ToStr(isolate, "CMysql"));
		tpl->InstanceTemplate()->SetInternalFieldCount(3);

		NJHandlerRoot::Init(exports, tpl);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(ToStr(isolate, "CMysql"), tpl->GetFunction());
	}

	Local<Object> NJMysql::New(Isolate* isolate, CMysql *ash, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)ash;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJMysql::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJMysql *obj = new NJMysql((CMysql*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `CMysql()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJMysql::Release() {
		{
			SPA::CAutoLock al(m_cs);
			if (m_mysql) {
				m_mysql = nullptr;
			}
		}
		NJHandlerRoot::Release();
	}
}
