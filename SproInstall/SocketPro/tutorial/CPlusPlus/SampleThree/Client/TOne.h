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
	CTOne(CClientSocket *pClientSocket = UNULL_PTR, IAsyncResultsHandler *pDefaultAsyncResultsHandler = UNULL_PTR)
	: CAsyncServiceHandler(sidCTOne, pClientSocket, pDefaultAsyncResultsHandler)
	{
	}

public:
	int QueryCount()
	{
		int QueryCountRtn;
		bool bProcessRy = ProcessR1(idQueryCountCTOne, QueryCountRtn);
		return QueryCountRtn;
	}

	int QueryGlobalCount()
	{
		int QueryGlobalCountRtn;
		bool bProcessRy = ProcessR1(idQueryGlobalCountCTOne, QueryGlobalCountRtn);
		return QueryGlobalCountRtn;
	}

	int QueryGlobalFastCount()
	{
		int QueryGlobalFastCountRtn;
		bool bProcessRy = ProcessR1(idQueryGlobalFastCountCTOne, QueryGlobalFastCountRtn);
		return QueryGlobalFastCountRtn;
	}

	void Sleep(int nTime)
	{
		bool bProcessRy = ProcessR0(idSleepCTOne, nTime);
	}

	CComVariant Echo(const VARIANT &objInput)
	{
		CComVariant EchoRtn;

		CComVariant vtMsg(L"Echo called");
		unsigned long pGroups[] = {1, 3};
		this->GetAttachedClientSocket()->GetPush()->Broadcast(vtMsg, pGroups, 2);

		bool bProcessRy = ProcessR1(idEchoCTOne, objInput, EchoRtn);
		return EchoRtn;
	}

	void GetAllCounts(int &nCount)
	{
		nCount = QueryCount();
	}

#if _MSC_VER >= 1600
	void GetAllCounts(int &nCount, int &nGlobalCount, int &nGlobalFastCount)
	{
		BeginBatching();
		SendRequest([&nCount](CAsyncResult &ar){
			ar.UQueue >> nCount;
		}, idQueryCountCTOne);

		SendRequest([&nGlobalFastCount](CAsyncResult &ar){
			ar.UQueue >> nGlobalFastCount;
		}, idQueryGlobalFastCountCTOne);

		SendRequest([&nGlobalCount](CAsyncResult &ar){
			ar.UQueue >> nGlobalCount;
		}, idQueryGlobalCountCTOne);

		CommitBatch(true); //true -- ask server to send three results back in one batch
		WaitAll();
	}

	void GetAllCounts(int &nCount, int &nGlobalCount)
	{
		BeginBatching();
		SendRequest([&nCount](CAsyncResult &ar){
			ar.UQueue >> nCount;
		}, idQueryCountCTOne);

		SendRequest([&nGlobalCount](CAsyncResult &ar){
			ar.UQueue >> nGlobalCount;
		}, idQueryGlobalCountCTOne);
		CommitBatch(true); //true -- ask server to send two results back in one batch
		WaitAll();
	}
#else
	void GetAllCounts(int &nCount, int &nGlobalCount, int &nGlobalFastCount)
	{
		BeginBatching();
		SendRequest(idQueryCountCTOne);
		SendRequest(idQueryGlobalFastCountCTOne);
		SendRequest(idQueryGlobalCountCTOne);
		CommitBatch(true); //true -- ask server to send three results back in one batch
		WaitAll();
		nCount = m_nCount;
		nGlobalCount = m_nGlobal;
		nGlobalFastCount = m_nFast;
	}

	void GetAllCounts(int &nCount, int &nGlobalCount)
	{
		BeginBatching();
		SendRequest(idQueryCountCTOne);
		SendRequest(idQueryGlobalCountCTOne);
		CommitBatch(true); //true -- ask server to send two results back in one batch
		WaitAll();
		nCount = m_nCount;
		nGlobalCount = m_nGlobal;
	}

protected:
	int m_nCount;
	int m_nGlobal;
	int m_nFast;
	virtual void OnResultReturned(unsigned short usRequestId, CUQueue &UQueue)
	{
		switch(usRequestId)
		{
		case idQueryCountCTOne:
			UQueue >> m_nCount;
			break;
		case idQueryGlobalFastCountCTOne:
			UQueue >> m_nFast;
			break;
		case idQueryGlobalCountCTOne:
			UQueue >> m_nGlobal;
			break;
		default:
			break;
		}
	}
#endif
};
#endif
