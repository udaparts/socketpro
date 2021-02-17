#include "streamingserver.h"
#include "../../../include/scloader.h"
#include "../../../include/3rdparty/rapidjson/include/rapidjson/filereadstream.h"
#include "../../../include/3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../../include/3rdparty/rapidjson/include/rapidjson/prettywriter.h"
#include "../../../include/pexports.h"
#include "../../../include/membuffer.h"


#define DEFAULT_LOCAL_CONNECTION_STRING     L"host=localhost;port=3306;timeout=30"
#define STREAM_DB_LOG_FILE                  "streaming_db.log"
#define STREAM_DB_CONFIG_FILE	            "sp_streaming_db_config.json"

#define STREAMING_DB_PORT		    "port"
#define STREAMING_DB_MAIN_THREADS	    "main_threads"
#define STREAMING_DB_NO_IPV6		    "disable_ipv6"
#define STREAMING_DB_CACHE_TABLES	    "monitored_tables"
#define STREAMING_DB_SERVICES		    "services"
#define STREAMING_DB_WORKING_DIR            "working_dir"
#define STREAMING_DB_SERVICES_CONFIG        "services_config"

#ifdef WIN32_64
#define STREAMING_DB_STORE		    "cert_root_store"
#define STREAMING_DB_SUBJECT_CN             "cert_subject_cn"
#else
#define STREAMING_DB_SSL_KEY                "ssl_key"
#define STREAMING_DB_SSL_CERT               "ssl_cert"
#define STREAMING_DB_SSL_PASSWORD           "ssl_key_password"
#endif

CStreamingServer *g_pStreamingServer = nullptr;

int async_sql_plugin_init(void *p) {
    CMysqlImpl::InitMySql();
    if (!CMysqlImpl::IsMysqlInitialized()) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Required MySQL/MariaDB client library not available");
        CSetGlobals::Globals.UpdateLog();
        return 1;
    }
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
    UpdateLog();
    SetConfig();
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
    if (CSetGlobals::Globals.Config.working_dir.size()) {
        ServerCoreLoader.SetServerWorkDirectory(CSetGlobals::Globals.Config.working_dir.c_str());
    }
    ServerCoreLoader.SetThreadEvent(CMysqlImpl::OnThreadEvent);
    bool ok = g_pStreamingServer->Run(CSetGlobals::Globals.Config.port, 32, !CSetGlobals::Globals.Config.disable_ipv6);
    if (!ok) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Starting listening socket failed(errCode=%d; errMsg=%s)", g_pStreamingServer->GetErrorCode(), g_pStreamingServer->GetErrorMessage().c_str());
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
        LogMsg(__FILE__, __LINE__, ("Can not open DB streaming configuration file " + std::string(STREAM_DB_CONFIG_FILE) + " for write").c_str());
        return;
    }
    Document doc;
    doc.SetObject();
    Document::AllocatorType& allocator = doc.GetAllocator();
    {
        Value cs;
        cs.SetUint(Config.port);
        doc.AddMember(STREAMING_DB_PORT, cs, allocator);
    }
    {
        Value cs;
        cs.SetInt(Config.main_threads);
        doc.AddMember(STREAMING_DB_MAIN_THREADS, cs, allocator);
    }
    {
        Value cs;
        cs.SetBool(Config.disable_ipv6);
        doc.AddMember(STREAMING_DB_NO_IPV6, cs, allocator);
    }
    {
        Value cs;
        cs.SetString(Config.working_dir.c_str(), (SizeType) Config.working_dir.size());
        doc.AddMember(STREAMING_DB_WORKING_DIR, cs, allocator);
    }
    {
        Value cs;
        cs.SetString(Config.services.c_str(), (SizeType) Config.services.size());
        doc.AddMember(STREAMING_DB_SERVICES, cs, allocator);
    }
    {
        Value cs;
        cs.SetString(Config.cached_tables.c_str(), (SizeType) Config.cached_tables.size());
        doc.AddMember(STREAMING_DB_CACHE_TABLES, cs, allocator);
    }
#ifdef WIN32_64
    {
        Value cs;
        cs.SetString(Config.store.c_str(), (SizeType) Config.store.size());
        doc.AddMember(STREAMING_DB_STORE, cs, allocator);
    }
    {
        Value cs;
        cs.SetString(Config.subject_cn.c_str(), (SizeType) Config.subject_cn.size());
        doc.AddMember(STREAMING_DB_SUBJECT_CN, cs, allocator);
    }
