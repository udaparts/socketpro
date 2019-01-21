#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(SPA::ClientSide::CStreamingFile *sh, bool locked) : CRootHandler(sh, locked), m_sh(sh) {
	}

	CPhpFile::~CPhpFile()
	{

	}

	void CPhpFile::__construct(Php::Parameters &params) {

	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> handler("CAsyncFile");
		handler.method("__construct", &CPhpFile::__construct, Php::Private);
		cs.add(handler);
	}

} //namespace PA