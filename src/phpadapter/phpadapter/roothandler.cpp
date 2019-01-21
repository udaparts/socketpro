#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CRootHandler::CRootHandler(SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked) : m_pHandler(pHandler), m_locked(locked) {
	}

	CRootHandler::~CRootHandler() {
	}

	void CRootHandler::__construct(Php::Parameters &params) {

	}

	void CRootHandler::RegisterInto(Php::Namespace &cs) {
		Php::Class<CRootHandler> handler("CAsyncHandler");
		handler.method("__construct", &CRootHandler::__construct, Php::Private);
		cs.add(handler);
	}

} //namespace PA