#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(CPhpPool *pool, CAsyncQueue *aq, bool locked) : CRootHandler(pool, aq, locked), m_aq(aq) {
	}

	void CPhpQueue::__construct(Php::Parameters &params) {

	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler(PHP_QUEUE_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpQueue::__construct, Php::Private);
		handler.method("SendRequest", &CRootHandler::SendRequest, {
			Php::ByVal("reqId", Php::Type::Numeric),
			Php::ByVal("buff", PHP_BUFFER, true, false),
			Php::ByVal("rh", Php::Type::Callable, false),
			Php::ByVal("ch", Php::Type::Callable, false),
			Php::ByVal("ex", Php::Type::Callable, false)
		});
		cs.add(handler);
	}
}