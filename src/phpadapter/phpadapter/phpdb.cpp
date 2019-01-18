#include "stdafx.h"
#include "phpdb.h"

namespace PA {

	CPhpDb::CPhpDb(CDBHandler *db, bool locked) : CRootHandler(db, locked), m_db(db) {
	
	}

	CPhpDb::~CPhpDb()
	{

	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> db("CAsyncDBHandler");

		cs.add(db);
	}
} //namespace PA