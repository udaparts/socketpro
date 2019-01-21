#ifndef SPA_PHP_DATABASE_H
#define SPA_PHP_DATABASE_H

#include "roothandler.h"

namespace PA {

	typedef SPA::ClientSide::CAsyncDBHandler<0> CDBHandler;

	class CPhpDb : public CRootHandler
	{
	public:
		CPhpDb(CDBHandler *db, bool locked);
		CPhpDb(const CPhpDb &db) = delete;
		~CPhpDb();

	public:
		CPhpDb& operator=(const CPhpDb &db) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);

	private:
		CDBHandler *m_db;
	};

} //namespace PA
#endif
