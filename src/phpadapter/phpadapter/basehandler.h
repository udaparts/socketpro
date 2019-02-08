
#include "phpbuffer.h"
#include "phpsocket.h"

#ifndef PHP_SPA_CLIENT_BASE_HANDLER_H
#define PHP_SPA_CLIENT_BASE_HANDLER_H

namespace PA {
	
	class CPhpBaseHandler : public Php::Base
	{
	protected:
		CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler *h, unsigned int poolId);
		CPhpBaseHandler(const CPhpBaseHandler& h) = delete;
		virtual ~CPhpBaseHandler();
	
	public:
		CPhpBaseHandler& operator=(const CPhpBaseHandler& h) = delete;
		Php::Value __get(const Php::Value &name);
		int __compare(const CPhpBaseHandler &pbh) const;
		unsigned int GetPoolId() const;

	protected:
		template <typename T>
		static void Register(Php::Class<T> &h) {
			h.method(PHP_CONSTRUCT, &T::__construct, Php::Private);
			h.method(PHP_SENDREQUEST, &T::SendRequest, {
				Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
				Php::ByVal(PHP_SENDREQUEST_BUFF, Php::Type::Null),
				Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
			});
			h.method("WaitAll", &T::WaitAll, {
				Php::ByVal(PHP_TIMEOUT, Php::Type::Numeric, false)
			});
			h.method("StartBatching", &T::StartBatching);
			h.method("AbortBatching", &T::AbortBatching);
			h.method("CommitBatching", &T::CommitBatching);
			h.method("Unlock", &T::Unlock);
			h.method("CleanCallbacks", &T::CleanCallbacks);
		}

		Php::Value Unlock();
		Php::Value SendRequest(Php::Parameters &params);
		void __construct(Php::Parameters &params);
		Php::Value WaitAll(Php::Parameters &params);
		Php::Value StartBatching();
		Php::Value CommitBatching();
		Php::Value AbortBatching();
		Php::Value CleanCallbacks(Php::Parameters &params);

		SPA::ClientSide::CAsyncServiceHandler::DDiscarded SetAbortCallback(const Php::Value& phpCanceled, unsigned short reqId, bool sync);
		void ReqSyncEnd(bool ok, std::unique_lock<std::mutex> &lk, unsigned int timeout);

	private:
		enum tagRequestReturnStatus {
			rrsServerException = -3,
			rrsCanceled = -2,
			rrsTimeout = -1,
			rrsClosed = 0,
			rrsOk = 1,
		};

		bool m_locked;
		SPA::ClientSide::CAsyncServiceHandler *m_h;
		unsigned int m_PoolId;
		tagRequestReturnStatus m_rrs;
		
	public:
		std::mutex m_mPhp;
		std::condition_variable m_cvPhp;
	};
}

#endif
