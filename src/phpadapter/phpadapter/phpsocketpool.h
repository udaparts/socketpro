#ifndef SPA_PHP_SOCKETPOOL_H
#define SPA_PHP_SOCKETPOOL_H

#include "roothandler.h"
#include "phpdb.h"
#include "phpfile.h"
#include "phpqueue.h"

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
		Php::Value Seek();
		Php::Value SeekByQueue(Php::Parameters &params);
		Php::Value Lock(Php::Parameters &params);
		Php::Value Start(Php::Parameters &params);

	private:
		static void ToCtx(const Php::Value &vCtx, SPA::ClientSide::CConnectionContext &ctx);

	private:
		SPA::CUCriticalSection m_cs;
		unsigned int m_nSvsId;
		union {
			CPhpPool *Handler;
			CPhpDbPool *Db;
			CPhpFilePool *File;
			CPhpQueuePool *Queue;
		};
		std::string m_defaultDb;
		int m_errCode;
		std::string m_errMsg;
		Php::Value m_pe;
		tagPoolType m_pt;
		Php::Value m_ssl;

		static const char* NOT_INITIALIZED;
	};

} //namespace PA

#endif