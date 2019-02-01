#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(CPhpQueuePool *pool, CAsyncQueue *aq, bool locked) 
		: CPhpBaseHandler(locked, aq, pool->GetPoolId()),
		m_queuePool(pool), m_aq(aq) {
	}

	int CPhpQueue::__compare(const CPhpQueue &q) const {
		if (!m_aq || !q.m_aq) {
			return 1;
		}
		return (m_aq == q.m_aq) ? 0 : 1;
	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		Register(handler);
		cs.add(handler);
	}
}
