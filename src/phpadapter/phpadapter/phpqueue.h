#ifndef SPA_PHP_ASYNC_QUEUE_HANDER_H
#define SPA_PHP_ASYNC_QUEUE_HANDER_H

#include "roothandler.h"

namespace PA {

	typedef SPA::ClientSide::CAsyncQueue CAsyncQueue;

	class CPhpQueue : public CRootHandler
	{
	public:
		CPhpQueue(CPhpPool *pool, CAsyncQueue *aq, bool locked);
		CPhpQueue(const CPhpQueue &q) = delete;

	public:
		CPhpQueue& operator=(const CPhpQueue &q) = delete;
		void __construct(Php::Parameters &params);
		static void RegisterInto(Php::Namespace &cs);

	private:
		CAsyncQueue *m_aq;
	};

} //namespace PA

#endif