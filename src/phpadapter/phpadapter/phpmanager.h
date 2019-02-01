#ifndef SPA_PHP_MANAGER_H
#define SPA_PHP_MANAGER_H

#include "poolstartcontext.h"

namespace PA {

	class CPhpManager : public Php::Base {
	private:
		CPhpManager(CPhpManager *manager);

	public:
		CPhpManager(const CPhpManager &m) = delete;
		~CPhpManager();

	public:
		CPhpManager& operator=(const CPhpManager &m) = delete;
		static Php::Value Parse();
		static CPhpManager Manager;
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);
		CConnectionContext FindByKey(const std::string &key);
		std::string GetErrorMsg();

	private:
		void SetErrorMsg(const std::string &em);
		Php::Value GetPool(Php::Parameters &params);
		void __construct(Php::Parameters &params);
		void CheckError();
		void Clean();
		Php::Value GetConfig();
		void CheckHostsError();
		void CheckPoolsError();
		bool FindHostKey(const std::string &key);
		void SetSettings();
		static size_t ComputeDiff(const std::vector<std::string> &v);

	private:
		SPA::CUCriticalSection m_cs;
		CPhpManager *m_pManager;
		std::string m_ConfigPath;
		std::string WorkingDir;
		std::string CertStore;
		CMapHost Hosts;
		CPoolStartContext::CMapPool Pools;
		int m_bQP;
		std::string m_errMsg;
		std::string m_jsonConfig;
		std::vector<std::string> m_vKeyAllowed;
		friend class CPoolStartContext;
	};

}

#endif