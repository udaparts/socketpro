#ifndef	__MY_SAMPLE_SERVICE_H___
#define __MY_SAMPLE_SERVICE_H___

//#include <iostream>
//using namesapce std;

const unsigned long	sidMySampleService = 0x1F0000AA;
const unsigned short idSayHello = 0x2001;
const unsigned short idEchoMyString = 0x2002;
const unsigned short idGetCountOfClients = 0x2003;
const unsigned short idDoSomeWorkWithMSSQLServer = 0x2004;
const unsigned short idSleep = 0x2005;
const unsigned short idEchoObjects = 0x2006;

#include "sprowrap.h"

class CMyClientPeer : public SocketProAdapter::ServerSide::CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
//		cout<<"Switched from the service = " <<ulServiceID<<", my service ID = "<<GetSvsID()<<endl;
	}

	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	{
		if(bClosing)
		{
//			cout<<"Socket = " << GetSocket() << " closed with error code = " << ulInfo <<endl;
		}
		else
		{
//			cout<<"Switched to the service ID = " << ulInfo <<endl;
		}

		//release your resource here 
	}
	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		switch (usRequestID)
		{
			case idSayHello:
			{
				unsigned long ulGet;
				USES_CONVERSION;
				WCHAR	strUID[256] = {0};
				WCHAR	strPassword[256] = {0};
				ATLASSERT(ulLen == 0);
				ulGet = GetUID(strUID, sizeof(strUID)/sizeof(WCHAR));
				ulGet = GetPassword(strPassword, sizeof(strPassword)/sizeof(WCHAR));
//				cout<<"Hello from: "<<OLE2A(strUID)<<", Password = "<<OLE2A(strPassword)<<endl;
				SendResult(usRequestID);
			}
				break;
			case idEchoObjects:
			case idEchoMyString:
				SendResult(usRequestID, m_UQueue);
				break;
			case idGetCountOfClients:
				SendResult(usRequestID, SocketProAdapter::ServerSide::CSocketProServer::GetCountOfClients());
				break;
			default:
				ATLASSERT(FALSE);
				break;
		}
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		switch (usRequestID)
		{
			case idSleep:
			{
				ULONG ulSleep;
				m_UQueue.Pop(&ulSleep);
				::Sleep(ulSleep);
//				cout<<"Sleep = "<<ulSleep<<endl;
				SendResult(usRequestID);
			}
				break;
			case idDoSomeWorkWithMSSQLServer:
			{
				WCHAR cEnd = 0;
				//make sure that string is null-ended
				m_UQueue.Push(&cEnd, 1);
				int nErrorCode = 0;
				WCHAR *strConnectionString = (LPWSTR)m_UQueue.GetBuffer();
				m_UQueue.SetSize(0);
				::Sleep(500);
				if((::GetTickCount()%10))
					nErrorCode = 1;
				if(nErrorCode != 0)
					SendResult(usRequestID, nErrorCode, L"Error happens in opening SQL server.");
				else
					SendResult(usRequestID, nErrorCode);
			}
				break;
			default:
				ATLASSERT(FALSE);; //shouldn't come here
				break;
		}

		return 0;
	}
};

typedef SocketProAdapter::ServerSide::CSocketProService<CMyClientPeer> CMySampleService;


#endif