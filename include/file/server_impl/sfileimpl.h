#pragma once

#include "../ufile_server.h"
#include "../../aserverw.h"
#include <fstream>

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

        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        private:
            void Download(const std::wstring &filePath, unsigned int flags, int &res, std::wstring &errMsg);
            void Upload(const std::wstring &filePath, unsigned int flags, UINT64 fileSize, int &res, std::wstring &errMsg);
            void Uploading(UINT64 &pos);
            void UploadCompleted();
            void CleanOF();

        private:
            UINT64 m_FileSize;
            std::ofstream m_of;
            std::wstring m_oFilePath;
            UINT64 m_oPos;
        };

        typedef CSocketProService<CSFileImpl> CSFileService;

    } //namespace ServerSide
} //namespace SPA