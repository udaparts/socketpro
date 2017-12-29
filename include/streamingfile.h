
#ifndef _SOCKETPRO_STREAMING_FILE_H_
#define _SOCKETPRO_STREAMING_FILE_H_

#include <fstream>
#include <deque>
#include "file/ufile.h"

namespace SPA {
    namespace ClientSide {

        class CStreamingFile : public CAsyncServiceHandler {
        public:
            typedef std::function<void(CStreamingFile *file, int res, const std::wstring &errMsg) > DDownload;
            typedef DDownload DUpload;
            typedef std::function<void(CStreamingFile *file, UINT64 downloaded) > DTransferring;

            CStreamingFile(CClientSocket * cs = nullptr) : CAsyncServiceHandler(SFile::sidFile, cs) {
            }

        private:

            struct CContext {

                CContext(bool uplaod, unsigned int flags)
                : Uploading(uplaod), FileSize(~0), Flags(flags), Sent(false), m_if(nullptr), m_of(nullptr) {
                }
                bool Uploading;
                UINT64 FileSize;
                unsigned int Flags;
                bool Sent;
                std::string LocalFile;
                std::wstring FilePath;
                DDownload Download;
                DTransferring Transferring;
                DCanceled Aborted;
                std::ifstream *m_if;
                std::ofstream *m_of;
            };

        public:

