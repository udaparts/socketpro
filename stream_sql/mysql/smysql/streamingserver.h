
#pragma once

#include "mysqlimpl.h"

class CStreamingServer : public SPA::ServerSide::CSocketProServer
{
public:
	CStreamingServer(int nParam = 0);
	~CStreamingServer();

public:
	void Clean();

protected:
	virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);

private:
	bool AddService();

private:
	SPA::ServerSide::CMysqlService *m_pMySql;

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
	st_mysql_daemon async_sql_plugin;
	static CSetGlobals Globals;
};