#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked)
		: CPhpBaseHandler(locked, aq, poolId), m_aq(aq) {
	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		Register(handler);
		cs.add(handler);
	}
}
