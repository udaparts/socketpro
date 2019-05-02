#include "StdAfx.h"
#include "TOneImpl.h"

unsigned long CTOnePeer::m_uGlobalCount = 0;
unsigned long CTOnePeer::m_uGlobalFastCount = 0;

void CTOnePeer::OnDispatchingSlowRequest(unsigned short usRequestID)
{
	m_uGlobalCount++;
}


void CTThreePeer::OnDispatchingSlowRequest(unsigned short usRequestID)
{
	if(usRequestID == idGetManyItemsCTThree) 
	{
		//move a given number of items from service stack into this stack
		unsigned int nCount;
		m_UQueue >> nCount;
		EmptyStack();
		ATLASSERT(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId());
		CTThreeSvs *pThreeSvs = (CTThreeSvs*)CBaseService::GetBaseService(GetSvsID());
		while(nCount > 0 && pThreeSvs->m_StackCenter.size() > 0)
		{
			CTestItem *pItem = pThreeSvs->m_StackCenter.top();
			pThreeSvs->m_StackCenter.pop();
			if(pItem != UNULL_PTR)
			{
				m_Stack.push(pItem);
			}
			nCount--;
		}
	}
}

void CTThreePeer::GetOneItem(/*out*/CTestItem &GetOneItemRtn)
{
	ATLASSERT(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId());
	CTThreeSvs *pThreeSvs = (CTThreeSvs*)CBaseService::GetBaseService(GetSvsID());
	if(pThreeSvs->m_StackCenter.size() > 0)
	{
		CTestItem *pItem = pThreeSvs->m_StackCenter.top();
		pThreeSvs->m_StackCenter.pop();
		GetOneItemRtn.m_lData = pItem->m_lData;
		GetOneItemRtn.m_strUID = pItem->m_strUID;
		GetOneItemRtn.m_vtDT = pItem->m_vtDT;
		delete pItem;
	}
	else
		GetOneItemRtn.m_bNull = true;
}

void CTThreePeer::SendOneItem(const CTestItem &Item)
{
	CTestItem *pItem = new CTestItem();
	pItem->m_lData = Item.m_lData;
	pItem->m_strUID = Item.m_strUID;
	pItem->m_vtDT = Item.m_vtDT;
	
	ATLASSERT(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId());
	CTThreeSvs *pThreeSvs = (CTThreeSvs*)CBaseService::GetBaseService(GetSvsID());
	pThreeSvs->m_StackCenter.push(pItem);
}

void CTThreePeer::SendBatchItems()
{
	while(m_UQueue.GetSize() > 0)
	{
		CTestItem *pItem;
		try
		{
			pItem = new CTestItem();
		}
		catch(...)
		{
			ULONG wError = ::GetLastError();
			cout << "Exception caught with error code = " << wError << endl;
			continue;
		}
		m_UQueue >> *pItem;
		m_Stack.push(pItem);
	}
}

void CTThreePeer::SendManyItems()
{
	ATLASSERT(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId());
	CTThreeSvs *pThreeSvs = (CTThreeSvs*)CBaseService::GetBaseService(GetSvsID());
	while(m_Stack.size() > 0)
	{
		CTestItem *pItem = m_Stack.top();
		m_Stack.pop();
		ATLASSERT(pItem != UNULL_PTR);
		pThreeSvs->m_StackCenter.push(pItem);
	}
}

void CTThreePeer::GetManyItems()
{
	long nRtn = 0;
	m_UQueue.SetSize(0);
	while(m_Stack.size() > 0)
	{
		//a client may either shut down the socket connection or call IUSocket::Cancel
		if(nRtn == SOCKET_NOT_FOUND || nRtn == REQUEST_CANCELED)
			break;
		CTestItem *pItem = (CTestItem*)m_Stack.top();
		m_Stack.pop();
		ATLASSERT(pItem != UNULL_PTR);
		m_UQueue << *pItem;
		delete pItem;
	
		
		//20 kbytes per batch at least
		//also shouldn't be too large. 
		//If the size is too large, it will cost more memory resource and reduce conccurency if online compressing is enabled.
		//for an opimal value, you'd better test it by yourself
		if(m_UQueue.GetSize() > 20*1024)
		{
			nRtn = SendResult(idGetBatchItemsCTThree, m_UQueue);
			//reset memory queue
			m_UQueue.SetSize(0);
		}
	}
	if(nRtn == SOCKET_NOT_FOUND || nRtn == REQUEST_CANCELED)
	{

	}
	else if(m_UQueue.GetSize() > 0)
	{
		nRtn = SendResult(idGetBatchItemsCTThree, m_UQueue);
	}
}

