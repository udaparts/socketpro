#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(CPhpFilePool *pool, CAsyncFile *sh, bool locked) : m_filePool(pool), m_sh(sh), m_locked(locked) {
	}

	CPhpFile::~CPhpFile() {
		if (m_locked && m_sh && m_filePool) {
			m_filePool->Unlock(m_sh->GetAttachedClientSocket());
		}
	}

	void CPhpFile::__construct(Php::Parameters &params) {

	}

	bool CPhpFile::IsLocked() {
		return m_locked;
	}

	Php::Value CPhpFile::SendRequest(Php::Parameters &params) {
		if (m_sh) {
			return m_sh->SendRequest(params);
		}
		return false;
	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpFile::__construct, Php::Private);
		handler.method(PHP_SENDREQUEST, &CPhpFile::SendRequest, {
			Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, PHP_BUFFER, true, false),
			Php::ByVal(PHP_SENDREQUEST_RH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_CH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_EX, Php::Type::Callable, false)
		});
		cs.add(handler);
	}

} //namespace PA