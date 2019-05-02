#pragma once

class CMySocket :
	public CClientSocket
{
public:
	CMySocket(void);
	~CMySocket(void);

protected:
	virtual HRESULT OnSocketClosed(long hSocket, long lError);
	virtual HRESULT OnSocketConnected(long hSocket, long lError);
	virtual HRESULT OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag);
};
