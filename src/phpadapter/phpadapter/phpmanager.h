#ifndef SPA_PHP_MANAGER_H
#define SPA_PHP_MANAGER_H

#include "roothandler.h"
#include "phpdb.h"
#include "phpfile.h"
#include "phpqueue.h"

namespace PA {

	typedef std::unordered_map<std::string, CConnectionContext> CMapHost;

	struct CPoolStartContext {
		CPoolStartContext() 
			: SvsId(0), Threads(1), AutoConn(true), AutoMerge(true),
			RecvTimeout(SPA::ClientSide::DEFAULT_RECV_TIMEOUT),
			ConnTimeout(SPA::ClientSide::DEFAULT_CONN_TIMEOUT),
			PhpHandler(nullptr) {
		}
		bool IsNormalPool() const { return (DefaultDb.size() == 0); }
		bool IsMaster() const { return (DefaultDb.size() && Slaves.size()); }
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
	};

	class CPhpManager {
	public:
		CPhpManager(const char *config_path);
		CPhpManager(const CPhpManager &m) = delete;
		~CPhpManager();

	public:
		CPhpManager& operator=(const CPhpManager &m) = delete;
		std::string Parse();

	private:
		std::string m_ConfigPath;

		std::string WorkingDir;
		std::string CertStore;
		CMapHost Hosts;
		CPoolStartContext::CMapPool Pools;
	};

}

#endif