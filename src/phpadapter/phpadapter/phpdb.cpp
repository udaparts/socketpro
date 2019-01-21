#include "stdafx.h"
#include "phpdb.h"

namespace PA {

	CPhpDb::CPhpDb(CPhpPool *pool, CDBHandler *db, bool locked) : CRootHandler(pool, db, locked), m_db(db) {
	
	}

	void CPhpDb::__construct(Php::Parameters &params) {

	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> handler(PHP_DB_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpDb::__construct, Php::Private);
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