
#pragma once

#include "mysqlimpl.h"

class CStreamingServer : public SPA::ServerSide::CSocketProServer {
public:
    CStreamingServer(int nParam = 0);
    ~CStreamingServer();

protected:
    virtual void OnAccept(USocket_Server_Handle h, int errCode);
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);
    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);
    virtual void OnClose(USocket_Server_Handle h, int errCode);
    virtual void OnIdle(SPA::INT64 milliseconds);
    virtual void OnSSLShakeCompleted(USocket_Server_Handle h, int errCode);

private:
    bool AddService();
    bool DoSQLAuthentication(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password);

private:
    SPA::ServerSide::CMysqlService m_MySql;

private:
    CStreamingServer(const CStreamingServer &ss);
    CStreamingServer& operator=(const CStreamingServer &ss);
};

extern CStreamingServer *g_pStreamingServer;

typedef int (*pdecimal2string) (const decimal_t *from, char *to, int *to_len, int fixed_precision, int fixed_decimals, char filler);

struct CSetGlobals {
private:
    CSetGlobals();
    static unsigned int GetVersion(const char *prog);

public:
    int m_nParam;
    bool DisableV6;
    unsigned int Port;
    bool TLSv;
    const char *server_version;
    CHARSET_INFO *utf8_general_ci;
    pdecimal2string decimal2string;
    st_mysql_daemon async_sql_plugin;
    static CSetGlobals Globals;
};

int async_sql_plugin_init(void *p);
int async_sql_plugin_deinit(void *p);