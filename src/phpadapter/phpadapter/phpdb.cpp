#include "stdafx.h"
#include "phpdb.h"

namespace PA {

	CPhpDb::CPhpDb(unsigned int poolId, CDBHandler *db, bool locked)
		: CPhpBaseHandler(locked, db, poolId), m_db(db) {
	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> handler(PHP_DB_HANDLER);
		Register(handler);
		cs.add(handler);
	}
} //namespace PA