#ifndef SPA_PHP_ROOT_HANDER_H
#define SPA_PHP_ROOT_HANDER_H

namespace PA {

typedef SPA::ClientSide::CSocketPool<CAsyncHandler> CPhpPool;

class CRootHandler : public Php::Base
{
public:
	CRootHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked);
	CRootHandler(const CRootHandler &rh) = delete;
	~CRootHandler();

public:
	CRootHandler& operator=(const CRootHandler &rh) = delete;
	void __construct(Php::Parameters &params);
	static void RegisterInto(Php::Namespace &cs);
	Php::Value SendRequest(Php::Parameters &params);
	bool IsLocked();

private:
	CPhpPool *m_pPool;
	SPA::ClientSide::CAsyncServiceHandler *m_pHandler;
	bool m_locked;
};

} //namespace PA
#endif
