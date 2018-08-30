
#include "stdafx.h"
#include "njfile.h"

namespace NJA {

	CSFile::CSFile(CClientSocket * cs) : CStreamingFile(cs) {
		SetLoop();
	}


	CSFile::~CSFile() {
		SPA::CAutoLock al(m_csFile);
		uv_close((uv_handle_t*)&m_fileType, nullptr);
	}

	SPA::UINT64 CSFile::Exchange(Isolate* isolate, int args, Local<Value> *argv, bool download, const wchar_t *localFile, const wchar_t *remoteFile, unsigned int flags) {
		SPA::UINT64 index = GetCallIndex();
		DDownload dd;
		DTransferring trans;
		DDiscarded aborted;
		if (args > 0) {
			if (argv[0]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[0]));
				dd = [func, download](CStreamingFile *file, int res, const std::wstring &errMsg) {
					FileCb fcb;
					fcb.Download = download;
					fcb.EventType = feExchange;
					fcb.Func = func;
					auto cs = file->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					fcb.Buffer = CScopeUQueue::Lock(os, endian);
					PSFile f = (PSFile)file;
					*fcb.Buffer << f << res << errMsg;
					CAutoLock al(f->m_csFile);
					f->m_deqFileCb.push_back(fcb);
					int fail = uv_async_send(&f->m_fileType);
					assert(!fail);
				};
			}
			else if (!argv[0]->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for file exchange end result")));
				return 0;
			}
		}
		if (args > 1) {
			if (argv[1]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[1]));
				trans = [func, download](CStreamingFile *file, SPA::UINT64 transferred) {
					FileCb fcb;
					fcb.Download = download;
					fcb.EventType = feTrans;
					fcb.Func = func;
					auto cs = file->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					fcb.Buffer = CScopeUQueue::Lock(os, endian);
					PSFile f = (PSFile)file;
					*fcb.Buffer << f << transferred << file->GetFileSize();
					CAutoLock al(f->m_csFile);
					f->m_deqFileCb.push_back(fcb);
					int fail = uv_async_send(&f->m_fileType);
					assert(!fail);
				};
			}
			else if (!argv[1]->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for monitoring file transferring progress")));
				return 0;
			}
		}
		if (args > 2) {
			if (argv[2]->IsFunction()) {
				std::shared_ptr<CNJFunc> func(new CNJFunc);
				func->Reset(isolate, Local<Function>::Cast(argv[2]));
				aborted = [func, download](CAsyncServiceHandler *file, bool canceled) {
					FileCb fcb;
					fcb.Download = download;
					fcb.EventType = feDiscarded;
					fcb.Func = func;
					auto cs = file->GetAttachedClientSocket();
					bool endian;
					tagOperationSystem os = cs->GetPeerOs(&endian);
					fcb.Buffer = CScopeUQueue::Lock(os, endian);
					PSFile f = (PSFile)file;
					*fcb.Buffer << f << canceled;
					CAutoLock al(f->m_csFile);
					f->m_deqFileCb.push_back(fcb);
					int fail = uv_async_send(&f->m_fileType);
					assert(!fail);
				};
			}
			else if (!argv[2]->IsNullOrUndefined()) {
				isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "A callback expected for tracking socket closed or canceled events")));
				return 0;
			}
		}
		if (download)
			return Download(localFile, remoteFile, dd, trans, aborted, flags) ? index : INVALID_NUMBER;
		return Upload(localFile, remoteFile, dd, trans, aborted, flags) ? index : INVALID_NUMBER;
	}

	void CSFile::file_cb(uv_async_t* handle) {
		Isolate* isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate); //required for Node 4.x
		CSFile* obj = (CSFile*)handle->data; //sender
		assert(obj);
		if (!obj)
			return;
		SPA::CAutoLock al(obj->m_csFile);
		while (obj->m_deqFileCb.size()) {
			FileCb &cb = obj->m_deqFileCb.front();
			PSFile processor;
			*cb.Buffer >> processor;
			Local<Function> func = Local<Function>::New(isolate, *cb.Func);
			Local<Object> njFile = NJFile::New(isolate, processor, true);
			Local<v8::Boolean> download = v8::Boolean::New(isolate, cb.Download);
			switch (cb.EventType) {
			case feExchange:
			{
				int res;
				std::wstring errMsg;
				*cb.Buffer >> res >> errMsg;
				Local<Value> jsRes = v8::Int32::New(isolate, res);
#ifdef WIN32_64
				Local<v8::String> jsMsg = v8::String::NewFromTwoByte(isolate, (const uint16_t*)errMsg.c_str(), v8::String::kNormalString, (int)errMsg.size());
#else

#endif
				Local<Value> argv[] = { jsMsg, jsRes, download, njFile };
				func->Call(Null(isolate), 4, argv);
			}
			break;
			case feTrans:
			{
				SPA::UINT64 pos, size;
				*cb.Buffer >> pos >> size;
				assert(!cb.Buffer->GetSize());
				Local<Value> argv[] = { v8::Number::New(isolate, (double)pos), v8::Number::New(isolate, (double)size), download, njFile };
				func->Call(Null(isolate), 4, argv);
			}
			break;
			case feDiscarded:
			{
				bool canceled;
				*cb.Buffer >> canceled;
				assert(!cb.Buffer->GetSize());
				Local<Value> argv[] = { v8::Boolean::New(isolate, canceled), download, njFile };
				func->Call(Null(isolate), 3, argv);
			}
			break;
			default:
				assert(false); //shouldn't come here
				break;
			}
			CScopeUQueue::Unlock(cb.Buffer);
			obj->m_deqFileCb.pop_front();
		}
	}

	void CSFile::SetLoop() {
		::memset(&m_fileType, 0, sizeof(m_fileType));
		m_fileType.data = this;
		int fail = uv_async_init(uv_default_loop(), &m_fileType, file_cb);
		assert(!fail);
	}
}