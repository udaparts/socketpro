#ifndef SPA_PHP_ROOT_HANDER_H
#define SPA_PHP_ROOT_HANDER_H

namespace PA {

typedef SPA::ClientSide::CSocketPool<CAsyncHandler> CPhpPool;
typedef SPA::CMasterPool<false, CAsyncHandler> CMasterPool;

class CPhpHandler : public Php::Base
{
public:
	CPhpHandler(CPhpPool *pool, SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked);
	CPhpHandler(const CPhpHandler &rh) = delete;
	~CPhpHandler();

public:
	CPhpHandler& operator=(const CPhpHandler &rh) = delete;
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
