#ifndef ___SOCKETPRO_SERVICES_IMPL_REMOTESUMIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_REMOTESUMIMPL_H__

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "..\RemoteSum_i.h"

#include "sprowrap.h"
using namespace SocketProAdapter;
using namespace SocketProAdapter::ServerSide;

//server implementation for service RemSum
class RemSumPeer : public CClientPeer
{
private:
	int m_nSum;
	int m_nStart;
	int m_nEnd;

	int Compute()
	{
		int n;
		for(n = m_nStart; n<= m_nEnd; ++n)
		{
			m_nSum += n;
			::Sleep(100);
			unsigned long rtn = SendResult(idReportProgress, n, m_nSum);
			if(rtn == SOCKET_NOT_FOUND || rtn == REQUEST_CANCELED)
				break;
		}
		m_nStart = (n + 1);
		return m_nSum;
	}

	virtual void OnSwitchFrom(unsigned long ulServiceID)
	{
		m_nSum = 0;
		m_nStart = 0;
		m_nEnd = 0;
	}

	void DoSum(int start, int end, /*out*/int &DoSumRtn)
	{
		m_nSum = 0;
		m_nStart = start;
		m_nEnd = end;
		DoSumRtn = Compute();
	}

	void Pause(/*out*/int &PauseRtn)
	{
		PauseRtn = m_nSum;
	}

	void RedoSum(/*out*/int &RedoSumRtn)
	{
		RedoSumRtn = Compute();
	}

protected:
	virtual void OnFastRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		BEGIN_SWITCH(usRequestID)
			M_I0_R1(idPauseRemSum, Pause, int)
		END_SWITCH
	}

	virtual long OnSlowRequestArrive(unsigned short usRequestID, unsigned long ulLen)
	{
		BEGIN_SWITCH(usRequestID)
			M_I2_R1(idDoSumRemSum, DoSum, int, int, int)
			M_I0_R1(idRedoSumRemSum, RedoSum, int)
		END_SWITCH
		return S_OK;
	}
};

class CMySocketProServer : public CSocketProServer
{
protected:
	virtual bool OnSettingServer()
	{
		//try amIntegrated and amMixed instead by yourself
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();
		return true; //true -- ok; false -- no listening server
	}

private:
	CSocketProService<RemSumPeer> m_RemSum;
	//One SocketPro server supports any number of services. You can list them here!

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_RemSum.AddMe(sidRemSum, 0, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_RemSum.AddSlowRequest(idDoSumRemSum);
		ok = m_RemSum.AddSlowRequest(idRedoSumRemSum);

		//Add all of other services into SocketPro server here!
	}
};


#endif