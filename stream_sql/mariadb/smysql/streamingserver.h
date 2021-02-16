#pragma once

#include "../../../include/mysql/include/plugin.h"
#include "../../../include/mysql/server_impl/mysqlimpl.h"
#include "../../../include/3rdparty/rapidjson/include/rapidjson/document.h"
#include <unordered_map>

using namespace rapidjson;
using namespace SPA::ServerSide;

class U_MODULE_HIDDEN CStreamingServer : public CSocketProServer {
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
    CMysqlService m_MySql;

private:
    CStreamingServer(const CStreamingServer &ss);
    CStreamingServer& operator=(const CStreamingServer &ss);
};

extern CStreamingServer *g_pStreamingServer;

class U_MODULE_HIDDEN UConfig {
public:

    UConfig() : port(20902), main_threads(1), disable_ipv6(false) {
        doc.SetObject();
    }
    unsigned int port;
    int main_threads;
    bool disable_ipv6;

#ifdef WIN32_64
    std::string store;
    std::string subject_cn;
#else
    std::string ssl_key;
    std::string ssl_cert;
    std::string ssl_key_password;
#endif
    std::string cached_tables;
    std::string services;
    std::string working_dir;
    Document doc;
};

class U_MODULE_HIDDEN CSetGlobals {
private:
    FILE *m_fLog;
    SPA::CUCriticalSection m_cs;

private:
    CSetGlobals();
    void LogEntry(const char* file, int fileLineNumber, const char* szBuf);
    static unsigned int GetVersion(const char *prog);
    void SetConfig();

public:
    const char *server_version;
    st_mysql_daemon async_sql_plugin;
    HINSTANCE m_hModule;
    const void *Plugin;
    std::vector<std::string> cached_tables;
    std::unordered_map<std::string, HINSTANCE> services;
    UConfig Config;

    static CSetGlobals Globals;

public:
    bool StartListening();
    void LogMsg(const char *file, int fileLineNumber, const char *format ...);
    void UpdateLog();
    void UpdateConfigFile();
};

int async_sql_plugin_init(void *p);
int async_sql_plugin_deinit(void *p);
