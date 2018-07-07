
#pragma once
#include "../../../include/odbc/server_impl/odbcimpl.h"
#include "httppeer.h"

class CSqlPlugin : public CSocketProServer
{
public:
	CSqlPlugin(int param = 0);
	~CSqlPlugin();

protected:
	virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);
	virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);
	virtual bool Run(unsigned int listeningPort, unsigned int maxBacklog = 32, bool v6 = false);

private:
	CSocketProService<COdbcImpl> m_odbc;
	CSocketProService<CHttpPeer> m_myHttp;
};

