#ifndef SPA_PHP_ASYNC_QUEUE_HANDER_H
#define SPA_PHP_ASYNC_QUEUE_HANDER_H

#include "roothandler.h"

namespace PA {

	class CPhpQueue : public CRootHandler
	{
	public:
		CPhpQueue(SPA::ClientSide::CAsyncQueue *aq, bool locked);
		CPhpQueue(const CPhpQueue &q) = delete;
		~CPhpQueue();

	public:
		CPhpQueue& operator=(const CPhpQueue &q) = delete;
		static void RegisterInto(Php::Namespace &cs);

	private:
		SPA::ClientSide::CAsyncQueue *m_aq;
	};

} //namespace PA

#endif