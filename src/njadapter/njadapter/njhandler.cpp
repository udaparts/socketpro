
#include "stdafx.h"
#include "njhandler.h"

namespace NJA {

	Persistent<Function> NJHandler::constructor;

	NJHandler::NJHandler(CAsyncServiceHandler *ash, NJSocketPool *pool) : m_ash(ash), m_Pool(pool) {
		assert(ash);
		assert(pool);
	}

	NJHandler::~NJHandler() {
	
	}

	bool NJHandler::IsValid(Isolate* isolate) {
		if (!m_Pool || !m_Pool->IsValid(isolate)) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "SocketPool object already disposed")));
			return false;
		}
		if (!m_ash) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Async handler disposed")));
			return false;
		}
		return true;
	}

	void NJHandler::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CAsyncHandler"));
		tpl->InstanceTemplate()->SetInternalFieldCount(2);

		NODE_SET_PROTOTYPE_METHOD(tpl, "getSvsId", getSvsId);
		NODE_SET_PROTOTYPE_METHOD(tpl, "AbortBatching", AbortBatching);
		NODE_SET_PROTOTYPE_METHOD(tpl, "AbortDequeuedMessage", AbortDequeuedMessage);
		NODE_SET_PROTOTYPE_METHOD(tpl, "CleanCallbacks", CleanCallbacks);


		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CAsyncHandler"), tpl->GetFunction());
	}

	void NJHandler::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			args.GetReturnValue().Set(Null(isolate));
		}
		else {
			// Invoked as plain function `CUQueue(...)`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJHandler::getSvsId(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_Pool->SvsId;
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJHandler::AbortBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_Pool->SvsId;
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJHandler::AbortDequeuedMessage(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			obj->m_ash->AbortDequeuedMessage();
		}
	}

	void NJHandler::CleanCallbacks(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->CleanCallbacks();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}
}