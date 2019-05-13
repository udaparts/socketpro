#include "stdafx.h"
#include "roothandler.h"

namespace PA
{

    CPhpHandler::CPhpHandler(CAsyncHandler *pHandler, bool locked) : CPhpBaseHandler(locked, pHandler), m_pHandler(pHandler) {
    }

    void CPhpHandler::RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace & cs) {
        Php::Class<CPhpHandler> handler(PHP_ASYNC_HANDLER);
        handler.extends(base);
        cs.add(handler);
    }

    void CPhpHandler::PopTopCallbacks(PACallback & cb) {
    }

} //namespace PA