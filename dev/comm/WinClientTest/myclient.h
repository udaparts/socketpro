
#ifndef __USCOKET_CLIENT_MFC_TEST_H__
#define __USCOKET_CLIENT_MFC_TEST_H__

#include "../utest.h"

class CMySocket : public MB::ClientSide::CUClientSocket
{
protected:
	virtual void OnSocketClosed(int nError)
	{
		MB::CAutoLock sl(g_mutex);
		std::cout<<"Socket closed with errCode = "<<nError<<std::endl;
	}

	virtual void OnHandShakeCompleted(int nError)
	{

	}

	virtual void OnSocketConnected(int nError)
	{

	}

	virtual void OnRequestProcessed(unsigned short requestId, unsigned int len)
	{

	}

	virtual void OnSubscribe(CMessageSender sender, const unsigned int *pGroup, unsigned int count)
	{
		MB::CAutoLock sl(g_mutex);
		std::cout<<"OnSubscribe called with group count = "<<count<<", self = "<<sender.SelfMessage<<", sender = "<<sender.UserId<<", ip addr = "<<sender.IpAddress<<", port = "<<sender.Port<<std::endl;
	}

	virtual void OnUnsubscribe(CMessageSender sender, const unsigned int *pGroup, unsigned int count)
	{
		MB::CAutoLock sl(g_mutex);
		std::cout<<"OnUnsubscribe called with group count = "<<count<<", self = "<<sender.SelfMessage<<", sender = "<<sender.UserId<<", ip addr = "<<sender.IpAddress<<", port = "<<sender.Port<<std::endl;
	}

	virtual void OnBroadcast(CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size)
	{
		MB::CAutoLock sl(g_mutex);
		std::cout<<"OnBroadcast called with count = "<<count<<", self = "<<sender.SelfMessage<<", sender = "<<sender.UserId<<", ip addr = "<<sender.IpAddress<<", port = "<<sender.Port;
		std::string str((const char*)pMessage, (const char*)(pMessage + size));
		std::cout<<", message = "<<str<<std::endl;
	}

	virtual void OnPostUserMessage(CMessageSender sender, const unsigned char *pMessage, unsigned int size)
	{
		MB::CAutoLock sl(g_mutex);
		std::cout<<"OnPostUserMessage called with self = "<<sender.SelfMessage<<", sender = "<<sender.UserId<<", ip addr = "<<sender.IpAddress<<", port = "<<sender.Port;
		std::string str((const char*)pMessage, (const char*)(pMessage + size));
		std::cout<<", message = "<<str<<std::endl;
	}
};

class CMyServiceHandler : public MB::ClientSide::CAsyncServiceHandler
{
public:
	CMyServiceHandler()
		: MB::ClientSide::CAsyncServiceHandler(sidTestService)
	{
	}

public:
	void Sleep(unsigned int time)
	{
		bool ok = SendRequestAsync(idSleep, time);
		if(!ok)
			return;
		ok = GetAttachedClientSocket()->WaitAll();
	}

	const std::wstring& BadRequest(unsigned int n, const wchar_t* input)
	{
		m_wout.clear();
		do
		{
			if(!SendRequestAsync(idBadRequest, n, input))
				break;
			GetAttachedClientSocket()->WaitAll();
		}while(false);
		return m_wout;
	}

	bool OpenDb(const char *connString)
	{
		bool ok = SendRequestAsync(idOpenDb, connString);
		if(!ok)
			return false;
		ok = GetAttachedClientSocket()->WaitAll();
		if(!ok)
			return false;
		return m_ok;
	}

	const std::string& Echo(const char *input)
	{
		m_out.clear();
		bool ok = SendRequestAsync(idEcho, input);
		if(!ok)
			return m_out;
		ok = GetAttachedClientSocket()->WaitAll();
		if(!ok)
			return m_out;
		return m_out;
	}

private:
	bool		m_ok;
	std::string m_out;
	std::wstring m_wout;

protected:
	void OnResultCome(unsigned short reqId, MB::CUQueue &mc)
	{
		switch(reqId)
		{
		case idEcho:
			mc >> m_out;
			break;
		case idOpenDb:
			mc >> m_ok;
			break;
		case idBadRequest:
			mc >> m_wout;
			break;
		default:
			break;
		}
	}
};

#endif //__USCOKET_CLIENT_MFC_TEST_H__

