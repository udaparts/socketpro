#ifndef SPA_PHP_ROOT_HANDER_H
#define SPA_PHP_ROOT_HANDER_H

#include "basehandler.h"

namespace PA {

typedef SPA::ClientSide::CSocketPool<CAsyncHandler> CPhpPool;
typedef SPA::CMasterPool<false, CAsyncHandler> CMasterPool;

class CPhpHandler : public CPhpBaseHandler<CPhpHandler>
{
public:
	CPhpHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked);
	CPhpHandler(const CPhpHandler &rh) = delete;

public:
	CPhpHandler& operator=(const CPhpHandler &rh) = delete;
	static void RegisterInto(Php::Namespace &cs);
	int __compare(const CPhpHandler &h) const;

private:
	CPhpPool *m_pPool;
	SPA::ClientSide::CAsyncServiceHandler *m_pHandler;
};

} //namespace PA
#endif
