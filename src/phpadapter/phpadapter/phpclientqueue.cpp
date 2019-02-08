#include "stdafx.h"
#include "phpclientqueue.h"

namespace PA {

	CPhpClientQueue::CPhpClientQueue(CClientQueue &cq) : m_cq(cq) {
	}

	void CPhpClientQueue::__construct(Php::Parameters &params) {
	}

	void CPhpClientQueue::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpClientQueue> cq(PHP_CLIENTQUEUE);
		cq.method(PHP_CONSTRUCT, &CPhpClientQueue::__construct, Php::Private);

		cs.add(cq);
	}
}
