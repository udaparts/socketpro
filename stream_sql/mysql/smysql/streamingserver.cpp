

#include "streamingserver.h"
#include "include/mysql/client_plugin.h"

CStreamingServer *g_pStreamingServer = nullptr;

int async_sql_plugin_init(void *p) {
    if (!g_pStreamingServer) {
        g_pStreamingServer = new CStreamingServer(CSetGlobals::Globals.m_nParam);
    }

    if (CSetGlobals::Globals.TLSv) {

    }

    if (!g_pStreamingServer->Run(CSetGlobals::Globals.Port, 32, !CSetGlobals::Globals.DisableV6)) {
        return 1;
    }

    return 0;
}

int async_sql_plugin_deinit(void *p) {
    if (g_pStreamingServer) {
        g_pStreamingServer->StopSocketProServer();
        delete g_pStreamingServer;
        g_pStreamingServer = nullptr;
    }
    return 0;
}

CSetGlobals::CSetGlobals() {
    //defaults
    m_nParam = 0;
    DisableV6 = false;
    Port = 20902;
    TLSv = false;
#ifdef WIN32_64
    HMODULE hModule = ::GetModuleHandle(nullptr);
#else
    HINSTANCE hModule = ::dlopen(nullptr, RTLD_LAZY);
#endif
    if (hModule) {
        void *v = ::GetProcAddress(hModule, "my_charset_utf8_general_ci");
        utf8_general_ci = (CHARSET_INFO*) v;
        decimal2string = (pdecimal2string)::GetProcAddress(hModule, "decimal2string");
        server_version = (const char*) ::GetProcAddress(hModule, "server_version");
        ::FreeLibrary(hModule);
    } else {
        assert(false);
        utf8_general_ci = nullptr;
        decimal2string = nullptr;
        server_version = nullptr;
    }
    unsigned int version = MYSQL_VERSION_ID;
    if (strlen(server_version)) {
        version = GetVersion(server_version);
        if (!version)
            version = MYSQL_VERSION_ID;
    }
    //set interface_version
    async_sql_plugin.interface_version = (version << 8);
}

unsigned int CSetGlobals::GetVersion(const char *version) {
    const char *end = nullptr;
    unsigned int minor = 0;
    unsigned int build = 0;
    unsigned int major = SPA::atoui(version, end);
    if (end && *end)
        minor = SPA::atoui(++end, end);
    if (end && *end)
        build = SPA::atoui(++end, end);
    return (major * 10000 + minor * 100 + build);
}

CSetGlobals CSetGlobals::Globals;

CStreamingServer::CStreamingServer(int nParam)
: SPA::ServerSide::CSocketProServer(nParam) {
}

CStreamingServer::~CStreamingServer() {

}

void CStreamingServer::OnClose(USocket_Server_Handle h, int errCode) {

}

bool CStreamingServer::DoSQLAuthentication(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password) {
    std::string userA = SPA::Utilities::ToUTF8(userId);
    std::shared_ptr<Srv_session> pMysql(srv_session_open(nullptr, this), [this](MYSQL_SESSION mysql) {
        if (mysql) {
            srv_session_close(mysql);
        }
    });
    if (!pMysql)
        return false;
    MYSQL_SECURITY_CONTEXT sc = nullptr;
    my_svc_bool fail = thd_get_security_context(srv_session_info_get_thd(pMysql.get()), &sc);
    if (fail)
        return false;
    char strIp[64] = {0};
    unsigned int port;
    bool ok = SPA::ServerSide::ServerCoreLoader.GetPeerName(h, &port, strIp, sizeof (strIp));
    fail = security_context_lookup(sc, userA.c_str(), "localhost", strIp, nullptr);
    if (fail)
        return false;
    return true;
}

bool CStreamingServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    switch (serviceId) {
        case SPA::Mysql::sidMysql:
            return DoSQLAuthentication(h, userId, password);
        default:
            break;
    }
    return true;
}

void CStreamingServer::OnIdle(SPA::INT64 milliseconds) {

}

void CStreamingServer::OnSSLShakeCompleted(USocket_Server_Handle h, int errCode) {

}

void CStreamingServer::OnAccept(USocket_Server_Handle h, int errCode) {

}

bool CStreamingServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(SPA::ServerSide::amOwn);

    //register streaming sql database events
    PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Streaming SQL Database Events");



    //add MySQL streaming service into SocketPro server
    return AddService();
}

bool CStreamingServer::AddService() {
    bool ok = m_MySql.AddMe(SPA::Mysql::sidMysql, SPA::taNone);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idOpen);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idBeginTrans);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idEndTrans);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idExecute);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idPrepare);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idExecuteParameters);
    if (!ok)
        return false;
    return true;
}