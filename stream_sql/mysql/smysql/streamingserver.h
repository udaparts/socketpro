#pragma once

#include "mysqlimpl.h"
#include "../../../include/jsonvalue.h"
#include "../../../include/udb_macros.h"
#include <unordered_map>
using namespace SPA::JSON;

#define DEAFULT_AUTH_ACCOUNT                u"root"
#define DEFAULT_LISTENING_PORT              20902

class U_MODULE_HIDDEN UConfig {
public:

    UConfig() : port(DEFAULT_LISTENING_PORT), main_threads(1), disable_ipv6(false), auth_account(DEAFULT_AUTH_ACCOUNT)
#ifdef WIN32_64
    , store(DEFAULT_CA_ROOT)
#endif 
    {
    }
    unsigned int port;
    int main_threads;
    bool disable_ipv6;
    SPA::CDBString auth_account;
    std::string cached_tables;
    std::string services;
#ifdef WIN32_64
    std::string store;
    std::string subject_cn;
#else
    std::string ssl_key;
    std::string ssl_cert;
    std::string ssl_key_password;
#endif
#ifdef ENABLE_WORKING_DIRECTORY
    std::string working_dir;
#endif
    std::shared_ptr<JValue> doc;
};

class U_MODULE_HIDDEN CStreamingServer : public SPA::ServerSide::CSocketProServer {
public:
    CStreamingServer(int nParam = 0);

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);
    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);
    virtual void OnIdle(SPA::INT64 milliseconds);

private:
    bool AddService();
    void ConfigServices();

private:
    SPA::ServerSide::CMysqlService m_MySql;

private:
    CStreamingServer(const CStreamingServer &ss) = delete;
    CStreamingServer& operator=(const CStreamingServer &ss) = delete;
};

extern CStreamingServer *g_pStreamingServer;

class U_MODULE_HIDDEN CSetGlobals {
private:
    FILE *m_fLog;
    SPA::CUCriticalSection m_cs;

private:
    CSetGlobals();
    void LogEntry(const char* file, int fileLineNumber, const char* szBuf);
    static unsigned int GetVersion(const char *prog);
    static void* ThreadProc(void *lpParameter);
    void SetConfig();

public:
    const char *server_version;
    st_mysql_daemon async_sql_plugin;
    HINSTANCE m_hModule;
    const void *Plugin;
    std::vector<std::string> cached_tables;
    std::unordered_map<std::string, HINSTANCE> services;
    UConfig Config;
    my_thread_handle m_thread;
    static CSetGlobals Globals;

public:
    bool StartListening();
    void LogMsg(const char *file, int fileLineNumber, const char *format ...);
    void UpdateLog();
    void UpdateConfigFile();
};

int async_sql_plugin_init(void *p);
int async_sql_plugin_deinit(void *p);
