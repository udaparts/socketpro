
#ifndef PHP_SPA_CLIENT_BASE_HANDLER_H
#define PHP_SPA_CLIENT_BASE_HANDLER_H

namespace PA {
	
	enum enumCallbackType {
		ctSendRequest = 0,
		ctDiscarded,
		ctServerException,
		ctFile,
		ctQueueTrans,
		ctEnqueueRes,
		ctDbRes,
		ctDbExeRes,
		ctDbR,
		ctDbRH
	};

	struct PACallback {
		enumCallbackType CallbackType;
		std::shared_ptr<Php::Value> CallBack;
		SPA::CUQueue *Res;
	};

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
		void __destruct();

	public:
		static void RegisterInto(Php::Class<CPhpBaseHandler> &h, Php::Namespace &cs);

	private:
		Php::Value Unlock();
		Php::Value SendRequest(Php::Parameters &params);
		void __construct(Php::Parameters &params);
		Php::Value WaitAll(Php::Parameters &params);
		Php::Value StartBatching();
		Php::Value CommitBatching();
		Php::Value AbortBatching();
		Php::Value CleanCallbacks(Php::Parameters &params);
		
	protected:
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded SetAbortCallback(const Php::Value& phpCanceled, unsigned short reqId, bool sync);
		void ReqSyncEnd(bool ok, std::unique_lock<std::mutex> &lk, unsigned int timeout);
		virtual void PopTopCallbacks(PACallback &cb) = 0;
		void PopCallbacks();

	protected:
		std::mutex m_mPhp;
		std::condition_variable m_cvPhp;
		std::deque<PACallback> m_vCallback;

	private:
		bool m_locked;
		SPA::ClientSide::CAsyncServiceHandler *m_h;
		unsigned int m_PoolId;

		enum tagRequestReturnStatus {
			rrsServerException = -3,
			rrsCanceled = -2,
			rrsTimeout = -1,
			rrsClosed = 0,
			rrsOk = 1,
		};
		tagRequestReturnStatus m_rrs;
	};
}

#endif
