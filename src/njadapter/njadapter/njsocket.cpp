
#include "stdafx.h"
#include "njsocket.h"

namespace NJA {
	
	SPA::CUCriticalSection NJSocket::m_cs;
	Persistent<Function> NJSocket::constructor;

	NJSocket::NJSocket(CClientSocket *socket) : m_socket(socket) {
	}

	NJSocket::~NJSocket() {
		Release();
	}

	bool NJSocket::IsValid(Isolate* isolate) {
		if (!m_socket) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Socket handle disposed")));
			return false;
		}
		return true;
	}

	void NJSocket::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CSocket"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Dispose);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CSocket"), tpl->GetFunction());
	}

	Local<Object> NJSocket::New(Isolate* isolate, CClientSocket *ash, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)ash;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJSocket::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJSocket *obj = new NJSocket((CClientSocket*)ptr);
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `CSocket()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJSocket::Release() {
		SPA::CAutoLock al(m_cs);
		if (m_socket) {
			m_socket = nullptr;
		}
	}

	void NJSocket::Dispose(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJSocket* obj = ObjectWrap::Unwrap<NJSocket>(args.Holder());
		obj->Release();
	}
}
