
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
	virtual void OnIdle(INT64 milliseconds);
	virtual void OnSSLShakeCompleted(USocket_Server_Handle h, int errCode);

private:
    bool AddService();

private:
    SPA::ServerSide::CMysqlService m_MySql;

private:
    CStreamingServer(const CStreamingServer &ss);
    CStreamingServer& operator=(const CStreamingServer &ss);
};

extern CStreamingServer *g_pStreamingServer;

struct CSetGlobals {
private:
    CSetGlobals();

public:
    int m_nParam;
    bool DisableV6;
    unsigned int Port;
    bool TLSv;
	CHARSET_INFO *utf8_general_ci;
    st_mysql_daemon async_sql_plugin;
    static CSetGlobals Globals;
};

int async_sql_plugin_init(void *p);
int async_sql_plugin_deinit(void *p);