            virtual unsigned int CleanCallbacks() {
                {
                    CAutoLock al(m_csFile);
                    for (auto it = m_vContext.begin(), end = m_vContext.end(); it != end; ++it) {
                        if (it->m_if)
                            delete it->m_if;
                        if (it->m_of)
                            delete it->m_of;
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

            bool Upload(const char *localFile, const wchar_t *remoteFile, DUpload up, DTransferring trans, DCanceled aborted = DCanceled(), unsigned int flags = 0) {
                if (!localFile || !::strlen(localFile))
                    return false;
                if (!remoteFile || !::wcslen(remoteFile))
                    return false;
                CContext context(true, flags);
                context.Download = up;
                context.Transferring = trans;
                context.Aborted = aborted;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                CAutoLock al(m_csFile);
                unsigned int uploadings = CheckUploadings();
                m_vContext.push_back(context);
                if (uploadings)
                    return true;
                return Transfer();
            }

            bool Download(const char *localFile, const wchar_t *remoteFile, DDownload dl, DTransferring trans, DCanceled aborted = DCanceled(), unsigned int flags = 0) {
                if (!localFile || !::strlen(localFile))
                    return false;
                if (!remoteFile || !::wcslen(remoteFile))
                    return false;
                CContext context(false, flags);
                context.Download = dl;
                context.Transferring = trans;
                context.Aborted = aborted;
                context.FilePath = remoteFile;
                context.LocalFile = localFile;
                CAutoLock al(m_csFile);
                unsigned int uploadings = CheckUploadings();
                m_vContext.push_back(context);
                if (uploadings) {
                    return true;
                }
                return Transfer();
            }

        protected:

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
                                if (context.m_of) {
                                    if (context.m_of->is_open())
                                        context.m_of->close();
                                    else if (!res) {
                                        res = SFile::CANNOT_OPEN_LOCAL_FILE_FOR_WRITING;
                                        errMsg = L"Cannot open a local file for writing data";
                                    }
                                    delete context.m_of;
                                } else {
                                    assert(res != 0);
                                }
                                dl = context.Download;
                                m_vContext.pop_front();
                            } else {
                                assert(false);
                            }
                            if (dl)
                                dl(this, res, errMsg);
                        }
                        CAutoLock al(m_csFile);
                        Transfer();
                    }
                        break;
                    case SFile::idStartDownloading:
                    {
                        CAutoLock al(m_csFile);
                        if (m_vContext.size()) {
                            CContext &context = m_vContext.front();
                            assert(!context.Uploading);
                            assert(!context.m_of);
                            mc >> context.FileSize;
                            context.m_of = new std::ofstream;
                            context.m_of->open(context.LocalFile, std::ios::out | std::ios::binary | std::ios::trunc);
                        } else {
                            mc.SetSize(0);
                            assert(false);
                        }
                    }
                        break;
                    case SFile::idDownloading:
                    {
                        DTransferring trans;
                        UINT64 downloaded = (~0);
                        {
                            CAutoLock al(m_csFile);
                            if (m_vContext.size()) {
                                CContext &context = m_vContext.front();
                                assert(!context.Uploading);
                                trans = context.Transferring;
                                if (context.m_of->is_open()) {
                                    context.m_of->write((const char*) mc.GetBuffer(), mc.GetSize());
                                    downloaded = context.m_of->tellp();
                                }
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
                                if (context.m_if) {
                                    if (context.m_if->is_open())
                                        context.m_if->close();
                                    delete context.m_of;
                                } else {
                                    assert(false);
                                }
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
                        UINT64 uploaded;
                        mc >> uploaded;
                        {
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
                            trans(this, uploaded);
                        CAutoLock al(m_csFile);
                        Transfer();
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
                                if (context.m_if) {
                                    if (context.m_if->is_open())
                                        context.m_if->close();
                                    delete context.m_of;
                                } else {
                                    assert(false);
                                }
                                upl = context.Download;
                                m_vContext.pop_front();
                            } else {
                                assert(false);
                            }
                        }
                        if (upl)
                            upl(this, 0, L"");
                        CAutoLock al(m_csFile);
                        Transfer();
                    }
                        break;
                    default:
                        CAsyncServiceHandler::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:

            static UINT64 CheckPos(std::ifstream *ifs) {
                UINT64 pos;
#ifdef WIN32_64
                pos = (UINT64) ifs->tellg().seekpos();
#else
                pos = (UINT64) ifs->tellg();
#endif
                return pos;
            }

            unsigned int CheckUploadings() {
                unsigned int uploadings = 0;
                for (auto it = m_vContext.begin(), end = m_vContext.end(); it != end; ++it) {
                    if (it->Uploading)
                        ++uploadings;
                }
                return uploadings;
            }

            bool Transfer() {
                size_t index = 0;
                ResultHandler rh;
                DServerException se;
                CClientSocket *cs = GetAttachedClientSocket();
                if (!cs->Sendable())
                    return false;
                unsigned int recv = cs->GetBytesInSendingBuffer();
                if (recv > 3 * SFile::STREAM_CHUNK_SIZE)
                    return true;
                while (index < m_vContext.size()) {
                    CContext &context = m_vContext[index];
                    if (context.Sent) {
                        ++index;
                        continue;
                    }
                    if (context.Uploading && context.m_if && !context.m_if->is_open()) {
                        if (index == 0) {
                            delete context.m_if;
                            if (context.Download) {
                                context.Download(this, SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING, L"Cannot open a local file for reading data");
                            }
                            m_vContext.erase(m_vContext.begin() + index);
                            if (!cs->Sendable())
                                return false;
                        } else {
                            ++index;
                        }
                        continue;
                    }
                    if (context.Uploading) {
                        if (!context.m_if) {
                            context.m_if = new std::ifstream;
                            context.m_if->open(context.LocalFile, std::ios::binary);
                            if (context.m_if->is_open()) {
                                context.m_if->seekg(0, std::ios_base::end);
                                context.FileSize = CheckPos(context.m_if);
                                context.m_if->seekg(0, std::ios_base::beg);
                                if (!SendRequest(SFile::idUpload, context.FilePath.c_str(), context.Flags, context.FileSize, rh, context.Aborted, se)) {
                                    return false;
                                }
                                if (!context.FileSize) {
                                    context.Sent = true;
                                    if (!SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Aborted, se)) {
                                        return false;
                                    }
                                }
                            }
                        }
                        if (!context.m_if->is_open()) {
                            if (index == 0) {
                                delete context.m_if;
                                if (context.Download) {
                                    context.Download(this, SFile::CANNOT_OPEN_LOCAL_FILE_FOR_READING, L"Cannot open a local file for reading data");
                                }
                                m_vContext.erase(m_vContext.begin() + index);
                            } else {
                                ++index;
                            }
                            continue;
                        } else if (context.FileSize > CheckPos(context.m_if)) {
                            CScopeUQueue sb(MY_OPERATION_SYSTEM, IsBigEndian(), SFile::STREAM_CHUNK_SIZE);
                            context.m_if->read((char*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                            unsigned int ret = (unsigned int) context.m_if->gcount();
                            while (ret > 0) {
                                if (!SendRequest(SFile::idUploading, sb->GetBuffer(), ret, rh, context.Aborted, se)) {
                                    return false;
                                }
                                if (ret < SFile::STREAM_CHUNK_SIZE)
                                    break;
                                recv = cs->GetBytesInSendingBuffer();
                                if (recv >= 5 * SFile::STREAM_CHUNK_SIZE)
                                    break;
                                context.m_if->read((char*) sb->GetBuffer(), SFile::STREAM_CHUNK_SIZE);
                                ret = (unsigned int) context.m_if->gcount();
                            }
                            if (ret && (ret < SFile::STREAM_CHUNK_SIZE || CheckPos(context.m_if) == 0)) {
                                context.Sent = true;
                                if (!SendRequest(SFile::idUploadCompleted, (const unsigned char*) nullptr, (unsigned int) 0, rh, context.Aborted, se)) {
                                    return false;
                                }
                            }
                            if (recv > 5 * SFile::STREAM_CHUNK_SIZE)
                                break;
                        }
                    } else {
                        if (!SendRequest(SFile::idDownload, context.FilePath.c_str(), context.Flags, rh, context.Aborted, se)) {
                            return false;
                        }
                        context.Sent = true;
                        recv = cs->GetBytesInSendingBuffer();
                        if (recv > 3 * SFile::STREAM_CHUNK_SIZE)
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