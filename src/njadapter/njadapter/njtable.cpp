#include "stdafx.h"
#include "njtable.h"

namespace NJA {

	Persistent<Function> NJTable::constructor;
	Persistent<v8::FunctionTemplate> NJTable::m_tpl;

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

		//methods
		NODE_SET_PROTOTYPE_METHOD(tpl, "FindOrdinal", FindOrdinal);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Find", Find);
		NODE_SET_PROTOTYPE_METHOD(tpl, "FindNull", FindNull);
		NODE_SET_PROTOTYPE_METHOD(tpl, "In", In);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Between", Between);
		NODE_SET_PROTOTYPE_METHOD(tpl, "NotIn", NotIn);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Append", Append);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Sort", Sort);

		//properties
		NODE_SET_PROTOTYPE_METHOD(tpl, "getData", getData);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getKeys", getKeys);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getMeta", getMeta);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(ToStr(isolate, "NJTable"), tpl->GetFunction());
		m_tpl.Reset(isolate, tpl);
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

	void NJTable::Dispose(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		obj->Release();
	}

	void NJTable::getMeta(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			Local<Array> jsMeta = ToMeta(isolate, obj->m_table->GetMeta());
			args.GetReturnValue().Set(jsMeta);
		}
	}

	void NJTable::getData(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int index = 0;
			Local<Array> jsRows = Array::New(isolate);
			const CDataMatrix &matrix = obj->m_table->GetDataMatrix();
			for (auto it = matrix.begin(), end = matrix.end(); it != end; ++it, ++index) {
				const CPRow &pr = *it;
				Local<Array> jsR = Array::New(isolate);
				unsigned int n = 0;
				const CRow &r = *pr;
				for (auto rit = r.begin(), rend = r.end(); rit != rend; ++rit, ++n) {
					jsR->Set(n, From(isolate, *rit));
				}
				jsRows->Set(index, jsR);
			}
			args.GetReturnValue().Set(jsRows);
		}
	}

	void NJTable::getKeys(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			Local<Array> jsMeta = ToMeta(isolate, obj->m_table->GetKeys());
			args.GetReturnValue().Set(jsMeta);
		}
	}

	void NJTable::Append(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p = args[0];
			if (p->IsObject()) {
				Local<Object> jsObj = p->ToObject();
				if (IsTable(jsObj)) {
					NJTable* table = ObjectWrap::Unwrap<NJTable>(jsObj);
					if (table->IsValid(isolate)) {
						int res = obj->m_table->Append(*table->m_table);
						args.GetReturnValue().Set(Int32::New(isolate, res));
						return;
					}
				}
			}
			ThrowException(isolate, "A valid table object expected");
		}
	}

	void NJTable::Sort(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->Uint32Value()) {
				ThrowException(isolate, "A column ordinal number expected");
				return;
			}
			unsigned int ordinal = p0->Uint32Value();
			auto p1 = args[1];
			bool desc = false;
			if (p1->IsBoolean()) {
				desc = p1->BooleanValue();
			}
			else if (!p1->IsNullOrUndefined()) {
				ThrowException(isolate, "A boolean value expected");
				return;
			}
			int res = obj->m_table->Sort(ordinal, desc);
			args.GetReturnValue().Set(Int32::New(isolate, res));
		}
	}

	void NJTable::FindOrdinal(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p = args[0];
			if (p->IsString()) {
				String::Utf8Value str(p);
				int res = (int)obj->m_table->FindOrdinal(*str);
				args.GetReturnValue().Set(Int32::New(isolate, res));
			}
			else {
				ThrowException(isolate, "A column name expected");
			}
		}
	}

	void NJTable::Find(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->Uint32Value()) {
				ThrowException(isolate, "A column ordinal number expected");
				return;
			}
			unsigned int ordinal = p0->Uint32Value();
			auto p1 = args[1];
			if (!p1->IsUint32()) {
				ThrowException(isolate, "Operation value expected");
				return;
			}
			unsigned int data = p1->Uint32Value();
			if (data > SPA::CTable::is_null) {
				ThrowException(isolate, "Bad operation value");
				return;
			}
			auto p2 = args[2];
			CComVariant vt;
			if (!From(p2, "", vt)) {
				ThrowException(isolate, "Unsupported data type value");
				return;
			}
			auto p3 = args[3];
			bool copy = false;
			if (p3->IsBoolean()) {
				copy = p3->BooleanValue();
			}
			else if (!p3->IsNullOrUndefined()) {
				ThrowException(isolate, "A boolean value expected");
				return;
			}
			SPA::CTable *pTable = new SPA::CTable;
			int res = obj->m_table->Find(ordinal, (CTable::Operator)data, vt, *pTable, copy);
			if (res <= 0) {
				delete pTable;
				Local<Value> jsRes = Int32::New(isolate, res);
				args.GetReturnValue().Set(jsRes);
			}
			else {
				Local<Object> jsTable = NJTable::New(isolate, pTable, true);
				args.GetReturnValue().Set(jsTable);
			}
		}
	}

	void NJTable::FindNull(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->Uint32Value()) {
				ThrowException(isolate, "A column ordinal number expected");
				return;
			}
			unsigned int ordinal = p0->Uint32Value();
			auto p1 = args[1];
			bool copy = false;
			if (p1->IsBoolean()) {
				copy = p1->BooleanValue();
			}
			else if (!p1->IsNullOrUndefined()) {
				ThrowException(isolate, "A boolean value expected");
				return;
			}
			SPA::CTable *pTable = new SPA::CTable;
			int res = obj->m_table->FindNull(ordinal, *pTable, copy);
			if (res <= 0) {
				delete pTable;
				Local<Value> jsRes = Int32::New(isolate, res);
				args.GetReturnValue().Set(jsRes);
			}
			else {
				Local<Object> jsTable = NJTable::New(isolate, pTable, true);
				args.GetReturnValue().Set(jsTable);
			}
		}
	}

	void NJTable::In(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->Uint32Value()) {
				ThrowException(isolate, "A column ordinal number expected");
				return;
			}
			unsigned int ordinal = p0->Uint32Value();
			SPA::UDB::CDBVariantArray v;
			auto p1 = args[1];
			if (!ToArray(isolate, p1, v)) {
				return;
			}
			auto p2 = args[2];
			bool copy = false;
			if (p2->IsBoolean()) {
				copy = p2->BooleanValue();
			}
			else if (!p2->IsNullOrUndefined()) {
				ThrowException(isolate, "A boolean value expected");
				return;
			}
			SPA::CTable *pTable = new SPA::CTable;
			int res = obj->m_table->In(ordinal, v, *pTable, copy);
			if (res <= 0) {
				delete pTable;
				Local<Value> jsRes = Int32::New(isolate, res);
				args.GetReturnValue().Set(jsRes);
			}
			else {
				Local<Object> jsTable = NJTable::New(isolate, pTable, true);
				args.GetReturnValue().Set(jsTable);
			}
		}
	}

	void NJTable::Between(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->Uint32Value()) {
				ThrowException(isolate, "A column ordinal number expected");
				return;
			}
			unsigned int ordinal = p0->Uint32Value();

			auto p1 = args[1];
			CComVariant vt0;
			if (!From(p1, "", vt0)) {
				ThrowException(isolate, "Unsupported data type value");
				return;
			}

			auto p2 = args[2];
			CComVariant vt1;
			if (!From(p2, "", vt1)) {
				ThrowException(isolate, "Unsupported data type value");
				return;
			}
			auto p3 = args[3];
			bool copy = false;
			if (p3->IsBoolean()) {
				copy = p3->BooleanValue();
			}
			else if (!p3->IsNullOrUndefined()) {
				ThrowException(isolate, "A boolean value expected");
				return;
			}
			SPA::CTable *pTable = new SPA::CTable;
			int res = obj->m_table->Between(ordinal, vt0, vt1, *pTable, copy);
			if (res <= 0) {
				delete pTable;
				Local<Value> jsRes = Int32::New(isolate, res);
				args.GetReturnValue().Set(jsRes);
			}
			else {
				Local<Object> jsTable = NJTable::New(isolate, pTable, true);
				args.GetReturnValue().Set(jsTable);
			}
		}
	}

	void NJTable::NotIn(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJTable* obj = ObjectWrap::Unwrap<NJTable>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->Uint32Value()) {
				ThrowException(isolate, "A column ordinal number expected");
				return;
			}
			unsigned int ordinal = p0->Uint32Value();
			SPA::UDB::CDBVariantArray v;
			auto p1 = args[1];
			if (!ToArray(isolate, p1, v)) {
				return;
			}
			auto p2 = args[2];
			bool copy = false;
			if (p2->IsBoolean()) {
				copy = p2->BooleanValue();
			}
			else if (!p2->IsNullOrUndefined()) {
				ThrowException(isolate, "A boolean value expected");
				return;
			}
			SPA::CTable *pTable = new SPA::CTable;
			int res = obj->m_table->NotIn(ordinal, v, *pTable, copy);
			if (res <= 0) {
				delete pTable;
				Local<Value> jsRes = Int32::New(isolate, res);
				args.GetReturnValue().Set(jsRes);
			}
			else {
				Local<Object> jsTable = NJTable::New(isolate, pTable, true);
				args.GetReturnValue().Set(jsTable);
			}
		}
	}

	bool NJTable::IsTable(Local<Object> obj) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope handleScope(isolate); //required for Node 4.x or later
		Local<FunctionTemplate> cb = Local<FunctionTemplate>::New(isolate, m_tpl);
		return cb->HasInstance(obj);
	}
}