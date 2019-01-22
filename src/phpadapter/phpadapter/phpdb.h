#ifndef SPA_PHP_DATABASE_H
#define SPA_PHP_DATABASE_H

namespace PA {

	typedef SPA::ClientSide::CAsyncDBHandler<0> CDBHandler;
	typedef SPA::ClientSide::CSocketPool<CDBHandler> CPhpDbPool;

	class CPhpDb : public Php::Base
	{
	public:
		CPhpDb(CPhpDbPool *pool, CDBHandler *db, bool locked);
		CPhpDb(const CPhpDb &db) = delete;
		~CPhpDb();

	public:
		CPhpDb& operator=(const CPhpDb &db) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);
		Php::Value SendRequest(Php::Parameters &params);
		bool IsLocked();

	private:
		CPhpDbPool *m_dbPool;
		CDBHandler *m_db;
		bool m_locked;
	};

} //namespace PA
#endif
