#ifndef SPA_PHP_ROOT_HANDER_H
#define SPA_PHP_ROOT_HANDER_H

#include "basehandler.h"

namespace PA {

    typedef SPA::ClientSide::CSocketPool<CAsyncHandler> CPhpPool;
    typedef SPA::CMasterPool<false, CAsyncHandler> CMasterPool;

    class CPhpHandler : public CPhpBaseHandler {
    public:
        CPhpHandler(unsigned int poolId, CAsyncHandler *pHandler, bool locked);
        CPhpHandler(const CPhpHandler &rh) = delete;

    public:
        CPhpHandler& operator=(const CPhpHandler &rh) = delete;
        static void RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace &cs);

    protected:
        void PopTopCallbacks(PACallback &cb);

    private:
        CAsyncHandler *m_pHandler;
    };

} //namespace PA
#endif
