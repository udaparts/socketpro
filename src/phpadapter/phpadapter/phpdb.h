#ifndef SPA_PHP_DATABASE_H
#define SPA_PHP_DATABASE_H

#include "phpbuffer.h"
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
		static void RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);

	protected:
		void PopTopCallbacks(PACallback &cb);

	private:
		Php::Value Open(Php::Parameters &params);
		Php::Value Execute(Php::Parameters &params);
		Php::Value ExecuteBatch(Php::Parameters &params);
		Php::Value Prepare(Php::Parameters &params);
		Php::Value Close(Php::Parameters &params);
		Php::Value BeginTrans(Php::Parameters &params);
		Php::Value EndTrans(Php::Parameters &params);

		CDBHandler::DResult SetResCallback(const Php::Value &phpRes, CQPointer &pV, unsigned int &timeout);
		CDBHandler::DExecuteResult SetExeResCallback(const Php::Value &phpRes, CQPointer &pV, unsigned int &timeout);
		CDBHandler::DRows SetRCallback(const Php::Value &phpRow);
		CDBHandler::DRowsetHeader SetRHCallback(const Php::Value &phpRh, bool batch = false);
		void GetParams(const Php::Value &vP, SPA::UDB::CDBVariantArray &vParam);

		static SPA::UDB::CParameterInfoArray ConvertFrom(Php::Value vP);
		static Php::Value ToPhpValue(SPA::CUQueue *q);
		static Php::Value ToPhpValueEx(SPA::CUQueue *q);
		static const char *PHP_DB_AFFECTED;
		static const char *PHP_DB_FAILS;
		static const char *PHP_DB_OKS;
		static const char *PHP_DB_LAST_ID;

	private:
		CDBHandler *m_db;
	};

} //namespace PA
#endif
