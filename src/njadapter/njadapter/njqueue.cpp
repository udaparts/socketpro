#include "stdafx.h"
#include "njqueue.h"
#include <algorithm>


namespace NJA {
	using SPA::CScopeUQueue;
	Persistent<Function> NJQueue::constructor;
	Persistent<FunctionTemplate> NJQueue::m_tpl;

	NJQueue::NJQueue(CUQueue *buffer, unsigned int initialSize, unsigned int blockSize) : m_Buffer(buffer), m_initSize(initialSize), m_blockSize(blockSize) {
		if (m_Buffer) {
#ifndef WIN32_64
			m_Buffer->ToUtf8(true);
#else
			m_Buffer->TimeEx(true);
#endif
		}
	}

	NJQueue::~NJQueue() {
		CScopeUQueue::Unlock(m_Buffer);
	}

	void NJQueue::Release() {
		CScopeUQueue::Unlock(m_Buffer);
	}

	void NJQueue::Ensure() {
		if (!m_Buffer) {
			m_Buffer = CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), m_initSize, m_blockSize);
#ifndef WIN32_64
			m_Buffer->ToUtf8(true);
#else
			m_Buffer->TimeEx(true);
#endif
		}
	}

	bool NJQueue::IsUQueue(Local<Object> obj) {
		Isolate* isolate = Isolate::GetCurrent();
		HandleScope handleScope(isolate); //required for Node 4.x or later
		Local<FunctionTemplate> cb = Local<FunctionTemplate>::New(isolate, m_tpl);
		return cb->HasInstance(obj);
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
		NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Empty);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getSize", getSize);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setSize", setSize);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getOS", getOS);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Realloc", Realloc);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getBufferSize", getMaxBufferSize);

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
		NODE_SET_PROTOTYPE_METHOD(tpl, "LoadObject", LoadObject);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Load", LoadByClass);
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
		NODE_SET_PROTOTYPE_METHOD(tpl, "SaveObject", SaveObject);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Save", SaveByClass);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CUQueue"), tpl->GetFunction());
		m_tpl.Reset(isolate, tpl);
	}

	void NJQueue::getOS(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->m_Buffer)
			args.GetReturnValue().Set(Int32::New(args.GetIsolate(), obj->m_Buffer->GetOS()));
		else
			args.GetReturnValue().Set(Int32::New(args.GetIsolate(), SPA::GetOS()));
	}

	void NJQueue::Discard(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int ret = 0;
		if (obj->m_Buffer && args[0]->IsUint32()) {
			ret = obj->m_Buffer->Pop((unsigned int)args[0]->Uint32Value());
		}
		args.GetReturnValue().Set(Uint32::New(args.GetIsolate(), ret));
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
		args.GetReturnValue().Set(Uint32::New(args.GetIsolate(), ret));
	}

	void NJQueue::getMaxBufferSize(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int ret = 0;
		if (obj->m_Buffer)
			ret = obj->m_Buffer->GetMaxSize();
		args.GetReturnValue().Set(Uint32::New(args.GetIsolate(), ret));
	}

	void NJQueue::Realloc(const FunctionCallbackInfo<Value>& args) {
		unsigned int size = 0;
		if (args.Length() && args[0]->IsUint32())
			size = args[0]->Uint32Value();
		else {
			args.GetIsolate()->ThrowException(Exception::TypeError(String::NewFromUtf8(args.GetIsolate(), "Unsigned int expected")));
			return;
		}
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->m_Buffer)
			obj->m_Buffer->ReallocBuffer(size);
		else
			obj->m_Buffer = CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), size, obj->m_blockSize);
	}

	void NJQueue::getSize(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		unsigned int ret = 0;
		if (obj->m_Buffer)
			ret = obj->m_Buffer->GetSize();
		args.GetReturnValue().Set(Uint32::New(args.GetIsolate(), ret));
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
			args.GetReturnValue().Set(ToDate(isolate, date));
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
		if (obj->get() && !obj->get()->GetSize())
			obj->Release();
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
			if (!obj->m_Buffer->GetSize())
				obj->Release();
		}
		catch (std::exception &err) {
			obj->Release();
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
			if (!obj->m_Buffer->GetSize())
				obj->Release();
		}
		catch (std::exception &err) {
			obj->Release();
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
			if (!obj->m_Buffer->GetSize())
				obj->Release();
		}
		catch (std::exception &err) {
			obj->Release();
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
				*obj->m_Buffer << (const char*)nullptr;
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
		Isolate* isolate = args.GetIsolate();
		if (args.Length()) {
			NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
			obj->Ensure();
			auto p = args[0];
			SPA::UINT64 millisSinceEpoch;
			if (p->IsDate()) {
				Date *dt = Date::Cast(*p);
				millisSinceEpoch = (SPA::UINT64) dt->ValueOf();
			}
			else if (p->IsNumber()) {
				millisSinceEpoch = (SPA::UINT64)(p->IntegerValue());
			}
			else {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
				return;
			}
			std::time_t t = millisSinceEpoch / 1000;
			unsigned int ms = (unsigned int)(millisSinceEpoch % 1000);
			std::tm *ltime = std::localtime(&t);
			SPA::UDateTime dt(*ltime, ms * 1000);
			*obj->m_Buffer << dt.time;
			args.GetReturnValue().Set(args.Holder());
			return;
		}
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Bad data type")));
	}

	unsigned int NJQueue::Load(Isolate* isolate, SPA::UDB::CDBVariant &vt) {
		if (!m_Buffer) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "No buffer available")));
			return 0;
		}
		try {
			unsigned int start = m_Buffer->GetSize();
			*m_Buffer >> vt;
			unsigned int size = (start - m_Buffer->GetSize());
			if (!m_Buffer->GetSize())
				Release();
			return size;
		}
		catch (std::exception &err) {
			Release();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, err.what())));
		}
		return 0;
	}

	Local<Value> NJQueue::ToDate(Isolate* isolate, SPA::UINT64 datetime) {
		SPA::UDateTime dt(datetime);
		unsigned int us;
		std::tm tm = dt.GetCTime(&us);
		double time = (double)std::mktime(&tm);
		time *= 1000;
		time += (us / 1000.0);
		if (tm.tm_isdst)
			time -= 3600000;
		return Date::New(isolate, time);
	}

	void NJQueue::LoadByClass(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (!obj->m_Buffer) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "No buffer available")));
		}
		else if (args[0]->IsFunction()) {
			Local<Function> cb = Local<Function>::Cast(args[0]);
			Local<Value> argv[] = { args.Holder() };
			args.GetReturnValue().Set(cb->Call(isolate->GetCurrentContext(), Null(isolate), 1, argv).ToLocalChecked());
			if (!obj->m_Buffer->GetSize())
				obj->Release();
		}
		else {
			obj->Release();
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "An function expected for class or struct de-serialization")));
		}
	}

	void NJQueue::LoadObject(const FunctionCallbackInfo<Value>& args) {
		SPA::UDB::CDBVariant vt;
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		if (obj->m_Buffer && obj->m_Buffer->GetSize() >= sizeof(VARTYPE)) {
			VARTYPE *pvt = (VARTYPE*)obj->m_Buffer->GetBuffer();
			if (*pvt == SPA::VT_USERIALIZER_OBJECT) {
				obj->m_Buffer->Pop((unsigned int)sizeof(VARTYPE));
				LoadByClass(args);
				return;
			}
		}
		if (obj->Load(isolate, vt)) {
			VARTYPE type = vt.Type();
			switch (type)
			{
			case VT_NULL:
			case VT_EMPTY:
				args.GetReturnValue().Set(Null(isolate));
				break;
			case VT_BOOL:
				args.GetReturnValue().Set(Boolean::New(isolate, vt.boolVal ? true : false));
				break;
			case VT_I1:
			case VT_I2:
			case VT_INT:
			case VT_I4:
				args.GetReturnValue().Set(Int32::New(isolate, vt.lVal));
				break;
			case VT_I8:
				args.GetReturnValue().Set(Number::New(isolate, (double)vt.llVal));
				break;
			case VT_UI1:
			case VT_UI2:
			case VT_UINT:
			case VT_UI4:
				args.GetReturnValue().Set(Uint32::New(isolate, vt.ulVal));
				break;
			case VT_UI8:
				args.GetReturnValue().Set(Number::New(isolate, (double)vt.ullVal));
				break;
			case VT_R4:
				args.GetReturnValue().Set(Number::New(isolate, vt.fltVal));
				break;
			case VT_R8:
				args.GetReturnValue().Set(Number::New(isolate, vt.dblVal));
				break;
			case VT_CY:
			{
				double d = (double)vt.llVal;
				d /= 10000;
				args.GetReturnValue().Set(Number::New(isolate, d));
			}
			break;
			case VT_DECIMAL:
				args.GetReturnValue().Set(String::NewFromUtf8(isolate, SPA::ToString(vt.decVal).c_str()));
				break;
			case VT_DATE:
				args.GetReturnValue().Set(ToDate(isolate, vt.ullVal));
				break;
			case VT_BSTR:
				if (vt.bstrVal) {
#ifdef WIN32_64
					args.GetReturnValue().Set(String::NewFromTwoByte(isolate, (const uint16_t*)vt.bstrVal, String::kNormalString, SysStringLen(vt.bstrVal)));
#else

#endif
				}
				else {
					args.GetReturnValue().Set(v8::Null(isolate));
				}
				break;
			case (VT_I1 | VT_ARRAY):
			{
				const char *str = nullptr;
				unsigned int len = vt.parray->rgsabound->cElements;
				::SafeArrayAccessData(vt.parray, (void**)&str);
				args.GetReturnValue().Set(String::NewFromUtf8(isolate, str, String::kNormalString, (int)len));
				::SafeArrayUnaccessData(vt.parray);
			}
			break;
			case VT_CLSID:
			case (VT_UI1 | VT_ARRAY):
			{
				char *str = nullptr;
				unsigned int len = vt.parray->rgsabound->cElements;
				::SafeArrayAccessData(vt.parray, (void**)&str);
				args.GetReturnValue().Set(node::Buffer::New(isolate, str, len).ToLocalChecked());
				::SafeArrayUnaccessData(vt.parray);
			}
			break;
			default:
			{
				bool is_array = ((type & VT_ARRAY) == VT_ARRAY);
				if (is_array) {
					void *pvt;
					bool ok = true;
					unsigned int count = vt.parray->rgsabound->cElements;
					::SafeArrayAccessData(vt.parray, &pvt);
					type = (type & (~VT_ARRAY));
					switch (type) {
					case VT_BOOL:
					case VT_BSTR:
					case VT_DATE:
					case VT_I8:
					case VT_UI8:
					case VT_CY:
					case VT_DECIMAL:
					case VT_VARIANT:
					{
						Local<Array> v = Array::New(isolate);
						for (unsigned int n = 0; n < count; ++n) {
							switch (type)
							{
							case VT_BOOL:
							{
								VARIANT_BOOL *p = (VARIANT_BOOL *)pvt;
								v->Set(n, Boolean::New(isolate, (p[n] == VARIANT_FALSE) ? false : true));
							}
							break;
							case VT_UI8:
							case VT_I8:
							{
								SPA::INT64 *p = (SPA::INT64 *)pvt;
								v->Set(n, Number::New(isolate, (double)(p[n])));
							}
							break;
							case VT_CY:
							{
								SPA::INT64 *p = (SPA::INT64 *)pvt;
								v->Set(n, Number::New(isolate, ((double)p[n]) / 10000));
							}
							break;
							case VT_DECIMAL:
							{
								DECIMAL *p = (DECIMAL *)pvt;
								v->Set(n, String::NewFromUtf8(isolate, SPA::ToString(p[n]).c_str()));
							}
							break;
							case VT_BSTR:
							{
								BSTR *p = (BSTR *)pvt;
								if (!p[n]) {
									v->Set(n, Null(isolate));
								}
								else {
#ifdef WIN32_64
									Local<String> s = String::NewFromTwoByte(isolate, (const uint16_t*)p[n], String::kNormalString, SysStringLen(p[n]));
#else

#endif
									v->Set(n, s);
								}
							}
							break;
							case VT_DATE:
							{
								SPA::UINT64 *p = (SPA::UINT64 *)pvt;
								v->Set(n, ToDate(isolate, p[n]));
							}
							break;
							case VT_VARIANT:
							{
								VARIANT *p = (VARIANT *)pvt;
								VARTYPE vtSub = p->vt;
								switch (vtSub)
								{
								case VT_NULL:
								case VT_EMPTY:
									v->Set(n, Null(isolate));
									break;
								case VT_I1:
									v->Set(n, Int32::New(isolate, p->cVal));
									break;
								case VT_UI1:
									v->Set(n, Int32::New(isolate, p->bVal));
									break;
								case VT_I2:
									v->Set(n, Int32::New(isolate, p->iVal));
									break;
								case VT_UI2:
									v->Set(n, Int32::New(isolate, p->uiVal));
									break;
								case VT_I4:
								case VT_INT:
									v->Set(n, Int32::New(isolate, p->lVal));
									break;
								case VT_UI4:
								case VT_UINT:
									v->Set(n, Number::New(isolate, p->ulVal));
									break;
								case VT_R4:
									v->Set(n, Number::New(isolate, p->fltVal));
									break;
								case VT_R8:
									v->Set(n, Number::New(isolate, p->dblVal));
									break;
								case VT_UI8:
									v->Set(n, Number::New(isolate, (double)p->ullVal));
									break;
								case VT_I8:
									v->Set(n, Number::New(isolate, (double)p->llVal));
									break;
								case VT_CY:
									v->Set(n, Number::New(isolate, (double)p->llVal / 10000));
									break;
								case VT_BOOL:
									v->Set(n, Boolean::New(isolate, (p->boolVal == VARIANT_FALSE) ? false : true));
									break;
								case VT_DECIMAL:
									v->Set(n, String::NewFromUtf8(isolate, SPA::ToString(p[n].decVal).c_str()));
									break;
								case VT_BSTR:
								{
#ifdef WIN32_64
									Local<String> s = String::NewFromTwoByte(isolate, (const uint16_t*)p[n].bstrVal, String::kNormalString, SysStringLen(p[n].bstrVal));
#else

#endif
									v->Set(n, s);
								}
									break;
								default:
									isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data array type")));
									return;
								}
							}
							break;
							default:
								assert(false);
								break;
							}
						}
						args.GetReturnValue().Set(v);
					}
					break;
					case VT_I4:
					case VT_INT:
					{
						Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(int));
						Local<v8::Int32Array> v = v8::Int32Array::New(buf, 0, count);
						Local<Value> p = v;
						char *bytes = node::Buffer::Data(p);
						memcpy(bytes, pvt, count * sizeof(int));
						args.GetReturnValue().Set(v);
					}
					break;
					case VT_UI4:
					case VT_UINT:
					{
						Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(unsigned int));
						Local<v8::Uint32Array> v = v8::Uint32Array::New(buf, 0, count);
						Local<Value> p = v;
						char *bytes = node::Buffer::Data(p);
						memcpy(bytes, pvt, count * sizeof(unsigned int));
						args.GetReturnValue().Set(v);
					}
					break;
					case VT_I2:
					{
						Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(short));
						Local<v8::Int16Array> v = v8::Int16Array::New(buf, 0, count);
						Local<Value> p = v;
						char *bytes = node::Buffer::Data(p);
						memcpy(bytes, pvt, count * sizeof(short));
						args.GetReturnValue().Set(v);
					}
					break;
					case VT_UI2:
					{
						Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(unsigned short));
						Local<v8::Uint16Array> v = v8::Uint16Array::New(buf, 0, count);
						Local<Value> p = v;
						char *bytes = node::Buffer::Data(p);
						memcpy(bytes, pvt, count * sizeof(unsigned short));
						args.GetReturnValue().Set(v);
					}
					break;
					case VT_R4:
					{
						Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(float));
						Local<v8::Float32Array> v = v8::Float32Array::New(buf, 0, count);
						Local<Value> p = v;
						char *bytes = node::Buffer::Data(p);
						memcpy(bytes, pvt, count * sizeof(float));
						args.GetReturnValue().Set(v);
					}
					break;
					case VT_R8:
					{
						Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof(double));
						Local<v8::Float64Array> v = v8::Float64Array::New(buf, 0, count);
						Local<Value> p = v;
						char *bytes = node::Buffer::Data(p);
						memcpy(bytes, pvt, count * sizeof(double));
						args.GetReturnValue().Set(v);
					}
					break;
					default:
						isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data array type")));
						break;
					}
					::SafeArrayUnaccessData(vt.parray);
				}
				else
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data type")));
			}
			break;
			}
		}
	}

	void NJQueue::SaveByClass(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		obj->Ensure();
		auto p0 = args[0];
		if (p0->IsFunction()) {
			Local<Function> cb = Local<Function>::Cast(args[0]);
			Local<Value> argv[] = { args.Holder() };
			cb->Call(isolate->GetCurrentContext(), Null(isolate), 1, argv);
			args.GetReturnValue().Set(args.Holder());
		}
		else {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A callback function expected")));
		}
	}

	void NJQueue::SaveObject(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (!args.Length()) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "An input data expected")));
			return;
		}
		VARTYPE vt;
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		obj->Ensure();
		std::string id;
		auto p1 = args[1];
		if (p1->IsString()) {
			String::Utf8Value str(p1);
			const char *s = *str;
			id.assign(s, str.length());
			std::transform(id.begin(), id.end(), id.begin(), ::tolower);
		}
		int argv = args.Length();
		auto p0 = args[0];
		if (p0->IsNullOrUndefined()) {
			vt = VT_NULL;
			*obj->m_Buffer << vt;
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsFunction()) {
			vt = SPA::VT_USERIALIZER_OBJECT;
			*obj->m_Buffer << vt;
			SaveByClass(args);
		}
		else if (p0->IsDate()) {
			vt = VT_DATE;
			*obj->m_Buffer << vt;
			SaveDate(args);
		}
		else if (p0->IsBoolean()) {
			vt = VT_BOOL;
			short v = p0->IsTrue() ? -1 : 0;
			*obj->m_Buffer << vt << v;
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsString()) {
			if (argv > 1 && args[1]->IsString()) {
				if (id == "a" || id == "ascii")
				{
					vt = (VT_ARRAY | VT_I1);
					*obj->m_Buffer << vt;
					SaveAString(args);
				}
				else if (id == "dec" || id == "decimal") {
					vt = VT_DECIMAL;
					*obj->m_Buffer << vt;
					SaveDecimal(args);
				}
				else {
					vt = VT_BSTR;
					*obj->m_Buffer << vt;
					SaveString(args);
				}
			}
			else {
				vt = VT_BSTR;
				*obj->m_Buffer << vt;
				SaveString(args);
			}
		}
		else if (p0->IsNumber()) {
			if (argv > 1 && args[1]->IsString()) {
				if (id == "f" || id == "float") {
					vt = VT_R4;
					*obj->m_Buffer << vt;
					SaveFloat(args);
				}
				else if (id == "d" || id == "double") {
					vt = VT_R8;
					*obj->m_Buffer << vt;
					SaveDouble(args);
				}
				else if (id == "i" || id == "int") {
					vt = VT_I4;
					*obj->m_Buffer << vt;
					SaveInt(args);
				}
				else if (id == "ui" || id == "uint") {
					vt = VT_UI4;
					*obj->m_Buffer << vt;
					SaveUInt(args);
				}
				else if (id == "l" || id == "long") {
					vt = VT_I8;
					*obj->m_Buffer << vt;
					SaveLong(args);
				}
				else if (id == "ul" || id == "ulong") {
					vt = VT_UI8;
					*obj->m_Buffer << vt;
					SaveULong(args);
				}
				else if (id == "s" || id == "short") {
					vt = VT_I2;
					*obj->m_Buffer << vt;
					SaveShort(args);
				}
				else if (id == "us" || id == "ushort") {
					vt = VT_UI2;
					*obj->m_Buffer << vt;
					SaveUShort(args);
				}
				else if (id == "dec" || id == "decimal") {
					vt = VT_DECIMAL;
					*obj->m_Buffer << vt;
					SaveDecimal(args);
				}
				else if (id == "c" || id == "char") {
					vt = VT_I1;
					*obj->m_Buffer << vt;
					SaveAChar(args);
				}
				else if (id == "b" || id == "byte") {
					vt = VT_UI1;
					*obj->m_Buffer << vt;
					SaveByte(args);
				}
				else if (id == "date") {
					vt = VT_DATE;
					*obj->m_Buffer << vt;
					SaveDate(args);
				}
				else {
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unknown number type")));
				}
			}
			else {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unknown number type")));
			}
		}
		else if (p0->IsInt8Array()) {
			vt = (VT_ARRAY | VT_I1);
			char *bytes = node::Buffer::Data(p0);
			unsigned int count = (unsigned int)node::Buffer::Length(p0);
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(char));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsInt16Array()) {
			vt = (VT_ARRAY | VT_I2);
			char *bytes = node::Buffer::Data(p0);
			Local<v8::Int16Array> vInt = Local<v8::Int16Array>::Cast(p0);
			unsigned int count = (unsigned int)vInt->Length();
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(short));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsInt32Array()) {
			vt = (VT_ARRAY | VT_I4);
			char *bytes = node::Buffer::Data(p0);
			Local<v8::Int32Array> vInt = Local<v8::Int32Array>::Cast(p0);
			unsigned int count = (unsigned int)vInt->Length();
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(int));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsUint16Array()) {
			vt = (VT_ARRAY | VT_UI2);
			char *bytes = node::Buffer::Data(p0);
			Local<v8::Uint16Array> vInt = Local<v8::Uint16Array>::Cast(p0);
			unsigned int count = (unsigned int)vInt->Length();
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(unsigned short));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsUint32Array()) {
			vt = (VT_ARRAY | VT_UI4);
			char *bytes = node::Buffer::Data(p0);
			Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(p0);
			unsigned int count = (unsigned int)vInt->Length();
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(unsigned int));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsFloat32Array()) {
			vt = (VT_ARRAY | VT_R4);
			char *bytes = node::Buffer::Data(p0);
			Local<v8::Float32Array> vInt = Local<v8::Float32Array>::Cast(p0);
			unsigned int count = (unsigned int)vInt->Length();
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(float));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsFloat64Array()) {
			vt = (VT_ARRAY | VT_R8);
			char *bytes = node::Buffer::Data(p0);
			Local<v8::Float64Array> vInt = Local<v8::Float64Array>::Cast(p0);
			unsigned int count = (unsigned int)vInt->Length();
			*obj->m_Buffer << vt << count;
			obj->m_Buffer->Push((const unsigned char*)bytes, count * sizeof(double));
			args.GetReturnValue().Set(args.Holder());
		}
		else if (node::Buffer::HasInstance(p0)) {
			char *bytes = node::Buffer::Data(p0);
			unsigned int len = (unsigned int)node::Buffer::Length(p0);
			if (len != sizeof(GUID) || argv == 1 || !args[1]->IsString()) {
				vt = (VT_ARRAY | VT_UI1);
				*obj->m_Buffer << vt << len;
				obj->m_Buffer->Push((const unsigned char*)bytes, len);
			}
			else
			{
				if (id == "u" || id == "uuid") {
					vt = VT_CLSID;
					*obj->m_Buffer << vt;
					obj->m_Buffer->Push((const unsigned char*)bytes, len);
				}
				else {
					vt = (VT_ARRAY | VT_UI1);
					*obj->m_Buffer << vt << len;
					obj->m_Buffer->Push((const unsigned char*)bytes, len);
				}
			}
			args.GetReturnValue().Set(args.Holder());
		}
		else if (p0->IsArray()) {
			SPA::CScopeUQueue sb;
			tagDataType dt = dtUnknown;
			Local<Array> jsArr = Local<Array>::Cast(p0);
			unsigned int count = jsArr->Length();
			for (unsigned int n = 0; n < count; ++n) {
				auto d = jsArr->Get(n);
				if (d->IsBoolean()) {
					if (dt && dt != dtBool) {
						isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data array type")));
						return;
					}
					else
						dt = dtBool;
					VARIANT_BOOL b = d->BooleanValue() ? VARIANT_TRUE : VARIANT_FALSE;
					sb << b;
				}
				else if (d->IsDate()) {
			 		if (dt && dt != dtDate) {
						isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data array type")));
						return;
					}
					else
						dt = dtDate;
					Date *dt = Date::Cast(*d);
					SPA::UINT64 millisSinceEpoch = (SPA::UINT64) dt->ValueOf();
					std::time_t t = millisSinceEpoch / 1000;
					unsigned int ms = (unsigned int)(millisSinceEpoch % 1000);
					std::tm *ltime = std::localtime(&t);
					SPA::UDateTime udt(*ltime, ms * 1000);
					sb << udt.time;
				}
				else if (d->IsString()) {
					if (dt && dt != dtString) {
						isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data array type")));
						return;
					}
					else
						dt = dtString;
					String::Value str(d);
					unsigned int len = (unsigned int)str.length();
					len *= sizeof(uint16_t);
					sb << len;
					sb->Push((const unsigned char*)(*str), len);
				}
				else
				{
					isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data array type")));
					return;
				}
			}
			VARTYPE vtType = VT_ARRAY;
			switch (dt)
			{
			case dtString:
				vtType |= VT_BSTR;
				break;
			case dtBool:
				vtType |= VT_BOOL;
				break;
			case dtDate:
				vtType |= VT_DATE;
				break;
			default:
				assert(false); //shouldn't come here
				break;
			}
			*obj->m_Buffer << vtType << count;
			obj->m_Buffer->Push(sb->GetBuffer(), sb->GetSize());
			args.GetReturnValue().Set(args.Holder());
		}
		else {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Unsupported data type")));
		}
	}

	void NJQueue::Empty(const FunctionCallbackInfo<Value>& args) {
		NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
		obj->Release();
	}

	Local<Object> NJQueue::New(Isolate* isolate, PUQueue &q) {
		SPA::UINT64 ptr = (SPA::UINT64)q;
		Local<Value> argv[] = { Boolean::New(isolate, true), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		q = nullptr;
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJQueue::New(const FunctionCallbackInfo<Value>& args) {
		if (args.IsConstructCall()) {
			NJQueue* obj;
			unsigned int initSize = SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
			unsigned int blockSize = SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
			if (args[0]->IsBoolean() && args[0]->BooleanValue() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				SPA::INT64 ptr = args[2]->IntegerValue();
				// Invoked as constructor: `new CUQueue(...)`
				obj = new NJQueue((CUQueue*)ptr, initSize, blockSize);
			}
			else {
				if (args.Length() > 0 && args[0]->IsUint32()) {
					initSize = args[0]->Uint32Value();
				}
				if (args.Length() > 1 && args[1]->IsUint32()) {
					blockSize = args[1]->Uint32Value();
				}
				// Invoked as constructor: `new CUQueue(...)`
				obj = new NJQueue(nullptr, initSize, blockSize);
			}
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
