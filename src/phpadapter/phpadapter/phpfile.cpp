#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(unsigned int poolId, CAsyncFile *sh, bool locked)
		: CPhpBaseHandler(locked, sh, poolId), m_sh(sh) {
	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
		Register(handler);

		handler.property("idDownload", SPA::SFile::idDownload, Php::Const);
		handler.property("idStartDownloading", SPA::SFile::idStartDownloading, Php::Const);
		handler.property("idDownloading", SPA::SFile::idDownloading, Php::Const);
		handler.property("idUpload", SPA::SFile::idUpload, Php::Const);
		handler.property("idUploading", SPA::SFile::idUploading, Php::Const);
		handler.property("idUploadCompleted", SPA::SFile::idUploadCompleted, Php::Const);

		handler.property("TRUNCACTED", (int64_t)SPA::SFile::FILE_OPEN_TRUNCACTED, Php::Const);
		handler.property("APPENDED", (int64_t)SPA::SFile::FILE_OPEN_APPENDED, Php::Const);
		handler.property("SHARE_READ", (int64_t)SPA::SFile::FILE_OPEN_SHARE_READ, Php::Const);
		handler.property("SHARE_WRITE", (int64_t)SPA::SFile::FILE_OPEN_SHARE_WRITE, Php::Const);

		handler.method("Download", &CPhpFile::Download, {
			Php::ByVal("local", Php::Type::String),
			Php::ByVal("remote", Php::Type::String),
			Php::ByVal("res", Php::Type::Null)
		});
		handler.method("Upload", &CPhpFile::Upload, {
			Php::ByVal("local", Php::Type::String),
			Php::ByVal("remote", Php::Type::String),
			Php::ByVal("res", Php::Type::Null)
		});
		cs.add(handler);
	}

	CAsyncFile::DDownload CPhpFile::SetResCallback(Php::Value phpDl, std::shared_ptr<Php::Value> &pV, unsigned int &timeout) {
		assert(pV);
		timeout = (~0);
		bool sync = false;
		if (phpDl.isNumeric()) {
			sync = true;
			timeout = (unsigned int)phpDl.numericValue();
		}
		else if (phpDl.isBool()) {
			sync = phpDl.boolValue();
		}
		else if (phpDl.isNull()) {
		}
		else if (!phpDl.isCallable()) {
			throw Php::Exception("A callback required for file exchange final result");
		}
		if (sync) {
			pV.reset(new Php::Value);
		}
		else {
			pV.reset();
		}
		CAsyncFile::DDownload Dl = [phpDl, pV, this](SPA::ClientSide::CStreamingFile *file, int res, const std::wstring& errMsg) {
			if (pV) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg);
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpDl.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg);
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpDl(v);
			}
		};
		return Dl;
	}

	void CPhpFile::MapFilePaths(Php::Value phpLocal, Php::Value phpRemote, std::wstring &local, std::wstring &remote) {
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

	Php::Value CPhpFile::Download(Php::Parameters &params) {
		unsigned int timeout;
		std::wstring local, remote;
		std::shared_ptr<Php::Value> pV;
		MapFilePaths(params[0], params[1], local, remote);
		
		Php::Value phpDl = params[2];
		auto Dl = SetResCallback(phpDl, pV, timeout);

		Php::Value phpProgress;
		size_t args = params.size();
		if (args > 3) {
			phpProgress = params[3];
		}
		auto Progress = SetTransCallback(phpProgress);

		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::SFile::idDownload, pV ? true : false);
		
		unsigned int flags = SPA::SFile::FILE_OPEN_TRUNCACTED;
		if (args > 5) {
			Php::Value vF = params[5];
			if (vF.isNumeric()) {
				flags = (unsigned int)vF.numericValue();
			}
			else {
				throw Php::Exception("One or more file writing flags (1=truncated, 2=appended, 4=shared read and 8=shared writing) required");
			}
		}
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_sh->Download(local.c_str(), remote.c_str(), Dl, Progress, discarded, flags), lk, timeout);
			return *pV;
		}
		return m_sh->Download(local.c_str(), remote.c_str(), Dl, Progress, discarded, flags);
	}

	CAsyncFile::DTransferring CPhpFile::SetTransCallback(Php::Value phpProgress) {
		if (phpProgress.isNull()) {
		}
		else if (!phpProgress.isCallable()) {
			throw Php::Exception("A callback required for file exchange progress");
		}
		CAsyncFile::DTransferring Progress = [phpProgress](SPA::ClientSide::CStreamingFile *file, SPA::UINT64 uploaded) {
			if (phpProgress.isCallable()) {
				phpProgress((int64_t)uploaded, (int64_t)file->GetFileSize());
			}
		};
		return Progress;
	}

	Php::Value CPhpFile::Upload(Php::Parameters &params) {
		unsigned int timeout;
		std::shared_ptr<Php::Value> pV;
		std::wstring local, remote;
		MapFilePaths(params[0], params[1], local, remote);
		Php::Value phpUl = params[2];
		auto Ul = SetResCallback(phpUl, pV, timeout);

		Php::Value phpProgress;
		size_t args = params.size();
		if (args > 3) {
			phpProgress = params[3];
		}
		auto Progress = SetTransCallback(phpProgress);
		
		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::SFile::idUpload, pV ? true : false);
		unsigned int flags = SPA::SFile::FILE_OPEN_TRUNCACTED;
		if (args > 5) {
			Php::Value vF = params[5];
			if (vF.isNumeric()) {
				flags = (unsigned int)vF.numericValue();
			}
			else {
				throw Php::Exception("One or more file writing flags (1=truncated, 2=appended, 4=shared read and 8=shared writing) required");
			}
		}
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_sh->Upload(local.c_str(), remote.c_str(), Ul, Progress, discarded, flags), lk, timeout);
			return *pV;
		}
		return m_sh->Upload(local.c_str(), remote.c_str(), Ul, Progress, discarded, flags);
	}

	Php::Value CPhpFile::__get(const Php::Value &name) {
		if (name == "FileSize") {
			return (int64_t)m_sh->GetFileSize();
		}
		else if (name == "FilesQueued") {
			return (int64_t)m_sh->GetFilesQueued();
		}
		else {
			return CPhpBaseHandler::__get(name);
		}
	}

} //namespace PA
