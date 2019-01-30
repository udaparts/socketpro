#include "stdafx.h"
#include "phpdb.h"

namespace PA {

	CPhpDb::CPhpDb(CPhpDbPool *pool, CDBHandler *db, bool locked) : CPhpBaseHandler<CPhpDb>(locked, db), m_dbPool(pool), m_db(db) {
	}

	CPhpDb::~CPhpDb() {
		if (IsLocked() && m_db && m_dbPool) {
			m_dbPool->Unlock(m_db->GetAttachedClientSocket());
		}
	}

	int CPhpDb::__compare(const CPhpDb &db) const {
		if (!m_db || !db.m_db) {
			return 1;
		}
		return (m_db == db.m_db) ? 0 : 1;
	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> handler(PHP_DB_HANDLER);
		CPhpBaseHandler<CPhpDb>::RegInto(handler, cs);
		cs.add(handler);
	}
} //namespace PA