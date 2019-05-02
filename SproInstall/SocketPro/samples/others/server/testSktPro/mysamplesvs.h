#ifndef	__MY_SAMPLE_SERVICE_H___
#define __MY_SAMPLE_SERVICE_H___

using namespace SocketProAdapter;

const unsigned long	sidMySampleService = 0x1F0000AA;
const unsigned short idSayHello = 0x2001;
const unsigned short idEchoMyString = 0x2002;
const unsigned short idGetCountOfClients = 0x2003;
const unsigned short idDoSomeWorkWithMSSQLServer = 0x2004;
const unsigned short idSleep = 0x2005;
const unsigned short idEchoObjects = 0x2006;

class CMyClientPeer : public CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		cout<<"Switched from the service = " <<ulServiceID<<", my service ID = "<<GetSvsID()<<endl;
	}
	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	{
		if(bClosing)
		{
			cout<<"Socket = " << GetSocket() << " closed with error code = " << ulInfo <<endl;
		}
		else
		{
			cout<<"Switched to the service ID = " << ulInfo <<endl;
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
				cout<<"Hello from: "<<OLE2A(strUID)<<", Password = "<<OLE2A(strPassword)<<endl;
				SendReturnData(usRequestID, NULL, 0);
			}
				break;
			case idEchoObjects:
			case idEchoMyString:
			{
				SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
			}
				break;
			case idGetCountOfClients:
			{
				ULONG ulCount = CSocketProServer::GetCountOfClients();
				SendReturnData(usRequestID, (BYTE*)&ulCount, sizeof(ulCount));
			}
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
 				cout<<"Sleep = "<<ulSleep<<endl;
				SendReturnData(usRequestID, NULL, 0);
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
				{
					m_UQueue.Push(&nErrorCode);
					m_UQueue.Push(L"Error happens in opening SQL server.");
					SendReturnData(usRequestID, m_UQueue.GetBuffer(), m_UQueue.GetSize());
				}
				else
				{
					SendReturnData(usRequestID, (BYTE*)&nErrorCode, sizeof(nErrorCode));
				}
			}
				break;
			default:
				ATLASSERT(FALSE);; //shouldn't come here
				break;
		}

		return 0;
	}
};

class CMySampleService : public CBaseService
{
public:
	CMySampleService()
	{
		//using pool is good to reuse objects, especially when you have to close socket connections repeatedly
		m_bUsePool = true;
	}

protected:
	virtual CClientPeer* GetPeerSocket(unsigned int hSocket)
	{
		//you shouldn't delete the created object, and the framework will do it for you
		return new CMyClientPeer;
	}

	virtual bool OnIsPermitted(unsigned int hSocket)
	{
		USES_CONVERSION;
		WCHAR strUID[256] = {0};
		WCHAR strPassword[256] = {0};
		
		GetUserID(hSocket, strUID, 256);
		GetPassword(hSocket, strPassword, 256);
		
		cout<<"User ID = "<<OLE2A(strUID)<<"; "<<"Password = "<<OLE2A(strPassword)<<endl;

		//!!!!! FOR THE SAKE OF HIGH SECURITY !!!!!

		//Starting from SocketPro server 4.3.0.1, OnIsPermited at service level is NOT used
		//to authenticate a client, but it is kept to notify that a client is given permission.

		//Also, you can NOT use the function GetPassword to retrieve password 
		//because password is already cleaned internally.

		//To authenticate a client, you need to use application level OnIsPermitted if you set
		//authentication method to amOwn or amMixing

		return true;
	}
};


#endif