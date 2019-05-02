
#include "stdafx.h"
#include "myspaserver.h"

namespace iCOS 
{
	CMySocketProServer::CMySocketProServer(const std::string &pathToGeoIpDbFile, const std::string &pathToGeoLocationFile)
		: GeoSourceData(pathToGeoIpDbFile, pathToGeoLocationFile)
	{
	}

	CMySocketProServer* CMySocketProServer::GetSpaServer() 
	{
		return (CMySocketProServer*)GetServer();
	}

	bool CMySocketProServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6)
	{
		//amIntegrated and amMixed not supported yet
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();

		bool ok = GeoSourceData.IsLoaded();
		if (!ok)
			std::cout << "Failed in loading geo source data" <<std::endl;
		return true; //true -- ok; false -- no listening server
	}

	void CMySocketProServer::AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_GeoIp.AddMe(sidGeoIp, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_GeoIp.AddSlowRequest(idLookupGeoIp);

		//Add all of other services into SocketPro server here!
	}

}