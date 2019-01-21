#include "stdafx.h"
#include "phpdb.h"

namespace PA {

	CPhpDb::CPhpDb(CDBHandler *db, bool locked) : CRootHandler(db, locked), m_db(db) {
	
	}

	CPhpDb::~CPhpDb()
	{

	}

	void CPhpDb::__construct(Php::Parameters &params) {

	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> handler("CAsyncDb");
		handler.method("__construct", &CPhpDb::__construct, Php::Private);
		cs.add(handler);
	}
} //namespace PA