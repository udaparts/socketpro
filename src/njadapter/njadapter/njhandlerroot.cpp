#include "stdafx.h"
#include "njhandlerroot.h"
#include "njfile.h"

namespace NJA {

	SPA::CUCriticalSection NJHandlerRoot::m_cs;

	NJHandlerRoot::NJHandlerRoot(CAsyncServiceHandler *ash) : m_ash(ash) {
		assert(ash);
		::memset(&m_typeReq, 0, sizeof(m_typeReq));
	}

	NJHandlerRoot::~NJHandlerRoot() {
		Release();
	}

	bool NJHandlerRoot::IsValid(Isolate* isolate) {
		if (!m_ash) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Async handler disposed")));
			return false;
		}
		return true;
	}

	void NJHandlerRoot::req_cb(uv_async_t* handle) {

	}

	void NJHandlerRoot::SetCb() {
		m_typeReq.data = this;
		int fail = uv_async_init(uv_default_loop(), &m_typeReq, req_cb);
		assert(!fail);
	}

	void NJHandlerRoot::Init(Local<Object> exports, Local<FunctionTemplate> &tpl) {
		
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
		NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Dispose);
	}

	void NJHandlerRoot::Release() {
		SPA::CAutoLock al(m_cs);
		if (m_ash) {
			m_ash = nullptr;
		}
		m_deqReqCb.clear();
	}

	void NJHandlerRoot::getSvsId(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->GetSvsID();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJHandlerRoot::CommitBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
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

	void NJHandlerRoot::getRequestsQueued(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->GetRequestsQueued();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJHandlerRoot::AbortBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->AbortBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandlerRoot::StartBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->StartBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandlerRoot::SendRequest(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
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

	void NJHandlerRoot::Dispose(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		obj->Release();
	}

	void NJHandlerRoot::IsBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandlerRoot::IsDequeuedMessageAborted(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsDequeuedMessageAborted();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandlerRoot::IsDequeuedResult(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsDequeuedResult();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandlerRoot::IsRouteeResult(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsRouteeRequest();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJHandlerRoot::AbortDequeuedMessage(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			obj->m_ash->AbortDequeuedMessage();
		}
	}

	void NJHandlerRoot::CleanCallbacks(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJHandlerRoot* obj = ObjectWrap::Unwrap<NJHandlerRoot>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->CleanCallbacks();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

}