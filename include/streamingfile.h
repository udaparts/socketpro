
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
            typedef DDownload DUpload;
            typedef std::function<void(CStreamingFile *file, UINT64 transferred) > DTransferring;

            CStreamingFile(CClientSocket * cs = nullptr) : CAsyncServiceHandler(SFile::sidFile, cs) {
            }

        protected:
            //You may use the protected constructor when extending this class

            CStreamingFile(unsigned int sid, CClientSocket * cs = nullptr) : CAsyncServiceHandler(sid, cs) {
            }

        private:

            struct CContext {

                CContext(bool uplaod, unsigned int flags)
                : Tried(false), Uploading(uplaod), FileSize(~0), Flags(flags), Sent(false),
#ifdef WIN32_64
                File(INVALID_HANDLE_VALUE)
#else
                File(-1)
#endif
                {
                }
                bool Tried;
                bool Uploading;
                UINT64 FileSize;
                unsigned int Flags;
                bool Sent;
                std::wstring LocalFile;
                std::wstring FilePath;
                DDownload Download;
                DTransferring Transferring;
                DDiscarded Discarded;
#ifdef WIN32_64
                HANDLE File;
#else
                int File;
#endif
                std::wstring ErrMsg;
            };

        public:

            virtual unsigned int CleanCallbacks() {
                {
                    CAutoLock al(m_csFile);
                    for (auto it = m_vContext.begin(), end = m_vContext.end(); it != end; ++it) {
#ifdef WIN32_64
                        if (it->File != INVALID_HANDLE_VALUE)
                            ::CloseHandle(it->File);
                        if (!it->Uploading)
                            ::DeleteFileW(it->LocalFile.c_str());
#else
                        if (it->File != -1)
                            ::close(it->File);
                        if (!it->Uploading) {
                            std::string path = Utilities::ToUTF8(it->LocalFile.c_str(), it->LocalFile.size());
                            unlink(path.c_str());
                        }
#endif
                    }
                    m_vContext.clear();
                }
                return CAsyncServiceHandler::CleanCallbacks();
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
                return Transfer();
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
                return Transfer();
            }

        protected:

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
                                assert(!context.Uploading);
#ifdef WIN32_64
                                if (context.File != INVALID_HANDLE_VALUE) {
                                    ::CloseHandle(context.File);
                                    context.File = INVALID_HANDLE_VALUE;
                                }
#else
                                if (context.File != -1) {
                                    ::close(context.File);
                                    context.File = -1;
                                }
