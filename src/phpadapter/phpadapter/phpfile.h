#ifndef SPA_PHP_FILE_H
#define SPA_PHP_FILE_H

#include "basehandler.h"

namespace PA {
    typedef SPA::ClientSide::CStreamingFile CAsyncFile;
    typedef SPA::ClientSide::CSocketPool<CAsyncFile> CPhpFilePool;

    class CPhpFile : public CPhpBaseHandler {
    public:
        CPhpFile(CAsyncFile *sh, bool locked);
        CPhpFile(const CPhpFile &file) = delete;

    public:
        CPhpFile& operator=(const CPhpFile &file) = delete;
        static void RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace &cs);
        Php::Value __get(const Php::Value &name);

    protected:
        void PopTopCallbacks(PACallback &cb);

    private:
        Php::Value Download(Php::Parameters &params);
        Php::Value Upload(Php::Parameters &params);
        Php::Value Cancel();
        CAsyncFile::DDownload SetResCallback(unsigned short reqId, const Php::Value &phpDl, CQPointer &pV, unsigned int &timeout);
        static void MapFilePaths(const Php::Value& phpLocal, const Php::Value& phpRemote, std::wstring &local, std::wstring &remote);
        static Php::Value ToError(SPA::CUQueue *q);

    private:
        CAsyncFile *m_sh;
    };
} //namespace PA
#endif
