
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
		NODE_SET_PROTOTYPE_METHOD(tpl, "CommitBatching", CommitBatching);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getRequestsQueued", getRequestsQueued);
		NODE_SET_PROTOTYPE_METHOD(tpl, "IsBatching", IsBatching);
		NODE_SET_PROTOTYPE_METHOD(tpl, "IsDequeuedMessageAborted", IsDequeuedMessageAborted);
		NODE_SET_PROTOTYPE_METHOD(tpl, "IsDequeuedResult", IsDequeuedResult);
		NODE_SET_PROTOTYPE_METHOD(tpl, "IsRouteeResult", IsRouteeResult);
		NODE_SET_PROTOTYPE_METHOD(tpl, "StartBatching", StartBatching);

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

	void NJHandler::CommitBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool server_commit = false;
			auto p = args[0];
			if (p->IsBoolean())
				server_commit = p->BooleanValue();
			else if (!p->IsNullOrUndefined()) {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A boolean expected")));
				return;
			}
			bool ok = obj->m_ash->CommitBatching(server_commit);
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::getRequestsQueued(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->GetRequestsQueued();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJHandler::AbortBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->AbortBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::StartBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->StartBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::SendRequest(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			auto p0 = args[0];
			if (!p0->IsUint32()) {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "A request id expected")));
				return;
			}
			unsigned int reqId = p0->Uint32Value();
			if (reqId > 0xffff || reqId <= SPA::tagBaseRequestID::idReservedTwo) {
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "An unsigned short request id expected")));
				return;
			}
			auto p1 = args[1];

			ResultHandler rh;
			auto p2 = args[2];
			CAsyncServiceHandler::DDiscarded abort;
			auto p3 = args[3];

			bool ok = false;
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::IsBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::IsDequeuedMessageAborted(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsDequeuedMessageAborted();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::IsDequeuedResult(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsDequeuedResult();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandler::IsRouteeResult(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandler* obj = ObjectWrap::Unwrap<NJHandler>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsRouteeRequest();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
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