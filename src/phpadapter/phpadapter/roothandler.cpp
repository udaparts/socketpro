#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CRootHandler::CRootHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked) : m_pPool(pool), m_pHandler(pHandler), m_locked(locked) {
	}

	CRootHandler::~CRootHandler() {
		if (m_locked && m_pHandler && m_pPool) {
			m_pPool->Unlock(m_pHandler->GetAttachedClientSocket());
		}
	}

	void CRootHandler::__construct(Php::Parameters &params) {

	}

	bool CRootHandler::IsLocked() {
		return m_locked;
	}

	Php::Value CRootHandler::SendRequest(Php::Parameters &params) {
		if (m_pHandler) {
			return m_pHandler->SendRequest(params);
		}
		return false;
	}

	void CRootHandler::RegisterInto(Php::Namespace &cs) {
		Php::Class<CRootHandler> handler(PHP_ASYNC_HANDLER);
		handler.method(PHP_CONSTRUCT, &CRootHandler::__construct, Php::Private);
		handler.method("SendRequest", &CRootHandler::SendRequest, {
			Php::ByVal("reqId", Php::Type::Numeric),
			Php::ByVal("buff", PHP_BUFFER, true, false),
			Php::ByVal("rh", Php::Type::Callable, false),
			Php::ByVal("ch", Php::Type::Callable, false),
			Php::ByVal("ex", Php::Type::Callable, false)
		});
		cs.add(handler);
	}

} //namespace PA