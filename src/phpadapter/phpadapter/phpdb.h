#ifndef SPA_PHP_DATABASE_H
#define SPA_PHP_DATABASE_H

#include "basehandler.h"

namespace PA {

	typedef SPA::ClientSide::CAsyncDBHandler<0> CDBHandler;
	typedef SPA::ClientSide::CSocketPool<CDBHandler> CPhpDbPool;
	typedef SPA::CSQLMasterPool<false, CDBHandler> CSQLMaster;

	class CPhpDb : public CPhpBaseHandler
	{
	public:
		CPhpDb(unsigned int poolId, CDBHandler *db, bool locked);
		CPhpDb(const CPhpDb &db) = delete;

	public:
		CPhpDb& operator=(const CPhpDb &db) = delete;
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);

	private:
		Php::Value Open(Php::Parameters &params);
		Php::Value Execute(Php::Parameters &params);
		Php::Value ExecuteBatch(Php::Parameters &params);
		Php::Value Prepare(Php::Parameters &params);
		Php::Value Close(Php::Parameters &params);
		Php::Value BeginTrans(Php::Parameters &params);
		Php::Value EndTrans(Php::Parameters &params);

	private:
		CDBHandler *m_db;
	};

} //namespace PA
#endif
