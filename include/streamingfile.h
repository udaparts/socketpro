
#ifndef _SOCKETPRO_STREAMING_FILE_H_
#define _SOCKETPRO_STREAMING_FILE_H_

#include <deque>
#include "file/ufile.h"
#include "aclientw.h"
#ifndef WIN32_64
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace SPA {
    namespace ClientSide {

        class CStreamingFile : public CAsyncServiceHandler {
        public:
            typedef std::function<void(CStreamingFile *file, int res, const std::wstring &errMsg) > DDownload;
            typedef std::function<void(CStreamingFile *file, UINT64 transferred) > DTransferring;
            typedef DDownload DUpload;

            static const unsigned int MAX_FILES_STREAMED = 32;

            CStreamingFile(CClientSocket * cs) : CAsyncServiceHandler(SFile::sidFile, cs), m_MaxDownloading(1) {
            }

            ~CStreamingFile() {
                CleanCallbacks();
            }

        protected:
            //You may use the protected constructor when extending this class

            CStreamingFile(unsigned int sid, CClientSocket * cs) : CAsyncServiceHandler(sid, cs) {
            }

        private:

            struct CContext {

                CContext(bool uplaod, unsigned int flags)
                : Uploading(uplaod), FileSize(~0), Flags(flags), QueueOk(false), ErrorCode(0), Sent(false), InitSize(-1),
#ifdef WIN32_64
                File(INVALID_HANDLE_VALUE)
#else
                File(-1)
#endif
                {
                }
                bool Uploading;
                UINT64 FileSize;
                unsigned int Flags;
                std::wstring LocalFile;
                std::wstring FilePath;
                DDownload Download;
                DTransferring Transferring;
                DDiscarded Discarded;
                bool QueueOk;
                int ErrorCode;
                bool Sent;
                INT64 InitSize;
                std::wstring ErrMsg;
                DServerException Se;
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE
                std::shared_ptr<std::promise<ErrInfo> > Promise;
#endif
#endif

#ifdef WIN32_64
                HANDLE File;
#else
                int File;
#endif

                inline INT64 GetFilePos() {
#ifdef WIN32_64
                    LARGE_INTEGER dM, newPos;
                    dM.QuadPart = 0;
                    auto ok = ::SetFilePointerEx(File, dM, &newPos, FILE_CURRENT);
                    assert(ok);
                    return newPos.QuadPart;
#else
                    auto newPos = ::lseek64(File, 0, SEEK_CUR);
                    assert(newPos != -1);
                    return newPos;
#endif
                }

                inline bool IsOpen() const {
#ifdef WIN32_64
                    return (File != INVALID_HANDLE_VALUE);
#else
                    return (File != -1);
#endif
                }

                inline bool HasError() const {
                    return (ErrorCode || ErrMsg.size());
                }

            };

        public:

            virtual unsigned int CleanCallbacks() {
                std::deque<CContext> vContext;
                m_csFile.lock();
                vContext.swap(m_vContext);
                m_csFile.unlock();
                for (auto it = vContext.begin(), end = vContext.end(); it != end; ++it) {
                    if (it->IsOpen()) {
                        CloseFile(*it);
                    }
                    if (it->Discarded) {
                        try {
                            it->Discarded(this, GetSocket()->GetCurrentRequestID() == idCancel);
                        } catch (...) {
                        }
                    }
                }
                return CAsyncServiceHandler::CleanCallbacks();
            }

            unsigned int GetFilesStreamed() {
                CAutoLock al(m_csFile);
                return m_MaxDownloading;
            }

            void SetFilesStreamed(unsigned int max) {
                if (max == 0) {
                    max = 1;
                } else if (max > MAX_FILES_STREAMED) {
                    max = MAX_FILES_STREAMED;
                }
                CAutoLock al(m_csFile);
                m_MaxDownloading = max;
            }

            size_t GetFilesQueued() {
                CAutoLock al(m_csFile);
                return m_vContext.size();
            }

            UINT64 GetFileSize() {
                UINT64 file_size = (~0);
                CAutoLock al(m_csFile);
                if (m_vContext.size()) {
                    file_size = m_vContext.front().FileSize;
                }
                return file_size;
            }

            size_t Cancel() {
                size_t canceled = 0;
                CAutoLock al(m_csFile);
                while (m_vContext.size()) {
                    auto &back = m_vContext.back();
                    if (back.IsOpen()) {
                        //Send an interrupt request onto server to shut down downloading as earlier as possible
                        Interrupt(DEFAULT_INTERRUPT_OPTION);
                        break;
                    }
                    if (back.Discarded) {
                        try {
                            m_csFile.unlock();
                            back.Discarded(this, true);
                        } catch (...) {
                        }
                        m_csFile.lock();
                    }
                    m_vContext.pop_back();
                    ++canceled;
                }
                return canceled;
            }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE

#ifdef HAVE_COROUTINE
        private:

            struct Awaiter : public CWaiter<ErrInfo> {

                Awaiter(CStreamingFile* file, unsigned short reqId, CContext &ctx)
                : CWaiter<ErrInfo>(reqId) {
                    ctx.Discarded = get_aborted();
                    ctx.Se = get_se();
                    ctx.Download = [this](CStreamingFile* file, int res, const std::wstring & errMsg) {
                        m_r.ec = res;
                        m_r.em = errMsg;
                        resume();
                    };
                    CAutoLock al(file->m_csFile);
                    file->m_vContext.push_back(ctx);
                    unsigned int filesOpened = file->GetFilesOpened();
                    if (file->m_MaxDownloading > filesOpened) {
                        ClientCoreLoader.PostProcessing(file->GetSocket()->GetHandle(), 0, 0);
                        if (!filesOpened) {
                            //make sure WaitAll works correctly
                            file->GetSocket()->DoEcho();
                        }
                    }
                }
            };

        public:

            auto wait_upload(const wchar_t* localFile, const wchar_t* remoteFile, DTransferring progress = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED) {
                if (!localFile || !::wcslen(localFile)) {
                    throw std::invalid_argument("Parameter localFile cannot be empty");
                }
                if (!remoteFile || !::wcslen(remoteFile)) {
                    throw std::invalid_argument("Parameter remoteFile cannot be empty");
                }
                CContext context(true, flags);
                context.Transferring = progress;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                return Awaiter(this, SFile::idUpload, context);
            }

            auto wait_download(const wchar_t* localFile, const wchar_t* remoteFile, DTransferring progress = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED) {
                if (!localFile || !::wcslen(localFile)) {
                    throw std::invalid_argument("Parameter localFile cannot be empty");
                }
                if (!remoteFile || !::wcslen(remoteFile)) {
                    throw std::invalid_argument("Parameter remoteFile cannot be empty");
                }
                CContext context(false, flags);
                context.Transferring = progress;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                return Awaiter(this, SFile::idDownload, context);
            }
#endif

            virtual std::future<ErrInfo> upload(const wchar_t *localFile, const wchar_t *remoteFile, DTransferring progress = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED) {
                if (!localFile || !::wcslen(localFile)) {
                    throw std::invalid_argument("Parameter localFile cannot be empty");
                }
                if (!remoteFile || !::wcslen(remoteFile)) {
                    throw std::invalid_argument("Parameter remoteFile cannot be empty");
                }
                CContext context(true, flags);
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                context.Download = [prom](CStreamingFile *file, int res, const std::wstring & errMsg) {
                    ErrInfo ei(res, errMsg.c_str());
                    try {
                        prom->set_value(ei);
                    } catch (std::future_error&) {
                        //ignore
                    }
                };
                context.Transferring = progress;
                context.Discarded = get_aborted(prom, SFile::idUpload);
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                context.Se = get_se(prom);
                context.Promise = prom;
                CAutoLock al(m_csFile);
                m_vContext.push_back(context);
                unsigned int filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened) {
                    ClientCoreLoader.PostProcessing(GetSocket()->GetHandle(), 0, 0);
                    if (!filesOpened) {
                        GetSocket()->DoEcho(); //make sure WaitAll works correctly
                    }
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> download(const wchar_t *localFile, const wchar_t *remoteFile, DTransferring progress = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED) {
                if (!localFile || !::wcslen(localFile)) {
                    throw std::invalid_argument("Parameter localFile cannot be empty");
                }
                if (!remoteFile || !::wcslen(remoteFile)) {
                    throw std::invalid_argument("Parameter remoteFile cannot be empty");
                }
                CContext context(false, flags);
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                context.Download = [prom](CStreamingFile *file, int res, const std::wstring & errMsg) {
                    ErrInfo ei(res, errMsg.c_str());
                    try {
                        prom->set_value(ei);
                    } catch (std::future_error&) {
                        //ignore
                    }
                };
                context.Transferring = progress;
                context.Discarded = get_aborted(prom, SFile::idDownload);
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                context.Se = get_se(prom);
                context.Promise = prom;
                CAutoLock al(m_csFile);
                m_vContext.push_back(context);
                unsigned int filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened) {
                    ClientCoreLoader.PostProcessing(GetSocket()->GetHandle(), 0, 0);
                    if (!filesOpened) {
                        GetSocket()->DoEcho(); //make sure WaitAll works correctly
                    }
                }
                return prom->get_future();
            }
#endif
#endif

            virtual bool Upload(const wchar_t *localFile, const wchar_t *remoteFile, DUpload up = nullptr, DTransferring progress = nullptr, DDiscarded aborted = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED, const DServerException& se = nullptr) {
                if (!localFile || !::wcslen(localFile)) {
                    return false;
                }
                if (!remoteFile || !::wcslen(remoteFile)) {
                    return false;
                }
                CContext context(true, flags);
                context.Download = up;
                context.Transferring = progress;
                context.Discarded = aborted;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                context.Se = se;
                CAutoLock al(m_csFile);
                m_vContext.push_back(context);
                unsigned int filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened) {
                    ClientCoreLoader.PostProcessing(GetSocket()->GetHandle(), 0, 0);
                    if (!filesOpened) {
                        GetSocket()->DoEcho(); //make sure WaitAll works correctly
                    }
                }
                return true;
            }

            virtual bool Download(const wchar_t *localFile, const wchar_t *remoteFile, DDownload dl = nullptr, DTransferring progress = nullptr, DDiscarded aborted = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED, const DServerException& se = nullptr) {
                if (!localFile || !::wcslen(localFile)) {
                    return false;
                }
                if (!remoteFile || !::wcslen(remoteFile)) {
                    return false;
                }
                CContext context(false, flags);
                context.Download = dl;
                context.Transferring = progress;
                context.Discarded = aborted;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                context.Se = se;
                CAutoLock al(m_csFile);
                m_vContext.push_back(context);
                unsigned int filesOpened = GetFilesOpened();
                if (m_MaxDownloading > filesOpened) {
                    ClientCoreLoader.PostProcessing(GetSocket()->GetHandle(), 0, 0);
                    if (!filesOpened) {
                        GetSocket()->DoEcho(); //make sure WaitAll works correctly
                    }
                }
                return true;
            }

        protected:

            virtual void OnPostProcessing(unsigned int hint, UINT64 data) {
                unsigned int d = 0;
                CAutoLock al(m_csFile);
                for (auto it = m_vContext.begin(), end = m_vContext.end(); it != end; ++it) {
                    if (d >= m_MaxDownloading) {
                        break;
                    }
                    if (it->IsOpen()) {
                        if (it->Uploading) {
                            break;
                        } else {
                            ++d;
                            continue;
                        }
                    }
                    if (it->HasError()) {
                        continue;
                    }
                    if (it->Uploading) {
                        OpenLocalRead(*it);
                        if (!it->HasError()) {
                            if (!SendRequest(SFile::idUpload, NULL_RH, it->Discarded, it->Se, it->FilePath, it->Flags, it->FileSize)) {
                                CClientSocket *cs = GetSocket();
                                int ec = cs->GetErrorCode();
                                if (ec) {
                                    it->ErrorCode = ec;
                                    it->ErrMsg = SPA::Utilities::ToWide(cs->GetErrorMsg());
                                } else {
                                    it->ErrorCode = SESSION_CLOSED_BEFORE;
                                    it->ErrMsg = L"Session already closed before sending the request";
                                }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE
                                if (it->Promise) {
                                    try {
                                        it->Promise->set_exception(std::make_exception_ptr(CSocketError(it->ErrorCode, it->ErrMsg.c_str(), SFile::idUpload, true)));
                                    } catch (std::future_error&) {
                                        //ignore
                                    }
                                    it->Se = nullptr;
                                    it->Discarded = nullptr;
                                }
#endif
#endif
                                continue;
                            }
                            break;
                        }
                    } else {
                        OpenLocalWrite(*it);
                        if (!it->HasError()) {
                            if (!SendRequest(SFile::idDownload, NULL_RH, it->Discarded, it->Se, it->LocalFile, it->FilePath, it->Flags, it->InitSize)) {
                                CClientSocket *cs = GetSocket();
                                int ec = cs->GetErrorCode();
                                if (ec) {
                                    it->ErrorCode = ec;
                                    it->ErrMsg = SPA::Utilities::ToWide(cs->GetErrorMsg());
                                } else {
                                    it->ErrorCode = SESSION_CLOSED_BEFORE;
                                    it->ErrMsg = L"Session already closed before sending the request";
                                }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE
                                if (it->Promise) {
                                    try {
                                        it->Promise->set_exception(std::make_exception_ptr(CSocketError(it->ErrorCode, it->ErrMsg.c_str(), SFile::idDownload, true)));
                                    } catch (std::future_error&) {
                                        //ignore
                                    }
                                    it->Se = nullptr;
                                    it->Discarded = nullptr;
                                }
#endif
#endif
                                continue;
                            }
                            ++d;
                        }
                    }
                }
                while (m_vContext.size()) {
                    auto it = m_vContext.begin();
                    if (it->HasError()) {
                        CloseFile(*it);
                        DDownload cb = it->Download;
                        if (cb) {
                            int errCode = it->ErrorCode;
                            std::wstring errMsg = it->ErrMsg;
                            m_csFile.unlock();
                            try {
                                cb(this, errCode, errMsg);
                            } catch (...) {
                            }
                            m_csFile.lock();
                        }
                        m_vContext.pop_front();
                    } else {
                        assert(it->IsOpen());
                        break;
                    }
                }
            }

            virtual void OnMergeTo(CAsyncServiceHandler & to) {
                CStreamingFile &fTo = (CStreamingFile &) to;
                CAutoLock al0(fTo.m_csFile);
                size_t count = fTo.m_vContext.size();
                std::deque<CContext>::iterator pos = fTo.m_vContext.end();
                for (auto it = fTo.m_vContext.begin(); it != pos; ++it) {
                    if (!it->IsOpen() && !it->HasError()) {
                        pos = it;
                        break;
                    }
                }
                {
                    CAutoLock al1(m_csFile);
                    if (m_vContext.size()) {
                        fTo.m_vContext.insert(pos, m_vContext.begin(), m_vContext.end());
                        m_vContext.clear();
                    }
                }
                if (!count) {
                    count = fTo.m_vContext.size();
                    if (count) {
                        ClientCoreLoader.PostProcessing(to.GetSocket()->GetHandle(), 0, 0);
                        to.GetSocket()->DoEcho(); //make sure WaitAll works correctly
                    }
                }
            }

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case SFile::idDownload:
                    {
                        int res;
                        std::wstring errMsg;
                        mc >> res >> errMsg;
                        DDownload dl;
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                dl = context.Download;
                                context.ErrorCode = res;
                                context.ErrMsg = errMsg;
                            }
                        }
                        if (dl) {
                            dl(this, res, errMsg);
                        }
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                CloseFile(context);
                                m_vContext.pop_front();
                            }
                        }
                        OnPostProcessing(0, 0);
                    }
                        break;
                    case SFile::idStartDownloading:
                    {
                        UINT64 fileSize;
                        std::wstring localFile, remoteFile;
                        unsigned int flags;
                        INT64 initSize;
                        mc >> fileSize >> localFile >> remoteFile >> flags >> initSize;
                        CAutoLock al(m_csFile);
                        if (!m_vContext.size()) {
                            CContext context(false, flags);
                            context.LocalFile = localFile;
                            context.FilePath = remoteFile;
                            OpenLocalWrite(context);
                            context.InitSize = initSize;
                            m_vContext.push_back(context);
                        }
                        CContext &context = m_vContext.front();
                        assert(context.LocalFile == localFile);
                        assert(context.FilePath == remoteFile);
                        assert(context.Flags == flags);
                        assert(context.InitSize == initSize);
                        context.FileSize = fileSize;
                        assert(!context.Uploading);
                        initSize = (context.InitSize > 0) ? context.InitSize : 0;
                        if (context.GetFilePos() > initSize) {
#ifdef WIN32_64
                            auto ok = ::FlushFileBuffers(context.File);
                            assert(ok);
                            LARGE_INTEGER moveDis, newPos;
                            moveDis.QuadPart = initSize;
                            ok = ::SetFilePointerEx(context.File, moveDis, &newPos, FILE_BEGIN);
                            assert(ok);
#else
                            auto fail = ::fsync(context.File);
                            assert(!fail);
                            auto newPos = ::lseek64(context.File, initSize, SEEK_SET);
                            assert(newPos != -1);
#endif
                        }
                    }
                        break;
                    case SFile::idDownloading:
                    {
                        DTransferring progress;
                        UINT64 downloaded = 0;
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(!context.Uploading);
                                progress = context.Transferring;
#ifdef WIN32_64
                                DWORD dw = mc.GetSize(), dwWritten;
                                BOOL ok = ::WriteFile(context.File, mc.GetBuffer(), dw, &dwWritten, nullptr);
                                assert(ok);
                                assert(dwWritten == mc.GetSize());
#else

                                auto ret = ::write(context.File, mc.GetBuffer(), mc.GetSize());
                                assert((unsigned int) ret == mc.GetSize());
#endif
                                downloaded = (UINT64) context.GetFilePos();
                            }
                        }
                        if (progress) {
                            progress(this, downloaded);
                        }
                        mc.SetSize(0);
                    }
                        break;
                    case SFile::idUploadBackup:
                        break;
                    case SFile::idUpload:
                    {
                        int res;
                        CContext ctx(false, 0);
                        std::wstring errMsg;
                        mc >> res >> errMsg;
                        if (res || errMsg.size()) {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                mc >> context.InitSize;
                                ctx = m_vContext.front();
                                ctx.ErrMsg = errMsg;
                                ctx.ErrorCode = res;
                                assert(context.Uploading);
                            }
                        } else {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                bool ok;
                                DResultHandler rh;
                                DServerException se = nullptr;
                                CContext &context = m_vContext.front();
                                mc >> context.InitSize;
                                CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), SFile::STREAM_CHUNK_SIZE);
                                context.QueueOk = GetSocket()->GetClientQueue().StartJob();
                                bool queue_enabled = GetSocket()->GetClientQueue().IsAvailable();
                                if (queue_enabled) {
                                    SendRequest(SFile::idUploadBackup, rh, context.Discarded, se, context.FilePath.c_str(), context.Flags, context.FileSize, context.InitSize);
                                }
