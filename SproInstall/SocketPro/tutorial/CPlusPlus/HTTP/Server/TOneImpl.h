#ifndef ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TONEIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */

#include <iostream>
using namespace std;

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;




class CHttpPeer : public CHttpPeerBase
{
private:
	void GetFile(const char *strFile)
	{
		CScopeUQueue	UQueue;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		m_UQueue.SetHeadPosition();
		do
		{
			hFile = ::CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ, UNULL_PTR, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, UNULL_PTR);
			if(hFile == INVALID_HANDLE_VALUE)
				break;

			DWORD dwSize = GetFileSize(hFile, UNULL_PTR);
			if(m_UQueue.GetMaxSize() -  m_UQueue.GetSize() < dwSize)
				m_UQueue.ReallocBuffer(dwSize + m_UQueue.GetSize() + 1024);

			DWORD dwRead;
			if(UQueue->GetMaxSize() < 10240)
				UQueue->ReallocBuffer(10240);
			unsigned char *pBuffer = (unsigned char*)UQueue->GetBuffer();
			do
			{
				dwRead = 0;
				if(!ReadFile(hFile, pBuffer, 10240, &dwRead, UNULL_PTR))
					break;
				m_UQueue.Push(pBuffer, dwRead);
			}while(dwRead == 10240);

		}while(false);
		if(hFile != INVALID_HANDLE_VALUE)
			::CloseHandle(hFile);
	}

protected:
	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		const char *str;
		CHttpPeerBase::OnFastRequestArrive(usRequestID, ulLen);
		switch(usRequestID)
		{
		case idHeader:
			str = GetHeaders();
			str = UNULL_PTR;
			break;
		default:
			break;
		}
	}
	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		const char *str = GetQuery();
		switch(usRequestID)
		{
		case idGet:
			{
				//set a customer header
				if(_stricmp(str, "/") == 0 || _stricmp(str, "/httppush.htm") == 0)
				{
					SetResponseHeader("Content-Type", "text/html");
					m_UQueue.SetSize(0);
					GetFile("httppush.htm");
					SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				}
				else if(_stricmp(str, "/ujsonxml.js") == 0)
				{
					SetResponseHeader("Content-Type", "application/javascript");
					
					//trun off connection right after sending response
					SetResponseHeader("Connection", "close");

					m_UQueue.SetSize(0);
					GetFile("ujsonxml.js");
					SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				}
				else if(strncmp(str, "/UCHAT", 6) == 0)
				{
					SetResponseHeader("Content-Type", "application/json");
					unsigned long nLen;
					const char *strValue = GetParamValue("UJS_DATA", nLen);
					
					CComVariant vtMsg(strValue);
					unsigned long pGroups[2] = {1, 2};
					bool ok = GetPush()->Broadcast(vtMsg, pGroups, 2);
					char *str = "{\"ret\":\"ok\"}";
					SendReturnData(usRequestID, (const BYTE*)str, strlen(str));
				}
				else
				{
					SetResponseCode(404); //404 Not Found
					SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				}
			}
			break;
		case idPost:
			break;
		default:
			break;
		}
		return S_OK;
	}
};

class CMySocketProServer : public CSocketProServer
{
protected:
	virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
	{
		cout<<"A socket connection is permitted for srvice id = "<<ulSvsID << endl;
		return true;
	}

	virtual void OnAccept(unsigned int hSocket, int nError)
	{
//		cout<<"A socket connection is initially established" << endl;
	}

	virtual void OnClose(unsigned int hSocket, int nError)
	{
		cout<<"A socket connection is closed with error code = " << nError << endl;
	}

	virtual bool OnSettingServer()
	{
		//try amIntegrated and amMixed
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();
		return true; //true -- ok; false -- no listening server
	}

private:
	CSocketProService<CHttpPeer>	m_HttpSvs;

private:
	void AddService()
	{
		bool ok;

		ok = m_HttpSvs.AddMe(sidHTTP, 0, taNone);
		
		//we process GET and POST from a worker thread in pool
		ok = m_HttpSvs.AddSlowRequest(idGet);
		ok = m_HttpSvs.AddSlowRequest(idPost);
		ok = m_HttpSvs.AddSlowRequest(idMultiPart);
	}
};


#endif