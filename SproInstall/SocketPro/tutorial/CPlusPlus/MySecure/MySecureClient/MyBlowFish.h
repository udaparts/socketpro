#ifndef ___SOCKETPRO_CLIENT_HANDLER_MYBLOWFISH_H__
#define ___SOCKETPRO_CLIENT_HANDLER_MYBLOWFISH_H__

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "MyBlowFish_i.h"

//client handler for service CMySecure
class CMySecure : public CAsyncServiceHandler
{
public:
	CMySecure(CClientSocket *pClientSocket = UNULL_PTR, IAsyncResultsHandler *pDefaultAsyncResultsHandler = UNULL_PTR)
		: CAsyncServiceHandler(sidCMySecure, pClientSocket, pDefaultAsyncResultsHandler)
	{
	}

	void BeginTransAsyn()
	{
		SendRequest(idBeginTransCMySecure);
	}
	void ExecuteNoQueryAsyn(LPCWSTR strSQL)
	{
		SendRequest(idExecuteNoQueryCMySecure, strSQL);
	}
	
	void CommitAsyn(bool bSmart)
	{
		SendRequest(idCommitCMySecure, bSmart);
	}

public:
	int			m_nErrorCode;
	CComBSTR	m_bstrErrorMessage;

private:
	void PopError(CUQueue &UQueue)
	{
		UQueue >> m_nErrorCode >> m_bstrErrorMessage;
	}

protected:
	CComBSTR m_OpenRtn;
	void OpenAsyn(LPCWSTR strUserIDToDB, LPCWSTR strPasswordToDB)
	{
		SendRequest(idOpenCMySecure, strUserIDToDB, strPasswordToDB);
	}

	bool m_BeginTransRtn;
	bool m_ExecuteNoQueryRtn;
	bool m_CommitRtn;
	
	//We can process returning results inside the function.
	virtual void OnResultReturned(unsigned short usRequestID, CUQueue &UQueue)
	{
		PopError(UQueue);
		switch(usRequestID)
		{
		case idOpenCMySecure:
			UQueue >> m_OpenRtn;
			break;
		case idBeginTransCMySecure:
			UQueue >> m_BeginTransRtn;
			break;
		case idExecuteNoQueryCMySecure:
			UQueue >> m_ExecuteNoQueryRtn;
			break;
		case idCommitCMySecure:
			UQueue >> m_CommitRtn;
			break;
		default:
			break;
		}
	}

public:
	const CComBSTR& Open(LPCWSTR strUserIDToDB, LPCWSTR strPasswordToDB)
	{
		OpenAsyn(strUserIDToDB, strPasswordToDB);
		GetAttachedClientSocket()->WaitAll();
		return m_OpenRtn;
	}

	bool BeginTrans()
	{
		BeginTransAsyn();
		GetAttachedClientSocket()->WaitAll();
		return m_BeginTransRtn;
	}

	bool ExecuteNoQuery(LPCWSTR strSQL)
	{
		ExecuteNoQueryAsyn(strSQL);
		GetAttachedClientSocket()->WaitAll();
		return m_ExecuteNoQueryRtn;
	}

	bool Commit(bool bSmart)
	{
		CommitAsyn(bSmart);
		GetAttachedClientSocket()->WaitAll();
		return m_CommitRtn;
	}
};
#endif