#endif
                                else {
                                    res = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                                    errMsg = context.ErrMsg;
                                }
                                dl = context.Download;
                                m_vContext.pop_front();
                            } else {
                                assert(false);
                            }
                            if (dl)
                                dl(this, res, errMsg);
                        }
                    }
                        break;
                    case SFile::idStartDownloading:
                    {
                        CAutoLock al(m_csFile);
                        if (m_vContext.size()) {
                            CContext &context = m_vContext.front();
                            assert(!context.Uploading);
                            mc >> context.FileSize;
#ifdef WIN32_64
                            DWORD sm = 0;
                            if ((context.Flags & SFile::FILE_OPEN_SHARE_WRITE) == SFile::FILE_OPEN_SHARE_WRITE)
                                sm |= FILE_SHARE_WRITE;
                            context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_WRITE, sm, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                            if (context.File == INVALID_HANDLE_VALUE) {
                                context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                            } else {
                                if ((context.Flags & SFile::FILE_OPEN_TRUNCACTED) == SFile::FILE_OPEN_TRUNCACTED) {
                                    BOOL ok = ::SetEndOfFile(context.File);
                                    assert(ok);
                                } else if ((context.Flags & SFile::FILE_OPEN_APPENDED) == SFile::FILE_OPEN_APPENDED) {
                                    sm = ::SetFilePointer(context.File, 0, nullptr, FILE_END);
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
                                std::string err = strerror(errno);
                                context.ErrMsg = Utilities::ToWide(err.c_str(), err.size());
                            } else if ((context.Flags & SFile::FILE_OPEN_SHARE_WRITE) == 0) {
                                struct flock fl;
                                fl.l_whence = SEEK_SET;
                                fl.l_start = 0;
                                fl.l_len = 0;
                                fl.l_type = F_WRLCK;
                                fl.l_pid = ::getpid();
                                if (::fcntl(context.File, F_SETLKW, &fl) == -1) {
                                    std::string err = strerror(errno);
                                    context.ErrMsg = Utilities::ToWide(err.c_str(), err.size());
                                }
                            }
#endif
                        } else {
                            assert(false);
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
#ifdef WIN32_64
                                if (context.File != INVALID_HANDLE_VALUE) {
                                    DWORD dw = mc.GetSize(), dwWritten;
                                    BOOL ok = ::WriteFile(context.File, mc.GetBuffer(), dw, &dwWritten, nullptr);
                                    assert(ok);
                                    assert(dwWritten == mc.GetSize());
                                    dwWritten = 0;
                                    dw = ::GetFileSize(context.File, &dwWritten);
                                    downloaded = dwWritten;
                                    downloaded <<= 32;
                                    downloaded += dw;
                                }
#else
                                if (context.File != -1) {
                                    auto ret = ::write(context.File, mc.GetBuffer(), mc.GetSize());
                                    assert((unsigned int) ret == mc.GetSize());
                                    struct stat st;
                                    static_assert(sizeof (st.st_size) >= sizeof (UINT64), "Big file not supported");
                                    auto res = ::fstat(context.File, &st);
                                    assert(res != -1);
                                    downloaded = st.st_size;
                                }
#endif
                            } else {
                                assert(false);
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
                        DUpload upl;
                        std::wstring errMsg;
                        mc >> res >> errMsg;
                        if (res) {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(context.Uploading);
#ifdef WIN32_64
                                if (context.File != INVALID_HANDLE_VALUE) {
                                    ::CloseHandle(context.File);
                                    context.File = INVALID_HANDLE_VALUE;
                                }
#else
                                if (context.File != -1) {
                                    ::close(context.File);
                                    context.File = -1;
                                }
#endif
                                upl = context.Download;
                                m_vContext.pop_front();
                            } else {
                                assert(false);
                            }
                        }
                        if (upl)
                            upl(this, res, errMsg);
                    }
                        break;
                    case SFile::idUploading:
                    {
                        DTransferring trans;
                        INT64 uploaded;
                        mc >> uploaded;
                        if (uploaded > 0) {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(context.Uploading);
                                trans = context.Transferring;
                            } else {
                                assert(false);
                            }
                        }
                        if (trans)
                            trans(this, (UINT64) uploaded);
                    }
                        break;
                    case SFile::idUploadCompleted:
                    {
                        DDownload upl;
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(context.Uploading);
#ifdef WIN32_64
                                if (context.File != INVALID_HANDLE_VALUE) {
                                    ::CloseHandle(context.File);
                                    context.File = INVALID_HANDLE_VALUE;
                                }
#else
                                if (context.File != -1) {
                                    ::close(context.File);
                                    context.File = -1;
                                }
#endif
                                upl = context.Download;
                                m_vContext.pop_front();
                            } else {
                                assert(false);
                            }
                        }
                        if (upl)
                            upl(this, 0, L"");
                    }
                        break;
                    default:
                        CAsyncServiceHandler::OnResultReturned(reqId, mc);
                        break;
                }
                CAutoLock al(m_csFile);
                Transfer();
            }

        private:

            static bool IsOpened(const CContext &context) {
#ifdef WIN32_64
                return (context.File != INVALID_HANDLE_VALUE);
#else
                return (context.File != -1);
#endif
            }

            bool Transfer() {
                size_t index = 0;
                ResultHandler rh;
                DServerException se;
                CClientSocket *cs = GetAttachedClientSocket();
                if (!cs->Sendable())
                    return false;
                unsigned int sent_buffer_size = cs->GetBytesInSendingBuffer();
                if (sent_buffer_size > 3 * SFile::STREAM_CHUNK_SIZE)
                    return true;
                while (index < m_vContext.size()) {
                    CContext &context = m_vContext[index];
                    if (context.Sent) {
                        ++index;
                        continue;
                    }
                    if (context.Uploading && context.Tried && !IsOpened(context)) {
                        if (index == 0) {
                            if (context.Download) {
                                context.Download(this, SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg);
                            }
                            m_vContext.erase(m_vContext.begin() + index);
                        } else {
                            ++index;
                        }
                        continue;
                    }
                    if (context.Uploading) {
                        if (!context.Tried) {
                            context.Tried = true;
#ifdef WIN32_64
                            DWORD sm = 0;
                            if ((context.Flags & SFile::FILE_OPEN_SHARE_READ) == SFile::FILE_OPEN_SHARE_READ)
                                sm |= FILE_SHARE_READ;
                            context.File = ::CreateFileW(context.LocalFile.c_str(), GENERIC_READ, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                            if (context.File == INVALID_HANDLE_VALUE) {
                                context.ErrMsg = Utilities::GetErrorMessage(::GetLastError());
                            }
#else
                            std::string s = Utilities::ToUTF8(context.LocalFile.c_str(), context.LocalFile.size());
                            context.File = ::open(s.c_str(), O_RDONLY);
                            if (context.File == -1) {
                                std::string err = strerror(errno);
                                context.ErrMsg = Utilities::ToWide(err.c_str(), err.size());
                            }
#endif
                            if (IsOpened(context)) {
#ifdef WIN32_64
                                DWORD dwHigh = 0;
                                DWORD dw = ::GetFileSize(context.File, &dwHigh);
                                context.FileSize = dwHigh;
                                context.FileSize <<= 32;
                                context.FileSize += dw;
#else
                                int res;
                                if ((context.Flags & SFile::FILE_OPEN_SHARE_READ) == 0) {
                                    struct flock fl;
                                    fl.l_whence = SEEK_SET;
                                    fl.l_start = 0;
                                    fl.l_len = 0;
                                    fl.l_type = F_RDLCK;
                                    fl.l_pid = ::getpid();
                                    res = ::fcntl(context.File, F_SETLKW, &fl);
                                    assert(res != -1);
                                }
                                struct stat st;
                                static_assert(sizeof (st.st_size) >= sizeof (UINT64), "Big file not supported");
                                res = ::fstat(context.File, &st);
                                assert(res != -1);
                                context.FileSize = st.st_size;
#endif
                                IClientQueue &cq = GetAttachedClientSocket()->GetClientQueue();
                                if (cq.IsAvailable()) {
                                    bool ok = cq.StartJob();
                                    assert(ok);
                                }
                                if (!SendRequest(SFile::idUpload, context.FilePath.c_str(), context.Flags, context.FileSize, rh, context.Discarded, se)) {
                                    return false;
                                }
                            }
                        }
                        if (!IsOpened(context)) {
                            if (index == 0) {
                                if (context.Download) {
                                    context.Download(this, SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING, context.ErrMsg);
                                }
                                m_vContext.erase(m_vContext.begin() + index);
                            } else {
                                ++index;
                            }
                            continue;
                        } else {
                            CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), SFile::STREAM_CHUNK_SIZE);
#ifdef WIN32_64
                            DWORD ret = 0;
                            BOOL ok = ::ReadFile(context.File, (LPVOID) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE, &ret, nullptr);
                            assert(ok);
#else
                            int ret = ::read(context.File, (void*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                            assert(ret != -1);
#endif
                            while (ret > 0) {
                                if (!SendRequest(SFile::idUploading, sb->GetBuffer(), (unsigned int) ret, rh, context.Discarded, se)) {
                                    return false;
                                }
                                sent_buffer_size = cs->GetBytesInSendingBuffer();
                                if ((unsigned int) ret < SFile::STREAM_CHUNK_SIZE)
                                    break;
                                if (sent_buffer_size >= 5 * SFile::STREAM_CHUNK_SIZE)
                                    break;
#ifdef WIN32_64
                                ret = 0;
                                ok = ::ReadFile(context.File, (LPVOID) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE, &ret, nullptr);
                                assert(ok);
#else
                                ret = ::read(context.File, (void*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                                assert(ret != -1);
#endif
                            }
                            if ((unsigned int) ret < SFile::STREAM_CHUNK_SIZE) {
                                context.Sent = true;
                                if (!SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Discarded, se)) {
                                    return false;
                                }
                                IClientQueue &cq = GetAttachedClientSocket()->GetClientQueue();
                                if (cq.IsAvailable()) {
                                    ok = cq.EndJob();
                                    assert(ok);
                                }
                            }
                            if (sent_buffer_size >= 4 * SFile::STREAM_CHUNK_SIZE)
                                break;
                        }
                    } else {
                        if (!SendRequest(SFile::idDownload, context.FilePath.c_str(), context.Flags, rh, context.Discarded, se)) {
                            return false;
                        }
                        context.Sent = true;
                        sent_buffer_size = cs->GetBytesInSendingBuffer();
                        if (sent_buffer_size > 3 * SFile::STREAM_CHUNK_SIZE)
                            break;
                    }
                    ++index;
                }
                return true;
            }

        protected:
            CUCriticalSection m_csFile;

        private:
            std::deque<CContext> m_vContext; //protected by m_csFile;
        };
    }; //ClientSide
}; //SPA

#endif 