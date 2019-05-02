#ifndef ___SOCKETPRO_SERVICES_IMPL_TTHREEIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_TTHREEIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "TThree_i.h"

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

#include "..\shared.h"

#include <stack>
using namespace std;


//server implementation for service CTThree
class CTThreePeer : public CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		cout <<"Socket is switched for the service CTThree"<<endl;
	}

	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	{
		if(bClosing)
		{
			cout<< "Socket " << GetSocket() << " closed with error code = " << ulInfo << "." << endl;
		}
		else
		{
			cout << "Switched to the service ID = " << ulInfo << "." << endl;
		}

		EmptyStack();
	}

	void SendBatchItems();
	void GetOneItem(/*out*/CTestItem &GetOneItemRtn);
	void SendOneItem(const CTestItem &Item);
	void GetManyItems();
	void SendManyItems();
	virtual void OnDispatchingSlowRequest(unsigned short usRequestID);
	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		BEGIN_SWITCH(usRequestID)
			M_I0_R1(idGetOneItemCTThree, GetOneItem, CTestItem)
			M_I1_R0(idSendOneItemCTThree, SendOneItem, CTestItem)
			M_I0_R0(idSendManyItemsCTThree, SendManyItems)
		END_SWITCH
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		BEGIN_SWITCH(usRequestID)
			M_I0_R0(idGetManyItemsCTThree, GetManyItems)
			M_I0_R0(idSendBatchItemsCTThree, SendBatchItems)
		END_SWITCH
		return S_OK;
	}

private:
	stack<CTestItem*> m_Stack;
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
};

class CTThreeSvs : public CSocketProService<CTThreePeer>
{
public:
	~CTThreeSvs()
	{
		EmptyStack();
	}

private:
	stack<CTestItem*> m_StackCenter;
	void EmptyStack()
	{
		ATLASSERT(CSocketProServer::GetMainThreadID() == ::GetCurrentThreadId());
		while(m_StackCenter.size() > 0)
		{
			CTestItem *pItem = m_StackCenter.top();
			m_StackCenter.pop();
			if(pItem)
				delete pItem;
		}
	}

	friend CTThreePeer;
};


#endif