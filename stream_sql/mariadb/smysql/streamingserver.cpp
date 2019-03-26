
#include "streamingserver.h"
#include "../../../include/scloader.h"
#include "config.h"

#define DEFAULT_LOCAL_CONNECTION_STRING     L"host=localhost;port=3306;timeout=30"
#define STREAM_DB_LOG_FILE                  "streaming_db.log"

#define STREAMING_DB_PORT		"port"
#define STREAMING_DB_MAIN_THREADS	"main_threads"
#define STREAMING_DB_NO_IPV6		"disable_ipv6"
#define STREAMING_DB_SSL_KEY		"ssl_key_or_store"
#define STREAMING_DB_SSL_CERT		"ssl_cert"
#define STREAMING_DB_SSL_PASSWORD	"ssl_key_password"
#define STREAMING_DB_CACHE_TABLES	"cached_tables"
#define STREAMING_DB_SERVICES		"services"
#define STREAMING_DB_HTTP_WEBSOCKET     "enable_http_websocket"

CStreamingServer *g_pStreamingServer = nullptr;

int async_sql_plugin_init(void *p) {
    CMysqlImpl::InitMySql();
    CSetGlobals::Globals.Plugin = (const void *) p;
    if (!CSetGlobals::Globals.StartListening()) {
        return 1;
    }
    return 0;
}

