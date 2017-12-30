
#include "sfileimpl.h"
#include <system_error>

extern std::wstring g_pathRoot;

namespace SPA{
    namespace ServerSide
    {
        CSFileImpl::CSFileImpl() : m_oFileSize(0), m_oPos(0),
#ifdef WIN32_64
			m_of(INVALID_HANDLE_VALUE)
#else

#endif
		{
        }

        void CSFileImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanOF();
        }

        void CSFileImpl::OnBaseRequestArrive(unsigned short reqId) {
            switch (reqId) {
                case idCancel:
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
            M_I2_R2(idDownload, Download, std::wstring, unsigned int, int, std::wstring)
            M_I3_R2(idUpload, Upload, std::wstring, unsigned int, UINT64, int, std::wstring)
            M_I0_R1(idUploading, Uploading, UINT64)
            M_I0_R0(idUploadCompleted, UploadCompleted)
            END_SWITCH
            return 0;
        }

        void CSFileImpl::CleanOF() {
#ifdef WIN32_64
			if (m_of != INVALID_HANDLE_VALUE) {
				::CloseHandle(m_of);
                ::DeleteFileW(m_oFilePath.c_str());
				m_of = INVALID_HANDLE_VALUE;
			}
#else
			if (m_of != INVALID_HANDLE_VALUE) {
                std::string path = SPA::Utilities::ToUTF8(m_oFilePath.c_str(), m_oFilePath.size());
                unlink(path.c_str());
            }
#endif
            m_oFileSize = 0;
        }

        void CSFileImpl::Uploading(UINT64 & pos) {
#ifdef WIN32_64
			if (m_of != INVALID_HANDLE_VALUE) {
				DWORD dw = m_UQueue.GetSize(), dwWritten;
				BOOL ok = ::WriteFile(m_of, m_UQueue.GetBuffer(), dw, &dwWritten, nullptr);
				assert(ok);
				assert(dw == dwWritten);
				m_oPos += m_UQueue.GetSize();
				pos = m_oPos;
			}
#else
            if (m_of.is_open()) {
                m_of.write((const char*) m_UQueue.GetBuffer(), m_UQueue.GetSize());
                m_oPos += m_UQueue.GetSize();
                pos = m_oPos;
            } 
#endif
			else {
                pos = (~0);
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
			if (m_of.is_open()) {
                auto pos = m_of.tellp();
                assert(m_oFileSize == (UINT64) pos);
                m_of.close();
            }
#endif
            m_oFileSize = 0;
        }

        void CSFileImpl::Upload(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, int &res, std::wstring & errMsg) {
            bool absoulute;
			assert(!m_oFileSize);
#ifdef WIN32_64
            assert(m_of == INVALID_HANDLE_VALUE);
#else

#endif
            CleanOF();
            m_oFileSize = fileSize;
            m_oPos = 0;
            res = 0;
#ifdef WIN32_64
			std::size_t pos = filePath.find(L":\\");
            absoulute = (pos != std::wstring::npos && pos > 0);
#else
			absoulute = (filePath.front() == L'/');
#endif
			if (absoulute) {
                m_oFilePath = filePath;
            } else {
                m_oFilePath = g_pathRoot + filePath;
            }

#ifdef WIN32_64
			DWORD sm = 0;
			if ((flags & FILE_OPEN_SHARE_WRITE) == FILE_OPEN_SHARE_WRITE)
				sm |= FILE_SHARE_WRITE;
			if ((flags & FILE_OPEN_SHARE_READ) == FILE_OPEN_SHARE_READ)
				sm |= FILE_SHARE_READ;
			DWORD create = OPEN_ALWAYS;
			m_of = ::CreateFileW(m_oFilePath.c_str(), GENERIC_WRITE, sm, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (m_of == INVALID_HANDLE_VALUE) {
				res = (int)::GetLastError();
				errMsg = Utilities::GetErrorMessage((DWORD)res);
			}
			else {
				if ((flags & FILE_OPEN_TRUNCACTED) == FILE_OPEN_TRUNCACTED)
				{
					create = ::SetFilePointer(m_of, 0, nullptr, FILE_BEGIN);
					BOOL ok = ::SetEndOfFile(m_of);
					assert(ok);
				}
				else if ((flags & FILE_OPEN_APPENDED) == FILE_OPEN_APPENDED) {
					create = ::SetFilePointer(m_of, 0, nullptr, FILE_END);
				}
			}
#else
			 std::string s = SPA::Utilities::ToUTF8(m_oFilePath.c_str(), m_oFilePath.size());
#endif
        }

        void CSFileImpl::Download(const std::wstring &filePath, unsigned int flags, int &res, std::wstring & errMsg) {
			 bool absoulute;
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
                    path = g_pathRoot + filePath;
                }
#ifdef WIN32_64
			HANDLE h = ::CreateFileW(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (h == INVALID_HANDLE_VALUE) {
				res = (int)::GetLastError();
				errMsg = Utilities::GetErrorMessage((DWORD)res);
				return;
			}
			DWORD dwHigh = 0;
			DWORD dwLow = ::GetFileSize(h, &dwHigh);
			UINT64 StreamSize = dwHigh;
			StreamSize <<= 32;
			StreamSize += dwLow;
			if (SendResult(idStartDownloading, StreamSize) != sizeof (StreamSize)) {
				::CloseHandle(h);
				return; //socket closed or canceled
			}
			CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), STREAM_CHUNK_SIZE);
			unsigned int size = (unsigned int )((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
			DWORD dwRead = 0;
			BOOL ok = ::ReadFile(h, (LPVOID)sb->GetBuffer(), size, &dwRead, nullptr);
			assert(ok);
			while (dwRead > 0) {
				if (SendResult(idDownloading, sb->GetBuffer(), (unsigned int)dwRead) != dwRead) {
					::CloseHandle(h);
					return; //socket closed or canceled
				}
				StreamSize -= dwRead;
				size = (unsigned int )((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
				dwRead = 0;
				ok = ::ReadFile(h, (LPVOID)sb->GetBuffer(), size, &dwRead, nullptr);
				assert(ok);
			}
			::CloseHandle(h);
#else

#endif
        }
    } //namespace ServerSide
} //namespace SPA
