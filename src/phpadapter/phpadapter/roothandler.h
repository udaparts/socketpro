#ifndef SPA_PHP_ROOT_HANDER_H
#define SPA_PHP_ROOT_HANDER_H

namespace PA {

class CRootHandler : public Php::Base
{
public:
	CRootHandler(SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked);
	CRootHandler(const CRootHandler &rh) = delete;
	~CRootHandler();

public:
	CRootHandler& operator=(const CRootHandler &rh) = delete;
	void __construct(Php::Parameters &params);
	static void RegisterInto(Php::Namespace &cs);

protected:
	SPA::ClientSide::CAsyncServiceHandler *m_pHandler;
	bool m_locked;
};

} //namespace PA
#endif
