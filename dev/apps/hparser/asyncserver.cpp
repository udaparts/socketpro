#include "stdafx.h"
#include "asyncserver.h"

namespace UHTTP {

    CAsyncServer *g_pAsyncServer;

    CAsyncServer::CAsyncServer(CIoService &io, unsigned short port, bool secure)
    : m_io(io),
    m_acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    m_sslContext(boost::asio::ssl::context::tlsv1_server) {
        if (secure) {
            m_sslContext.set_options(
                    boost::asio::ssl::context::default_workarounds
                    | boost::asio::ssl::context::no_sslv2
                    | boost::asio::ssl::context::single_dh_use);
            m_sslContext.set_password_callback(boost::bind(&CAsyncServer::GetPassword, this));
            m_sslContext.use_certificate_chain_file("server.pem");
            m_sslContext.use_private_key_file("server.pem", boost::asio::ssl::context::pem);
            m_sslContext.use_tmp_dh_file("dh512.pem");
        }

        g_pAsyncServer = this;

    }

    CAsyncServer::~CAsyncServer(void) {
        g_pAsyncServer = NULL;
    }

    bool CAsyncServer::IsSecure() {
        return (m_sslContext.native_handle()->default_passwd_callback != NULL);
    }

    std::string CAsyncServer::GetPassword() const {
        return "test";
    }

    void CAsyncServer::OnAccepted(CAsyncSession *as, const CErrorCode &ec) {
        m_ec = ec;
        if (ec) {
            m_vDeadAsyncSession.push_back(as);
        } else {
            as->Start();
            CAsyncSession *as;
            size_t size = m_vDeadAsyncSession.size();
            if (size) {
                as = m_vDeadAsyncSession[size - 1];
                m_vDeadAsyncSession.pop_back();
            } else
                as = new CAsyncSession(m_io, m_sslContext);
            m_acceptor.async_accept(as->GetSocket(), boost::bind(&CAsyncServer::OnAccepted, this, as, nsPlaceHolders::error));
        }
    }

    void CAsyncServer::Run() {
        CAsyncSession* as = new CAsyncSession(m_io, m_sslContext);
        m_acceptor.async_accept(as->GetSocket(), boost::bind(&CAsyncServer::OnAccepted, this, as, nsPlaceHolders::error));
        m_io.run();
    }


}