#ifdef WIN32_64
                                DWORD ret = 0;
                                ok = ::ReadFile(context.File, (LPVOID) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE, &ret, nullptr) ? true : false;
#else
                                int ret = ::read(context.File, (void*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                                ok = (ret != -1);
#endif
                                while (ok && (unsigned int) ret == SFile::STREAM_CHUNK_SIZE) {
                                    if (!SendRequest(SFile::idUploading, sb->GetBuffer(), (unsigned int) ret, rh, context.Discarded, se)) {
                                        ok = false;
                                        break;
                                    }
#ifdef WIN32_64
                                    ret = 0;
                                    ok = ::ReadFile(context.File, (LPVOID) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE, &ret, nullptr) ? true : false;
#else
                                    ret = ::read(context.File, (void*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                                    ok = (ret != -1);
#endif
                                    if (!ok) {
                                        break;
                                    } else if (queue_enabled) {
                                        //save file into client message queue
                                    } else if (GetSocket()->GetBytesInSendingBuffer() > 40 * SFile::STREAM_CHUNK_SIZE || GetSocket()->GetConnectionState() < csConnected) {
                                        break;
                                    }
                                }
                                if (!ok) {
                                } else if (ret > 0) {
                                    ok = SendRequest(SFile::idUploading, sb->GetBuffer(), (unsigned int) ret, rh, context.Discarded, se);
                                }
                                if (ok && (unsigned int) ret < SFile::STREAM_CHUNK_SIZE) {
                                    context.Sent = true;
                                    ok = SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Discarded, se);
                                    if (context.QueueOk) {
                                        GetSocket()->GetClientQueue().EndJob();
                                    }
                                }
                                if (!ok) {
#ifdef WIN32_64
                                    context.ErrorCode = ::GetLastError();
                                    context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
#else
                                    context.ErrorCode = errno;
                                    std::string err = strerror(errno);
                                    context.ErrMsg = Utilities::ToWide(err);
#endif
                                    ctx = m_vContext.front();
                                }

                            }
                        }
                        if (ctx.HasError()) {
                            CloseFile(ctx);
                            if (ctx.Download) {
                                ctx.Download(this, ctx.ErrorCode, ctx.ErrMsg);
                            }
                            {
                                CAutoLock al(m_csFile);
                                m_vContext.pop_front();
                            }
                            if (ctx.QueueOk) {
                                GetSocket()->GetClientQueue().AbortJob();
                            }
                            OnPostProcessing(0, 0);
                        }
                    }
                        break;
                    case SFile::idUploading:
                    {
                        int res = 0;
                        std::wstring errMsg;
                        CContext ctx(false, 0);
                        DTransferring progress;
                        INT64 uploaded;
                        mc >> uploaded;
                        if (mc.GetSize() >= sizeof (int) + sizeof (unsigned int)) {
                            mc >> res >> errMsg;
                        }
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(context.Uploading);
                                progress = context.Transferring;
                                if (uploaded < 0 || res || errMsg.size()) {
                                    assert(context.QueueOk);
                                    context.ErrorCode = res;
                                    context.ErrMsg = errMsg;
                                    ctx = context;
                                } else if (!context.Sent) {
                                    bool ok;
                                    CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), SFile::STREAM_CHUNK_SIZE);
