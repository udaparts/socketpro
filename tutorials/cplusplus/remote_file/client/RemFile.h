#include "../../../../include/aclientw.h"

#ifndef ___SOCKETPRO_CLIENT_HANDLER_REMFILE_H__
#define ___SOCKETPRO_CLIENT_HANDLER_REMFILE_H__

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../RemFile_i.h"

//client handler for service RemotingFile
class RemotingFile : public CAsyncServiceHandler {
public:
	RemotingFile(CClientSocket *pClientSocket)
		: CAsyncServiceHandler(sidRemotingFile, pClientSocket) {
			m_sh.reset(new CStreamHelper(*this));
	}

public:
	std::shared_ptr<CStreamHelper> GetStreamHelper() {
		return m_sh;
	}

private:
	std::shared_ptr<CStreamHelper> m_sh;
};

#endif
