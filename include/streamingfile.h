
#ifndef _SOCKETPRO_STREAMING_FILE_H_
#define _SOCKETPRO_STREAMING_FILE_H_

#include <deque>
#include "file/ufile.h"
#include "aclientw.h"
#ifndef WIN32_64
#include <fcntl.h>
#include <sys/stat.h>
#endif

namespace SPA {
    namespace ClientSide {

        class CStreamingFile : public CAsyncServiceHandler {
        public:
            typedef std::function<void(CStreamingFile *file, int res, const std::wstring &errMsg) > DDownload;
            typedef std::function<void(CStreamingFile *file, UINT64 transferred) > DTransferring;
            typedef DDownload DUpload;

            CStreamingFile(CClientSocket * cs) : CAsyncServiceHandler(SFile::sidFile, cs) {
            }

        protected:
            //You may use the protected constructor when extending this class

            CStreamingFile(unsigned int sid, CClientSocket * cs) : CAsyncServiceHandler(sid, cs) {
            }

        private:

            struct CContext {

                CContext(bool uplaod, unsigned int flags)
                : Uploading(uplaod), FileSize(~0), Flags(flags), QueueOk(false), ErrorCode(0), Sent(false), Finished(0),
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
                UINT64 Finished;
#ifdef WIN32_64
                HANDLE File;
#else
                int File;
#endif
                std::wstring ErrMsg;

                inline bool IsOpen() const {
#ifdef WIN32_64
                    return (File != INVALID_HANDLE_VALUE);
#else
                    return (File != -1);
#endif
                }

            };

        public:

            virtual unsigned int CleanCallbacks() {
                {
                    CAutoLock al(m_csFile);
                    for (auto it = m_vContext.begin(), end = m_vContext.end(); it != end; ++it) {
                        CloseFile(*it);
                    }
                    m_vContext.clear();
                }
                return CAsyncServiceHandler::CleanCallbacks();
            }

            size_t GetFilesQueued() {
                CAutoLock al(m_csFile);
                return m_vContext.size();
            }

            UINT64 GetFileSize() {
                UINT64 file_size = (~0);
                CAutoLock al(m_csFile);
                if (m_vContext.size())
                    file_size = m_vContext.front().FileSize;
                return file_size;
            }

            bool Upload(const wchar_t *localFile, const wchar_t *remoteFile, DUpload up = nullptr, DTransferring trans = nullptr, DDiscarded aborted = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED) {
                if (!localFile || !::wcslen(localFile))
                    return false;
                if (!remoteFile || !::wcslen(remoteFile))
                    return false;
                CContext context(true, flags);
                context.Download = up;
                context.Transferring = trans;
                context.Discarded = aborted;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                CAutoLock al(m_csFile);
                m_vContext.push_back(context);
                if (m_vContext.size() == 1) {
                    ClientCoreLoader.PostProcessing(GetAttachedClientSocket()->GetHandle(), 0, 0);
                    GetAttachedClientSocket()->DoEcho(); //make sure WaitAll works correctly
                }
                return true;
            }

            bool Download(const wchar_t *localFile, const wchar_t *remoteFile, DDownload dl = nullptr, DTransferring trans = nullptr, DDiscarded aborted = nullptr, unsigned int flags = SFile::FILE_OPEN_TRUNCACTED) {
                if (!localFile || !::wcslen(localFile))
                    return false;
                if (!remoteFile || !::wcslen(remoteFile))
                    return false;
                CContext context(false, flags);
                context.Download = dl;
                context.Transferring = trans;
                context.Discarded = aborted;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                CAutoLock al(m_csFile);
                m_vContext.push_back(context);
                if (m_vContext.size() == 1) {
                    ClientCoreLoader.PostProcessing(GetAttachedClientSocket()->GetHandle(), 0, 0);
                    GetAttachedClientSocket()->DoEcho(); //make sure WaitAll works correctly
                }
                return true;
            }

        protected:

