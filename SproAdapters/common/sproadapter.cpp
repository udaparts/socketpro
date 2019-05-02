#include "stdafx.h"

#include "usocket.h"

namespace SocketProAdapter
{
namespace ClientSide
{

HRESULT __stdcall CCSEvent::OnDataAvailable(long hSocket, long lBytes, long lError)
{
	if(m_pClientSocket->m_OnDataAvailable != nullptr)
	{
		m_pClientSocket->m_OnDataAvailable->Invoke(hSocket, lBytes, lError);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnOtherMessage(long hSocket, long nMsg, long wParam, long lParam)
{
	if(nMsg == msgAllRequestsProcessed && m_pClientSocket->m_OnAllRequestsProcessed != nullptr)
	{
		m_pClientSocket->m_OnAllRequestsProcessed->Invoke(hSocket, (short)wParam);
	}

	if(m_pClientSocket->m_OnOtherMessage != nullptr)
	{
		m_pClientSocket->m_OnOtherMessage->Invoke(hSocket, nMsg, wParam, lParam);
	}

	if(nMsg == msgAllRequestsProcessed && m_pClientSocket->m_cb != nullptr)
	{
		m_pClientSocket->m_cb->Invoke(m_pClientSocket);
		m_pClientSocket->m_cb = nullptr;
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnSocketClosed(long hSocket, long lError)
{
	if(m_pClientSocket->m_OnSocketClosed != nullptr)
	{
		m_pClientSocket->m_OnSocketClosed->Invoke(hSocket, lError);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnSocketConnected(long hSocket, long lError)
{
	if(m_pClientSocket->m_OnSocketConnected != nullptr)
	{
		m_pClientSocket->m_OnSocketConnected->Invoke(hSocket, lError);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnConnecting(long hSocket, long hWnd)
{
	if(m_pClientSocket->m_OnConnecting != nullptr)
	{
		m_pClientSocket->m_OnConnecting->Invoke(hSocket, hWnd);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnSendingData(long hSocket, long lError, long lSent)
{
	if(m_pClientSocket->m_OnSendingData != nullptr)
	{
		m_pClientSocket->m_OnSendingData->Invoke(hSocket, lError, lSent);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnGetHostByAddr(long lHandle, BSTR bstrHostName, BSTR bstrHostAlias, long lError)
{
	if(m_pClientSocket->m_OnGetHostByAddr != nullptr)
	{
		m_pClientSocket->m_OnGetHostByAddr->Invoke(lHandle, gcnew String(bstrHostName), gcnew String(bstrHostAlias), lError);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnGetHostByName(long lHandle, BSTR bstrHostName, BSTR bstrAlias, BSTR bstrIPAddr, long lError)
{
	if(m_pClientSocket->m_OnGetHostByName != nullptr)
	{
		m_pClientSocket->m_OnGetHostByName->Invoke(lHandle, gcnew String(bstrHostName), gcnew String(bstrHostName), gcnew String(bstrIPAddr), lError);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnClosing(long hSocket, long hWnd)
{
	if(m_pClientSocket->m_OnClosing != nullptr)
	{
		m_pClientSocket->m_OnClosing->Invoke(hSocket, hWnd);
	}
	return S_OK;
}

HRESULT __stdcall CCSEvent::OnRequestProcessed(long hSocket, short nRequestID, long lLen, long lLenInBuffer, short sFlag)
{
	USOCKETLib::tagReturnFlag rf = (USOCKETLib::tagReturnFlag)sFlag;
	m_pClientSocket->OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, rf);
	return S_OK;
}

}; //ClientSide
}; //SocketProAdapter