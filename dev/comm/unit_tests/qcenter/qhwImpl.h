#ifndef ___SOCKETPRO_SERVICES_IMPL_QHWIMPL_H__
#define ___SOCKETPRO_SERVICES_IMPL_QHWIMPL_H__

#include "aserverw.h"
using namespace SPA;
using namespace SPA::ServerSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../qhw_i.h"

//server implementation for service HWReceiver
class HWReceiverPeer : public CClientPeer
{
public:
	static CMessageQueue m_mq;

protected:
	virtual void OnSwitchFrom(unsigned int serviceId)
	{
		//initialize the object here
	}

	virtual void OnReleaseResource(bool closing, unsigned int info)
	{
		if(closing)
		{
			//closing the socket with error code = info
		}
		else
		{
			//switch to a new service with the service id = info
		}

		//release all of your resources here as early as possible
	}

	void DoDequeue(unsigned int count, bool notified, /*out*/SPA::UINT64 &DoDequeueRtn) {
		DoDequeueRtn = Dequeue(m_mq.GetHandle(), count, notified);
	}

	virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len)
	{
		BEGIN_SWITCH(reqId)
		END_SWITCH
	}

	virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len)
	{
		BEGIN_SWITCH(reqId)
			M_I2_R1(idDoDequeueHWReceiver, DoDequeue, unsigned int, bool, SPA::UINT64)
		END_SWITCH
		return 0;
	}

};

//server implementation for service HWSender
class HWSenderPeer : public CClientPeer
{
protected:
	virtual void OnSwitchFrom(unsigned int serviceId)
	{
		//initialize the object here
	}

	virtual void OnReleaseResource(bool closing, unsigned int info)
	{
		if(closing)
		{
			//closing the socket with error code = info
		}
		else
		{
			//switch to a new service with the service id = info
		}

		//release all of your resources here as early as possible
	}

	void SayHelloWord(const std::wstring &fullName, unsigned int index, /*out*/unsigned int &SayHelloWordRtn)
	{
		HWReceiverPeer::m_mq.Enqueue(idHWMessage, fullName, index);
		SayHelloWordRtn = (unsigned int)HWReceiverPeer::m_mq.GetMessageCount();
	}

	virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len)
	{
		BEGIN_SWITCH(reqId)
		END_SWITCH
	}

	virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len)
	{
		BEGIN_SWITCH(reqId)
			M_I2_R1(idSayHelloWordHWSender, SayHelloWord, std::wstring, unsigned int, unsigned int)
		END_SWITCH
		return 0;
	}

};

class CMySocketProServer : public CSocketProServer
{
protected:
	virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6)
	{
		//try amIntegrated and amMixed instead by yourself
		CSocketProServer::Config::SetAuthenticationMethod(amOwn);

		//add service(s) into SocketPro server
		AddService();
		return true; //true -- ok; false -- no listening server
	}

	virtual void OnAccept(USocket_Server_Handle h, unsigned int errCode)
	{
		//when a socket is initially established
	}

	virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId)
	{
		//give permission to all
		return true;
	}

	virtual void OnClose(USocket_Server_Handle h, unsigned int errCode)
	{
		//when a socket is closed
	}

private:
	CSocketProService<HWReceiverPeer> m_HWReceiver;
	CSocketProService<HWSenderPeer> m_HWSender;
	//One SocketPro server supports any number of services. You can list them here!

private:
	void AddService()
	{
		bool ok;

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_HWReceiver.AddMe(sidHWReceiver, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_HWReceiver.AddSlowRequest(idDoDequeueHWReceiver);

		//No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
		ok = m_HWSender.AddMe(sidHWSender, taNone);
		//If ok is false, very possibly you have two services with the same service id!

		ok = m_HWSender.AddSlowRequest(idSayHelloWordHWSender);

		//Add all of other services into SocketPro server here!
	}
};


#endif