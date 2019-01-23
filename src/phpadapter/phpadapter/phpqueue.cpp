#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(CPhpQueuePool *pool, CAsyncQueue *aq, bool locked) : m_queuePool(pool), m_aq(aq), m_locked(locked) {
	}

	CPhpQueue::~CPhpQueue() {
		if (m_locked && m_aq && m_queuePool) {
			m_queuePool->Unlock(m_aq->GetAttachedClientSocket());
		}
	}

	void CPhpQueue::__construct(Php::Parameters &params) {

	}

	int CPhpQueue::__compare(const CPhpQueue &q) const {
		if (!m_aq || !q.m_aq) {
			return 1;
		}
		return (m_aq == q.m_aq) ? 0 : 1;
	}

	bool CPhpQueue::IsLocked() {
		return m_locked;
	}

	Php::Value CPhpQueue::SendRequest(Php::Parameters &params) {
		if (m_aq) {
			return m_aq->SendRequest(params);
		}
		return false;
	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpQueue::__construct, Php::Private);
		handler.method(PHP_SENDREQUEST, &CPhpQueue::SendRequest, {
			Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, PHP_BUFFER, true, false),
			Php::ByVal(PHP_SENDREQUEST_RH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_CH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_EX, Php::Type::Callable, false)
		});
		cs.add(handler);
	}
}