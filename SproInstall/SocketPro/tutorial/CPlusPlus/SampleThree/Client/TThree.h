#ifndef ___SOCKETPRO_CLIENT_HANDLER_TTHREE_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TTHREE_H__

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "TThree_i.h"
#include "..\Shared.h"
#include <stack>
using namespace std;

//client handler for service CTThree
class CTThree : public CAsyncServiceHandler
{
public:
	CTThree(CClientSocket *pClientSocket = UNULL_PTR, IAsyncResultsHandler *pDefaultAsyncResultsHandler = UNULL_PTR)
		: CAsyncServiceHandler(sidCTThree, pClientSocket, pDefaultAsyncResultsHandler), m_pLargeStack(UNULL_PTR)
	{
	}

	~CTThree()
	{
		EmptyStack();	
	}

protected:
	CTestItem m_GetOneItemRtn;
	void GetOneItemAsyn()
	{
		SendRequest(idGetOneItemCTThree);
	}

	stack<CTestItem*> m_Stack;
	
	//We can process returning results inside the function.
	virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
	{
		switch(usRequestID)
		{
		case idGetOneItemCTThree:
			UQueue >> m_GetOneItemRtn; //Correct deserialization?
			break;
		case idSendBatchItemsCTThree:
			SendBatchItems();
			break;
		case idGetBatchItemsCTThree:
			AppendItems(UQueue);
			break;
		default:
			break;
		}
	}

private:
	void EmptyStack()
	{
		while(m_Stack.size() > 0)
		{
			CTestItem *pItem = m_Stack.top();
			m_Stack.pop();
			if(pItem)
				delete pItem;
		}
	}

	void AppendItems(CUQueue &UQueue)
	{
		while(UQueue.GetSize() > 0)
		{
			CTestItem *pItem = new CTestItem();
			UQueue >> *pItem;
			m_Stack.push(pItem);
		}
	}

	void SendBatchItems()
	{
		long lBytesInSndMemory;
		int nMaxSndMemory = 40 * 1024;
		int nBatchSize = 200;
		if(m_pLargeStack == UNULL_PTR || m_pLargeStack->size() == 0)
			return;

		CScopeUQueue UQueue;
		
		GetAttachedClientSocket()->GetIUSocket()->get_BytesInSndMemory(&lBytesInSndMemory);
		while(lBytesInSndMemory < nMaxSndMemory && m_pLargeStack->size() > 0 && nBatchSize > 0)
		{
			CTestItem *pItem = m_pLargeStack->top();
			m_pLargeStack->pop();
			
			UQueue << *pItem;
			delete pItem;

			nBatchSize--;
			if(nBatchSize == 0)
			{
				GetAttachedClientSocket()->GetIUSocket()->get_BytesInSndMemory(&lBytesInSndMemory);
				if(lBytesInSndMemory < nMaxSndMemory)
				{
					nBatchSize = 200;
				}
				SendRequest(idSendBatchItemsCTThree, UQueue->GetBuffer(), UQueue->GetSize());
				UQueue->SetSize(0);
			}
		}
		if(UQueue->GetSize() > 0)
		{
			SendRequest(idSendBatchItemsCTThree, UQueue->GetBuffer(), UQueue->GetSize());
		}
	}

	stack<CTestItem*> *m_pLargeStack;


public:
	const CTestItem& GetOneItem()
	{
		GetOneItemAsyn();
		GetAttachedClientSocket()->WaitAll();
		return m_GetOneItemRtn;
	}

	void SendOneItem(const CTestItem &Item)
	{
		bool b = ProcessR0(idSendOneItemCTThree, Item);
	}

	stack<CTestItem*>& GetManyItems(int nCount)
	{
		EmptyStack();
		bool bProcessRy = ProcessR0(idGetManyItemsCTThree, nCount);
		return m_Stack;
	}

	void SendManyItems(stack<CTestItem*> *pLargeStack)
	{
		m_pLargeStack = pLargeStack;
		SendBatchItems();
		WaitAll();		
		//inform server that there is no more item to be sent
		bool b = ProcessR0(idSendManyItemsCTThree);
		m_pLargeStack = UNULL_PTR;
	}
};
#endif
