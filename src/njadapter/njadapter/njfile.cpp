
#include "stdafx.h"
#include "njfile.h"

namespace NJA {

	Persistent<Function> NJFile::constructor;

	NJFile::NJFile(CStreamingFile *file) : NJHandlerRoot(file), m_file(file) {
		assert(file);
		::memset(&m_typeFile, 0, sizeof(m_typeFile));
	}

	NJFile::~NJFile() {
		Release();
	}

	bool NJFile::IsValid(Isolate* isolate) {
		if (!m_file) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "File handler disposed")));
			return false;
		}
		return NJHandlerRoot::IsValid(isolate);
	}

	void NJFile::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "CAsyncFile"));
		tpl->InstanceTemplate()->SetInternalFieldCount(4);

		NJHandlerRoot::Init(exports, tpl);

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
		{
			SPA::CAutoLock al(m_cs);
			if (m_file) {
				m_file = nullptr;
			}
			m_deqFileCb.clear();
		}
		NJHandlerRoot::Release();
	}

	void NJFile::getFilesQueued(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			size_t data = obj->m_file->GetFilesQueued();
			args.GetReturnValue().Set(Number::New(isolate, (double)data));
		}
	}

	void NJFile::getFileSize(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
		if (obj->IsValid(isolate)) {
			SPA::UINT64 data = obj->m_file->GetFileSize();
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
