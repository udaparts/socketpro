
#include "streamingserver.h"
#include <algorithm>

#define STREAM_DB_LOG_FILE "streaming_db.log"

CStreamingServer *g_pStreamingServer = nullptr;

int async_sql_plugin_init(void *p) {
    CSetGlobals::Globals.Plugin = (const void *) p;
    if (!CSetGlobals::Globals.StartListening()) {
        return 1;
    }
    return 0;
}

int async_sql_plugin_deinit(void *p) {
    if (g_pStreamingServer) {
        g_pStreamingServer->PostQuit();
#ifdef WIN32_64
        ::WaitForSingleObject(CSetGlobals::Globals.m_hThread, INFINITE);
        ::CloseHandle(CSetGlobals::Globals.m_hThread);
#else

#endif
        delete g_pStreamingServer;
        g_pStreamingServer = nullptr;
    }
    if (CSetGlobals::Globals.m_hModule) {
        ::FreeLibrary(CSetGlobals::Globals.m_hModule);
        CSetGlobals::Globals.m_hModule = nullptr;
    }
    return 0;
}

CSetGlobals::CSetGlobals() : m_fLog(nullptr), m_nParam(0), DisableV6(false), Port(20902),
server_version(nullptr), utf8_general_ci(nullptr), decimal2string(nullptr),
m_hModule(nullptr), Plugin(nullptr) {
    //defaults
#ifdef WIN32_64
    m_hThread = nullptr;
    m_dwThreadId = 0;
    m_hModule = ::GetModuleHandle(nullptr);
#else
    m_hModule = ::dlopen(nullptr, RTLD_LAZY);
#endif
    if (m_hModule) {
        void *v = ::GetProcAddress(m_hModule, "my_charset_utf8_general_ci");
        utf8_general_ci = (CHARSET_INFO*) v;
        decimal2string = (pdecimal2string)::GetProcAddress(m_hModule, "decimal2string");
        server_version = (const char*) ::GetProcAddress(m_hModule, "server_version");
    } else {
        assert(false);
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

void CSetGlobals::LogMsg(const char *file, int fileLineNumber, const char *format ...) {
    SPA::CScopeUQueue sb;
    SPA::CUQueue &q = *sb;
    va_list ap;
    va_start(ap, format);
#ifdef WIN32_64
    int res = ::_vsnprintf_s((char*) q.GetBuffer(), q.GetMaxSize(), _TRUNCATE, format, ap);
#else
    int res = ::vsnprintf((char*) q.GetBuffer(), q.GetMaxSize(), format, ap);
#endif
    va_end(ap);
    LogEntry(file, fileLineNumber, (const char*) q.GetBuffer());
}

void CSetGlobals::LogEntry(const char* file, int fileLineNumber, const char* szBuf) {
    SPA::CAutoLock al(m_cs);
    if (!m_fLog) {
#ifdef WIN32_64
        errno_t errCode = ::fopen_s(&m_fLog, STREAM_DB_LOG_FILE, "a+");
#else
        m_fLog = ::fopen("streaming_db.log", "a+");
#endif
    }
    if (!m_fLog) {
        return;
    }
    SYSTEMTIME st;
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    SPA::UDB::CDBVariant vtDT(st);
    SPA::UDateTime dt(vtDT.ullVal);
    char str[32] = {0};
    dt.ToDBString(str, sizeof (str));
    int res = fprintf(m_fLog, "%s - %s:%d - ", str, file, fileLineNumber);
    if (szBuf)
        res = fprintf(m_fLog, "%s\n", szBuf);
    else
        res = fprintf(m_fLog, "\n");
    ::fflush(m_fLog);
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

bool CSetGlobals::StartListening() {
#ifdef WIN32_64
    m_hThread = ::CreateThread(nullptr, 0, ThreadProc, this, 0, &m_dwThreadId);
    return (m_hThread != nullptr);
#else

#endif
}

#ifdef WIN32_64

DWORD WINAPI CSetGlobals::ThreadProc(LPVOID lpParameter) {
    my_bool available = srv_session_server_is_available();
    while (!available) {
        ::Sleep(40);
        available = srv_session_server_is_available();
    }
    std::unordered_map<std::string, std::string> mapConfig = SPA::ServerSide::CMysqlImpl::ConfigStreamingDB();
    if (!mapConfig.size()) {
        return 1;
    }
    CSetGlobals::SetConfig(mapConfig);
    if (!g_pStreamingServer) {
        g_pStreamingServer = new CStreamingServer(CSetGlobals::Globals.m_nParam);
    }
    if (CSetGlobals::Globals.ssl_key.size() && (CSetGlobals::Globals.ssl_cert.size() || CSetGlobals::Globals.ssl_pwd.size())) {
        std::string key = CSetGlobals::Globals.ssl_key;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        auto pos = key.find_last_of(".pfx");
        if (pos == key.size() - 4) {
            g_pStreamingServer->UseSSL(CSetGlobals::Globals.ssl_key.c_str(), "", CSetGlobals::Globals.ssl_pwd.c_str());
        } else {
            g_pStreamingServer->UseSSL(CSetGlobals::Globals.ssl_key.c_str(), CSetGlobals::Globals.ssl_cert.c_str(), "");
        }
        CSetGlobals::Globals.ssl_pwd.clear();
    }
    SPA::ServerSide::ServerCoreLoader.SetThreadEvent(SPA::ServerSide::CMysqlImpl::OnThreadEvent);
    if (!g_pStreamingServer->Run(CSetGlobals::Globals.Port, 32, !CSetGlobals::Globals.DisableV6)) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Starting listening socket failed(errCode=%d; errMsg=%s)", g_pStreamingServer->GetErrorCode(), g_pStreamingServer->GetErrorMessage().c_str());
        return 1;
    }
    return 0;
}
#else

#endif

void CSetGlobals::SetConfig(const std::unordered_map<std::string, std::string>& mapConfig) {
    auto it = mapConfig.find(STREAMING_DB_PORT);
    if (it != mapConfig.end()) {
        std::string s = it->second;
        int port = std::atoi(s.c_str());
        if (port > 0) {
            CSetGlobals::Globals.Port = (unsigned int) port;
        }
    }
    it = mapConfig.find(STREAMING_DB_MAIN_THREADS);
    if (it != mapConfig.end()) {
        std::string s = it->second;
        int n = std::atoi(s.c_str());
        if (n > 0) {
            CSetGlobals::Globals.m_nParam = (unsigned int) n;
        }
    }
    it = mapConfig.find(STREAMING_DB_NO_IPV6);
    if (it != mapConfig.end()) {
        std::string s = it->second;
        int n = std::atoi(s.c_str());
        CSetGlobals::Globals.DisableV6 = n ? true : false;
    }
    it = mapConfig.find(STREAMING_DB_SSL_KEY);
    if (it != mapConfig.end()) {
        CSetGlobals::Globals.ssl_key = it->second;
        SPA::ServerSide::CMysqlImpl::Trim(CSetGlobals::Globals.ssl_key);
    }
    it = mapConfig.find(STREAMING_DB_SSL_CERT);
    if (it != mapConfig.end()) {
        CSetGlobals::Globals.ssl_cert = it->second;
        SPA::ServerSide::CMysqlImpl::Trim(CSetGlobals::Globals.ssl_cert);
    }
    it = mapConfig.find(STREAMING_DB_SSL_PASSWORD);
    if (it != mapConfig.end()) {
        CSetGlobals::Globals.ssl_pwd = it->second;
        SPA::ServerSide::CMysqlImpl::Trim(CSetGlobals::Globals.ssl_pwd);
    }
    it = mapConfig.find(STREAMING_DB_CACHE_TABLES);
    if (it != mapConfig.end()) {
        std::string tok;
        std::string s = it->second;
        SPA::ServerSide::CMysqlImpl::Trim(s);
        std::stringstream ss(s);
        while (std::getline(ss, tok, ';')) {
            SPA::ServerSide::CMysqlImpl::Trim(tok);
            if (tok.size())
                CSetGlobals::Globals.cached_tables.push_back(tok);
        }
    }
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
    std::wstring user(userId);
    char strIp[64] = {0};
    unsigned int port;
    bool ok = SPA::ServerSide::ServerCoreLoader.GetPeerName(h, &port, strIp, sizeof (strIp));
    std::string ip(strIp);
    return SPA::ServerSide::CMysqlImpl::Authenticate(user, password, ip);
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
    ok = m_MySql.AddSlowRequest(SPA::UDB::idClose);
    if (!ok)
        return false;
    return true;
}