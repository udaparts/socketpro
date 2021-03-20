#include "streamingserver.h"
#include "../../../include/scloader.h"
#include "../../../include/pexports.h"
#include "../../../include/membuffer.h"

#define MY_VERSION                          "1.5.0.5" //this DB plugin version

#define DEFAULT_LOCAL_CONNECTION_STRING     L"host=localhost;port=3306;timeout=30"

CStreamingServer *g_pStreamingServer = nullptr;

int async_sql_plugin_init(void *p) {
    CMysqlImpl::InitMySql();
    if (!CMysqlImpl::IsMysqlInitialized()) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Required MySQL/MariaDB client library not available");
        CSetGlobals::Globals.UpdateLog();
        return 1;
    }
    CSetGlobals::Globals.SetConfig();
    CSetGlobals::Globals.UpdateLog();
    CSetGlobals::Globals.Plugin = (const void *) p;
    if (!CSetGlobals::Globals.StartListening()) {
        CSetGlobals::Globals.UpdateLog();
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
    CSetGlobals::Globals.UpdateLog();
    return 0;
}

CSetGlobals::CSetGlobals() : m_fLog(nullptr), server_version(nullptr), m_hModule(nullptr), Plugin(nullptr) {

    unsigned int version = MYSQL_VERSION_ID;
    async_sql_plugin.interface_version = (version << 8);
    //defaults
#ifdef WIN32_64
    m_hModule = ::GetModuleHandle(nullptr);
#else
    m_hModule = ::dlopen(nullptr, RTLD_LAZY);
#endif
    HINSTANCE hModule = nullptr;
    if (m_hModule) {
        server_version = (const char*) ::GetProcAddress(m_hModule, "server_version");
        if (!server_version) {
#ifdef WIN32_64
            hModule = ::GetModuleHandleW(L"server.dll"); //mariadb
#endif
            if (hModule) {
                server_version = (const char*) ::GetProcAddress(hModule, "server_version");
            }
        }
        if (!server_version) {
            LogMsg(__FILE__, __LINE__, "Variable server_version not found inside mysqld application");
        } else {
            LogMsg(__FILE__, __LINE__, "Variable server_version = (%s)", server_version);
        }
    } else {
        LogMsg(__FILE__, __LINE__, "m_hModule is nullptr");
    }
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
    if (hModule) {
        ::FreeLibrary(hModule);
    }
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
        g_pStreamingServer = new CStreamingServer(CSetGlobals::Globals.Config.main_threads);
    }
#ifdef WIN32_64
    if (CSetGlobals::Globals.Config.store.size() && CSetGlobals::Globals.Config.subject_cn.size()) {
        g_pStreamingServer->UseSSL(CSetGlobals::Globals.Config.store.c_str(), CSetGlobals::Globals.Config.subject_cn.c_str(), "");
    }
#else
    if (CSetGlobals::Globals.Config.ssl_key.size() && CSetGlobals::Globals.Config.ssl_cert.size()) {
        g_pStreamingServer->UseSSL(CSetGlobals::Globals.Config.ssl_cert.c_str(), CSetGlobals::Globals.Config.ssl_key.c_str(), CSetGlobals::Globals.Config.ssl_key_password.c_str());
    }
#endif
#ifdef ENABLE_WORKING_DIRECTORY
    if (CSetGlobals::Globals.Config.working_dir.size()) {
        ServerCoreLoader.SetServerWorkDirectory(CSetGlobals::Globals.Config.working_dir.c_str());
    }
#endif
    ServerCoreLoader.SetThreadEvent(CMysqlImpl::OnThreadEvent);
    bool ok = g_pStreamingServer->Run(CSetGlobals::Globals.Config.port, 32, !CSetGlobals::Globals.Config.disable_ipv6);
    if (!ok) {
        std::string em = g_pStreamingServer->GetErrorMessage();
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Starting listening socket failed(errCode=%d; errMsg=%s)", g_pStreamingServer->GetErrorCode(), em.c_str());
    }
    return ok;
}