            virtual void OnPostProcessing(unsigned int hint, UINT64 data) {
                ResultHandler rh;
                DServerException se = nullptr;
                CContext ctx(false, 0);
                {
                    CAutoLock al(m_csFile);
                    if (m_vContext.size()) {
                        CContext &context = m_vContext.front();
                        if (context.Uploading) {
                            OpenLocalRead(context);
                        } else {
                            OpenLocalWrite(context);
                        }
                        if (context.ErrorCode || context.ErrMsg.size()) {
                            ctx = m_vContext.front();
                        } else if (context.Uploading) {
                            if (!SendRequest(SFile::idUpload, context.FilePath.c_str(), context.Flags, context.FileSize, rh, context.Discarded, se)) {
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
                        } else {
                            //downloading
                            if (!SendRequest(SFile::idDownload, context.FilePath.c_str(), context.Flags, rh, context.Discarded, se)) {
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
                }
                if (ctx.ErrorCode || ctx.ErrMsg.size()) {
                    CloseFile(ctx);
                    if (ctx.Download) {
                        ctx.Download(this, ctx.ErrorCode, ctx.ErrMsg);
                    }
                    CAutoLock al(m_csFile);
                    m_vContext.pop_front();
                    if (m_vContext.size()) {
                        //post processing the next one
                        ClientCoreLoader.PostProcessing(GetAttachedClientSocket()->GetHandle(), 0, 0);
                        GetAttachedClientSocket()->DoEcho(); //make sure WaitAll works correctly
                    }
                }
            }

            virtual void OnMergeTo(CAsyncServiceHandler & to) {
                CStreamingFile &fTo = (CStreamingFile &) to;
                CAutoLock al0(fTo.m_csFile);
                {
                    CAutoLock al1(m_csFile);
                    for (auto it = m_vContext.begin(), end = m_vContext.end(); it != end; ++it) {
                        fTo.m_vContext.push_back(*it);
                    }
                    m_vContext.clear();
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
                            }
                        }
                        if (dl)
                            dl(this, res, errMsg);
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
                        CAutoLock al(m_csFile);
                        if (m_vContext.size()) {
                            CContext &context = m_vContext.front();
                            assert(!context.Uploading);
                            if (context.Finished) {
#ifdef WIN32_64
                                BOOL ok = FlushFileBuffers(context.File);
                                assert(ok);
                                LARGE_INTEGER moveDis, newPos;
                                moveDis.QuadPart = -((INT64) context.Finished);
                                ok = SetFilePointerEx(context.File, moveDis, &newPos, FILE_END);
                                assert(ok);
                                ok = SetEndOfFile(context.File);
                                assert(ok);
                                context.Finished = 0;
#else

#endif
                            }
                            mc >> context.FileSize;
                        } else {
                            mc.SetSize(0);
                        }
                    }
                        break;
                    case SFile::idDownloading:
                    {
                        DTransferring trans;
                        UINT64 downloaded = 0;
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(!context.Uploading);
                                trans = context.Transferring;
                                context.Finished += mc.GetSize();
#ifdef WIN32_64
                                if (context.File != INVALID_HANDLE_VALUE) {
                                    DWORD dw = mc.GetSize(), dwWritten;
                                    BOOL ok = ::WriteFile(context.File, mc.GetBuffer(), dw, &dwWritten, nullptr);
                                    assert(ok);
                                    assert(dwWritten == mc.GetSize());
                                }
#else
                                if (context.File != -1) {
                                    auto ret = ::write(context.File, mc.GetBuffer(), mc.GetSize());
                                    assert((unsigned int) ret == mc.GetSize());
                                }
#endif
                                downloaded = context.Finished;
                            }
                        }
                        if (trans)
                            trans(this, downloaded);
                        mc.SetSize(0);
                    }
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
                                ctx = m_vContext.front();
                                ctx.ErrMsg = errMsg;
                                ctx.ErrorCode = res;
                                assert(context.Uploading);
                            }
                        } else {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                bool ok;
                                ResultHandler rh;
                                DServerException se = nullptr;
                                CContext &context = m_vContext.front();
                                CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), SFile::STREAM_CHUNK_SIZE);
                                context.QueueOk = GetAttachedClientSocket()->GetClientQueue().StartJob();
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
                                    } else if (context.QueueOk) {
                                        //save file into client message queue
                                    } else if (GetAttachedClientSocket()->GetBytesInSendingBuffer() > 40 * SFile::STREAM_CHUNK_SIZE || GetAttachedClientSocket()->GetConnectionState() < csConnected) {
                                        break;
                                    }
                                }
                                if (!ok) {
                                } else if (ret > 0) {
                                    ok = SendRequest(SFile::idUploading, sb->GetBuffer(), (unsigned int) ret, rh, context.Discarded, se);
                                }
                                if (ok && ret < SFile::STREAM_CHUNK_SIZE) {
                                    context.Sent = true;
                                    ok = SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Discarded, se);
                                    if (context.QueueOk) {
                                        GetAttachedClientSocket()->GetClientQueue().EndJob();
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
                        if (ctx.ErrorCode || ctx.ErrMsg.size()) {
                            CloseFile(ctx);
                            if (ctx.Download) {
                                ctx.Download(this, ctx.ErrorCode, ctx.ErrMsg);
                            }
                            {
                                CAutoLock al(m_csFile);
                                m_vContext.pop_front();
                            }
                            if (ctx.QueueOk) {
                                GetAttachedClientSocket()->GetClientQueue().AbortJob();
                            }
                            OnPostProcessing(0, 0);
                        }
                    }
                        break;
                    case SFile::idUploading:
                    {
                        CContext ctx(false, 0);
                        DTransferring trans;
                        INT64 uploaded;
                        mc >> uploaded;
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(context.Uploading);
                                trans = context.Transferring;
                                if (uploaded < 0) {
                                    assert(context.QueueOk);
                                    CloseFile(context);
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
                                    ResultHandler rh;
                                    DServerException se = nullptr;
                                    if (!ok) {
                                    } else if (ret > 0) {
                                        ok = SendRequest(SFile::idUploading, sb->GetBuffer(), (unsigned int) ret, rh, context.Discarded, se);
                                    }
                                    if (ok && ret < SFile::STREAM_CHUNK_SIZE) {
                                        context.Sent = true;
                                        ok = SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Discarded, se);
                                        if (context.QueueOk) {
                                            GetAttachedClientSocket()->GetClientQueue().EndJob();
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
                        }
                        if (ctx.ErrorCode || ctx.ErrMsg.size()) {
                            CloseFile(ctx);
                            if (ctx.Download) {
                                ctx.Download(this, ctx.ErrorCode, ctx.ErrMsg);
                            }
                            m_vContext.pop_front();
                            OnPostProcessing(0, 0);
                        } else if (trans) {
                            trans(this, (UINT64) uploaded);
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
                        if (upl)
                            upl(this, 0, L"");
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

            static void CloseFile(CContext &context) {
#ifdef WIN32_64
                if (context.File != INVALID_HANDLE_VALUE) {
                    ::CloseHandle(context.File);
                    context.File = INVALID_HANDLE_VALUE;
                    if (!context.Uploading && (context.ErrorCode || context.ErrMsg.size())) {
                        ::DeleteFileW(context.LocalFile.c_str());
                    }
                }
#else
                if (context.File != -1) {
                    ::close(context.File);
                    context.File = -1;
                    if (!context.Uploading && (context.ErrorCode || context.ErrMsg.size())) {
                        std::string path = Utilities::ToUTF8(it->LocalFile.c_str(), it->LocalFile.size());
                        unlink(path.c_str());
                    }
                }
#endif
            }

            void OpenLocalWrite(CContext &context) {
                do {
#ifdef WIN32_64
                    DWORD sm = 0;
                    if ((context.Flags & SFile::FILE_OPEN_SHARE_WRITE) == SFile::FILE_OPEN_SHARE_WRITE)
                        sm |= FILE_SHARE_WRITE;
                    context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_WRITE, sm, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (context.File == INVALID_HANDLE_VALUE) {
                        context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                        context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                        break;
                    }
                    if ((context.Flags & SFile::FILE_OPEN_TRUNCACTED) == SFile::FILE_OPEN_TRUNCACTED) {
                        if (!::SetEndOfFile(context.File)) {
                            context.ErrorCode = ::GetLastError();
                            context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                        }
                    } else if ((context.Flags & SFile::FILE_OPEN_APPENDED) == SFile::FILE_OPEN_APPENDED) {
                        LARGE_INTEGER dis, pos;
                        dis.QuadPart = 0;
                        if (!::SetFilePointerEx(context.File, dis, &pos, FILE_END)) {
                            context.ErrorCode = ::GetLastError();
                            context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                        }
                    }
#else
                    std::string s = Utilities::ToUTF8(context.LocalFile.c_str(), context.LocalFile.size());
                    int mode = (O_WRONLY | O_CREAT);
                    if ((context.Flags & SFile::FILE_OPEN_TRUNCACTED) == SFile::FILE_OPEN_TRUNCACTED) {
                        mode |= O_TRUNC;
                    } else if ((context.Flags & SFile::FILE_OPEN_APPENDED) == SFile::FILE_OPEN_APPENDED) {
                        mode |= O_APPEND;
                    }
                    mode_t m = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
                    context.File = ::open(s.c_str(), mode, m);
                    if (context.File == -1) {
                        context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                        std::string err = strerror(errno);
                        context.ErrMsg = Utilities::ToWide(err);
                        break;
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
                    if ((context.Flags & SFile::FILE_OPEN_SHARE_READ) == SFile::FILE_OPEN_SHARE_READ)
                        sm |= FILE_SHARE_READ;
                    context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_READ, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (context.File == INVALID_HANDLE_VALUE) {
                        context.ErrorCode = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                        context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                        break;
                    }
                    LARGE_INTEGER li;
                    if (!GetFileSizeEx(context.File, &li)) {
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
        };
        typedef CSocketPool<CStreamingFile> CStreamingFilePool;
    } //ClientSide
} //SPA

#endif