int async_sql_plugin_deinit(void *p) {
    if (g_pStreamingServer) {
        g_pStreamingServer->PostQuit();
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
server_version(nullptr),
m_hModule(nullptr), Plugin(nullptr), enable_http_websocket(false) {
    unsigned int version = MYSQL_VERSION_ID;
    async_sql_plugin.interface_version = (version << 8);
    //defaults
#ifdef WIN32_64
    m_hModule = ::GetModuleHandle(nullptr);
#else
    m_hModule = ::dlopen(nullptr, RTLD_LAZY);
#endif
    if (m_hModule) {
        server_version = (const char*) ::GetProcAddress(m_hModule, "server_version");
        if (!server_version) {
            LogMsg(__FILE__, __LINE__, "Variable server_version not found inside mysqld application");
        } else {
            LogMsg(__FILE__, __LINE__, "Variable server_version = (%s)", server_version);
        }
    } else {
        LogMsg(__FILE__, __LINE__, "m_hModule is nullptr");
    }
    UpdateLog();

    DefaultConfig[STREAMING_DB_PORT] = "20902";
    DefaultConfig[STREAMING_DB_MAIN_THREADS] = "1";
    DefaultConfig[STREAMING_DB_NO_IPV6] = "0";
    DefaultConfig[STREAMING_DB_SSL_KEY] = "";
    DefaultConfig[STREAMING_DB_SSL_CERT] = "";
    DefaultConfig[STREAMING_DB_SSL_PASSWORD] = "";
    DefaultConfig[STREAMING_DB_CACHE_TABLES] = "";
    DefaultConfig[STREAMING_DB_SERVICES] = "";
    DefaultConfig[STREAMING_DB_HTTP_WEBSOCKET] = "0";

    CConfig::Update(DefaultConfig);
    SetConfig(DefaultConfig);

    if (server_version && strlen(server_version)) {
        version = GetVersion(server_version);
        if (!version) {
            LogMsg(__FILE__, __LINE__, "Version not found inside mysqld application");
        } else {
            LogMsg(__FILE__, __LINE__, "Version %d found inside mysqld application", version);
            //set interface_version
            async_sql_plugin.interface_version = (version << 8);
        }
    }
    UpdateLog();
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

void CSetGlobals::UpdateLog() {
    SPA::CAutoLock al(m_cs);
    if (m_fLog) {
        ::fclose(m_fLog);
        m_fLog = nullptr;
    }
}

void CSetGlobals::LogEntry(const char* file, int fileLineNumber, const char* szBuf) {
    SPA::CAutoLock al(m_cs);
    if (!m_fLog) {
#ifdef WIN32_64
        errno_t errCode = ::fopen_s(&m_fLog, STREAM_DB_LOG_FILE, "a+");
#else
        m_fLog = ::fopen(STREAM_DB_LOG_FILE, "a+");
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
    ServerCoreLoader.SetThreadEvent(CMysqlImpl::OnThreadEvent);
    bool ok = g_pStreamingServer->Run(CSetGlobals::Globals.Port, 32, !CSetGlobals::Globals.DisableV6);
    if (!ok) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Starting listening socket failed(errCode=%d; errMsg=%s)", g_pStreamingServer->GetErrorCode(), g_pStreamingServer->GetErrorMessage().c_str());
    }
    return ok;
}

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
    it = mapConfig.find(STREAMING_DB_HTTP_WEBSOCKET);
    if (it != mapConfig.end()) {
        std::string s = it->second;
        int n = std::atoi(s.c_str());
        CSetGlobals::Globals.enable_http_websocket = n ? true : false;
    }
    it = mapConfig.find(STREAMING_DB_SSL_KEY);
    if (it != mapConfig.end()) {
        CSetGlobals::Globals.ssl_key = it->second;
        SPA::Utilities::Trim(CSetGlobals::Globals.ssl_key);
    }
    it = mapConfig.find(STREAMING_DB_SSL_CERT);
    if (it != mapConfig.end()) {
        CSetGlobals::Globals.ssl_cert = it->second;
        SPA::Utilities::Trim(CSetGlobals::Globals.ssl_cert);
    }
    it = mapConfig.find(STREAMING_DB_SSL_PASSWORD);
    if (it != mapConfig.end()) {
        CSetGlobals::Globals.ssl_pwd = it->second;
        SPA::Utilities::Trim(CSetGlobals::Globals.ssl_pwd);
    }
    it = mapConfig.find(STREAMING_DB_CACHE_TABLES);
    if (it != mapConfig.end()) {
        std::string tok;
        std::string s = it->second;
        SPA::Utilities::Trim(s);
        std::stringstream ss(s);
        while (std::getline(ss, tok, ';')) {
            SPA::Utilities::Trim(tok);
            if (tok.size())
                CSetGlobals::Globals.cached_tables.push_back(tok);
        }
    }
    it = mapConfig.find(STREAMING_DB_SERVICES);
    if (it != mapConfig.end()) {
        std::string tok;
        std::string s = it->second;
        SPA::Utilities::Trim(s);
        std::stringstream ss(s);
        while (std::getline(ss, tok, ';')) {
            SPA::Utilities::Trim(tok);
            if (tok.size())
                CSetGlobals::Globals.services.push_back(tok);
        }
    }
}

CSetGlobals CSetGlobals::Globals;

CStreamingServer::CStreamingServer(int nParam) : CSocketProServer(nParam) {
}

bool CHttpPeer::DoAuthentication(const wchar_t *userId, const wchar_t *password) {
    if (GetTransport() != tWebSocket)
        return true;
    return CMysqlImpl::DoSQLAuthentication(GetSocketHandle(), userId, password, SPA::sidHTTP, DEFAULT_LOCAL_CONNECTION_STRING);
}

void CHttpPeer::OnFastRequestArrive(unsigned short requestId, unsigned int len) {
    switch (requestId) {
        case idDelete:
        case idPut:
        case idTrace:
        case idOptions:
        case idHead:
        case idMultiPart:
        case idConnect:
            SetResponseCode(501);
            SendResult("Server doesn't support DELETE, PUT, TRACE, OPTIONS, HEAD, CONNECT and POST with multipart");
            break;
        default:
            SetResponseCode(405);
            SendResult("Server only supports GET and POST without multipart");
            break;
    }
}

int CHttpPeer::OnSlowRequestArrive(unsigned short requestId, unsigned int len) {
    switch (requestId) {
        case idGet:
        {
            const char *path = GetPath();
            if (::strstr(path, "."))
                DownloadFile(path + 1);
            else
                SendResult("Unsupported GET request");
        }
            break;
        case idPost:
            SendResult("Unsupported POST request");
            break;
        case idUserRequest:
        {
            const std::string &RequestName = GetUserRequestName();
            if (RequestName == "subscribeTableEvents") {
                GetPush().Subscribe(&SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
                SendResult("ok");
            } else if (RequestName == "unsubscribeTableEvents") {
                GetPush().Unsubscribe();
                SendResult("ok");
            } else
                SendResult("Unsupported user request");
        }
            break;
        default:
            break;
    }
    return 0;
}

bool CStreamingServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    switch (serviceId) {
        case SPA::sidHTTP:
            break;
        default:
            return CMysqlImpl::DoSQLAuthentication(h, userId, password, serviceId, DEFAULT_LOCAL_CONNECTION_STRING);
            break;
    }
    return true;
}

void CStreamingServer::ConfigServices() {
    for (auto p = CSetGlobals::Globals.services.begin(), end = CSetGlobals::Globals.services.end(); p != end; ++p) {
        HINSTANCE hModule = CSocketProServer::DllManager::AddALibrary(p->c_str(), 0);
        if (!hModule) {
#ifdef WIN32_64
            CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able to load server plugin %s", p->c_str());
#else
            CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able to load server plugin %s (%s)", p->c_str(), dlerror());
#endif
        }
    }
}

void CStreamingServer::OnIdle(SPA::INT64 milliseconds) {
    CSetGlobals::Globals.UpdateLog();
}

bool CStreamingServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(amOwn);

    //register streaming sql database events
    PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Streaming SQL Database Events");

    ConfigServices();

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
    ok = m_MySql.AddSlowRequest(SPA::UDB::idExecuteBatch);
    if (!ok)
        return false;
    ok = m_MySql.AddSlowRequest(SPA::UDB::idClose);
    if (!ok)
        return false;
    if (!CSetGlobals::Globals.enable_http_websocket)
        return true;
    ok = m_myHttp.AddMe(SPA::sidHTTP);
    if (!ok)
        return false;
    ok = m_myHttp.AddSlowRequest(idGet);
    if (!ok)
        return false;
    ok = m_myHttp.AddSlowRequest(idPost);
    if (!ok)
        return false;
    ok = m_myHttp.AddSlowRequest(idUserRequest);
    if (!ok)
        return false;
    return true;
}