#else
    {
        Value cs;
        cs.SetString(Config.ssl_key.c_str(), (SizeType) Config.ssl_key.size());
        doc.AddMember(STREAMING_DB_SSL_KEY, cs, allocator);
    }
    {
        Value cs;
        cs.SetString(Config.ssl_cert.c_str(), (SizeType) Config.ssl_cert.size());
        doc.AddMember(STREAMING_DB_SSL_CERT, cs, allocator);
    }
    {
        Value cs;
        cs.SetString(Config.ssl_key_password.c_str(), (SizeType) Config.ssl_key_password.size());
        doc.AddMember(STREAMING_DB_SSL_PASSWORD, cs, allocator);
    }
#endif
    {
        SPA::CScopeUQueue sb;
        if (sb->GetMaxSize() < 16 * SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE) {
            sb->ReallocBuffer(16 * SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE);
        }
        sb->CleanTrack();
        Value cs(kObjectType);
        for (auto it = services.cbegin(), end = services.cend(); it != end; ++it) {
            do {
                if (!it->second) {
                    break;
                }
                PGetSPluginGlobalOptions GetSPluginGlobalOptions = (PGetSPluginGlobalOptions) ::GetProcAddress(it->second, "GetSPluginGlobalOptions");
                if (!GetSPluginGlobalOptions) {
                    break;
                }
                unsigned int len = GetSPluginGlobalOptions((char*) sb->GetBuffer(), sb->GetMaxSize());
                sb->SetSize(len);
                sb->SetNull();
                Document d;
                ParseResult ok = d.Parse((const char*) sb->GetBuffer(), sb->GetSize());
                if (!ok) {
                    LogMsg(__FILE__, __LINE__, ("Plugin " + it->first + " has a wrong JSON global options").c_str());
                    break;
                }
                Value vJson(kObjectType);
                for (auto m = d.MemberBegin(), mend = d.MemberEnd(); m != mend; ++m) {
                    std::string ks = m->name.GetString();
                    Value k(ks.c_str(), (SizeType) ks.size(), allocator);
                    vJson.AddMember(k, m->value, allocator);
                }
                Value key(it->first.c_str(), (SizeType) it->first.size(), allocator);
                cs.AddMember(key, vJson, allocator);
            } while (false);
        }
        doc.AddMember(STREAMING_DB_SERVICES_CONFIG, cs, allocator);
    }
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    doc.Accept(writer);
    fprintf(fp.get(), "%s", buffer.GetString());
}

