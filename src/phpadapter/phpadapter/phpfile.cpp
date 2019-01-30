#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(CPhpFilePool *pool, CAsyncFile *sh, bool locked) : CPhpBaseHandler<CPhpFile>(locked, sh), m_filePool(pool), m_sh(sh) {
	}

	CPhpFile::~CPhpFile() {
		if (IsLocked() && m_sh && m_filePool) {
			m_filePool->Unlock(m_sh->GetAttachedClientSocket());
		}
	}

	int CPhpFile::__compare(const CPhpFile &f) const {
		if (!m_sh || !f.m_sh) {
			return 1;
		}
		return (m_sh == f.m_sh) ? 0 : 1;
	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
		CPhpBaseHandler<CPhpFile>::RegInto(handler, cs);
		cs.add(handler);
	}

} //namespace PA