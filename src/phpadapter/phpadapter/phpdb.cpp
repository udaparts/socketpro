#include "stdafx.h"
#include "phpdb.h"

namespace PA {

	CPhpDb::CPhpDb(CPhpDbPool *pool, CDBHandler *db, bool locked) : m_dbPool(pool), m_db(db), m_locked(locked) {
	}

	CPhpDb::~CPhpDb() {
		if (m_locked && m_db && m_dbPool) {
			m_dbPool->Unlock(m_db->GetAttachedClientSocket());
		}
	}

	void CPhpDb::__construct(Php::Parameters &params) {
	}

	bool CPhpDb::IsLocked() {
		return m_locked;
	}

	int CPhpDb::__compare(const CPhpDb &db) const {
		if (!m_db || !db.m_db) {
			return 1;
		}
		return (m_db == db.m_db) ? 0 : 1;
	}

	Php::Value CPhpDb::SendRequest(Php::Parameters &params) {
		if (m_db) {
			return m_db->SendRequest(params);
		}
		return false;
	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> handler(PHP_DB_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpDb::__construct, Php::Private);
		handler.method(PHP_SENDREQUEST, &CPhpDb::SendRequest, {
			Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, PHP_BUFFER, true, false),
			Php::ByVal(PHP_SENDREQUEST_RH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_CH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_EX, Php::Type::Callable, false)
		});
		cs.add(handler);
	}
} //namespace PA