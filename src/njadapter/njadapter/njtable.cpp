#include "stdafx.h"
#include "njtable.h"

namespace NJA {

	Persistent<Function> NJTable::constructor;

	NJTable::NJTable(SPA::CTable *tbl) : m_table(tbl) {
	}

	NJTable::~NJTable() {
		Release();
	}

	bool NJTable::IsValid(Isolate* isolate) {
		if (!m_table) {
			ThrowException(isolate, "Table handler disposed");
			return false;
		}
		return true;
	}

	void NJTable::Release() {
		if (m_table) {
			delete m_table;
			m_table = nullptr;
		}
	}

	void NJTable::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(ToStr(isolate, "NJTable"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);


		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(ToStr(isolate, "NJTable"), tpl->GetFunction());
	}

	Local<Object> NJTable::New(Isolate* isolate, SPA::CTable *tbl, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)tbl;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJTable::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJTable *obj = new NJTable((SPA::CTable*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `NJTable()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

}