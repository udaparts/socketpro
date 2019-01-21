#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(SPA::ClientSide::CAsyncQueue *aq, bool locked) : CRootHandler(aq, locked), m_aq(aq) {
	}

	CPhpQueue::~CPhpQueue() {
	}

	void CPhpQueue::__construct(Php::Parameters &params) {

	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpQueue> handler("CAsyncQueue");
		handler.method("__construct", &CPhpQueue::__construct, Php::Private);
		cs.add(handler);
	}
}