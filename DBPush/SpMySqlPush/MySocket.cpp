#include "StdAfx.h"
#include "MySocket.h"
#include "MyConnectionContext.h"

extern CMyConnectionContext		g_cc;

CMySocket::CMySocket(void)
{
}

CMySocket::~CMySocket(void)
{

}

HRESULT CMySocket::OnSocketConnected(long hSocket, long lError)
{
	if(lError == 0)
	{
		USES_CONVERSION;
		short sEncryptionMethod = 0;
		GetIUSocket()->get_EncryptionMethod(&sEncryptionMethod);
		if(sEncryptionMethod != NoEncryption)
		{
			bool ok = false;
			do
			{
				long res = 0;
				CComBSTR bstrError, bstrSubject;
				CComVariant vtCert;
				CComQIPtr<IUCert> pIUCert;
				GetIUSocket()->get_PeerCertificate(&vtCert);
				switch(vtCert.vt)
				{
				case VT_UNKNOWN:
					pIUCert = vtCert.punkVal;
					break;
				case VT_DISPATCH:
					pIUCert = vtCert.pdispVal;
					break;
				default:
					break;
				}
				if(!pIUCert)
				{
					g_cc.m_VerifyCode = 7;
					break;
				}
				pIUCert->get_Subject(&bstrSubject);
				bstrSubject.ToLower();
				if(g_cc.m_strLocation.Length() > 0)
					pIUCert->put_VerifyLocation(g_cc.m_strLocation);
				
				if(pIUCert->Verify(&res, &bstrError) != S_OK)
				{
					g_cc.m_VerifyCode = 8;
					break;
				}

				if(FAILED(res) && g_cc.m_strLocation.Length() > 0 && ::wcsstr(bstrSubject.m_str, g_cc.m_strLocation.m_str) == NULL)
				{
					g_cc.m_VerifyCode = 8;
					break;
				}

				if(g_cc.m_bStrict && res != S_OK)
				{
					g_cc.m_VerifyCode = 9;
					break;
				}
				ok = true;
			}while(false);
			if(!ok)
			{
				Disconnect();
				return S_OK;
			}
			g_cc.m_VerifyCode = 0;
		}

		GetIUSocket()->put_ZipIsOn(g_cc.m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
		GetIUSocket()->SetSockOpt(soSndBuf, 116800, slSocket);
        GetIUSocket()->SetSockOpt(soRcvBuf, 116800, slSocket);
		GetIUSocket()->SetSockOpt(soTcpNoDelay, 1, slTcp);
		SetUID(OLE2T(g_cc.m_strUID));
		SetPassword(OLE2T(g_cc.m_strPassword));
		BeginBatching();
		SwitchTo(sidChat);
		GetIUSocket()->TurnOnZipAtSvr(g_cc.m_bZip ? VARIANT_TRUE : VARIANT_FALSE);
		GetIUSocket()->SetSockOptAtSvr(soSndBuf, 116800, slSocket);
        GetIUSocket()->SetSockOptAtSvr(soRcvBuf, 116800, slSocket);
		Commit(false);
	}
	return CClientSocket::OnSocketConnected(hSocket, lError);
}

HRESULT CMySocket::OnSocketClosed(long hSocket, long lError)
{
	::SetEvent(g_hEvent);
	return CClientSocket::OnSocketClosed(hSocket, lError);
}

HRESULT CMySocket::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	if(sFlag == rfCompleted)
	{
		switch(nRequestID)
		{
		case idSwitchTo:
			::SetEvent(g_hEvent);
			break;
		default:
			break;
		}
	}
	return CClientSocket::OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
}
