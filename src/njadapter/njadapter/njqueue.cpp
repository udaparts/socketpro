#include "stdafx.h"
#include "njqueue.h"

namespace NJA {
	using SPA::CScopeUQueue;
	Persistent<Function> NJQueue::constructor;

	NJQueue::NJQueue(unsigned int initialSize, unsigned int blockSize)
		: m_initSize(initialSize), m_blockSize(blockSize),
		m_Buffer(CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), initialSize, blockSize)) {
	}

	NJQueue::~NJQueue() {
		CScopeUQueue::Unlock(m_Buffer);
	}

	void NJQueue::Release() {
		CScopeUQueue::Unlock(m_Buffer);
		m_Buffer = nullptr;
	}

	void NJQueue::Ensure() {
		if (!m_Buffer) {
			m_Buffer = CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), m_initSize, m_blockSize);
		}
	}

	void NJQueue::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CUQueue"));
		tpl->InstanceTemplate()->SetInternalFieldCount(3);

		// Prototype
		NODE_SET_PROTOTYPE_METHOD(tpl, "Discard", Discard);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Empty", Empty);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getSize", getSize);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setSize", setSize);

		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadBool", LoadBoolean);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadByte", LoadByte);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadAChar", LoadAChar);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadShort", LoadShort);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadInt", LoadInt);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadLong", LoadLong);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadFloat", LoadFloat);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadDouble", LoadDouble);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadBytes", LoadBytes);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadAString", LoadAString);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadString", LoadString);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadUShort", LoadUShort);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadUInt", LoadUInt);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadULong", LoadULong);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadDecimal", LoadDecimal);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadDate", LoadDate);
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadUUID", LoadUUID);
		NODE_SET_PROTOTYPE_METHOD(tpl, "PopBytes", PopBytes);

		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveBool", SaveBoolean);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveByte", SaveByte);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveAChar", SaveAChar);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveShort", SaveShort);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveInt", SaveInt);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveLong", SaveLong);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveFloat", SaveFloat);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveDouble", SaveDouble);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveBytes", SaveBytes);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveAString", SaveAString);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveString", SaveString);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveUShort", SaveUShort);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveUInt", SaveUInt);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveULong", SaveULong);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveDecimal", SaveDecimal);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveDate", SaveDate);
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveUUID", SaveUUID);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CUQueue"), tpl->GetFunction());
	}

	void NJQueue::Discard(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int ret = 0;
		if (obj->m_Buffer && args[0]->IsUint32()) {
			ret = obj->m_Buffer->Pop((unsigned int)args[0]->Uint32Value());
		}
		args.GetReturnValue().Set(Number::New(args.GetIsolate(), ret));
	}

	void NJQueue::setSize(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int size = 0;
		if (args.Length() && args[0]->IsUint32())
			size = args[0]->Uint32Value();
		else {
			args.GetIsolate()->ThrowException(Exception::TypeError(String::NewFromUtf8(args.GetIsolate(), "Unsigned int expected")));
			return;
		}
		unsigned int ret = 0;
		if (obj->m_Buffer) {
			obj->m_Buffer->SetSize(size);
			ret = obj->m_Buffer->GetSize();
		}
		args.GetReturnValue().Set(Number::New(args.GetIsolate(), ret));
	}

	void NJQueue::getSize(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int ret = 0;
		if (obj->m_Buffer)
			ret = obj->m_Buffer->GetSize();
		args.GetReturnValue().Set(Number::New(args.GetIsolate(), ret));
	}

	void NJQueue::LoadBoolean(const FunctionCallbackInfo<Value>& args) {
		bool b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Boolean::New(isolate, b));
		}
	}

	void NJQueue::LoadByte(const FunctionCallbackInfo<Value>& args) {
		unsigned char b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Uint32::New(isolate, b));
		}
	}

	void NJQueue::LoadAChar(const FunctionCallbackInfo<Value>& args) {
		char b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Int32::New(isolate, b));
		}
	}

	void NJQueue::LoadShort(const FunctionCallbackInfo<Value>& args) {
		short b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Int32::New(isolate, b));
		}
	}

	void NJQueue::LoadInt(const FunctionCallbackInfo<Value>& args) {
		int b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Int32::New(isolate, b));
		}
	}

	void NJQueue::LoadFloat(const FunctionCallbackInfo<Value>& args) {
		float b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Number::New(isolate, b));
		}
	}

	void NJQueue::LoadLong(const FunctionCallbackInfo<Value>& args) {
		SPA::INT64 b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Number::New(isolate, (double)b));
		}
	}

	void NJQueue::LoadULong(const FunctionCallbackInfo<Value>& args) {
		SPA::UINT64 b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Number::New(isolate, (double)b));
		}
	}

	void NJQueue::LoadUInt(const FunctionCallbackInfo<Value>& args) {
		unsigned int b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Uint32::New(isolate, b));
		}
	}

	void NJQueue::LoadUShort(const FunctionCallbackInfo<Value>& args) {
		unsigned short b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Uint32::New(isolate, b));
		}
	}

	void NJQueue::LoadDate(const FunctionCallbackInfo<Value>& args) {
		SPA::UINT64 date;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, date)) {
			SPA::UDateTime dt(date);
			unsigned int us;
			std::tm tm = dt.GetCTime(&us);
			double time = (double) std::mktime(&tm);
			time *= 1000;
			time += (us / 1000.0);
			Local<Value> jsDate = Date::New(isolate, time);
			args.GetReturnValue().Set(jsDate);
		}
	}

	void NJQueue::LoadDecimal(const FunctionCallbackInfo<Value>& args) {
		DECIMAL b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(String::NewFromUtf8(isolate, SPA::ToString(b).c_str()));
		}
	}

	void NJQueue::LoadDouble(const FunctionCallbackInfo<Value>& args) {
		double b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(Number::New(isolate, b));
		}
	}

	void NJQueue::LoadUUID(const FunctionCallbackInfo<Value>& args) {
		GUID b;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->Load(isolate, b)) {
			args.GetReturnValue().Set(node::Buffer::New(isolate, (char*)&b, sizeof(b)).ToLocalChecked());
		}
	}

	void NJQueue::PopBytes(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int size = 0;
		unsigned int all = (~0);
		if (obj->m_Buffer) {
			size = obj->m_Buffer->GetSize();
		}
		if (args.Length() && args[0]->IsUint32())
			all = args[0]->Uint32Value();
		if (size > all)
			size = all;
		if (size) {
			args.GetReturnValue().Set(node::Buffer::New(isolate, (char*)obj->m_Buffer->GetBuffer(), size).ToLocalChecked());
			obj->m_Buffer->Pop(size);
		}
		else {
			args.GetReturnValue().Set(node::Buffer::New(isolate, "", 0).ToLocalChecked());
		}
	}

	void NJQueue::LoadBytes(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (!obj->m_Buffer) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "No buffer available")));
			return;
		}
		try {
			unsigned int len;
			*obj->m_Buffer >> len;
			if (len == (~0)) {
				args.GetReturnValue().SetNull();
			}
			else {
				if (len <= obj->m_Buffer->GetSize()) {
					args.GetReturnValue().Set(node::Buffer::New(isolate, (char*)obj->m_Buffer->GetBuffer(), len).ToLocalChecked());
					obj->m_Buffer->Pop(len);
				}
				else {
					obj->m_Buffer->Pop(len);
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data for loading byte array")));
				}
			}
		}
		catch (std::exception &err) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, err.what())));
		}
	}

	void NJQueue::LoadAString(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (!obj->m_Buffer) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "No buffer available")));
			return;
		}
		try {
			unsigned int len;
			*obj->m_Buffer >> len;
			if (len == (~0)) {
				args.GetReturnValue().SetNull();
			}
			else {
				if (len <= obj->m_Buffer->GetSize()) {
					args.GetReturnValue().Set(String::NewFromUtf8(isolate, (const char*)obj->m_Buffer->GetBuffer(), String::kNormalString, (int)len));
					obj->m_Buffer->Pop(len);
				}
				else {
					obj->m_Buffer->Pop(len);
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data for loading ASCII string")));
				}
			}
		}
		catch (std::exception &err) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, err.what())));
		}
	}

	void NJQueue::LoadString(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (!obj->m_Buffer) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "No buffer available")));
			return;
		}
		try {
			unsigned int len;
			*obj->m_Buffer >> len;
			if (len == (~0)) {
				args.GetReturnValue().SetNull();
			}
			else {
				if (len <= obj->m_Buffer->GetSize()) {
					args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t*)obj->m_Buffer->GetBuffer(), String::kNormalString, (int)(len / sizeof(uint16_t))));
					obj->m_Buffer->Pop(len);
				}
				else {
					obj->m_Buffer->Pop(len);
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data for loading UNICODE string")));
				}
			}
		}
		catch (std::exception &err) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, err.what())));
		}
	}

	void NJQueue::SaveBoolean(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			bool b = args[0]->IsTrue();
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveByte(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsUint32()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			unsigned char b = (unsigned char)(args[0]->Uint32Value());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveAChar(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsInt32()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			char b = (char)(args[0]->Int32Value());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveShort(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsInt32()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			short b = (short)(args[0]->Int32Value());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveInt(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsInt32()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			int b = (int)(args[0]->Int32Value());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveFloat(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsNumber()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			float b = (float)(args[0]->NumberValue());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveDouble(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsNumber()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			double b = (args[0]->NumberValue());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveLong(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsNumber()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			SPA::INT64 b = args[0]->IntegerValue();
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveULong(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsNumber()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			SPA::UINT64 b = (SPA::UINT64)(args[0]->IntegerValue());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveUInt(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsUint32()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			unsigned int b = (args[0]->Uint32Value());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveUShort(const FunctionCallbackInfo<Value>& args) {
		if (args.Length() && args[0]->IsUint32()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			unsigned short b = (unsigned short)(args[0]->Uint32Value());
			obj->Save(args, b);
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveBytes(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			auto p = args[0];
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			if (p->IsNullOrUndefined()) {
				*obj->m_Buffer << (const char *)nullptr;
				args.GetReturnValue().Set(args.Holder());
				return;
			}
			else if (node::Buffer::HasInstance(p)) {
				char *bytes = node::Buffer::Data(p);
				unsigned int len = (unsigned int)node::Buffer::Length(p);
				*obj->m_Buffer << len;
				obj->m_Buffer->Push((const unsigned char*)bytes, len);
				args.GetReturnValue().Set(args.Holder());
				return;
			}
		}
		Isolate* isolate = args.GetIsolate();
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Invalid Node.js Buffer object")));
	}

	void NJQueue::SaveUUID(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			auto p = args[0];
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			if (node::Buffer::HasInstance(p)) {
				char *bytes = node::Buffer::Data(p);
				unsigned int len = (unsigned int)node::Buffer::Length(p);
				if (len == sizeof(GUID)) {
					obj->m_Buffer->Push((const unsigned char*)bytes, len);
					args.GetReturnValue().Set(args.Holder());
					return;
				}
			}
		}
		Isolate* isolate = args.GetIsolate();
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Invalid UUID Node.js Buffer object")));
	}

	void NJQueue::SaveAString(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			auto p = args[0];
			if (p->IsNullOrUndefined()) {
				*obj->m_Buffer << (const wchar_t *)nullptr;
			}
			else {
				String::Utf8Value str(p->IsString() ? p : p->ToString());
				unsigned int len = (unsigned int)str.length();
				*obj->m_Buffer << len;
				obj->m_Buffer->Push((const unsigned char*)(*str), len);
			}
			args.GetReturnValue().Set(args.Holder());
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveString(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			auto p = args[0];
			if (p->IsNullOrUndefined()) {
				*obj->m_Buffer << (const wchar_t *)nullptr;
			}
			else {
				String::Value str(p->IsString() ? p : p->ToString());
				unsigned int len = (unsigned int)str.length();
				len *= sizeof(uint16_t);
				*obj->m_Buffer << len;
				obj->m_Buffer->Push((const unsigned char*)(*str), len);
			}
			args.GetReturnValue().Set(args.Holder());
		}
		else {
			Isolate* isolate = args.GetIsolate();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
		}
	}

	void NJQueue::SaveDecimal(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			auto p = args[0];
			if (p->IsNumber() || p->IsString()) {
				String::Utf8Value str(p->IsString() ? p : p->ToString());
				const char *s = *str;
				DECIMAL dec;
				SPA::ParseDec(s, dec);
				*obj->m_Buffer << dec;
				args.GetReturnValue().Set(args.Holder());
				return;
			}
		}
		Isolate* isolate = args.GetIsolate();
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
	}

	void NJQueue::SaveDate(const FunctionCallbackInfo<Value>& args) {
		if (args.Length()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			auto p = args[0];
			if (p->IsNumber()) {
				SPA::UINT64 millisSinceEpoch = (SPA::UINT64)(p->IntegerValue());
				std::time_t t = millisSinceEpoch / 1000;
				unsigned int ms = (unsigned int)(millisSinceEpoch % 1000);
				std::tm *ltime = std::localtime(&t);
				SPA::UDateTime dt(*ltime, ms * 1000);
				*obj->m_Buffer << dt.time;
				args.GetReturnValue().Set(args.Holder());
				return;
			}
		}
		Isolate* isolate = args.GetIsolate();
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
	}

	void NJQueue::Empty(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		obj->Release();
	}

	void NJQueue::New(const FunctionCallbackInfo<Value>& args) {
		if (args.IsConstructCall()) {
			unsigned int initSize = SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
			unsigned int blockSize = SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
			if (args.Length() > 0 && args[0]->IsUint32()) {
				initSize = args[0]->Uint32Value();
			}
			if (args.Length() > 1 && args[1]->IsUint32()) {
				blockSize = args[1]->Uint32Value();
			}
			// Invoked as constructor: `new CUQueue(...)`
			NJQueue* obj = new NJQueue(initSize, blockSize);
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
		else {
			// Invoked as plain function `CUQueue(...)`, turn into construct call.
			Local<Value> argv[] = { args[0], args[1] };
			Isolate* isolate = args.GetIsolate();
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 2, argv).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}
}
