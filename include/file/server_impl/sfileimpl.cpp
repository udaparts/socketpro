
#include "sfileimpl.h"
#ifndef WIN32_64
#include <system_error>
#endif

extern std::wstring g_pathRoot;

namespace SPA{
    namespace ServerSide
    {
        CSFileImpl::CSFileImpl() : m_FileSize(0), m_oPos(0) {
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
            if (m_of.is_open()) {
                m_of.close();
#ifdef WIN32_64
                ::DeleteFileW(m_oFilePath.c_str());
#else
                std::string path = SPA::Utilities::ToUTF8(m_oFilePath.c_str(), m_oFilePath.size());
                unlink(path.c_str());
#endif
            }
            m_FileSize = 0;
        }

        void CSFileImpl::Uploading(UINT64 & pos) {
            if (m_of.is_open()) {
                m_of.write((const char*) m_UQueue.GetBuffer(), m_UQueue.GetSize());
                m_oPos += m_UQueue.GetSize();
                pos = m_oPos;
            } else {
                pos = (~0);
            }
            m_UQueue.SetSize(0);
        }

        void CSFileImpl::UploadCompleted() {
#ifndef NDEBUG
            assert(m_of.is_open());
#ifdef WIN32_64
            auto pos = m_of.tellp().seekpos();
#else
            auto pos = m_of.tellp();
#endif
            assert(m_FileSize == (UINT64) pos);
#endif
            m_FileSize = 0;
            m_of.close();
        }

        void CSFileImpl::Upload(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, int &res, std::wstring & errMsg) {
            assert(!m_FileSize);
            assert(!m_of.is_open());
            m_FileSize = fileSize;
            m_oPos = 0;
            try
            {
                bool absoulute;
                res = 0;
                m_of.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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
                m_of.open(m_oFilePath, std::ios::binary | std::ios::trunc);
#else
                std::string s = SPA::Utilities::ToUTF8(m_oFilePath.c_str(), m_oFilePath.size());
                m_of.open(s, std::ios::binary | std::ios::trunc);
#endif
            }

            catch(std::system_error & e) {
                res = e.code().value();
                std::string msg = e.code().message();
                errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
                return;
            }
			catch(std::runtime_error & e) {
                res = errno;
				if (!res)
					res = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                std::string msg = e.what();
                errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
                return;
            }
			catch(std::exception & e) {
                res = errno;
				if (!res)
					res = CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                std::string msg = e.what();
                errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
                return;
            }
            catch(...) {
                res = UNKNOWN_ERROR;
                errMsg = L"Unknown error";
                return;
            }
        }

        void CSFileImpl::Download(const std::wstring &filePath, unsigned int flags, int &res, std::wstring & errMsg) {
            std::ifstream reader;
            try
            {
                bool absoulute;
                res = 0;
                reader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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
                reader.open(path, std::ios::binary);
#else
                std::string s = SPA::Utilities::ToUTF8(path.c_str(), path.size());
                reader.open(s, std::ios::binary);
#endif
                reader.seekg(0, std::ios::end);
#ifdef WIN32_64
                UINT64 StreamSize = reader.tellg().seekpos();
#else
                static_assert(sizeof (std::streampos) >= sizeof (INT64), "Large file not supported");
                UINT64 StreamSize = reader.tellg();
#endif
                if (SendResult(idStartDownloading, StreamSize) != sizeof (StreamSize))
                    return; //socket closed or canceled
                if (StreamSize) {
                    reader.seekg(0, std::ios_base::beg);
                    CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), STREAM_CHUNK_SIZE);
                    std::streamsize size = ((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
                    reader.read((char*) sb->GetBuffer(), size);
                    unsigned int ret = (unsigned int) reader.gcount();
                    while (ret > 0) {
                        if (SendResult(idDownloading, sb->GetBuffer(), ret) != ret)
                            return; //socket closed or canceled
                        StreamSize -= ret;
                        size = ((StreamSize > STREAM_CHUNK_SIZE) ? STREAM_CHUNK_SIZE : StreamSize);
                        reader.read((char*) sb->GetBuffer(), size);
                        ret = (unsigned int) reader.gcount();
                    }
                }
            }

            catch(std::system_error & e) {
                res = e.code().value();
                std::string msg = e.code().message();
                errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
                return;
            }
			catch(std::runtime_error & e) {
                res = errno;
				if (!res)
					res = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                std::string msg = e.what();
                errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
                return;
            }
			catch(std::exception & e) {
                res = errno;
				if (!res)
					res = CANNOT_OPEN_LOCAL_FILE_FOR_READING;
                std::string msg = e.what();
                errMsg = SPA::Utilities::ToWide(msg.c_str(), msg.size());
                return;
            }
            catch(...) {
                res = UNKNOWN_ERROR;
                errMsg = L"Unknown error";
                return;
            }
        }
    } //namespace ServerSide
} //namespace SPA
