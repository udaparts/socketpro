

#include "streamingserver.h"
#include "include/mysql/client_plugin.h"

CStreamingServer *g_pStreamingServer = nullptr;
CSetGlobals CSetGlobals::Globals;

CSetGlobals::CSetGlobals() {
	//::Sleep(30000);

	//defaults
	m_nParam = 0;
	DisableV6 = false;
	Port = 20902;
	TLSv = false;

	//set interface_version
	unsigned int version = MYSQL_VERSION_ID;
	async_sql_plugin.interface_version = (version << 8);
}

CStreamingServer::CStreamingServer(int nParam)
	: SPA::ServerSide::CSocketProServer(nParam) {
}

CStreamingServer::~CStreamingServer() {
}

bool CStreamingServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(SPA::ServerSide::amOwn);

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
    return true;
}