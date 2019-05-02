#ifndef ___SOCKETPRO_CLIENT_HANDLER_TONE_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TONE_H__

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "TOne_i.h"

//client handler for service CTOne
class CTOne : public CAsyncServiceHandler
{
public:
	CTOne(CClientSocket *pClientSocket = NULL, IAsyncResultsHandler *pDefaultAsyncResultsHandler = NULL)
		: CAsyncServiceHandler(sidCTOne, pClientSocket, pDefaultAsyncResultsHandler)
	{
	}

protected:
	int m_QueryCountRtn;
	void QueryCountAsyn()
	{
		SendRequest(idQueryCountCTOne);
	}

	int m_QueryGlobalCountRtn;
	void QueryGlobalCountAsyn()
	{
		SendRequest(idQueryGlobalCountCTOne);
	}

	int m_QueryGlobalFastCountRtn;
	void QueryGlobalFastCountAsyn()
	{
		SendRequest(idQueryGlobalFastCountCTOne);
	}

	void SleepAsyn(int nTime)
	{
		SendRequest(idSleepCTOne, nTime);
	}

	CComVariant m_EchoRtn;
	void EchoAsyn(const VARIANT &objInput)
	{
		SendRequest(idEchoCTOne, objInput);
	}

	//When a result comes from a remote SocketPro server, the below virtual function will be called.
	//We always process returning results inside the function.
	virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
	{
		switch(usRequestID)
		{
		case idQueryCountCTOne:
			UQueue >> m_QueryCountRtn;
			break;
		case idQueryGlobalCountCTOne:
			UQueue >> m_QueryGlobalCountRtn;
			break;
		case idQueryGlobalFastCountCTOne:
			UQueue >> m_QueryGlobalFastCountRtn;
			break;
		case idSleepCTOne:
			break;
		case idEchoCTOne:
			UQueue >> m_EchoRtn;
			break;
		default:
			break;
		}
	}

public:
	int QueryCount()
	{
		QueryCountAsyn();
		GetAttachedClientSocket()->WaitAll();
		return m_QueryCountRtn;
	}

	int QueryGlobalCount()
	{
		QueryGlobalCountAsyn();
		GetAttachedClientSocket()->WaitAll();
		return m_QueryGlobalCountRtn;
	}

	int QueryGlobalFastCount()
	{
		QueryGlobalFastCountAsyn();
		GetAttachedClientSocket()->WaitAll();
		return m_QueryGlobalFastCountRtn;
	}

	void Sleep(int nTime)
	{
		SleepAsyn(nTime);
		GetAttachedClientSocket()->WaitAll();
	}

	const VARIANT& Echo(const VARIANT &objInput)
	{
		EchoAsyn(objInput);
		GetAttachedClientSocket()->WaitAll();
		return m_EchoRtn;
	}

	void GetAllCounts(int &nCount, int &nGlobalCount, int &nGlobalFastCount)
	{
		GetAttachedClientSocket()->BeginBatching();
		QueryCountAsyn();
		QueryGlobalCountAsyn();
		QueryGlobalFastCountAsyn();
		GetAttachedClientSocket()->Commit(true); //true -- ask server to send three results back in one batch
		GetAttachedClientSocket()->WaitAll();
		nCount = m_QueryCountRtn;
		nGlobalCount = m_QueryGlobalCountRtn;
		nGlobalFastCount = m_QueryGlobalFastCountRtn;
	}

	void GetAllCounts(int &nCount, int &nGlobalCount)
	{
		GetAttachedClientSocket()->BeginBatching();
		QueryCountAsyn();
		QueryGlobalCountAsyn();
		GetAttachedClientSocket()->Commit(true); //true -- ask server to send three results back in one batch
		GetAttachedClientSocket()->WaitAll();
		nCount = m_QueryCountRtn;
		nGlobalCount = m_QueryGlobalCountRtn;
	}

	void GetAllCounts(int &nCount)
	{
		nCount = QueryCount();
	}
};
#endif
