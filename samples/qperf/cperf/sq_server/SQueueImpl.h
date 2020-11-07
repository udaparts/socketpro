
#ifndef ___SOCKETPRO_SERVICES_IMPL_SQUEUEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_SQUEUEIMPL_H__

class CMySocketProServer : public CSocketProServer {
protected:

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        Config::SetAuthenticationMethod(tagAuthenticationMethod::amOwn);
        return true;
    }
};
#endif
