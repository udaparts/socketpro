#pragma once

#include "asyncsession.h"

namespace UHTTP {

    class CAsyncServer {
    public:
        CAsyncServer(CIoService &io, unsigned short port, bool secure);
        ~CAsyncServer();

        void Run();
        bool IsSecure();

    private:
        CAsyncServer(const CAsyncServer &as);
        CAsyncServer& operator=(const CAsyncServer &as);
        std::string GetPassword() const;
        void OnAccepted(CAsyncSession *as, const CErrorCode &ec);

    private:
        std::vector<CAsyncSession*> m_vDeadAsyncSession;
        CIoService &m_io;
        CAcceptor m_acceptor;
        CSslContext m_sslContext;
        CErrorCode m_ec;

        friend class CAsyncSession;
    };

    extern CAsyncServer *g_pAsyncServer;

}

