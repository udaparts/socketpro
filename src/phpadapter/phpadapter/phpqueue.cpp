#include "stdafx.h"
#include "phpqueue.h"

namespace PA {

	CPhpQueue::CPhpQueue(SPA::ClientSide::CAsyncQueue *aq, bool locked) : CRootHandler(aq, locked), m_aq(aq) {
	}

	CPhpQueue::~CPhpQueue() {
	}

	void CPhpQueue::RegisterInto(Php::Namespace &cs) {

	}
}