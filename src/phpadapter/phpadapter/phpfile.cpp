#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(unsigned int poolId, CAsyncFile *sh, bool locked)
		: CPhpBaseHandler(locked, sh, poolId), m_sh(sh) {
	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
		Register(handler);
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

	Php::Value CPhpFile::Download(Php::Parameters &params) {
		std::string local = params[0].stringValue();
		Trim(local);
		if (!local.size()) {
			throw Php::Exception("Local file path can not be empty");
		}
		std::string remote = params[1].stringValue();
		Trim(remote);
		if (!remote.size()) {
			throw Php::Exception("remote file path can not be empty");
		}
		std::wstring localFile = SPA::Utilities::ToWide(local.c_str(), local.size());
		std::wstring remoteFile = SPA::Utilities::ToWide(remote.c_str(), remote.size());
	
		unsigned int timeout = (~0);
		bool sync = false;
		Php::Value phpDl = params[2];
		if (phpDl.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDl.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDl.isBool()) {
			sync = phpDl.boolValue();
		}
		else if (phpDl.isNull()) {
		}
		else if (!phpDl.isCallable()) {
			throw Php::Exception("A callback required for download final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		SPA::ClientSide::CStreamingFile::DDownload Dl = [sync, phpDl, pV, this](SPA::ClientSide::CStreamingFile *file, int res, const std::wstring& errMsg) {
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_sh->m_mPhp);
				pV->set("ec", res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				pV->set("em", em);
				this->m_sh->m_cvPhp.notify_all();
			}
			else if (phpDl.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				phpDl(res, em);
			}
		};

		Php::Value phpProgress;
		size_t args = params.size();
		if (args > 3) {
			phpProgress = params[3];
			if (phpProgress.isNull()) {
			}
			else if (!phpProgress.isCallable()) {
				throw Php::Exception("A callback required for downloading progress");
			}
		}
		SPA::ClientSide::CStreamingFile::DTransferring Progress = [phpProgress](SPA::ClientSide::CStreamingFile *file, SPA::UINT64 downloaded) {
			if (phpProgress.isCallable()) {
				phpProgress((int64_t)downloaded, (int64_t)file->GetFileSize());
			}
		};

		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for download aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_sh->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_sh->m_cvPhp.notify_all();
			}
		};
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
		if (sync) {
			std::unique_lock<std::mutex> lk(m_sh->m_mPhp);
			if (!m_sh->Download(localFile.c_str(), remoteFile.c_str(), Dl, Progress, discarded, flags)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			
			auto status = m_sh->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_sh->Download(localFile.c_str(), remoteFile.c_str(), Dl, Progress, discarded, flags) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpFile::Upload(Php::Parameters &params) {
		std::string local = params[0].stringValue();
		Trim(local);
		if (!local.size()) {
			throw Php::Exception("Local file path can not be empty");
		}
		std::string remote = params[1].stringValue();
		Trim(remote);
		if (!remote.size()) {
			throw Php::Exception("remote file path can not be empty");
		}
		std::wstring localFile = SPA::Utilities::ToWide(local.c_str(), local.size());
		std::wstring remoteFile = SPA::Utilities::ToWide(remote.c_str(), remote.size());

		unsigned int timeout = (~0);
		bool sync = false;
		Php::Value phpUl = params[2];
		if (phpUl.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpUl.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpUl.isBool()) {
			sync = phpUl.boolValue();
		}
		else if (phpUl.isNull()) {
		}
		else if (!phpUl.isCallable()) {
			throw Php::Exception("A callback required for upload final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		SPA::ClientSide::CStreamingFile::DUpload Ul = [sync, phpUl, pV, this](SPA::ClientSide::CStreamingFile *file, int res, const std::wstring& errMsg) {
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_sh->m_mPhp);
				pV->set("ec", res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				pV->set("em", em);
				this->m_sh->m_cvPhp.notify_all();
			}
			else if (phpUl.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				phpUl(res, em);
			}
		};

		Php::Value phpProgress;
		size_t args = params.size();
		if (args > 3) {
			phpProgress = params[3];
			if (phpProgress.isNull()) {
			}
			else if (!phpProgress.isCallable()) {
				throw Php::Exception("A callback required for uploading progress");
			}
		}
		SPA::ClientSide::CStreamingFile::DTransferring Progress = [phpProgress](SPA::ClientSide::CStreamingFile *file, SPA::UINT64 downloaded) {
			if (phpProgress.isCallable()) {
				phpProgress((int64_t)downloaded, (int64_t)file->GetFileSize());
			}
		};

		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for upload aborting event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_sh->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_sh->m_cvPhp.notify_all();
			}
		};
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
		if (sync) {
			std::unique_lock<std::mutex> lk(m_sh->m_mPhp);
			if (!m_sh->Upload(localFile.c_str(), remoteFile.c_str(), Ul, Progress, discarded, flags)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_sh->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_sh->Upload(localFile.c_str(), remoteFile.c_str(), Ul, Progress, discarded, flags) ? rrsOk : rrsClosed;
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
