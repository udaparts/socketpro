
#include "stdafx.h"
#include "njfile.h"

namespace NJA {

    CSFile::CSFile(CClientSocket * cs) : CStreamingFile(cs) {
        SetLoop();
    }

    CSFile::~CSFile() {
        SPA::CAutoLock al(m_csFile);
        uv_close((uv_handle_t*) & m_fileType, nullptr);
    }

    SPA::UINT64 CSFile::Exchange(Isolate* isolate, int args, Local<Value> *argv, bool download, const wchar_t *localFile, const wchar_t *remoteFile, unsigned int flags) {
        SPA::UINT64 index = GetCallIndex();
        DDownload dd;
        DTransferring trans;
        DDiscarded aborted;
        DServerException se;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                dd = [func, download](CStreamingFile *file, int res, const std::wstring & errMsg) {
                    FileCb fcb(download, tagFileEvent::feExchange);
                    fcb.Func = func;
                    PSFile f = (PSFile) file;
                    *fcb.Buffer << f << res << errMsg;
                    CAutoLock al(f->m_csFile);
                    f->m_deqFileCb.push_back(std::move(fcb));
                    if (f->m_deqFileCb.size() < 2) {
                        int fail = uv_async_send(&f->m_fileType);
                        assert(!fail);
                    }
                };
            } else if (!IsNullOrUndefined(argv[0])) {
                ThrowException(isolate, "A callback expected for file exchange end result");
                return 0;
            }
        }
        if (args > 1) {
            if (argv[1]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[1])));
                trans = [func, download](CStreamingFile *file, SPA::UINT64 transferred) {
                    FileCb fcb(download, tagFileEvent::feTrans);
                    fcb.Func = func;
                    PSFile f = (PSFile) file;
                    *fcb.Buffer << f << transferred << file->GetFileSize();
                    CAutoLock al(f->m_csFile);
                    f->m_deqFileCb.push_back(std::move(fcb));
                    if (f->m_deqFileCb.size() < 2) {
                        int fail = uv_async_send(&f->m_fileType);
                        assert(!fail);
                    }
                };
            } else if (!IsNullOrUndefined(argv[1])) {
                ThrowException(isolate, "A callback expected for monitoring file transferring progress");
                return 0;
            }
        }
        if (args > 2) {
            if (argv[2]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[2])));
                aborted = [func, download](CAsyncServiceHandler *file, bool canceled) {
                    FileCb fcb(download, tagFileEvent::feDiscarded);
                    fcb.Func = func;
                    PSFile f = (PSFile) file;
                    *fcb.Buffer << f << canceled;
                    CAutoLock al(f->m_csFile);
                    f->m_deqFileCb.push_back(std::move(fcb));
                    if (f->m_deqFileCb.size() < 2) {
                        int fail = uv_async_send(&f->m_fileType);
                        assert(!fail);
                    }
                };
            } else if (!IsNullOrUndefined(argv[2])) {
                ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
                return 0;
            }
        }
        if (args > 3) {
            if (argv[3]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[3])));
                se = [func, download](CAsyncServiceHandler* file, unsigned short requestId, const wchar_t* errMessage, const char* errWhere, unsigned int errCode) {
                    FileCb fcb(download, tagFileEvent::feException);
                    fcb.Func = func;
                    PSFile f = (PSFile) file;
                    *fcb.Buffer << f << requestId << errMessage << errWhere << errCode;
                    CAutoLock al(f->m_csFile);
                    f->m_deqFileCb.push_back(std::move(fcb));
                    if (f->m_deqFileCb.size() < 2) {
                        int fail = uv_async_send(&f->m_fileType);
                        assert(!fail);
                    }
                };
            } else if (!IsNullOrUndefined(argv[3])) {
                ThrowException(isolate, "A callback expected for tracking exception from server");
                return 0;
            }
        }
        if (download)
            return Download(localFile, remoteFile, dd, trans, aborted, flags, se) ? index : INVALID_NUMBER;
        return Upload(localFile, remoteFile, dd, trans, aborted, flags, se) ? index : INVALID_NUMBER;
    }

    void CSFile::file_cb(uv_async_t * handle) {
        CSFile* obj = (CSFile*) handle->data; //sender
        assert(obj);
        if (!obj) return;
        Isolate* isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate); //required for Node 4.x
        {
            CUCriticalSection& cs = obj->m_csFile;
            cs.lock();
            while (obj->m_deqFileCb.size()) {
                FileCb cb(std::move(obj->m_deqFileCb.front()));
                obj->m_deqFileCb.pop_front();
                cs.unlock();
                PSFile processor;
                *cb.Buffer >> processor;
                assert(processor);
                Local<Function> func = Local<Function>::New(isolate, *cb.Func);
                //Local<Object> njFile = NJFile::New(isolate, processor, true);
                Local<v8::Boolean> download = v8::Boolean::New(isolate, cb.Download);
                switch (cb.EventType) {
                    case tagFileEvent::feExchange:
                    {
                        int res;
                        SPA::CDBString errMsg;
                        *cb.Buffer >> res >> errMsg;
                        Local<Value> jsRes = v8::Int32::New(isolate, res);
                        Local<v8::String> jsMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                        Local<Value> argv[] = {jsMsg, jsRes, download};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                    }
                        break;
                    case tagFileEvent::feTrans:
                    {
                        SPA::UINT64 pos, size;
                        *cb.Buffer >> pos >> size;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> argv[] = {v8::Number::New(isolate, (double) pos), v8::Number::New(isolate, (double) size), download};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                    }
                        break;
                    case tagFileEvent::feDiscarded:
                    {
                        bool canceled;
                        *cb.Buffer >> canceled;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> argv[] = {v8::Boolean::New(isolate, canceled), download};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                    }
                        break;
                    case tagFileEvent::feException:
                        if (!func.IsEmpty()) {
                            unsigned short reqId;
                            SPA::CDBString errMsg;
                            std::string errWhere;
                            int errCode;
                            *cb.Buffer >> reqId >> errMsg >> errWhere >> errCode;
                            assert(!cb.Buffer->GetSize());
                            Local<String> jsMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                            Local<String> jsWhere = ToStr(isolate, errWhere.c_str());
                            Local<Value> jsCode = Number::New(isolate, errCode);
                            Local<Value> argv[] = {jsMsg, jsCode, jsWhere, Number::New(isolate, reqId), download};
                            func->Call(isolate->GetCurrentContext(), Null(isolate), 5, argv);
                        }
                        break;
                    default:
                        assert(false); //shouldn't come here
                        break;
                }
                CScopeUQueue::Unlock(cb.Buffer);
                cs.lock();
            }
            cs.unlock();
        }
        isolate->RunMicrotasks(); //may speed up pumping
    }

    void CSFile::SetLoop() {
        ::memset(&m_fileType, 0, sizeof (m_fileType));
        m_fileType.data = this;
        int fail = uv_async_init(uv_default_loop(), &m_fileType, file_cb);
        assert(!fail);
    }
}