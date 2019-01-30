#ifndef SPA_PHP_DATABASE_H
#define SPA_PHP_DATABASE_H

#include "basehandler.h"

namespace PA {

	typedef SPA::ClientSide::CAsyncDBHandler<0> CDBHandler;
	typedef SPA::ClientSide::CSocketPool<CDBHandler> CPhpDbPool;
	typedef SPA::CSQLMasterPool<false, CDBHandler> CSQLMaster;

	class CPhpDb : public CPhpBaseHandler<CPhpDb>, public Php::Base
	{
	public:
		CPhpDb(CPhpDbPool *pool, CDBHandler *db, bool locked);
		CPhpDb(const CPhpDb &db) = delete;
		~CPhpDb();

	public:
		CPhpDb& operator=(const CPhpDb &db) = delete;
		static void RegisterInto(Php::Namespace &cs);
		int __compare(const CPhpDb &db) const;

	private:
		CPhpDbPool *m_dbPool;
		CDBHandler *m_db;
	};

} //namespace PA
#endif
