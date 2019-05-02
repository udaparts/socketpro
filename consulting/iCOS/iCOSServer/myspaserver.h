
#include "IpLookupImpl.h"
#include "GeoSourceData.h"

#ifndef ___SOCKETPRO_ICOS_SERVER_H__
#define ___SOCKETPRO_ICOS_SERVER_H__

namespace iCOS 
{
	class CMySocketProServer : public CSocketProServer
	{
	public:
		CMySocketProServer(const std::string &pathToGeoIpDbFile, const std::string &pathToGeoLocationFile);

	public:
		CGeoSourceData	GeoSourceData;

	private:
		CSocketProService<CGeoIpPeer> m_GeoIp;
		//One SocketPro server supports any number of services. You can list them here!

	public:
		static CMySocketProServer* GetSpaServer();

	protected:
		virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);

	private:
		void AddService();
	};

}

#endif