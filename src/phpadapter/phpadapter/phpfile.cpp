#include "stdafx.h"
#include "phpfile.h"

namespace PA
{

    CPhpFile::CPhpFile(CAsyncFile *sh, bool locked) : CPhpBaseHandler(locked, sh), m_sh(sh) {
    }

    void CPhpFile::RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace & cs) {
        Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
        handler.extends(base);

        handler.property("idDownload", SPA::SFile::idDownload, Php::Const);
        handler.property("idStartDownloading", SPA::SFile::idStartDownloading, Php::Const);
        handler.property("idDownloading", SPA::SFile::idDownloading, Php::Const);
        handler.property("idUpload", SPA::SFile::idUpload, Php::Const);
        handler.property("idUploading", SPA::SFile::idUploading, Php::Const);
        handler.property("idUploadCompleted", SPA::SFile::idUploadCompleted, Php::Const);

        handler.property("TRUNCACTED", (int64_t) SPA::SFile::FILE_OPEN_TRUNCACTED, Php::Const);
        handler.property("APPENDED", (int64_t) SPA::SFile::FILE_OPEN_APPENDED, Php::Const);
        handler.property("SHARE_READ", (int64_t) SPA::SFile::FILE_OPEN_SHARE_READ, Php::Const);
        handler.property("SHARE_WRITE", (int64_t) SPA::SFile::FILE_OPEN_SHARE_WRITE, Php::Const);

        handler.method<&CPhpFile::Download>("Download",{
            Php::ByVal("local", Php::Type::String),
            Php::ByVal("remote", Php::Type::String)
        });
        handler.method<&CPhpFile::Upload>("Upload",{
            Php::ByVal("local", Php::Type::String),
            Php::ByVal("remote", Php::Type::String)
        });
        cs.add(handler);
    }

    Php::Value CPhpFile::ToError(SPA::CUQueue * q) {
        int res;
        std::string em;
        *q >> res >> em;
        Php::Value v;
        v.set(PHP_ERR_CODE, res);
        v.set(PHP_ERR_MSG, em);
        return v;
    }

    void CPhpFile::PopTopCallbacks(PACallback & cb) {
        switch (cb.CallbackType) {
            case ctFile:
            {
                unsigned short reqId;
                *cb.Res >> reqId;
                auto &callback = *cb.CallBack;
                callback(ToError(cb.Res), reqId);
            }
                break;
            default:
                assert(false); //shouldn't come here
                break;
        }
    }

    CAsyncFile::DDownload CPhpFile::SetResCallback(unsigned short reqId, const Php::Value& phpDl, CQPointer &pV, unsigned int &timeout) {
        assert(pV);
        timeout = (~0);
        bool sync = false;
        if (phpDl.isNumeric()) {
            sync = true;
            timeout = (unsigned int) phpDl.numericValue();
        } else if (phpDl.isBool()) {
            sync = phpDl.boolValue();
        } else if (phpDl.isNull()) {
        } else if (!phpDl.isCallable()) {
            throw Php::Exception("A callback required for file exchange final result");
        }
        if (sync) {
            pV.reset(SPA::CScopeUQueue::Lock(), [](SPA::CUQueue * q) {
                SPA::CScopeUQueue::Unlock(q);
            });
        } else {
            pV.reset();
        }
        CPVPointer callback;
        if (phpDl.isCallable()) {
            callback.reset(new Php::Value(phpDl));
        }
        CAsyncFile::DDownload Dl = [reqId, callback, pV, this](SPA::ClientSide::CStreamingFile *file, int res, const std::wstring & errMsg) {
            std::string em = SPA::Utilities::ToUTF8(errMsg);
            Trim(em);
            try{
                if (pV) {
                    *pV << res << em;
                    std::unique_lock<std::mutex> lk(this->m_mPhp);
                    this->m_cvPhp.notify_all();
                } else if (callback) {
                    SPA::CScopeUQueue sb;
                    sb << reqId << res << em;
                    PACallback cb;
                    cb.CallbackType = ctFile;
                    cb.Res = sb.Detach();
                    cb.CallBack = callback;
                    std::unique_lock<std::mutex> lk(this->m_mPhp);
                    this->m_vCallback.push_back(cb);
                }
            }

            catch(std::system_error&) {
            }
        };
        return Dl;
    }

    void CPhpFile::MapFilePaths(const Php::Value& phpLocal, const Php::Value& phpRemote, std::wstring &local, std::wstring & remote) {
        std::string l = phpLocal.stringValue();
        Trim(l);
        if (!l.size()) {
            throw Php::Exception("Local file path can not be empty");
        }
        std::string r = phpRemote.stringValue();
        Trim(r);
        if (!r.size()) {
            throw Php::Exception("remote file path can not be empty");
        }
        local = SPA::Utilities::ToWide(l);
        remote = SPA::Utilities::ToWide(r);
    }

    Php::Value CPhpFile::Download(Php::Parameters & params) {
        unsigned int timeout;
        std::wstring local, remote;
        MapFilePaths(params[0], params[1], local, remote);
        size_t args = params.size();
        CQPointer pV;
        CAsyncFile::DDownload Dl;
        if (args > 2) {
            Dl = SetResCallback(SPA::SFile::idDownload, params[2], pV, timeout);
        }
        Php::Value phpCanceled;
        if (args > 3) {
            phpCanceled = params[3];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::SFile::idDownload, pV ? true : false);
        unsigned int flags = SPA::SFile::FILE_OPEN_TRUNCACTED;
        if (args > 4) {
            const Php::Value& vF = params[4];
            if (vF.isNumeric()) {
                flags = (unsigned int) vF.numericValue();
            } else {
                throw Php::Exception("One or more file writing flags (1=truncated, 2=appended, 4=shared read and 8=shared writing) required");
            }
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_sh->Download(local.c_str(), remote.c_str(), Dl, nullptr, discarded, flags), lk, timeout);
                return ToError(pV.get());
            }
            PopCallbacks();
        }
        return m_sh->Download(local.c_str(), remote.c_str(), Dl, nullptr, discarded, flags);
    }

    Php::Value CPhpFile::Upload(Php::Parameters & params) {
        unsigned int timeout;
        std::wstring local, remote;
        MapFilePaths(params[0], params[1], local, remote);
        size_t args = params.size();
        CQPointer pV;
        CAsyncFile::DUpload Ul;
        if (args > 2) {
            Ul = SetResCallback(SPA::SFile::idUpload, params[2], pV, timeout);
        }
        Php::Value phpCanceled;
        if (args > 3) {
            phpCanceled = params[3];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::SFile::idUpload, pV ? true : false);
        unsigned int flags = SPA::SFile::FILE_OPEN_TRUNCACTED;
        if (args > 4) {
            const Php::Value& vF = params[4];
            if (vF.isNumeric()) {
                flags = (unsigned int) vF.numericValue();
            } else {
                throw Php::Exception("One or more file writing flags (1=truncated, 2=appended, 4=shared read and 8=shared writing) required");
            }
        }
        if (pV) {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (!m_sh->Upload(local.c_str(), remote.c_str(), Ul, nullptr, discarded, flags)) {
                PopCallbacks();
                throw Php::Exception(PA::PHP_SOCKET_CLOSED);
            }
            if (!pV->GetSize()) {
                ReqSyncEnd(true, lk, timeout);
            } else {
                PopCallbacks();
            }
            return ToError(pV.get());
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            PopCallbacks();
        }
        return m_sh->Upload(local.c_str(), remote.c_str(), Ul, nullptr, discarded, flags);
    }

    Php::Value CPhpFile::__get(const Php::Value & name) {
        if (name == "FileSize") {
            return (int64_t) m_sh->GetFileSize();
        } else if (name == "FilesQueued") {
            return (int64_t) m_sh->GetFilesQueued();
        } else {
            return CPhpBaseHandler::__get(name);
        }
    }

} //namespace PA
