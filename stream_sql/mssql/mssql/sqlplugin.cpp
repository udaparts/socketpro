#include "stdafx.h"
#include "sqlplugin.h"
#include "config.h"

CSqlPlugin::CSqlPlugin(int param) : CSocketProServer(param) {
	COdbcImpl::SetODBCEnv(param);
}

CSqlPlugin::~CSqlPlugin() {
	COdbcImpl::FreeODBCEnv();
}

bool CSqlPlugin::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
	//amIntegrated and amMixed not supported yet
	CSocketProServer::Config::SetAuthenticationMethod(amOwn);
	if (!m_odbc.AddMe(SPA::Odbc::sidOdbc, SPA::taNone))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idExecute))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idExecuteBatch))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idExecuteParameters))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idPrepare))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idEndTrans))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idBeginTrans))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idOpen))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::UDB::idClose))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLColumnPrivileges))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLColumns))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLForeignKeys))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLPrimaryKeys))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLProcedureColumns))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLProcedures))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLSpecialColumns))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLStatistics))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLTablePrivileges))
		return false;
	if (!m_odbc.AddSlowRequest(SPA::Odbc::idSQLTables))
		return false;
	if (!SQLConfig::HttpWebSocket)
		return true;
	m_myHttp.AddMe(SPA::sidHTTP);
	m_myHttp.AddSlowRequest(SPA::ServerSide::idGet);
	m_myHttp.AddSlowRequest(SPA::ServerSide::idPost);
	m_myHttp.AddSlowRequest(SPA::ServerSide::idUserRequest);
	return true;
}

bool CSqlPlugin::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int nSvsID) {
	switch (nSvsID)
	{
	case SPA::sidHTTP:
		return true; //do authentication inside the method CMyHttpPeer.DoAuthentication
	default:
		return COdbcImpl::DoDBAuthentication(h, userId, password, nSvsID);
	}
	return false;
}

bool CSqlPlugin::Run(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
	array<String^>^ vService = SQLConfig::Services->Split(';');
	int count = vService->Length;
	for (int n = 0; n < count; ++n) {
		String ^lib = vService[n];
		if (lib) {
			pin_ptr<const wchar_t> ws = PtrToStringChars(lib);
			std::string s = SPA::Utilities::ToUTF8(ws);
			DllManager::AddALibrary(s.c_str());
		}
	}
	PushManager::AddAChatGroup(STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for MS SQL SERVER Table events, DELETE, INSERT and UPDATE");
	return CSocketProServer::Run(listeningPort, maxBacklog, v6);
}

bool CHttpPeer::DoAuthentication(const wchar_t *userId, const wchar_t *password) {
	if (GetTransport() != SPA::ServerSide::tWebSocket)
		return true;
	return COdbcImpl::DoDBAuthentication(GetSocketHandle(), userId, password, SPA::sidHTTP);
}

void CHttpPeer::OnFastRequestArrive(unsigned short requestId, unsigned int len) {
	switch (requestId) {
	case SPA::ServerSide::idDelete:
	case SPA::ServerSide::idPut:
	case SPA::ServerSide::idTrace:
	case SPA::ServerSide::idOptions:
	case SPA::ServerSide::idHead:
	case SPA::ServerSide::idMultiPart:
	case SPA::ServerSide::idConnect:
		SetResponseCode(501);
		SendResult("Server doesn't support DELETE, PUT, TRACE, OPTIONS, HEAD, CONNECT and POST with multipart");
		break;
	default:
		SetResponseCode(405);
		SendResult("Server only supports GET and POST without multipart");
		break;
	}
}

int CHttpPeer::OnSlowRequestArrive(unsigned short requestId, unsigned int len) {
	switch (requestId) {
	case SPA::ServerSide::idGet:
	{
		const char *path = GetPath();
		if (::strstr(path, "."))
			DownloadFile(path + 1);
		else
			SendResult("Unsupported GET request");
	}
	break;
	case SPA::ServerSide::idPost:
		SendResult("Unsupported POST request");
		break;
	case SPA::ServerSide::idUserRequest:
	{
		const std::string &RequestName = GetUserRequestName();
		if (RequestName == "subscribeTableEvents") {
			GetPush().Subscribe(&SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
			SendResult("ok");
		}
		else if (RequestName == "unsubscribeTableEvents") {
			GetPush().Unsubscribe();
			SendResult("ok");
		}
		else
			SendResult("Unsupported user request");
	}
	break;
	default:
		break;
	}
	return 0;
}