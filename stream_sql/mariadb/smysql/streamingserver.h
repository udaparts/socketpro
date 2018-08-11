
#pragma once

#include "../../../include/mysql/include/plugin.h"
#include "../../../include/mysql/server_impl/mysqlimpl.h"
#include "httppeer.h"
using namespace SPA::ServerSide;

class CStreamingServer : public CSocketProServer {
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
    CSocketProService<CHttpPeer> m_myHttp;

private:
    CStreamingServer(const CStreamingServer &ss);
    CStreamingServer& operator=(const CStreamingServer &ss);
};

extern CStreamingServer *g_pStreamingServer;

class CSetGlobals {
private:
    FILE *m_fLog;
    SPA::CUCriticalSection m_cs;
    std::unordered_map<std::string, std::string> DefaultConfig;

private:
    CSetGlobals();
    void LogEntry(const char* file, int fileLineNumber, const char* szBuf);
    static unsigned int GetVersion(const char *prog);
    static void SetConfig(const std::unordered_map<std::string, std::string>& mapConfig);

public:
    int m_nParam;
    bool DisableV6;
    unsigned int Port;
    const char *server_version;
    st_mysql_daemon async_sql_plugin;
    HINSTANCE m_hModule;
    const void *Plugin;
    std::string ssl_key;
    std::string ssl_cert;
    std::string ssl_pwd;
    std::vector<std::string> cached_tables;
    std::vector<std::string> services;
    bool enable_http_websocket;

    static CSetGlobals Globals;
public:
    bool StartListening();
    void LogMsg(const char *file, int fileLineNumber, const char *format ...);
    void UpdateLog();
};

int async_sql_plugin_init(void *p);
int async_sql_plugin_deinit(void *p);
