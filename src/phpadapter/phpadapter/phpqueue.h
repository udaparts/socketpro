#ifndef SPA_PHP_ASYNC_QUEUE_HANDER_H
#define SPA_PHP_ASYNC_QUEUE_HANDER_H

namespace PA {

	typedef SPA::ClientSide::CAsyncQueue CAsyncQueue;
	typedef SPA::ClientSide::CSocketPool<CAsyncQueue> CPhpQueuePool;

	class CPhpQueue : public Php::Base
	{
	public:
		CPhpQueue(CPhpQueuePool *pool, CAsyncQueue *aq, bool locked);
		CPhpQueue(const CPhpQueue &q) = delete;
		~CPhpQueue();

	public:
		CPhpQueue& operator=(const CPhpQueue &q) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);
		Php::Value SendRequest(Php::Parameters &params);
		bool IsLocked();

	private:
		CPhpQueuePool *m_queuePool;
		CAsyncQueue *m_aq;
		bool m_locked;
	};

} //namespace PA

#endif