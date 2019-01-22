#include "stdafx.h"
#include "roothandler.h"

namespace PA {

	CPhpHandler::CPhpHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked) : m_pPool(pool), m_pHandler(pHandler), m_locked(locked) {
	}

	CPhpHandler::~CPhpHandler() {
		if (m_locked && m_pHandler && m_pPool) {
			m_pPool->Unlock(m_pHandler->GetAttachedClientSocket());
		}
	}

	void CPhpHandler::__construct(Php::Parameters &params) {

	}

	bool CPhpHandler::IsLocked() {
		return m_locked;
	}

	Php::Value CPhpHandler::SendRequest(Php::Parameters &params) {
		if (m_pHandler) {
			return m_pHandler->SendRequest(params);
		}
		return false;
	}

	void CPhpHandler::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpHandler> handler(PHP_ASYNC_HANDLER);
		handler.method(PHP_CONSTRUCT, &CPhpHandler::__construct, Php::Private);
		handler.method(PHP_SENDREQUEST, &CPhpHandler::SendRequest, {
			Php::ByVal(PHP_SENDREQUEST_REQID, Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_BUFF, PHP_BUFFER, true, false),
			Php::ByVal(PHP_SENDREQUEST_RH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_CH, Php::Type::Callable, false),
			Php::ByVal(PHP_SENDREQUEST_EX, Php::Type::Callable, false)
		});
		cs.add(handler);
	}

} //namespace PA