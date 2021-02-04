
#include "sfileimpl.h"
#include <system_error>
#ifndef WIN32_64
#include <fcntl.h>
#include <sys/stat.h>
#endif
#include "../../scloader.h"

#ifdef WIN32_64
#include <memory>
#endif

namespace SPA{
    namespace ServerSide
    {
        CUCriticalSection CSFileImpl::m_cs;
        std::wstring CSFileImpl::m_pathRoot;

        std::wstring CSFileImpl::GetRootDirectory() {
            CAutoLock al(m_cs);
            return m_pathRoot;
        }

        void CSFileImpl::SetRootDirectory(const wchar_t* pathRoot) {
            CAutoLock al(m_cs);
            if (pathRoot && ::wcslen(pathRoot))
                m_pathRoot = pathRoot;
#ifdef WIN32_64
            if (m_pathRoot.back() != L'\\')
                m_pathRoot += L'\\';
#else
            if (m_pathRoot.back() != L'/')
                m_pathRoot += L'/';
#endif
        }

        CSFileImpl::CSFileImpl() : m_oFileSize(0), m_oPos(0), InitSize(-1),
#ifdef WIN32_64
        m_of(INVALID_HANDLE_VALUE)
#else
        m_of(-1)
#endif
        {

        }

        void CSFileImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanOF();
        }

        void CSFileImpl::OnBaseRequestArrive(unsigned short reqId) {
            switch (reqId) {
                case (unsigned short) tagBaseRequestID::idCancel:
                    CleanOF();
                    break;
                default:
                    break;
            }
        }

        void CSFileImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {

        }

