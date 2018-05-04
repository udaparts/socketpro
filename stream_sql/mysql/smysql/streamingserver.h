
#pragma once

#include "mysqlimpl.h"
#include "httppeer.h"

struct CService {
    unsigned int ServiceId;
    std::string Library;
    int Param;
    std::string Description;
};

class CStreamingServer : public SPA::ServerSide::CSocketProServer {
public:
    CStreamingServer(int nParam = 0);

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);
    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);
    virtual void OnIdle(SPA::INT64 milliseconds);

private:
    bool AddService();

private:
    SPA::ServerSide::CMysqlService m_MySql;
    SPA::ServerSide::CSocketProService<CHttpPeer> m_myHttp;

private:
    CStreamingServer(const CStreamingServer &ss);
    CStreamingServer& operator=(const CStreamingServer &ss);
};

extern CStreamingServer *g_pStreamingServer;

class CSetGlobals {
private:
    FILE *m_fLog;
    SPA::CUCriticalSection m_cs;

private:
    CSetGlobals();
    void LogEntry(const char* file, int fileLineNumber, const char* szBuf);
    static unsigned int GetVersion(const char *prog);
    static void SetConfig(const std::unordered_map<std::string, std::string>& mapConfig);
    static void* ThreadProc(void *lpParameter);

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
    std::unordered_map<std::string, std::string> DefaultConfig;
    std::vector<CService> table_service;
    bool enable_http_websocket;
    my_thread_handle m_thread;

    static CSetGlobals Globals;

public:
    bool StartListening();
    void LogMsg(const char *file, int fileLineNumber, const char *format ...);
    void UpdateLog();
};

int async_sql_plugin_init(void *p);
int async_sql_plugin_deinit(void *p);