void CSetGlobals::SetConfig() {
    std::shared_ptr<FILE> fp(fopen(STREAM_DB_CONFIG_FILE, "r"), [](FILE * f) {
        if (f) {
            ::fclose(f);
        }
    });
    if (!fp || ferror(fp.get())) {
        LogMsg(__FILE__, __LINE__, ("Can not open DB streaming configuration file " + std::string(STREAM_DB_CONFIG_FILE) + " for read").c_str());
        fp.reset();
        UpdateConfigFile();
        return;
    }
    fseek(fp.get(), 0, SEEK_END);
    long size = ftell(fp.get()) + sizeof (wchar_t);
    fseek(fp.get(), 0, SEEK_SET);
    SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int) size);
    sb->CleanTrack();
    FileReadStream is(fp.get(), (char*) sb->GetBuffer(), sb->GetMaxSize());
    std::string json = (const char*) sb->GetBuffer();
    SPA::Trim(json);
    if (json.size()) {
        Document& doc = Config.doc;
        ParseResult ok = doc.Parse(json.c_str(), json.size());
        if (!ok) {
            LogMsg(__FILE__, __LINE__, ("Bad JSON configuration file " + std::string(STREAM_DB_CONFIG_FILE) + " found").c_str());
        } else {
            if (doc.HasMember(STREAMING_DB_PORT) && doc[STREAMING_DB_PORT].IsUint()) {
                Config.port = doc[STREAMING_DB_PORT].GetUint();
                if (!Config.port) {
                    Config.port = DEFAULT_LISTENING_PORT;
                }
            }
            if (doc.HasMember(STREAMING_DB_MAIN_THREADS) && doc[STREAMING_DB_MAIN_THREADS].IsInt()) {
                Config.main_threads = doc[STREAMING_DB_MAIN_THREADS].GetInt();
                if (Config.main_threads <= 0) Config.main_threads = 1;
            }
            if (doc.HasMember(STREAMING_DB_NO_IPV6) && doc[STREAMING_DB_NO_IPV6].IsBool()) {
                Config.disable_ipv6 = doc[STREAMING_DB_NO_IPV6].GetBool();
            }
            if (doc.HasMember(STREAMING_DB_WORKING_DIR) && doc[STREAMING_DB_WORKING_DIR].IsString()) {
                Config.working_dir = doc[STREAMING_DB_WORKING_DIR].GetString();
                SPA::Trim(Config.working_dir);
            }
            if (doc.HasMember(STREAMING_DB_SERVICES) && doc[STREAMING_DB_SERVICES].IsString()) {
                Config.services = doc[STREAMING_DB_SERVICES].GetString();
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
            if (doc.HasMember(STREAMING_DB_CACHE_TABLES) && doc[STREAMING_DB_CACHE_TABLES].IsString()) {
                std::string tok;
                Config.cached_tables = doc[STREAMING_DB_CACHE_TABLES].GetString();
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
            if (doc.HasMember(STREAMING_DB_STORE) && doc[STREAMING_DB_STORE].IsString()) {
                Config.store = doc[STREAMING_DB_STORE].GetString();
                SPA::Trim(Config.store);
            }
            if (doc.HasMember(STREAMING_DB_SUBJECT_CN) && doc[STREAMING_DB_SUBJECT_CN].IsString()) {
                Config.subject_cn = doc[STREAMING_DB_SUBJECT_CN].GetString();
                SPA::Trim(Config.subject_cn);
            }
#else
            if (doc.HasMember(STREAMING_DB_SSL_KEY) && doc[STREAMING_DB_SSL_KEY].IsString()) {
                Config.ssl_key = doc[STREAMING_DB_SSL_KEY].GetString();
                SPA::Trim(Config.ssl_key);
            }
            if (doc.HasMember(STREAMING_DB_SSL_CERT) && doc[STREAMING_DB_SSL_CERT].IsString()) {
                Config.ssl_cert = doc[STREAMING_DB_SSL_CERT].GetString();
                SPA::Trim(Config.ssl_cert);
            }
            if (doc.HasMember(STREAMING_DB_SSL_PASSWORD) && doc[STREAMING_DB_SSL_PASSWORD].IsString()) {
                Config.ssl_key_password = doc[STREAMING_DB_SSL_PASSWORD].GetString();
                SPA::Trim(Config.ssl_key_password);
            }
#endif   
        }
    } else {
        fp.reset();
        UpdateConfigFile();
    }
}

CSetGlobals CSetGlobals::Globals;

CStreamingServer::CStreamingServer(int nParam) : CSocketProServer(nParam) {
}

bool CStreamingServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    if (!CMysqlImpl::DoSQLAuthentication(h, userId, password, serviceId, DEFAULT_LOCAL_CONNECTION_STRING)) {
        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, ("Authentication failed for user " + SPA::Utilities::ToUTF8(userId)).c_str());
        return false;
    }
    return true;
}

void CStreamingServer::ConfigServices() {
    bool changed = false;
    auto& doc = CSetGlobals::Globals.Config.doc;
    for (auto p = CSetGlobals::Globals.services.begin(), end = CSetGlobals::Globals.services.end(); p != end; ++p) {
        HINSTANCE hModule = CSocketProServer::DllManager::AddALibrary(p->first.c_str(), 0);
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
                if (!(doc.HasMember(STREAMING_DB_SERVICES_CONFIG) && doc[STREAMING_DB_SERVICES_CONFIG].IsObject())) {
                    changed = true;
                    break;
                }
                auto obj = doc[STREAMING_DB_SERVICES_CONFIG].GetObject();
                if (!obj.HasMember(p->first.c_str())) {
                    changed = true;
                    break;
                }
                auto setting = obj.FindMember(p->first.c_str());
                StringBuffer sb;
                Writer<StringBuffer> writer(sb);
                setting->value.Accept(writer);
                std::string s = sb.GetString();
                if (!SetSPluginGlobalOptions(s.c_str())) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able to set global options for plugin %s", p->first.c_str());
                }
            } while (false);
        }
    }
    if (!changed && CSetGlobals::Globals.services.size()) {
        auto obj = doc[STREAMING_DB_SERVICES_CONFIG].GetObject();
        changed = (CSetGlobals::Globals.services.size() != (size_t)obj.MemberCount());
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
