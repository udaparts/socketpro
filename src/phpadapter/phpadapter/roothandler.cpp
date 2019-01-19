#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CRootHandler::CRootHandler(SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked) : m_pHandler(pHandler), m_locked(locked) {
	}

	CRootHandler::~CRootHandler() {
	}

} //namespace PA