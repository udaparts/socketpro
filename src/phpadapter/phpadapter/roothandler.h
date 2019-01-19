#ifndef SPA_PHP_ROOT_HANDER_H
#define SPA_PHP_ROOT_HANDER_H

namespace PA {

class CRootHandler : public Php::Base
{
protected:
	CRootHandler(SPA::ClientSide::CAsyncServiceHandler *pHandler, bool locked);

public:
	~CRootHandler();

protected:
	SPA::ClientSide::CAsyncServiceHandler *m_pHandler;
	bool m_locked;
};

} //namespace PA
#endif
