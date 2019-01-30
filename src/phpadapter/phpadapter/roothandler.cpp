#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CPhpHandler::CPhpHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked) : CPhpBaseHandler<CPhpHandler>(locked, pHandler), m_pPool(pool), m_pHandler(pHandler) {
	}

	CPhpHandler::~CPhpHandler() {
		if (IsLocked() && m_pHandler && m_pPool) {
			m_pPool->Unlock(m_pHandler->GetAttachedClientSocket());
		}
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