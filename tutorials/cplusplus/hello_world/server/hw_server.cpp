// hw_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HWImpl.h"

class CMySocketProServer : public CSocketProServer {
private:
    CSocketProService<HelloWorldPeer> m_HelloWorld;
    //One SocketPro server supports any number of services. You can list them here!

private:

    void AddService() {
        //No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
        bool ok = m_HelloWorld.AddMe(sidHelloWorld, taNone);
        //If ok is false, very possibly you have two services with the same service id!

        ok = m_HelloWorld.AddSlowRequest(idSleep);
        //Add all of other services into SocketPro server here!
    }

protected:

    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        //amIntegrated and amMixed not supported yet
        CSocketProServer::Config::SetAuthenticationMethod(amOwn);

        //add service(s) into SocketPro server
        AddService();
        return true; //true -- ok; false -- no listening server
    }

    virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
        std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;
        return true;
    }

    virtual void OnClose(USocket_Server_Handle h, int errCode) {
        CBaseService *bs = CBaseService::SeekService(h);
        if (bs != nullptr) {
            CSocketPeer *sp = bs->Seek(h);
            // ......
        }
    }
};

int main(int argc, char* argv[]) {
	CMySocketProServer MySocketProServer;
	if (!MySocketProServer.Run(20901)) {
		int errCode = MySocketProServer.GetErrorCode();
		std::cout << "Error happens with code = " << errCode << std::endl;
	}
	std::cout << "Press any key to stop the server ......" << std::endl;
	::getchar();
	return 0;
}