void CSetGlobals::UpdateConfigFile() {
    std::shared_ptr<FILE> fp(fopen(STREAM_DB_CONFIG_FILE, "w"), [](FILE * f) {
        if (f) {
            ::fclose(f);
        }
    });
    if (!fp || ferror(fp.get())) {
        std::string em = "Can not open DB streaming configuration file " + std::string(STREAM_DB_CONFIG_FILE) + " for write";
        LogMsg(__FILE__, __LINE__, em.c_str());
        return;
    }
    JObject<char> obj;
    SPA::ServerSide::tagMaualBatching mb = SPA::ServerSide::CMysqlImpl::m_mb;
    obj[MANUAL_BATCHING] = (int) mb;
    obj[PLUGIN_SERVICE_ID] = SPA::Mysql::sidMysql;
    obj[SP_SERVER_CORE_VERSION] = SPA::ServerSide::ServerCoreLoader.GetUServerSocketVersion();
    obj[STREAMING_DB_MYSQL_CLIENT_LIB] = CMysqlImpl::GetClientLibName();
    obj[STREAMING_DB_VERSION] = MY_VERSION;
    obj[STREAMING_DB_PORT] = Config.port;
    obj[STREAMING_DB_MAIN_THREADS] = Config.main_threads;
    obj[STREAMING_DB_NO_IPV6] = Config.disable_ipv6;
#ifdef ENABLE_WORKING_DIRECTORY
    obj[STREAMING_DB_WORKING_DIR] = Config.working_dir;
#endif
    obj[STREAMING_DB_SERVICES] = Config.services;
    obj[STREAMING_DB_CACHE_TABLES] = Config.cached_tables;
#ifdef WIN32_64
    obj[STREAMING_DB_STORE] = Config.store;
    obj[STREAMING_DB_SUBJECT_CN] = Config.subject_cn;
#else
    obj[STREAMING_DB_SSL_KEY] = Config.ssl_key;
    obj[STREAMING_DB_SSL_CERT] = Config.ssl_cert;
    obj[STREAMING_DB_SSL_PASSWORD] = Config.ssl_key_password;
#endif
    JValue<char> jobj(std::move(obj));
    {
        SPA::CScopeUQueue sb;
        if (sb->GetMaxSize() < 16 * SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
            sb->ReallocBuffer(16 * SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
        }
        sb->CleanTrack();
        for (auto it = services.cbegin(), end = services.cend(); it != end; ++it) {
            do {
                if (!it->second) {
                    break;
                }
                PGetSPluginGlobalOptions GetSPluginGlobalOptions = (PGetSPluginGlobalOptions) ::GetProcAddress(it->second, "GetSPluginGlobalOptions");
                if (GetSPluginGlobalOptions) {
                    unsigned int len = GetSPluginGlobalOptions((char*) sb->GetBuffer(), sb->GetMaxSize());
                    sb->SetSize(len);
                    sb->SetNull();
                    std::unique_ptr<JValue<char>> jv(Parse((const char*) sb->GetBuffer()));
                    if (!jv || jv->GetType() != enumType::Object) {
                        std::string em = "Plugin " + it->first + " has a wrong JSON global options";
                        LogMsg(__FILE__, __LINE__, em.c_str());
                    }
                    obj[it->first] = std::move(*jv);
                } else {
                    obj[it->first] = JObject<char>();
                }
                PGetSPluginVersion GetSPluginVersion = (PGetSPluginVersion)::GetProcAddress(it->second, "GetSPluginVersion");
                if (!GetSPluginVersion) {
                    break;
                }
                obj[it->first][STREAMING_DB_VERSION] = GetSPluginVersion();
            } while (false);
        }
    }
    jobj[STREAMING_DB_SERVICES_CONFIG] = std::move(obj);
    fprintf(fp.get(), "%s", jobj.Stringify().c_str());
}

void CSetGlobals::SetConfig() {
    int errCode = 0;
    Config.doc.reset(ParseFromFile(STREAM_DB_CONFIG_FILE, errCode));
    if (errCode) {
        std::string em = "Can not open DB streaming configuration file " + std::string(STREAM_DB_CONFIG_FILE) + " for read";
        LogMsg(__FILE__, __LINE__, em.c_str());
        UpdateConfigFile();
        return;
    } else if (!Config.doc || Config.doc->GetType() != enumType::Object) {
        std::string em = "Bad JSON configuration file " + std::string(STREAM_DB_CONFIG_FILE) + " found";
        LogMsg(__FILE__, __LINE__, em.c_str());
        UpdateConfigFile();
        return;
    }
    auto& doc = Config.doc;
    JValue<char>* v = doc->Child(STREAMING_DB_PORT);
    if (v && v->GetType() == enumType::Uint64) {
        Config.port = (unsigned short) v->AsUint64();
        if (!Config.port) {
            Config.port = DEFAULT_LISTENING_PORT;
        }
    }
    v = doc->Child(MANUAL_BATCHING);
    if (v && v->GetType() == enumType::Uint64) {
        int mb = (int) v->AsUint64();
        SPA::ServerSide::CMysqlImpl::m_mb = (SPA::ServerSide::tagMaualBatching) mb;
    }
    v = doc->Child(STREAMING_DB_MAIN_THREADS);
    if (v && v->GetType() == enumType::Uint64) {
        Config.main_threads = (int) v->AsUint64();
        if (Config.main_threads <= 0) Config.main_threads = 1;
    }
    v = doc->Child(STREAMING_DB_NO_IPV6);
    if (v && v->GetType() == enumType::Bool) {
        Config.disable_ipv6 = v->AsBool();
    }
#ifdef ENABLE_WORKING_DIRECTORY
    v = doc->Child(STREAMING_DB_WORKING_DIR);
    if (v && v->GetType() == enumType::String) {
        Config.working_dir = v->AsString();
        SPA::Trim(Config.working_dir);
    }
#endif
    v = doc->Child(STREAMING_DB_SERVICES);
    if (v && v->GetType() == enumType::String) {
        Config.services = v->AsString();
        SPA::Trim(Config.services);
        if (Config.services.size()) {
            std::string tok;
            std::stringstream ss(Config.services);
            while (std::getline(ss, tok, ';')) {
                SPA::Trim(tok);
                if (tok.size()) {
                    services[tok] = nullptr;
                }
            }
        }
    }
    v = doc->Child(STREAMING_DB_CACHE_TABLES);
    if (v && v->GetType() == enumType::String) {
        std::string tok;
        Config.cached_tables = v->AsString();
        SPA::Trim(Config.cached_tables);
        std::stringstream ss(Config.cached_tables);
        while (std::getline(ss, tok, ';')) {
            SPA::Trim(tok);
            if (tok.size()) {
                cached_tables.push_back(tok);
            }
        }
    }
#ifdef WIN32_64
    v = doc->Child(STREAMING_DB_STORE);
    if (v && v->GetType() == enumType::String) {
        Config.store = v->AsString();
        SPA::Trim(Config.store);
    }
    v = doc->Child(STREAMING_DB_SUBJECT_CN);
    if (v && v->GetType() == enumType::String) {
        Config.subject_cn = v->AsString();
        SPA::Trim(Config.subject_cn);
    }
#else
    v = doc->Child(STREAMING_DB_SSL_KEY);
    if (v && v->GetType() == enumType::String) {
        Config.ssl_key = v->AsString();
        SPA::Trim(Config.ssl_key);
    }
    v = doc->Child(STREAMING_DB_SSL_CERT);
    if (v && v->GetType() == enumType::String) {
        Config.ssl_cert = v->AsString();
        SPA::Trim(Config.ssl_cert);
    }
    v = doc->Child(STREAMING_DB_SSL_PASSWORD);
    if (v && v->GetType() == enumType::String) {
        Config.ssl_key_password = v->AsString();
        SPA::Trim(Config.ssl_key_password);
    }
#endif
}

CSetGlobals CSetGlobals::Globals;

CStreamingServer::CStreamingServer(int nParam) : CSocketProServer(nParam) {
}

bool CStreamingServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    if (!CMysqlImpl::DoSQLAuthentication(h, userId, password, serviceId, DEFAULT_LOCAL_CONNECTION_STRING)) {
        std::string em = "Authentication failed for user " + SPA::Utilities::ToUTF8(userId);
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, em.c_str());
        return false;
    }
    return true;
}

