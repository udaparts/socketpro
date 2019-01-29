#ifndef SPA_PHP_SOCKETPOOL_H
#define SPA_PHP_SOCKETPOOL_H

#include "poolstartcontext.h"

namespace PA {

	class CPhpSocketPool : public Php::Base {
	public:
		CPhpSocketPool(const CPoolStartContext &psc);
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
		Php::Value Seek();
		Php::Value SeekByQueue(Php::Parameters &params);
		Php::Value Lock(Php::Parameters &params);
		Php::Value Start(Php::Parameters &params);
		int __compare(const CPhpSocketPool &pool) const;

	private:
		static void ToCtx(const Php::Value &vCtx, SPA::ClientSide::CConnectionContext &ctx);
		bool DoSSLAuth(CClientSocket *cs);

	private:
		unsigned int m_nSvsId;
		union {
			CPhpPool *Handler;
			CPhpDbPool *Db;
			CPhpFilePool *File;
			CPhpQueuePool *Queue;
		};
		std::string m_defaultDb;
		tagPoolType m_pt;
		static const char* NOT_INITIALIZED;
	};

} //namespace PA

#endif