
#include "stdafx.h"
#include "RemFileImpl.h"

class CMySocketProServer : public CSocketProServer {
protected:

	virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
		//amIntegrated and amMixed not supported yet
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();

		//create four chat groups or topics
		PushManager::AddAChatGroup(1, L"R&D Department");
		PushManager::AddAChatGroup(2, L"Sales Department");
		PushManager::AddAChatGroup(3, L"Management Department");

		return true; //true -- ok; false -- no listening server
	}

private:
	CSocketProService<RemotingFilePeer> m_RemotingFile;
	//One SocketPro server supports any number of services. You can list them here!

private:

	void AddService() {
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_RemotingFile.AddMe(sidRemotingFile, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_RemotingFile.AddSlowRequest(SPA::CStreamSerializationHelper::idReadDataFromServerToClient);
		ok = m_RemotingFile.AddSlowRequest(SPA::CStreamSerializationHelper::idWriteDataFromClientToServer);
		ok = m_RemotingFile.AddSlowRequest(SPA::CStreamSerializationHelper::idStartUploading);
		ok = m_RemotingFile.AddSlowRequest(SPA::CStreamSerializationHelper::idStartDownloading);
		//Add all of other services into SocketPro server here!
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

