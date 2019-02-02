#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CPhpHandler::CPhpHandler(unsigned int poolId, CAsyncHandler *pHandler, bool locked)
		: CPhpBaseHandler(locked, pHandler, poolId), m_pHandler(pHandler) {
	}

	void CPhpHandler::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpHandler> handler(PHP_ASYNC_HANDLER);
		Register(handler);
		cs.add(handler);
	}

} //namespace PA