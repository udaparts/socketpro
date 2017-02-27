
#include "../../../../include/aserverw.h"

#ifndef ___SOCKETPRO_SERVICES_IMPL_REMFILEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_REMFILEIMPL_H__


using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../RemFile_i.h"

//server implementation for service RemotingFile
class RemotingFilePeer : public CClientPeer {
private:
	void StartDownloadingFile(const std::wstring &RemoteFilePath, SPA::UINT64 &len, std::wstring &res) {
		SPA::ServerSide::CStreamHelper::DownloadFile(GetSocketHandle(), RemoteFilePath, len, res, m_br);
	}
	void MoveDataFromServerToClient() {
		SPA::ServerSide::CStreamHelper::ReadDataFromServerToClient(GetSocketHandle(), m_br);
	}
	void WaitDownloadCompleted() {
		m_br.close();
	}


	void StartUploadingFile(const std::wstring &RemoteFilePath, std::wstring &res) {
		CScopeUQueue su;
		Utilities::ToUTF8(RemoteFilePath.c_str(), RemoteFilePath.size(), *su);
		m_bw.open((const char*) su->GetBuffer(), std::ios_base::out | std::ios_base::binary | std::ios_base::app);
		if (!m_bw.is_open())
			res = L"Error in openning a file for writing";
	}
	void MoveDataFromClientToServer() {
		SPA::ServerSide::CStreamHelper::WriteDataFromClientToServer(m_UQueue, m_bw);
	}
	void WaitUploadingCompleted() {
		m_bw.close();
	}

protected:
	virtual void OnReleaseResource(bool closing, unsigned int info) {
		m_br.close();
		m_bw.close();
	}
	virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len) {
		BEGIN_SWITCH(reqId)
			M_I0_R0(SPA::CStreamSerializationHelper::idDownloadCompleted, WaitDownloadCompleted)
			M_I0_R0(SPA::CStreamSerializationHelper::idUploadCompleted, WaitUploadingCompleted)
			END_SWITCH
	}
	virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
		BEGIN_SWITCH(reqId)
			M_I1_R2(SPA::CStreamSerializationHelper::idStartDownloading, StartDownloadingFile, std::wstring, SPA::UINT64, std::wstring)
			M_I1_R1(SPA::CStreamSerializationHelper::idStartUploading, StartUploadingFile, std::wstring, std::wstring)
			M_I0_R0(SPA::CStreamSerializationHelper::idReadDataFromServerToClient, MoveDataFromServerToClient)
			M_I0_R0(SPA::CStreamSerializationHelper::idWriteDataFromClientToServer, MoveDataFromClientToServer)
			END_SWITCH
			return 0;
	}

private:
	std::ifstream m_br;
	std::ofstream m_bw;
};


#endif