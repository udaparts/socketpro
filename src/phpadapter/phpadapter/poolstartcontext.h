#ifndef PHP_POOL_START_CONTEXT
#define PHP_POOL_START_CONTEXT

#include "roothandler.h"
#include "phpdb.h"
#include "phpfile.h"
#include "phpqueue.h"

namespace PA {

	typedef std::unordered_map<std::string, CConnectionContext> CMapHost;

	class CPoolStartContext {

	public:
		CPoolStartContext();
	
	public:
		unsigned int SvsId;
		std::vector<std::string> Hosts;
		unsigned int Threads;
		std::string Queue;
		bool AutoConn;
		bool AutoMerge;
		unsigned int RecvTimeout;
		unsigned int ConnTimeout;
		std::string DefaultDb;
		typedef std::unordered_map<std::string, CPoolStartContext> CMapPool;
		CMapPool Slaves;
		union {
			CPhpPool *PhpHandler;
			CPhpDbPool *PhpDb;
			CPhpFilePool *PhpFile;
			CPhpQueuePool *PhpQueue;
		};
		tagPoolType PoolType;

	private:
		int m_errCode;
		std::string m_errMsg;

	public:
		Php::Value GetPool();
		void Clean();

	protected:
		bool DoSSLAuth(CClientSocket *cs);
		void DealWithPoolEvents(tagSocketPoolEvent spe);

	private:
		std::string StartPool();
		
	};
} //namespace PA


#endif