        int CSFileImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I4_R2(idDownload, Download, std::wstring, std::wstring, unsigned int, INT64, int, std::wstring)
            M_I3_R3(idUpload, Upload, std::wstring, unsigned int, UINT64, int, std::wstring, INT64)
            M_I4_R0(idUploadBackup, UploadBackup, std::wstring, unsigned int, UINT64, INT64)
            M_I0_R3(idUploading, Uploading, UINT64, int, std::wstring)
            M_I0_R0(idUploadCompleted, UploadCompleted)
            END_SWITCH
            return 0;
        }

        void CSFileImpl::CleanOF() {
#ifdef WIN32_64
            if (m_of != INVALID_HANDLE_VALUE) {
                if (InitSize == -1) {
                    ::CloseHandle(m_of);
                    ::DeleteFileW(m_oFilePath.c_str());
                } else {
                    BOOL ok = ::FlushFileBuffers(m_of);
                    assert(ok);
                    LARGE_INTEGER moveDis, newPos;
                    moveDis.QuadPart = InitSize;
                    ok = ::SetFilePointerEx(m_of, moveDis, &newPos, FILE_BEGIN);
                    assert(ok);
                    ok = ::SetEndOfFile(m_of);
                    assert(ok);
                    ::CloseHandle(m_of);
                }
                m_of = INVALID_HANDLE_VALUE;
            }
#else
            if (m_of != -1) {
                if (InitSize == -1) {
                    ::close(m_of);
                    std::string path = Utilities::ToUTF8(m_oFilePath);
                    auto fail = ::remove(path.c_str());
                    assert(!fail);
                } else {
                    auto fail = ::fsync(m_of);
                    assert(!fail);
                    auto newPos = ::lseek64(m_of, InitSize, SEEK_SET);
                    assert(newPos != -1);
                    fail = ::ftruncate(m_of, newPos);
                    assert(!fail);
                    ::close(m_of);
                }
                m_of = -1;
            }
#endif
            m_oFileSize = 0;
            m_oFilePath.clear();
            m_oPos = 0;
            InitSize = -1;
        }

        void CSFileImpl::Uploading(UINT64 & pos, int &res, std::wstring & errMsg) {
            res = 0;
#ifdef WIN32_64
            if (m_of != INVALID_HANDLE_VALUE) {
                bool ok;
                DWORD dw = m_UQueue.GetSize(), dwWritten;
                ok = (FALSE != ::WriteFile(m_of, m_UQueue.GetBuffer(), dw, &dwWritten, nullptr));
                assert(ok);
                if (ok) {
                    assert(dw == dwWritten);
                    m_oPos += m_UQueue.GetSize();
                    pos = m_oPos;
                } else {
                    res = (int) ::GetLastError();
                    errMsg = Utilities::GetErrorMessage((DWORD) res);
                }
            }
#else
            if (m_of != -1) {
                auto ret = ::write(m_of, m_UQueue.GetBuffer(), m_UQueue.GetSize());
                bool ok = ((unsigned int) ret == m_UQueue.GetSize());
                assert(ok);
                if (ok) {
                    m_oPos += m_UQueue.GetSize();
                    pos = m_oPos;
                } else {
                    res = errno;
                    std::string err = strerror(res);
                    errMsg = Utilities::ToWide(err.c_str(), err.size());
                }
            }
#endif
            else {
                pos = (~0);
                res = FILE_BAD_OPERATION;
                errMsg = L"Bad file operation";
            }
            m_UQueue.SetSize(0);
        }

        void CSFileImpl::UploadCompleted() {
#ifdef WIN32_64
            if (m_of != INVALID_HANDLE_VALUE) {
                ::CloseHandle(m_of);
                m_of = INVALID_HANDLE_VALUE;
            }
#else
            if (m_of != -1) {
                ::close(m_of);
                m_of = -1;
            }
#endif
            m_oFileSize = 0;
            m_oFilePath.clear();
            m_oPos = 0;
            InitSize = -1;
        }

        void CSFileImpl::UploadBackup(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, INT64 initSize) {
            assert(IsDequeueRequest()); //client queue is enabled
            bool absoulute;
#ifdef WIN32_64
            if (m_of != INVALID_HANDLE_VALUE)
#else
            if (m_of != -1)
#endif
            {
                return;
            }
            m_oFileSize = fileSize;
            m_oPos = 0;
            InitSize = -1;
#ifdef WIN32_64
            std::size_t pos = filePath.find(L":\\");
            absoulute = (pos != std::wstring::npos && pos > 0);
#else
            absoulute = (filePath.front() == L'/');
#endif
            if (absoulute) {
                m_oFilePath = filePath;
            } else {
                m_oFilePath = GetRootDirectory() + filePath;
            }
            bool existing = false;
#ifdef WIN32_64
            DWORD sm = 0;
            if ((flags & FILE_OPEN_SHARE_WRITE) == FILE_OPEN_SHARE_WRITE) {
                sm |= FILE_SHARE_WRITE;
            }
            m_of = ::CreateFileW(m_oFilePath.c_str(), GENERIC_WRITE, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (m_of == INVALID_HANDLE_VALUE) {
                m_of = ::CreateFileW(m_oFilePath.c_str(), GENERIC_WRITE, sm, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
            } else {
                existing = true;
            }
            if (m_of == INVALID_HANDLE_VALUE) {
            } else if (existing) {
                BOOL ok = TRUE;
                InitSize = initSize;
                if (initSize == -1) {
                    initSize = 0;
                }
                if ((flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED) {
                    ok = ::SetEndOfFile(m_of);
                } else if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    LARGE_INTEGER dis, pos;
                    dis.QuadPart = 0;
                    ok = ::SetFilePointerEx(m_of, dis, &pos, FILE_END);
                    INT64 isize = pos.QuadPart;
                    if (isize && isize > initSize) {
                        InitSize = initSize;
                        dis.QuadPart = initSize;
                        assert(ok);
                        ok = ::SetFilePointerEx(m_of, dis, &pos, FILE_BEGIN);
                    }
                }
                assert(ok);
            }
#else
            std::string s = Utilities::ToUTF8(m_oFilePath.c_str(), m_oFilePath.size());
            int mode = (O_WRONLY | O_CREAT | O_EXCL);
            mode_t m = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            m_of = ::open(s.c_str(), mode, m);
            if (m_of == -1) {
                existing = true;
                mode = (O_WRONLY | O_CREAT);
                if ((flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED) {
                    mode |= O_TRUNC;
                } else if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    mode |= O_APPEND;
                }
                m_of = ::open(s.c_str(), mode, m);
                if (m_of == -1) {
                    return;
                }
            }
            if (existing) {
                InitSize = initSize;
                if (initSize == -1) {
                    initSize = 0;
                }
                if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    INT64 isize = ::lseek64(m_of, 0, SEEK_END);
                    if (isize && isize > initSize) {
                        isize = ::lseek64(m_of, initSize, SEEK_SET);
                    }
                }
            }
            if ((flags & FILE_OPEN_SHARE_WRITE) == 0) {
                struct flock fl;
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0;
                fl.l_type = F_WRLCK;
                fl.l_pid = ::getpid();
                if (::fcntl(m_of, F_SETLKW, &fl) == -1) {
                }
            }
#endif
        }

        void CSFileImpl::Upload(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, int &res, std::wstring & errMsg, INT64 & initPos) {
            bool absoulute;
            res = 0;
            initPos = -1;
#ifdef WIN32_64
            assert(m_of == INVALID_HANDLE_VALUE);
#else
            assert(m_of == -1);
#endif
            CleanOF();
            m_oFileSize = fileSize;
#ifdef WIN32_64
            std::size_t pos = filePath.find(L":\\");
            absoulute = (pos != std::wstring::npos && pos > 0);
#else
            absoulute = (filePath.front() == L'/');
#endif
            if (absoulute) {
                m_oFilePath = filePath;
            } else {
                m_oFilePath = GetRootDirectory() + filePath;
            }
            bool existing = false;
#ifdef WIN32_64
            DWORD sm = 0;
            if ((flags & FILE_OPEN_SHARE_WRITE) == FILE_OPEN_SHARE_WRITE) {
                sm |= FILE_SHARE_WRITE;
            }
            m_of = ::CreateFileW(m_oFilePath.c_str(), GENERIC_WRITE, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (m_of == INVALID_HANDLE_VALUE) {
                m_of = ::CreateFileW(m_oFilePath.c_str(), GENERIC_WRITE, sm, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
            } else {
                existing = true;
            }
            if (m_of == INVALID_HANDLE_VALUE) {
                res = (int) ::GetLastError();
                errMsg = Utilities::GetErrorMessage((DWORD) res);
            } else if (existing) {
                BOOL ok = TRUE;
                InitSize = 0;
                initPos = 0;
                if ((flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED) {
                    ok = ::SetEndOfFile(m_of);
                } else if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    LARGE_INTEGER dis, pos;
                    dis.QuadPart = 0;
                    ok = ::SetFilePointerEx(m_of, dis, &pos, FILE_END);
                    InitSize = pos.QuadPart;
                    initPos = InitSize;
                }
                assert(ok);
                if (!ok) {
                    res = (int) ::GetLastError();
                    errMsg = Utilities::GetErrorMessage((DWORD) res);
                }
            }
#else
            std::string s = Utilities::ToUTF8(m_oFilePath.c_str(), m_oFilePath.size());
            int mode = (O_WRONLY | O_CREAT | O_EXCL);
            mode_t m = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            m_of = ::open(s.c_str(), mode, m);
            if (m_of == -1) {
                existing = true;
                mode = (O_WRONLY | O_CREAT);
                if ((flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED) {
                    mode |= O_TRUNC;
                } else if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    mode |= O_APPEND;
                }
                m_of = ::open(s.c_str(), mode, m);
                if (m_of == -1) {
                    res = errno;
                    std::string err = strerror(res);
                    errMsg = Utilities::ToWide(err.c_str(), err.size());
                    return;
                }
            }
            if (existing) {
                InitSize = 0;
                if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
                    InitSize = ::lseek64(m_of, 0, SEEK_END);
                }
                initPos = InitSize;
            }
            if ((flags & FILE_OPEN_SHARE_WRITE) == 0) {
                struct flock fl;
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0;
                fl.l_type = F_WRLCK;
                fl.l_pid = ::getpid();
                if (::fcntl(m_of, F_SETLKW, &fl) == -1) {
                    res = errno;
                    std::string err = strerror(res);
                    errMsg = Utilities::ToWide(err.c_str(), err.size());
                }
            }
#endif
        }

        void CSFileImpl::Download(const std::wstring &localFile, const std::wstring &filePath, unsigned int flags, INT64 initSize, int &res, std::wstring & errMsg) {
            bool absoulute;
            CleanOF(); //clean file uploading in case it is not completed yet
            res = 0;
#ifdef WIN32_64
            std::size_t pos = filePath.find(L":\\");
            absoulute = (pos != std::wstring::npos && pos > 0);
#else
            absoulute = (filePath.front() == L'/');
#endif
            std::wstring path;
            if (absoulute) {
                path = filePath;
            } else {
                path = GetRootDirectory() + filePath;
            }
#ifdef WIN32_64
            DWORD sm = 0;
            if ((flags & FILE_OPEN_SHARE_READ) == FILE_OPEN_SHARE_READ) {
                sm |= FILE_SHARE_READ;
            }
            std::shared_ptr<void> h(::CreateFileW(path.c_str(), GENERIC_READ, sm, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr), [](HANDLE p) {
                if (p != INVALID_HANDLE_VALUE) {
                    ::CloseHandle(p);
                }
            });
            if (h.get() == INVALID_HANDLE_VALUE) {
                res = (int) ::GetLastError();
                errMsg = Utilities::GetErrorMessage((DWORD) res);
                return;
            }
            LARGE_INTEGER li;
            if (!::GetFileSizeEx(h.get(), &li)) {
                res = ::GetLastError();
                errMsg = Utilities::GetErrorMessage((DWORD) res);
                return;
            }
            UINT64 StreamSize = (UINT64) li.QuadPart;
            {
                CScopeUQueue sb;
                sb << StreamSize << localFile << filePath << flags << initSize;
                if (SendResult(idStartDownloading, sb->GetBuffer(), sb->GetSize()) != sb->GetSize()) {
                    return; //socket closed or canceled
                }
            }
            CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), STREAM_CHUNK_SIZE);
            unsigned int size = (unsigned int) ((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
            DWORD dwRead = 0;
            BOOL ok = ::ReadFile(h.get(), (LPVOID) sb->GetBuffer(), size, &dwRead, nullptr);
            assert(ok);
            if (!ok) {
                res = ::GetLastError();
                errMsg = Utilities::GetErrorMessage((DWORD) res);
                return;
            }
            while (dwRead > 0) {
                if (GetInterruptOptions()) {
                    res = FILE_DOWNLOADING_INTERRUPTED;
                    errMsg = L"File downloading interrupted";
                }
                if (SendResult(idDownloading, sb->GetBuffer(), (unsigned int) dwRead) != dwRead) {
                    return; //socket closed or canceled
                }
                StreamSize -= dwRead;
                size = (unsigned int) ((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
                dwRead = 0;
                ok = ::ReadFile(h.get(), (LPVOID) sb->GetBuffer(), size, &dwRead, nullptr);
                assert(ok);
                if (!ok) {
                    res = ::GetLastError();
                    errMsg = Utilities::GetErrorMessage((DWORD) res);
                    return;
                }
            }
#else
            std::string s = Utilities::ToUTF8(path.c_str(), path.size());
            int h = ::open(s.c_str(), O_RDONLY);
            if (h == -1) {
                res = errno;
                std::string err = strerror(res);
                errMsg = Utilities::ToWide(err.c_str(), err.size());
                return;
            }
            if ((flags & FILE_OPEN_SHARE_READ) == 0) {
                struct flock fl;
                fl.l_whence = SEEK_SET;
                fl.l_start = 0;
                fl.l_len = 0;
                fl.l_type = F_RDLCK;
                fl.l_pid = ::getpid();
                if (::fcntl(h, F_SETLKW, &fl) == -1) {
                    res = errno;
                    std::string err = strerror(res);
                    errMsg = Utilities::ToWide(err.c_str(), err.size());
                    ::close(h);
                    return;
                }
            }
            struct stat st;
            if (::fstat(h, &st) == -1) {
                res = errno;
                std::string err = strerror(res);
                errMsg = Utilities::ToWide(err.c_str(), err.size());
                ::close(h);
                return;
            }
            static_assert(sizeof (st.st_size) >= sizeof (UINT64), "Big file not supported");
            UINT64 StreamSize = st.st_size;
            {
                CScopeUQueue sb;
                sb << StreamSize << localFile << filePath << flags << initSize;
                if (SendResult(idStartDownloading, sb->GetBuffer(), sb->GetSize()) != sb->GetSize()) {
                    ::close(h);
                    return; //socket closed or canceled
                }
            }
            unsigned int size = (unsigned int) ((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
            CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), STREAM_CHUNK_SIZE);
            auto ret = ::read(h, (void*) sb->GetBuffer(), size);
            while (ret > 0) {
                if (GetInterruptOptions()) {
                    res = FILE_DOWNLOADING_INTERRUPTED;
                    errMsg = L"File downloading interrupted";
                }
                if (SendResult(idDownloading, sb->GetBuffer(), (unsigned int) ret) != (unsigned int) ret) {
                    ::close(h);
                    return; //socket closed or canceled
                }
                StreamSize -= (unsigned int) ret;
                size = (unsigned int) ((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
                ret = ::read(h, (void*) sb->GetBuffer(), size);
            }
            if (ret == -1) {
                res = errno;
                std::string err = strerror(res);
                errMsg = Utilities::ToWide(err.c_str(), err.size());
            }
            close(h);
#endif
        }
    } //namespace ServerSide
} //namespace SPA
