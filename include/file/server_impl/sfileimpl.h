
#ifndef _SOCKETPRO_FILE_STREAMING_IMPL_H_
#define _SOCKETPRO_FILE_STREAMING_IMPL_H_

#include "../ufile_server.h"
#include "../../aserverw.h"

using namespace SPA::SFile;

namespace SPA {
    namespace ServerSide {

        class CSFileImpl : public CClientPeer {
            //no copy constructor
            CSFileImpl(const CSFileImpl &sf);
            //no assignment operator
            CSFileImpl& operator=(const CSFileImpl &sf);

        public:
            CSFileImpl();
            static std::wstring GetRootDirectory();
            static void SetRootDirectory(const wchar_t* pathRoot);

        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        private:
            void Download(const std::wstring &localFile, const std::wstring &filePath, unsigned int flags, INT64 initSize, int &res, std::wstring &errMsg);
            void Upload(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, int &res, std::wstring &errMsg, INT64 &initPos);
            void UploadBackup(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, INT64 initSize);
            void Uploading(UINT64 &pos, int &res, std::wstring &errMsg);
            void UploadCompleted();
            void CleanOF();

        private:
            UINT64 m_oFileSize;
            std::wstring m_oFilePath;
            UINT64 m_oPos;
            INT64 InitSize;
#ifdef WIN32_64
            HANDLE m_of;
#else
            int m_of;
#endif
            static CUCriticalSection m_cs;
            static std::wstring m_pathRoot;
        };

        typedef CSocketProService<CSFileImpl> CSFileService;

    } //namespace ServerSide
} //namespace SPA

#endif