#ifdef WIN32_64
                                    DWORD ret = 0;
                                    ok = ::ReadFile(context.File, (LPVOID) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE, &ret, nullptr) ? true : false;
#else
                                    int ret = ::read(context.File, (void*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                                    ok = (ret != -1);
#endif
                                    assert(ok);
                                    DResultHandler rh;
                                    DServerException se = nullptr;
                                    if (!ok) {
                                    } else if (ret > 0) {
                                        ok = SendRequest(SFile::idUploading, sb->GetBuffer(), (unsigned int) ret, rh, context.Discarded, se);
                                    }
                                    if (ok && (unsigned int) ret < SFile::STREAM_CHUNK_SIZE) {
                                        context.Sent = true;
                                        ok = SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Discarded, se);
                                    }
                                    if (!ok) {
#ifdef WIN32_64
                                        context.ErrorCode = ::GetLastError();
                                        context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
#else
                                        context.ErrorCode = errno;
                                        std::string err = strerror(errno);
                                        context.ErrMsg = Utilities::ToWide(err);
#endif
                                        ctx = context;
                                    }
                                }
                            }
                        }
                        if (ctx.HasError()) {
                            CloseFile(ctx);
                            if (ctx.Download) {
                                ctx.Download(this, ctx.ErrorCode, ctx.ErrMsg);
                            }
                            {
                                CAutoLock al(m_csFile);
                                m_vContext.pop_front();
                            }
                            OnPostProcessing(0, 0);
                        } else if (progress) {
                            progress(this, (UINT64) uploaded);
                        }
                    }
                        break;
                    case SFile::idUploadCompleted:
                    {
                        DDownload upl;
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                if (context.IsOpen()) {
                                    assert(context.Uploading);
                                    upl = context.Download;
                                } else {
                                    assert(context.QueueOk);
                                    context.Sent = false;
                                    context.QueueOk = false;
                                }
                            }
                        }
                        if (upl) {
                            upl(this, 0, L"");
                        }
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                if (context.IsOpen()) {
                                    CloseFile(context);
                                    m_vContext.pop_front();
                                }
                            }
                        }
                        OnPostProcessing(0, 0);
                    }
                        break;
                    default:
                        CAsyncServiceHandler::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:

            unsigned int GetFilesOpened() {
                unsigned int opened = 0;
                for (auto it = m_vContext.cbegin(), end = m_vContext.cend(); it != end; ++it) {
                    if (it->IsOpen()) {
                        ++opened;
                    } else if (!it->HasError()) {
                        break;
                    }
                }
                return opened;
            }

            static void CloseFile(CContext &context) {
#ifdef WIN32_64
                if (context.File != INVALID_HANDLE_VALUE) {
                    if (!context.Uploading && context.HasError()) {
                        if (context.InitSize == -1) {
                            ::CloseHandle(context.File);
                            ::DeleteFileW(context.LocalFile.c_str());
                        } else {
                            BOOL ok = ::FlushFileBuffers(context.File);
                            assert(ok);
                            LARGE_INTEGER moveDis, newPos;
                            moveDis.QuadPart = context.InitSize;
                            ok = ::SetFilePointerEx(context.File, moveDis, &newPos, FILE_BEGIN);
                            assert(ok);
                            ok = ::SetEndOfFile(context.File);
                            assert(ok);
                            ::CloseHandle(context.File);
                        }
                    } else {
                        ::CloseHandle(context.File);
                    }
                    context.File = INVALID_HANDLE_VALUE;
                }
#else
                if (context.File != -1) {
                    if (!context.Uploading && context.HasError()) {
                        std::string path = Utilities::ToUTF8(context.LocalFile);
                        if (context.InitSize == -1) {
                            ::close(context.File);
                            auto fail = ::remove(path.c_str());
                            assert(!fail);
                        } else {
                            auto fail = ::fsync(context.File);
                            assert(!fail);
                            auto newPos = ::lseek64(context.File, context.InitSize, SEEK_SET);
                            assert(newPos != -1);
                            fail = ::ftruncate(context.File, newPos);
                            assert(!fail);
                            ::close(context.File);
                        }
                    } else {
                        ::close(context.File);
                    }
                    context.File = -1;
                }
#endif
            }

            void OpenLocalWrite(CContext &context) {
                do {
                    bool existing = false;
#ifdef WIN32_64
                    DWORD sm = 0;
                    if ((context.Flags & SFile::FILE_OPEN_SHARE_WRITE) == SFile::FILE_OPEN_SHARE_WRITE) {
                        sm |= FILE_SHARE_WRITE;
                    }
                    context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_WRITE, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (context.File == INVALID_HANDLE_VALUE) {
                        context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_WRITE, sm, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                        if (context.File == INVALID_HANDLE_VALUE) {
                            context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                            context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                            break;
                        }
                    } else {
                        existing = true;
                    }
                    if (existing) {
                        context.InitSize = 0;
                        if ((context.Flags & SFile::FILE_OPEN_TRUNCACTED) == SFile::FILE_OPEN_TRUNCACTED) {
                            if (!::SetEndOfFile(context.File)) {
                                context.ErrorCode = ::GetLastError();
                                context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                                break;
                            }
                        } else if ((context.Flags & SFile::FILE_OPEN_APPENDED) == SFile::FILE_OPEN_APPENDED) {
                            LARGE_INTEGER dis, pos;
                            dis.QuadPart = 0;
                            if (!::SetFilePointerEx(context.File, dis, &pos, FILE_END)) {
                                context.ErrorCode = ::GetLastError();
                                context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                            } else {
                                context.InitSize = pos.QuadPart;
                            }
                        }
                    }
#else
                    std::string s = Utilities::ToUTF8(context.LocalFile.c_str(), context.LocalFile.size());
                    int mode = (O_WRONLY | O_CREAT | O_EXCL);
                    if ((context.Flags & SFile::FILE_OPEN_TRUNCACTED) == SFile::FILE_OPEN_TRUNCACTED) {
                        mode |= O_TRUNC;
                    } else if ((context.Flags & SFile::FILE_OPEN_APPENDED) == SFile::FILE_OPEN_APPENDED) {
                        mode |= O_APPEND;
                    }
                    mode_t m = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
                    context.File = ::open(s.c_str(), mode, m);
                    if (context.File == -1) {
                        existing = true;
                        mode = (O_WRONLY | O_CREAT);
                        context.File = ::open(s.c_str(), mode, m);
                        if (context.File == -1) {
                            context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                            std::string err = strerror(errno);
                            context.ErrMsg = Utilities::ToWide(err);
                            break;
                        }
                    }
                    if (existing) {
                        context.InitSize = context.GetFilePos();
                    }
                    if ((context.Flags & SFile::FILE_OPEN_SHARE_WRITE) == 0) {
                        struct flock fl;
                        fl.l_whence = SEEK_SET;
                        fl.l_start = 0;
                        fl.l_len = 0;
                        fl.l_type = F_WRLCK;
                        fl.l_pid = ::getpid();
                        if (::fcntl(context.File, F_SETLKW, &fl) == -1) {
                            context.ErrorCode = errno;
                            std::string err = strerror(errno);
                            context.ErrMsg = Utilities::ToWide(err);
                        }
                    }
#endif
                } while (false);
            }

            void OpenLocalRead(CContext &context) {
                do {
#ifdef WIN32_64
                    DWORD sm = 0;
                    if ((context.Flags & SFile::FILE_OPEN_SHARE_READ) == SFile::FILE_OPEN_SHARE_READ) {
                        sm |= FILE_SHARE_READ;
                    }
                    context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_READ, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (context.File == INVALID_HANDLE_VALUE) {
                        context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                        context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                        break;
                    }
                    LARGE_INTEGER li;
                    if (!::GetFileSizeEx(context.File, &li)) {
                        context.ErrorCode = ::GetLastError();
                        context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                        break;
                    }
                    context.FileSize = (UINT64) li.QuadPart;
#else
                    std::string s = Utilities::ToUTF8(context.LocalFile.c_str(), context.LocalFile.size());
                    context.File = ::open(s.c_str(), O_RDONLY);
                    if (context.File == -1) {
                        context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                        std::string err = strerror(errno);
                        context.ErrMsg = Utilities::ToWide(err);
                        break;
                    }
                    if ((context.Flags & SFile::FILE_OPEN_SHARE_READ) == 0) {
                        struct flock fl;
                        fl.l_whence = SEEK_SET;
                        fl.l_start = 0;
                        fl.l_len = 0;
                        fl.l_type = F_RDLCK;
                        fl.l_pid = ::getpid();
                        if (::fcntl(context.File, F_SETLKW, &fl) == -1) {
                            context.ErrorCode = errno;
                            std::string err = strerror(errno);
                            context.ErrMsg = Utilities::ToWide(err);
                            break;
                        }
                    }
                    struct stat st;
                    static_assert(sizeof (st.st_size) >= sizeof (UINT64), "Big file not supported");
                    if (::fstat(context.File, &st) == -1) {
                        context.ErrorCode = errno;
                        std::string err = strerror(errno);
                        context.ErrMsg = Utilities::ToWide(err);
                        break;
                    }
                    context.FileSize = st.st_size;
#endif
                } while (false);
            }

        protected:
            CUCriticalSection m_csFile;

        private:
            std::deque<CContext> m_vContext; //protected by m_csFile;
            unsigned int m_MaxDownloading; //protected by m_csFile;
        };
        typedef CSocketPool<CStreamingFile> CStreamingFilePool;
    } //ClientSide
} //SPA

#endif
