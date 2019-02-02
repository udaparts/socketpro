#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(unsigned int poolId, CAsyncFile *sh, bool locked)
		: CPhpBaseHandler(locked, sh, poolId), m_sh(sh) {
	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
		Register(handler);
		cs.add(handler);
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