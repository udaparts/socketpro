#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(CPhpPool *pool, CAsyncFile *sh, bool locked) : CRootHandler(pool, sh, locked), m_sh(sh) {
	}

	void CPhpFile::__construct(Php::Parameters &params) {

	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler(PHP_FILE_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpFile::__construct, Php::Private);
		handler.method("SendRequest", &CRootHandler::SendRequest, {
			Php::ByVal("reqId", Php::Type::Numeric),
			Php::ByVal("buff", PHP_BUFFER, true, false),
			Php::ByVal("rh", Php::Type::Callable, false),
			Php::ByVal("ch", Php::Type::Callable, false),
			Php::ByVal("ex", Php::Type::Callable, false)
		});
		cs.add(handler);
	}

} //namespace PA