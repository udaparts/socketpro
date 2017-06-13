

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

    HMODULE hModule = ::GetModuleHandle(nullptr);
    if (hModule) {
        void *v = ::GetProcAddress(hModule, "my_charset_utf8_general_ci");
        utf8_general_ci = (CHARSET_INFO*) v;
        decimal2string = (pdecimal2string)::GetProcAddress(hModule, "decimal2string");
        ::FreeLibrary(hModule);
    } else {
        assert(false);
        utf8_general_ci = nullptr;
        decimal2string = nullptr;
    }
    //set interface_version
    unsigned int version = 50718; //MYSQL_VERSION_ID;
    async_sql_plugin.interface_version = (version << 8);
}

CSetGlobals CSetGlobals::Globals;

CStreamingServer::CStreamingServer(int nParam)
: SPA::ServerSide::CSocketProServer(nParam) {
}

CStreamingServer::~CStreamingServer() {

}

void CStreamingServer::OnClose(USocket_Server_Handle h, int errCode) {

}

bool CStreamingServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {




    return true;
}

void CStreamingServer::OnIdle(INT64 milliseconds) {

}

void CStreamingServer::OnSSLShakeCompleted(USocket_Server_Handle h, int errCode) {

}

void CStreamingServer::OnAccept(USocket_Server_Handle h, int errCode) {

}

bool CStreamingServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {

    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(SPA::ServerSide::amOwn);

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