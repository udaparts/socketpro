#include "stdafx.h"
#include "phpfile.h"

namespace PA {

	CPhpFile::CPhpFile(SPA::ClientSide::CStreamingFile *sh, bool locked) : CRootHandler(sh, locked), m_sh(sh) {
	}

	CPhpFile::~CPhpFile()
	{

	}

	void CPhpFile::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpFile> file("CStreamingFile");

		cs.add(file);
	}

} //namespace PA