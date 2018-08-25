
#include "stdafx.h"
#include "njfile.h"

namespace NJA {

	Persistent<Function> NJFile::constructor;
	SPA::CUCriticalSection NJFile::m_cs;

	NJFile::NJFile(CStreamingFile *file) : m_ash(file) {
		assert(file);
		::memset(&m_typeFile, 0, sizeof(m_typeFile));
	}

	NJFile::~NJFile() {
		Release();
	}

	bool NJFile::IsValid(Isolate* isolate) {
		if (!m_ash) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "File handler disposed")));
			return false;
		}
		return true;
	}

	void NJFile::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CAsyncFile"));
		tpl->InstanceTemplate()->SetInternalFieldCount(6);

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

		NODE_SET_PROTOTYPE_METHOD(tpl, "getFilesQueued", getFilesQueued);
		NODE_SET_PROTOTYPE_METHOD(tpl, "getFileSize", getFileSize);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Upload", Upload);
		NODE_SET_PROTOTYPE_METHOD(tpl, "Download", Download);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "CAsyncFile"), tpl->GetFunction());
	}

	void  NJFile::file_cb(uv_async_t* handle) {

	}

	void NJFile::SetCb() {
		m_typeFile.data = this;
		int fail = uv_async_init(uv_default_loop(), &m_typeFile, file_cb);
		assert(!fail);
	}

	Local<Object> NJFile::New(Isolate* isolate, CStreamingFile *ash, bool setCb) {
		SPA::UINT64 ptr = (SPA::UINT64)ash;
		Local<Value> argv[] = { Boolean::New(isolate, setCb), Number::New(isolate, (double)SECRECT_NUM), Number::New(isolate, (double)ptr) };
		Local<Context> context = isolate->GetCurrentContext();
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		return cons->NewInstance(context, 3, argv).ToLocalChecked();
	}

	void NJFile::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
				bool setCb = args[0]->BooleanValue();
				SPA::INT64 ptr = args[2]->IntegerValue();
				NJFile *obj = new NJFile((CStreamingFile*)ptr);
				if (setCb) {
					obj->SetCb();
				}
				obj->Wrap(args.This());
				args.GetReturnValue().Set(args.This());
			}
			else {
				args.GetReturnValue().Set(Null(isolate));
			}
		}
		else {
			// Invoked as plain function `CAsyncFile()`, turn into construct call.
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}

	void NJFile::Release() {
		SPA::CAutoLock al(m_cs);
		if (m_ash) {
			m_ash = nullptr;
		}
		m_deqFileCb.clear();
	}

	void NJFile::getSvsId(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->GetSvsID();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJFile::CommitBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
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

	void NJFile::getRequestsQueued(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->GetRequestsQueued();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJFile::AbortBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->AbortBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJFile::StartBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->StartBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJFile::SendRequest(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
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

	void NJFile::Dispose(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		obj->Release();
	}

	void NJFile::IsBatching(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsBatching();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJFile::IsDequeuedMessageAborted(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsDequeuedMessageAborted();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJFile::IsDequeuedResult(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsDequeuedResult();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJFile::IsRouteeResult(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			bool ok = obj->m_ash->IsRouteeRequest();
			args.GetReturnValue().Set(Boolean::New(isolate, ok));
		}
	}

	void NJFile::AbortDequeuedMessage(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			obj->m_ash->AbortDequeuedMessage();
		}
	}

	void NJFile::CleanCallbacks(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			unsigned int data = obj->m_ash->CleanCallbacks();
			args.GetReturnValue().Set(Uint32::New(isolate, data));
		}
	}

	void NJFile::getFilesQueued(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			size_t data = obj->m_ash->GetFilesQueued();
			args.GetReturnValue().Set(Number::New(isolate, (double)data));
		}
	}

	void NJFile::getFileSize(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			SPA::UINT64 data = obj->m_ash->GetFileSize();
			args.GetReturnValue().Set(Number::New(isolate, (double)data));
		}
	}

	void NJFile::Upload(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}

	void NJFile::Download(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {

		}
	}
}
