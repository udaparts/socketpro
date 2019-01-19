#ifndef SPA_PHP_SOCKETPOOL_H
#define SPA_PHP_SOCKETPOOL_H

namespace PA {

	class CPhpSocketPool : public Php::Base {
	public:
		CPhpSocketPool();
		CPhpSocketPool(const CPhpSocketPool &p) = delete;
		~CPhpSocketPool();

	public:
		CPhpSocketPool& operator=(const CPhpSocketPool &p) = delete;
		static void RegisterInto(Php::Namespace &cs);
		Php::Value __get(const Php::Value &name);
		void __set(const Php::Value &name, const Php::Value &value);
		void __construct(Php::Parameters &params);
		Php::Value NewSlave(Php::Parameters &params);
		void ShutdownPool(Php::Parameters &params);
		Php::Value DisconnectAll(Php::Parameters &params);

	private:
		SPA::CUCriticalSection m_cs;
		unsigned int m_nSvsId;
		union {
			SPA::ClientSide::CSocketPool<CAsyncHandler> *Handler;
			
			/*SPA::ClientSide::CSocketPool<CNjDb> *Db;
			SPA::ClientSide::CSocketPool<CSFile> *File;
			SPA::ClientSide::CSocketPool<CAQueue> *Queue;*/
		};
		std::string m_defaultDb;
		int m_errCode;
		std::string m_errMsg;
	};

} //namespace PA

#endif