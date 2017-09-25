
#include "stdafx.h"
#include "ssserver.h"
#include "config.h"

std::shared_ptr<CMySQLPool> CSSServer::Master;
std::shared_ptr<CMySQLPool> CSSServer::Slave;

void CSSServer::SetMySQLPools() {

}

CSSServer::CSSServer(int nParam) : CSocketProServer(nParam)
{

}

CSSServer::~CSSServer()
{

}

bool CSSServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
	std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;
	return true; //true -- ok; false -- no permission
}

bool CSSServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
	//amIntegrated and amMixed not supported yet
	CSocketProServer::Config::SetAuthenticationMethod(amOwn);

	SetOnlineMessage();

	if (!AddServices()) {
		std::cout << "Unable to register a service" << std::endl;
		return false;
	}
	return true; //true -- ok; false -- no listening server
}

bool CSSServer::AddServices() {
	bool ok = m_SSPeer.AddMe(sidStreamSystem);
	if (!ok)
		return false;
	return true;
}

void CSSServer::SetOnlineMessage() {

}