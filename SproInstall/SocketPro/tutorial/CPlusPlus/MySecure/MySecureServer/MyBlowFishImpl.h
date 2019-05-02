#ifndef ___SOCKETPRO_SERVICES_IMPL_MYBLOWFISHIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_MYBLOWFISHIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "MyBlowFish_i.h"

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

//server implementation for service CMySecure
class CMySecurePeer : public CClientPeer
{
private:
	int			m_nErrorCode;
	CComBSTR	m_strErrorMessage;

private:
	void PushError()
	{
		m_UQueue << m_nErrorCode << m_strErrorMessage;
	}

protected:
	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		m_nErrorCode = 0;
		
		//All return results are not required to be secure
		SetEncryptionMethod(NoEncryption);
	}

	virtual void OnReleaseResource(bool bClosing, unsigned long ulInfo)
	{
		if(bClosing)
		{
			//closing the socket with error code = ulInfo
		}
		else
		{
			//switch to a new service with the service id = ulInfo
		}

		//release all of your resources here as early as possible
	}

	void Open(const CComBSTR &strUserIDToDB, const CComBSTR &strPasswordToDB, /*out*/CComBSTR &OpenRtn)
	{
		OpenRtn = L"Oracle Database";

		m_nErrorCode = 0;
		m_strErrorMessage = L"Ok!";
	}

	void BeginTrans(/*out*/bool &BeginTransRtn)
	{
		BeginTransRtn = true;
		m_nErrorCode = 0;
		m_strErrorMessage = L"BeginTrans -- Ok!";
	}

	void ExecuteNoQuery(const CComBSTR &strSQL, /*out*/bool &ExecuteNoQueryRtn)
	{
		ExecuteNoQueryRtn = true;
		m_nErrorCode = 0;
		m_strErrorMessage = L"ExecuteNoQuery -- Ok!";
	}

	void Commit(bool bSmart, /*out*/bool &CommitRtn)
	{
		CommitRtn = true;
		m_nErrorCode = 0;
		m_strErrorMessage = L"Commit -- Ok!";
	}

	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		switch(usRequestID)
		{
		case idBeginTransCMySecure:
		{
			bool BeginTransRtn;
			BeginTrans(BeginTransRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
			PushError();
			m_UQueue << BeginTransRtn;
			SendResult(usRequestID, m_UQueue);
		}
			break;
		case idCommitCMySecure:
		{
			bool bSmart;
			bool CommitRtn;
			m_UQueue >> bSmart;
			Commit(bSmart, CommitRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
			PushError();
			m_UQueue << CommitRtn;
			SendResult(usRequestID, m_UQueue);
		}
			break;
		default:
			break;
		}
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		switch(usRequestID)
		{
		case idOpenCMySecure:
		{
			CComBSTR strUserIDToDB;
			CComBSTR strPasswordToDB;
			CComBSTR OpenRtn;
			m_UQueue >> strUserIDToDB;
			m_UQueue >> strPasswordToDB;
			Open(strUserIDToDB, strPasswordToDB, OpenRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
			PushError();
			m_UQueue << OpenRtn;
			SendResult(usRequestID, m_UQueue);
		}
			break;
		case idExecuteNoQueryCMySecure:
		{
			CComBSTR strSQL;
			bool ExecuteNoQueryRtn;
			m_UQueue >> strSQL;
			ExecuteNoQuery(strSQL, ExecuteNoQueryRtn);
			m_UQueue.SetSize(0); //initialize memory chunk size to 0
			PushError();
			m_UQueue << ExecuteNoQueryRtn;
			SendResult(usRequestID, m_UQueue);
		}
			break;
		default:
			break;
		}
		return S_OK;
	}
};

class CMySocketProServer : public CSocketProServer
{
private:
	void GetPasswordFromStore(LPCWSTR strUserID, CComBSTR &strPassword)
    {
		strPassword.Empty();
        if (::_wcsicmp(strUserID, L"SocketPro") == 0)
            strPassword = "PassOne";
        else if (::_wcsicmp(strUserID, L"RDBClient") == 0)
            strPassword = "PassTwo";
    }

protected:
	virtual bool OnIsPermitted(unsigned int hSocket, unsigned long ulSvsID)
	{
		WCHAR strUserID[256] = {0};
		GetUserID(hSocket, strUserID, 256);
		CComBSTR bstrPassword;
		GetPasswordFromStore(strUserID, bstrPassword);

		//set password so that the hash of the password with salt can be sent to a client for authentication on client side.
		SetPassword(hSocket, bstrPassword);
		return true;
	}

	virtual void OnAccept(unsigned int hSocket, int nError)
	{
		//when a socket is initially established
	}

	virtual void OnClose(unsigned int hSocket, int nError)
	{
		//when a socket is closed
	}

	virtual bool OnSettingServer()
	{
		//try amIntegrated and amMixed
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);
		
		//set encryption method to BlowFish
		CSocketProServer::Config::SetDefaultEncryptionMethod(BlowFish);

		//add service(s) into SocketPro server
		AddService();

		return true;
	}

private:
	CSocketProService<CMySecurePeer> m_CMySecure;

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_CMySecure.AddMe(sidCMySecure, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_CMySecure.AddSlowRequest(idOpenCMySecure);
		ok = m_CMySecure.AddSlowRequest(idExecuteNoQueryCMySecure);
	}
};


#endif