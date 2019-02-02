#ifndef SPA_PHP_ASYNC_QUEUE_HANDER_H
#define SPA_PHP_ASYNC_QUEUE_HANDER_H

#include "basehandler.h"

namespace PA {

	typedef SPA::ClientSide::CAsyncQueue CAsyncQueue;
	typedef SPA::ClientSide::CSocketPool<CAsyncQueue> CPhpQueuePool;

	class CPhpQueue : public CPhpBaseHandler
	{
	public:
		CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked);
		CPhpQueue(const CPhpQueue &q) = delete;

	public:
		CPhpQueue& operator=(const CPhpQueue &q) = delete;
		static void RegisterInto(Php::Namespace &cs);

	private:
		CAsyncQueue *m_aq;
	};

} //namespace PA

#endif