void CStreamingServer::ConfigServices() {
    bool changed = false;
    auto& doc = CSetGlobals::Globals.Config.doc;
    if (!doc) {
        return;
    }
    JValue<char>* jv = doc->Child(STREAMING_DB_VERSION);
    if (!jv || jv->GetType() != enumType::String || jv->AsString() != MY_VERSION) {
        changed = true;
    }
    jv = doc->Child(STREAMING_DB_MYSQL_CLIENT_LIB);
    if (!jv || jv->GetType() != enumType::String || jv->AsString() != CMysqlImpl::GetClientLibName()) {
        changed = true;
    }
    jv = doc->Child(SP_SERVER_CORE_VERSION);
    if (!jv || jv->GetType() != enumType::String || jv->AsString() != SPA::ServerSide::ServerCoreLoader.GetUServerSocketVersion()) {
        changed = true;
    }
    for (auto p = CSetGlobals::Globals.services.begin(), end = CSetGlobals::Globals.services.end(); p != end; ++p) {
        HINSTANCE hModule = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary(p->first.c_str(), 0);
        if (!hModule) {
#ifdef WIN32_64
            CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able to load server plugin %s", p->first.c_str());
#else
            CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able to load server plugin %s (%s)", p->first.c_str(), dlerror());
#endif
            changed = true;
        } else {
            p->second = hModule;
            do {
                PSetSPluginGlobalOptions SetSPluginGlobalOptions = (PSetSPluginGlobalOptions)::GetProcAddress(hModule, "SetSPluginGlobalOptions");
                if (!SetSPluginGlobalOptions) {
                    break;
                }
                JValue<char>* jv = doc->Child(STREAMING_DB_SERVICES_CONFIG);
                if (!(jv && jv->GetType() == enumType::Object)) {
                    changed = true;
                    break;
                }
                jv = jv->Child(p->first.c_str());
                if (!(jv && jv->GetType() == enumType::Object)) {
                    changed = true;
                    break;
                }
                std::string s = jv->Stringify();
                if (!SetSPluginGlobalOptions(s.c_str())) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able to set global options for plugin %s", p->first.c_str());
                }
                PGetSPluginVersion GetSPluginVersion = (PGetSPluginVersion)::GetProcAddress(hModule, "GetSPluginVersion");
                if (!GetSPluginVersion) {
                    break;
                }
                auto version = GetSPluginVersion();
                JValue<char>* v = jv->Child(STREAMING_DB_VERSION);
                if (!v || v->GetType() != enumType::String || !version || v->AsString() != version) {
                    changed = true;
                }
            } while (false);
        }
    }
    if (!changed && CSetGlobals::Globals.services.size()) {
        JValue<char>* jv = doc->Child(STREAMING_DB_SERVICES_CONFIG);
        changed = (CSetGlobals::Globals.services.size() != jv->Size());
    }
    if (changed) {
        while (changed) {
            //remove all unloaded plugin
            changed = false;
            for (auto p = CSetGlobals::Globals.services.begin(), end = CSetGlobals::Globals.services.end(); p != end; ++p) {
                if (!p->second) {
                    CSetGlobals::Globals.services.erase(p->first);
                    changed = true;
                    break;
                }
            }
        }
        std::string services;
        for (auto p = CSetGlobals::Globals.services.begin(), end = CSetGlobals::Globals.services.end(); p != end; ++p) {
            if (services.size()) services.push_back(';');
            services += p->first;
        }
        CSetGlobals::Globals.Config.services = services;
        CSetGlobals::Globals.UpdateConfigFile();
    }
}

void CStreamingServer::OnIdle(SPA::INT64 milliseconds) {
    CSetGlobals::Globals.UpdateLog();
}

bool CStreamingServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(tagAuthenticationMethod::amOwn);

    //register streaming sql database events
    PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Streaming SQL Database Events");

    ConfigServices();

    //add MySQL streaming service into SocketPro server
    return AddService();
}

bool CStreamingServer::AddService() {
    bool ok = m_MySql.AddMe(SPA::Mysql::sidMysql, SPA::tagThreadApartment::taNone);
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
    return true;
}
