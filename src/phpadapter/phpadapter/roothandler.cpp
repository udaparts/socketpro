#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CPhpHandler::CPhpHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked) 
		: CPhpBaseHandler<CPhpHandler>(locked, pHandler, pool->GetPoolId()),
		m_pPool(pool), m_pHandler(pHandler) {
	}

	int CPhpHandler::__compare(const CPhpHandler &h) const {
		if (!m_pHandler || !h.m_pHandler) {
			return 1;
		}
		return (m_pHandler == h.m_pHandler) ? 0 : 1;
	}

	void CPhpHandler::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpHandler> handler(PHP_ASYNC_HANDLER);
		CPhpBaseHandler<CPhpHandler>::RegInto(handler, cs);
		cs.add(handler);
	}

} //namespace PA