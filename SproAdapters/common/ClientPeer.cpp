#include "StdAfx.h"

namespace SocketProAdapter
{
#ifndef _WIN32_WCE
namespace ServerSide
{

CClientPeer::CClientPeer() : m_sCurrentRequestId(0)
{
	m_bAutoBuffer = true;
	m_UQueue = gcnew CUQueue();
	m_pPush = gcnew CUPushServerImpl();
	m_pPush->m_cp = this;
}

CClientPeer::~CClientPeer()
{
	m_UQueue->Empty();
}

bool CClientPeer::CUPushServerImpl::Enter(array<int> ^Groups)
{
	if(Groups == nullptr || Groups->Length == 0)
		return Exit();
	if(g_SocketProLoader.XEnter == NULL)
	{
		unsigned long ulGroups = 0;
		for each(int n in Groups)
			ulGroups |= n;
		return g_SocketProLoader.Enter(m_cp->m_hSocket, ulGroups);
	}
	CComVariant vtGroups;
	System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Groups, IntPtr(&vtGroups));
	return g_SocketProLoader.XEnter(m_cp->m_hSocket, &vtGroups);
}

bool CClientPeer::CUPushServerImpl::Broadcast(Object ^Message, array<int> ^Groups)
{
	if(Groups == nullptr || Groups->Length == 0)
		return false;
	CComVariant vtMessage;
	System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Message, IntPtr(&vtMessage));

	if(g_SocketProLoader.XSpeak == NULL)
	{
		unsigned long ulGroups = 0;
		for each(int n in Groups)
			ulGroups |= n;
		return g_SocketProLoader.Speak(m_cp->m_hSocket, &vtMessage, ulGroups);
	}
	CComVariant vtGroups;
	System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Groups, IntPtr(&vtGroups));
	return g_SocketProLoader.XSpeak(m_cp->m_hSocket, &vtMessage, &vtGroups);
}

bool CClientPeer::CUPushServerImpl::Broadcast(array<unsigned char> ^Message, array<int> ^Groups)
{
	if(Groups == nullptr || Groups->Length == 0)
		return false;
	pin_ptr<int> pGroup = &Groups[0];
	unsigned long *pulGroup = (unsigned long *)pGroup;
	pin_ptr<unsigned char> wch;
	BYTE *pBuffer = NULL;
	unsigned long ulMessageSize = 0;
	if(Message != nullptr)
	{
		ulMessageSize = Message->Length;
		if(ulMessageSize)
		{
			wch = &Message[0];
			pBuffer = wch;
		}
	}
	if(g_SocketProLoader.XSpeakEx == NULL)
	{
		unsigned long ulGroups = 0;
		for each(int n in Groups)
			ulGroups |= n;
		return g_SocketProLoader.SpeakEx(m_cp->m_hSocket, pBuffer, ulMessageSize, ulGroups);
	}
	return g_SocketProLoader.XSpeakEx(m_cp->m_hSocket, pBuffer, ulMessageSize, Groups->Length, pulGroup);
}

bool CClientPeer::CUPushServerImpl::SendUserMessage(Object ^Message, String ^UserId)
{
	CComVariant vtMessage;
	System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Message, IntPtr(&vtMessage));
	pin_ptr<const wchar_t> wch = PtrToStringChars(UserId);
	CComBSTR bstrUserId(wch);
	return g_SocketProLoader.SendUserMessage(m_cp->m_hSocket, bstrUserId, &vtMessage);
}

bool CClientPeer::CUPushServerImpl::SendUserMessage(String ^UserId, array<unsigned char> ^Message)
{
	pin_ptr<const wchar_t> wch = PtrToStringChars(UserId);
	CComBSTR strUserId(wch);
	pin_ptr<unsigned char> pMessage;
	unsigned long ulMessageSize = 0;
	if(Message != nullptr)
	{
		ulMessageSize = Message->Length;
		if(ulMessageSize)
			pMessage = &Message[0];
	}
	return g_SocketProLoader.SendUserMessageEx(m_cp->m_hSocket, strUserId, pMessage, ulMessageSize);
}

bool CClientPeer::CUPushServerImpl::Exit()
{
	return g_SocketProLoader.Exit(m_cp->m_hSocket);
}

bool CClientPeer::AbortBatching()
{
	return g_SocketProLoader.AbortBatching(m_hSocket);
}

bool CClientPeer::CommitBatching()
{
	return g_SocketProLoader.CommitBatching(m_hSocket);
}

CBaseService^ CClientPeer::GetBaseService()
{
	return CBaseService::GetBaseService(SvsID);
}

/*
bool CClientPeer::Enter(int nGroupIDs)
{
	return g_SocketProLoader.Enter(m_hSocket, nGroupIDs);
}*/

void CClientPeer::OnFRA(short sRequestID, int nLen)
{
	m_sCurrentRequestId = sRequestID;
	OnFastRequestArrive(sRequestID, nLen);
}

int CClientPeer::OnSRA(short sRequestID, int nLen)
{
	m_sCurrentRequestId = sRequestID;
	return OnSlowRequestArrive(sRequestID, nLen);
}

void CClientPeer::OnRR(bool bClosing, int nInfo)
{
	OnReleaseResource(bClosing, nInfo);
}

void CClientPeer::OnSF(int nSvsID)
{
	OnSwitchFrom(nSvsID);
}

void CClientPeer::OnR(int nError)
{
	OnReceive(nError);
}

void CClientPeer::OnS(int nError)
{
	OnSend(nError);
}

void CClientPeer::OnBRC(short sRequestID)
{
	if(sRequestID == idCleanTrack)
	{
		m_UQueue->CleanTrack();
	}
	OnBaseRequestCame(sRequestID);
}

void CClientPeer::OnDSR(short sRequestID)
{
	m_sCurrentRequestId = sRequestID;
	OnDispatchingSlowRequest(sRequestID);
}

void CClientPeer::OnSRP(short sRequestID)
{
	m_sCurrentRequestId = sRequestID;
	OnSlowRequestProcessed(sRequestID);
}

void CClientPeer::OnCRComing(USOCKETLib::tagChatRequestID sRequestID, Object ^Param0, Object ^Param1)
{
	OnChatRequestComing(sRequestID, Param0, Param1);
}

void CClientPeer::OnCRCame(USOCKETLib::tagChatRequestID sRequestID)
{
	OnChatRequestCame(sRequestID);
}

bool CClientPeer::OnSRData(short sRequestID, int nLen, IntPtr pBuffer)
{
	return OnSendReturnData(sRequestID, nLen, pBuffer);
}

/*
bool CClientPeer::Exit()
{
	return g_SocketProLoader.Exit(m_hSocket);
}*/

bool CClientPeer::GetInterfaceAttributes(int %nMTU, int %nMaxSpeed, tagInterfaceType %Type, int %nMask)
{
	ULONG ulMTU;
	ULONG ulSpeed;
	ULONG ulType;
	ULONG ulMask;

	bool b = g_SocketProLoader.GetInterfaceAttributes(m_hSocket, &ulMTU, &ulSpeed, &ulType, &ulMask);
	if(b)
	{
		nMTU = ulMTU;
		nMaxSpeed = ulSpeed;
		Type = (tagInterfaceType)ulType;
		nMask = ulMask;
	}
	return b;
}

int CClientPeer::GetMySpecificBytes(array<BYTE> ^Buffer)
{
	int nLen = 0;
	pin_ptr<BYTE> pBuffer = nullptr;
	if(Buffer != nullptr)
	{
		nLen = Buffer->Length;
		if(nLen > 0)
			pBuffer = &(Buffer[0]);
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return g_SocketProLoader.GetMySpecificBytes(m_hSocket, pBuffer, nLen);
}

String^ CClientPeer::GetPeerName(int %nPeerPort)
{
	WCHAR strData[1025] = {0};
	UINT unPeerPort = 0;
	g_SocketProLoader.GetPeerName(m_hSocket, &unPeerPort, strData, sizeof(strData)/sizeof(WCHAR));
	nPeerPort = unPeerPort;
	return gcnew String(strData);
}

int CClientPeer::GetRequestIDsInQueue(array<short> ^psRequestID)
{
	return GetRequestIDsInQueue(psRequestID, psRequestID ? psRequestID->Length : 0);
}

int CClientPeer::GetRequestIDsInQueue(array<short> ^psRequestID, int nSize)
{
	if(nSize <= 0)
		return 0;
	pin_ptr<short> pReqID = nullptr;
	if(psRequestID != nullptr)
	{
		if(psRequestID->Length > 0)
		{
			pReqID = &(psRequestID[0]);
			if(nSize > psRequestID->Length)
				nSize = psRequestID->Length;
		}
	}
	return g_SocketProLoader.GetRequestIDsInQueue(m_hSocket, (unsigned short*)pReqID, nSize);
}

String^ CClientPeer::GetSockAddr(int %nSocketPort)
{
	WCHAR strData[1025] = {0};
	UINT unPeerPort = 0;
	g_SocketProLoader.GetSockAddr(m_hSocket, &unPeerPort, strData, sizeof(strData)/sizeof(WCHAR));
	nSocketPort = unPeerPort;
	return gcnew String(strData);
}

void CClientPeer::OnReleaseResource(bool bClosing, int nInfo)
{

}

void CClientPeer::OnSwitchFrom(int nSvsID)
{

}

void CClientPeer::OnReceive(int nError)
{

}

void CClientPeer:: OnSend(int nError)
{

}

void CClientPeer::OnDispatchingSlowRequest(short sRequestID)
{

}

void CClientPeer::OnBaseRequestCame(short sRequestID)
{

}

void CClientPeer::OnSlowRequestProcessed(short sRequestID)
{

}

void CClientPeer::OnChatRequestComing(USOCKETLib::tagChatRequestID sRequestID, Object ^Param0, Object ^Param1)
{

}

void CClientPeer::OnChatRequestCame(USOCKETLib::tagChatRequestID sRequestID)
{

}

bool CClientPeer::OnSendReturnData(short sRequestID, int nLen, IntPtr pBuffer)
{
	return false;
}

bool CClientPeer::PostClose(short sError)
{
	return g_SocketProLoader.PostClose(m_hSocket, sError);
}

int CClientPeer::RetrieveBuffer(IntPtr pBuffer, int nLen, bool bPeek)
{
	pin_ptr<BYTE> pByte = (BYTE*)pBuffer.ToPointer();
	if(pByte == nullptr || nLen == 0)
		return 0;
	return g_SocketProLoader.RetrieveBuffer(m_hSocket, (ULONG)nLen, pByte, bPeek);
}

int CClientPeer::RetrieveBuffer(IntPtr pBuffer, int nLen)
{
	return RetrieveBuffer(pBuffer, nLen, false);
}

int CClientPeer::RetrieveBuffer(int hSocket, IntPtr pBuffer, int nLen, bool bPeek)
{
	pin_ptr<BYTE> pByte = (BYTE*)pBuffer.ToPointer();
	if(pByte == nullptr || nLen == 0)
		return 0;
	return g_SocketProLoader.RetrieveBuffer(hSocket, (ULONG)nLen, pByte, bPeek);
}

int CClientPeer::RetrieveBuffer(int hSocket, IntPtr pBuffer, int nLen)
{
	return RetrieveBuffer(hSocket, pBuffer, nLen, false);
}

int CClientPeer::SendResult(short sRequestID)
{
	CScopeUQueue UQueue;
	if(TransferServerException)
		UQueue.m_UQueue->Push((long)0); //required by client side
	return SendReturnData(m_hSocket, sRequestID, UQueue.m_UQueue->GetBuffer(), UQueue.m_UQueue->GetSize());
}

int CClientPeer::SendResult(short sRequestID, CUQueue ^UQueue)
{
	if(TransferServerException)
	{
		int nRtn = 0;
		CScopeUQueue su;
		su.m_UQueue->Push(nRtn);
		if(UQueue != nullptr)
		{
			su.m_UQueue->Push(UQueue->GetBuffer(), UQueue->GetSize());
		}
		return SendReturnData(sRequestID, su.m_UQueue->GetBuffer(), su.m_UQueue->GetSize());
	}
	return SendReturnData(sRequestID, UQueue);
}

int CClientPeer::SendResult(short sRequestID, CScopeUQueue ^UQueue)
{
	if(TransferServerException)
	{
		int nRtn = 0;
		CScopeUQueue su;
		su.m_UQueue->Push(nRtn);
		if(UQueue != nullptr)
		{
			su.m_UQueue->Push(UQueue->m_UQueue->GetBuffer(), UQueue->m_UQueue->GetSize());
		}
		return SendReturnData(sRequestID, su.m_UQueue->GetBuffer(), su.m_UQueue->GetSize());
	}
	if(UQueue == nullptr)
		return SendReturnData(m_hSocket, sRequestID, IntPtr::Zero, 0);
	return SendReturnData(sRequestID, UQueue->m_UQueue);
}

int CClientPeer::SendReturnData(int hSocket, array<BYTE> ^pBuffer, short sRequestID)
{
	int nLen = 0;
	if(pBuffer != nullptr)
		nLen = pBuffer->Length;
	return SendReturnData(hSocket, sRequestID, pBuffer, nLen);
}

int CClientPeer::SendReturnData(int hSocket, short sRequestID, array<BYTE> ^pBuffer, int nLen)
{
	if(pBuffer == nullptr)
		return g_SocketProLoader.SendReturnData(hSocket, sRequestID, 0, NULL);
	if((UINT)nLen > (UINT)pBuffer->Length)
		nLen = pBuffer->Length;
	if(nLen == 0)
		return g_SocketProLoader.SendReturnData(hSocket, sRequestID, 0, NULL);
	pin_ptr<BYTE> pByte = &pBuffer[0];
	return g_SocketProLoader.SendReturnData(hSocket, sRequestID, nLen, pByte);
}

int CClientPeer::SendReturnData(int hSocket, short sRequestID, CUQueue ^UQueue)
{
	if(UQueue == nullptr)
		return g_SocketProLoader.SendReturnData(hSocket, sRequestID, 0, NULL);
	return SendReturnData(hSocket, sRequestID, UQueue->GetBuffer(), UQueue->GetSize());
}

int CClientPeer::SendReturnData(short sRequestID)
{
	return SendReturnData(sRequestID, nullptr, 0);
}

int CClientPeer::SendReturnData(int hSocket, short sRequestID, IntPtr pBuffer, int nLen)
{
	pin_ptr<BYTE> pByte = (BYTE*)pBuffer.ToPointer();
	if(pByte == nullptr)
		return g_SocketProLoader.SendReturnData(hSocket, sRequestID, 0, NULL);;
	return g_SocketProLoader.SendReturnData(hSocket, sRequestID, nLen, pByte);
}

int CClientPeer::SendReturnData(array<BYTE> ^pBuffer, short sRequestID)
{
	int nLen = 0;
	if(pBuffer != nullptr)
		nLen = pBuffer->Length;
	return SendReturnData(sRequestID, pBuffer, nLen);
}

int CClientPeer::SendReturnData(short sRequestID, array<BYTE> ^pBuffer, int nLen)
{
	if(pBuffer == nullptr)
		return g_SocketProLoader.SendReturnData(m_hSocket, sRequestID, 0, NULL);
	if((UINT)nLen > (UINT)pBuffer->Length)
		nLen = pBuffer->Length;
	if(nLen == 0)
		return g_SocketProLoader.SendReturnData(m_hSocket, sRequestID, 0, NULL);
	pin_ptr<BYTE> pByte = &pBuffer[0];
	return g_SocketProLoader.SendReturnData(m_hSocket, sRequestID, (ULONG)nLen, pByte);
}

int CClientPeer::SendReturnData(short sRequestID, CUQueue ^UQueue)
{
	if(UQueue == nullptr)
		return g_SocketProLoader.SendReturnData(m_hSocket, sRequestID, 0, NULL);
	return SendReturnData(sRequestID, UQueue->GetBuffer(), UQueue->GetSize());
}

int CClientPeer::SendReturnData(short sRequestID, IntPtr pBuffer, int nLen)
{
	pin_ptr<BYTE> pByte = (BYTE*)pBuffer.ToPointer();
	if(pByte == nullptr)
		return g_SocketProLoader.SendReturnData(m_hSocket, sRequestID, 0, NULL);
	return g_SocketProLoader.SendReturnData(m_hSocket, sRequestID, (ULONG)nLen, pByte);
}

bool CClientPeer::SetBlowFish(array<BYTE> ^pKey)
{
	pin_ptr<BYTE> pByte;
	ULONG ulLen;
	if(pKey == nullptr || pKey->Length == 0)
	{
		pByte = nullptr;
		ulLen = 0;
	}
	else
	{
		ulLen = (ULONG)pKey->Length;
		pByte = &pKey[0];
	}
	return g_SocketProLoader.SetBlowFish(m_hSocket, (BYTE)ulLen, pByte);
}

void CClientPeer::ShrinkMemory()
{
	g_SocketProLoader.ShrinkMemory(m_hSocket);
}

/*
bool CClientPeer::Speak(Object^ vtMsg)
{
	return Speak(vtMsg, -1);
}*/

void CClientPeer::ConvertFromObjectToVariant(Object ^obj, VARIANT *pvt)
{
	if(pvt == NULL)
		return;
	if(obj == nullptr)
	{
		pvt->vt = VT_NULL;
		return;
	}
	Type ^t = obj->GetType();
	if(t == array<DateTime^>::typeid)
	{
		unsigned long ulIndex;
		__int64 *pllTime;
		array<DateTime^> ^aDT = (array<DateTime^>^)obj;
		unsigned long ulCount = aDT->Length;
		SAFEARRAYBOUND sab[1] = {ulCount, 0};
		pvt->vt = (VT_ARRAY | VT_FILETIME);
		pvt->parray = ::SafeArrayCreate(VT_FILETIME, 1, sab);
		::SafeArrayAccessData(pvt->parray, (void**)&pllTime);
		for(ulIndex=0; ulIndex<ulCount; ulIndex++)
		{
			pllTime[ulIndex] = (aDT[ulIndex])->ToFileTime();
		}
		::SafeArrayUnaccessData(pvt->parray);
	}
	else if(t == DateTime::typeid)
	{
		pvt->vt = VT_FILETIME;
		pvt->llVal = ((DateTime^)obj)->ToFileTime();
	}
	else
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(obj, IntPtr(pvt));
}

/*
bool CClientPeer::Speak(Object^ vtMsg, int nGroups)
{
	CComVariant vtData;
	if(vtMsg != nullptr)
		ConvertFromObjectToVariant(vtMsg, &vtData);
	return g_SocketProLoader.Speak(m_hSocket, &vtData, nGroups);
}

bool CClientPeer::SpeakEx(IntPtr pMessage, int nLen)
{
	return SpeakEx(pMessage, nLen, -1);
}

bool CClientPeer::SpeakEx(array<BYTE> ^pMessage)
{
	return SpeakEx(pMessage, -1);
}

bool CClientPeer::SpeakEx(IntPtr pMessage, int nLen, int nGroups)
{
	if(pMessage.ToPointer() == nullptr)
	{
		return g_SocketProLoader.SpeakEx(m_hSocket, NULL, 0, nGroups);
	}
	pin_ptr<BYTE> pData = (BYTE*)pMessage.ToPointer();
	return g_SocketProLoader.SpeakEx(m_hSocket, pData, nLen, nGroups);
}

bool CClientPeer::SpeakEx(array<BYTE> ^pMessage, int nGroups)
{
	if(pMessage == nullptr || pMessage->Length == 0)
	{
		return g_SocketProLoader.SpeakEx(m_hSocket, NULL, 0, nGroups);
	}
	pin_ptr<BYTE> pData = &pMessage[0];
	return g_SocketProLoader.SpeakEx(m_hSocket, pData, pMessage->Length, nGroups);
}*/

bool CClientPeer::StartBatching()
{
	return g_SocketProLoader.StartBatching(m_hSocket);
}

void CClientPeer::DropRequestResult(short sRequestId)
{
	if(g_SocketProLoader.DropRequestResult != NULL)
	{
		g_SocketProLoader.DropRequestResult(m_hSocket, sRequestId);
	}
}

void CClientPeer::DropCurrentSlowRequest()
{
	return g_SocketProLoader.DropCurrentSlowRequest(m_hSocket);
}

bool CClientPeer::IsClosing()
{
	return g_SocketProLoader.IsClosing(m_hSocket);
}

/*
bool CClientPeer::SpeakTo(int hSocketReceiver, Object^ vtMsg)
{
	CComVariant vtData;
	if(vtMsg != nullptr)
		ConvertFromObjectToVariant(vtMsg, &vtData);
	return g_SocketProLoader.SpeakTo(m_hSocket, &vtData, hSocketReceiver);
}

bool CClientPeer::SendUserMessage(String ^strUserID, Object^ vtMsg)
{
	if(strUserID == nullptr)
		return false;
	pin_ptr<const wchar_t> wch = PtrToStringChars(strUserID);
	CComVariant vtData;
	if(vtMsg != nullptr)
		ConvertFromObjectToVariant(vtMsg, &vtData);
	return g_SocketProLoader.SendUserMessage(m_hSocket, wch, &vtData);
}

bool CClientPeer::SendUserMessageEx(String ^strUserID, array<BYTE> ^pMessage)
{
	if(strUserID == nullptr)
		return false;
	pin_ptr<const wchar_t> wch = PtrToStringChars(strUserID);
	
	if(pMessage == nullptr || pMessage->Length == 0)
	{
		return g_SocketProLoader.SendUserMessageEx(m_hSocket, wch, NULL, 0);
	}
	pin_ptr<BYTE> pData = &pMessage[0];
	return g_SocketProLoader.SendUserMessageEx(m_hSocket, wch, pData, pMessage->Length);
}

bool CClientPeer::SpeakToEx(int hSocketReceiver, array<BYTE> ^pMessage)
{
	if(pMessage == nullptr || pMessage->Length == 0)
	{
		return g_SocketProLoader.SpeakToEx(m_hSocket, NULL, 0, hSocketReceiver);
	}
	pin_ptr<BYTE> pData = &pMessage[0];
	return g_SocketProLoader.SpeakToEx(m_hSocket, pData, pMessage->Length, hSocketReceiver);
}

bool CClientPeer::SpeakToEx(int hSocketReceiver, IntPtr pMessage, int nLen)
{
	if(pMessage.ToPointer() == nullptr)
	{
		return g_SocketProLoader.SpeakToEx(m_hSocket, NULL, 0, hSocketReceiver);
	}
	pin_ptr<BYTE> pData = (BYTE*)pMessage.ToPointer();
	return g_SocketProLoader.SpeakToEx(m_hSocket, pData, nLen, hSocketReceiver);
}

bool CClientPeer::SpeakTo(CClientPeer ^Receiver, Object^ vtMsg)
{
	if(Receiver == nullptr)
		return false;
	CComVariant vtData;
	if(vtMsg != nullptr)
		ConvertFromObjectToVariant(vtMsg, &vtData);
	return g_SocketProLoader.SpeakTo(m_hSocket, &vtData, Receiver->m_hSocket);
}

bool CClientPeer::SpeakToEx(CClientPeer ^Receiver, array<BYTE> ^pMessage)
{
	if(Receiver == nullptr)
		return false;
	if(pMessage == nullptr || pMessage->Length == 0)
	{
		return g_SocketProLoader.SpeakToEx(m_hSocket, NULL, 0, Receiver->m_hSocket);
	}
	pin_ptr<BYTE> pData = &pMessage[0];
	return g_SocketProLoader.SpeakToEx(m_hSocket, pData, pMessage->Length, Receiver->m_hSocket);
}

bool CClientPeer::SpeakToEx(CClientPeer ^Receiver, IntPtr pMessage, int nLen)
{
	if(Receiver == nullptr)
		return false;
	if(pMessage.ToPointer() == nullptr)
	{
		return g_SocketProLoader.SpeakToEx(m_hSocket, NULL, 0, Receiver->m_hSocket);
	}
	pin_ptr<BYTE> pData = (BYTE*)pMessage.ToPointer();
	return g_SocketProLoader.SpeakToEx(m_hSocket, pData, nLen, Receiver->m_hSocket);
}*/

CUQueue^ CClientPeer::GetUQueue()
{
	return m_UQueue;
}

bool CHttpPeerBase::SetResponseHeader(String ^strHeader, String ^strValue)
{
	USES_CONVERSION;
	if(!strHeader || strHeader->Length == 0)
		return false;
	if(g_SocketProLoader.SetHTTPResponseHeader == NULL)
		return false;
	char *strUTF8Value = NULL;
	pin_ptr<const wchar_t> wch = PtrToStringChars( strHeader );
	char *strUTF8Header = OLE2A(wch);
	pin_ptr<const wchar_t> wstr;
	if(strValue != nullptr)
	{
		wstr = PtrToStringChars(strValue);
		strUTF8Value = OLE2A(wstr);
	}
	return g_SocketProLoader.SetHTTPResponseHeader(m_hSocket, strUTF8Header, strUTF8Value);
}

array<long>^ CHttpPeerBase::CHttpPushImpl::GetHttpChatGroupIds(String^ strSessionId)
{
	pin_ptr<const wchar_t> wstr =  PtrToStringChars(strSessionId);
	if(g_SocketProLoader.GetHTTPJoinedGroups)
	{
		const VARIANT *pvtGroups = g_SocketProLoader.GetHTTPJoinedGroups(m_HttpPeer->m_hSocket, wstr);
		if(pvtGroups)
		{
			CComVariant vtGroups;
			::VariantChangeType(&vtGroups, (VARIANT *)pvtGroups, VARIANT_NOVALUEPROP, (VT_ARRAY|VT_I4));
			if(vtGroups.vt == (VT_ARRAY|VT_I4))
			{
				Object ^obj = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtGroups));
				return (array<long>^)obj;
			}
		}
	}
	return nullptr;
}

String^ CHttpPeerBase::CHttpPushImpl::Enter(array<long>^ GroupIds, String^ strUserID, int nLeaseTime)
{
	return Enter(GroupIds, strUserID, nLeaseTime, nullptr);
}

String^ CHttpPeerBase::CHttpPushImpl::Enter(array<long>^ GroupIds, String^ strUserID, int nLeaseTime, String^ strIpAddr)
{
	LPCWSTR strSessionID;
	if(strUserID == nullptr)
		g_SocketProLoader.SetUserID(m_HttpPeer->m_hSocket, L"");
	else
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars(strUserID);
		g_SocketProLoader.SetUserID(m_HttpPeer->m_hSocket, wch);
	}
	if(g_SocketProLoader.XHTTPEnter && GroupIds != nullptr && GroupIds->Length > 0)
	{
		CComVariant vtGroups;
		Object ^objGroups = GroupIds;
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(objGroups, IntPtr(&vtGroups));
		if(strIpAddr == nullptr || strIpAddr->Length == 0)
			strSessionID = g_SocketProLoader.XHTTPEnter(m_HttpPeer->m_hSocket, &vtGroups, (unsigned long)nLeaseTime, NULL);
		else
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars(strIpAddr);
			strSessionID = g_SocketProLoader.XHTTPEnter(m_HttpPeer->m_hSocket, &vtGroups, (unsigned long)nLeaseTime, wch);
		}
		return gcnew String(strSessionID);
	}
	unsigned long ulGroups = 0;
	if(GroupIds != nullptr)
	{
		for each(long group in GroupIds)
		{
			ulGroups |= group;
		}
	}
	if(strIpAddr == nullptr || strIpAddr->Length == 0)
		strSessionID = g_SocketProLoader.HTTPEnter(m_HttpPeer->m_hSocket, ulGroups, (unsigned long)nLeaseTime, NULL);
	else
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars(strIpAddr);
		strSessionID = g_SocketProLoader.HTTPEnter(m_HttpPeer->m_hSocket, ulGroups, (unsigned long)nLeaseTime, wch);
	}
	return gcnew String(strSessionID);
}

bool CHttpPeerBase::CHttpPushImpl::HTTPSubscribe(String^ strSessionId, int nTimeout, String ^strCrossSiteJSCallback)
{
	if(strSessionId == nullptr)
		return false;
	pin_ptr<const wchar_t> session = PtrToStringChars(strSessionId);
	if(strCrossSiteJSCallback == nullptr)
		strCrossSiteJSCallback = gcnew String(L"");
	pin_ptr<const wchar_t> cb = PtrToStringChars(strCrossSiteJSCallback);
	bool bSuc = g_SocketProLoader.HTTPSubscribe(m_HttpPeer->m_hSocket, session, (unsigned long)nTimeout, cb);
	return bSuc;
}

bool CHttpPeerBase::CHttpPushImpl::Exit(String^ strSessionId)
{
	if(strSessionId == nullptr)
		return false;
	pin_ptr<const wchar_t> wch = PtrToStringChars(strSessionId);
	return g_SocketProLoader.HTTPExit(m_HttpPeer->m_hSocket, wch);
}

int CHttpPeerBase::SendResult(short sRequestID, String ^strData)
{
	if(strData == nullptr || strData->Length == 0)
		return SendResult(sRequestID);
	System::Text::Encoding ^encoding = gcnew System::Text::UTF8Encoding();
	array<BYTE> ^bytes = encoding->GetBytes(strData);
	return SendReturnData(sRequestID, bytes, bytes->Length);
}

bool CHttpPeerBase::CHttpPushImpl::SendUserMessage(String^ strSessionId, String^ strUserID, Object ^msg)
{
	if(strSessionId == nullptr || strUserID == nullptr || msg == nullptr)
		return false;
	pin_ptr<const wchar_t> strSession = PtrToStringChars(strSessionId);
	pin_ptr<const wchar_t> strUID = PtrToStringChars(strUserID);
	CComVariant vtMsg;
	ConvertFromObjectToVariant(msg, &vtMsg);
	return g_SocketProLoader.HTTPSendUserMessage(m_HttpPeer->m_hSocket, strSession, strUID, &vtMsg);
}

bool CHttpPeerBase::CHttpPushImpl::Speak(String^ strSessionId, Object ^msg, array<long>^ GroupIds)
{
	if(strSessionId == nullptr || GroupIds == nullptr || GroupIds->Length == 0)
		return false;
	pin_ptr<const wchar_t> strSession = PtrToStringChars(strSessionId);
	CComVariant vtMsg;
	ConvertFromObjectToVariant(msg, &vtMsg);
	if(g_SocketProLoader.XHTTPSpeak)
	{
		CComVariant vtGroups;
		ConvertFromObjectToVariant(GroupIds, &vtGroups);
		return g_SocketProLoader.XHTTPSpeak(m_HttpPeer->m_hSocket, strSession, &vtMsg, &vtGroups);
	}
	unsigned long ulGroups = 0;
	for each(long group in GroupIds)
	{
		ulGroups |= group;
	}
	return g_SocketProLoader.HTTPSpeak(m_HttpPeer->m_hSocket, strSession, &vtMsg, ulGroups);
}

void CHttpPeerBase::SetResponseCode(int nHttpErrorCode)
{
	if(g_SocketProLoader.SetHTTPResponseCode != NULL)
		g_SocketProLoader.SetHTTPResponseCode(m_hSocket, nHttpErrorCode);
}

void CHttpPeerBase::OnFastRequestArrive(short usRequestID, int nLen)
{
	bool bOk = false;
	switch(usRequestID)
	{
	case idHeader:
		do
		{
			char *str; 
			char cEnd = 0;
			m_UQueue->GetInternalUQueue()->Push((unsigned char*)&cEnd, 1);
			str = (char*)m_UQueue->GetInternalUQueue()->GetBuffer();
			int nPos;
			if(memcmp(str, "GET ", 4) == 0)
			{
				m_HttpRequest = hrGet;
				nPos = 4;
				m_Params = nullptr;
			}
			else if(memcmp(str, "POST ", 5) == 0)
			{
				m_HttpRequest = hrPost;
				nPos = 5;
				m_Params = nullptr;
			}
			else if(memcmp(str, "HEAD ", 5) == 0)
			{
				m_HttpRequest = hrHead;
				nPos = 5;
				m_Params = nullptr;
			}
			else if(memcmp(str, "PUT ", 4) == 0)
			{
				m_HttpRequest = hrPut;
				nPos = 4;
				m_Params = nullptr;
			}
			else if(memcmp(str, "DELETE ", 7) == 0)
			{
				m_HttpRequest = hrDelete;
				nPos = 7;
				m_Params = nullptr;
			}
			else if(memcmp(str, "OPTIONS ", 8) == 0)
			{
				m_HttpRequest = hrOptions;
				nPos = 8;
				m_Params = nullptr;
			}
			else if(memcmp(str, "TRACE ", 6) == 0)
			{
				m_HttpRequest = hrTrace;
				nPos = 6;
				m_Params = nullptr;
			}	
			else if(memcmp(str, "CONNECT ", 8) == 0)
			{
				m_HttpRequest = hrConnect;
				nPos = 8;
				m_Params = nullptr;
			}	
			else
			{
				m_HttpRequest = hrPost; //multi part header
				nPos = 0;
			}
			m_Headers = nullptr;
			const char *strHTTP = ::strstr(str, " HTTP/");
			if(strHTTP)
			{
				m_qQueue->SetSize(0);
				m_qQueue->Push((unsigned char*)(str + nPos), (unsigned long)(strHTTP - (str + nPos)));
				m_qHeader->SetSize(0);
				if(m_qHeader->GetMaxSize() < m_qQueue->GetSize())
					m_qHeader->ReallocBuffer(m_qQueue->GetSize());

				const char *strTemp = (const char*)m_qQueue->GetBuffer();
				if(::strstr(strTemp, "%") != NULL)
				{
					unsigned long ulGet = UriDecode(m_qQueue->GetBuffer(), m_qQueue->GetSize(), (unsigned char*)m_qHeader->GetBuffer());
					m_qQueue->SetSize(0);
					m_qQueue->Push(m_qHeader->GetBuffer(), ulGet);
				}
				m_qQueue->Push((unsigned char*)&cEnd, 1); //null terminated
				
				const char *strHeader = ::strstr(str, "\r\n");
				if(!strHeader)
					break;
				m_dVersion = atof(strHTTP + 6);
				m_qHeader->SetSize(0);
				m_qHeader->Push(strHeader + 2);
				m_qHeader->Push((unsigned char*)&cEnd, 1); //null terminated
			}
			else
			{
				m_qHeader->SetSize(0);
				m_qHeader->Push(m_UQueue->GetInternalUQueue()->GetBuffer(), m_UQueue->GetSize());
				m_qHeader->Push((unsigned char*)&cEnd, 1); //null terminated
				m_UQueue->SetSize(0);
			}
			bOk = true;
		}while(false);
		if(!bOk)
		{
			SetResponseCode(400); //400 Bad Request
			SetResponseHeader("Connection", "close");
			SendReturnData(usRequestID, nullptr, 0);
		}
		break;
	default: 
		break;
	}
	m_UQueue->SetSize(0);
}

void CHttpPeerBase::OnSwitchFrom(int ServiceID)
{
	m_dVersion = 0.0;
	m_HttpRequest = hrUnknown;
	m_qHeader->SetSize(0);
	m_qQueue->SetSize(0);
	m_Params = nullptr;
	m_Headers = nullptr;
}

const char HEX2DEC[256] = 
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

	/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

	/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

unsigned long  CHttpPeerBase::UriDecode(const unsigned char *strIn, unsigned long nLenIn, unsigned char *strOut)
{
	const unsigned char *pSrc = strIn;
	unsigned long SRC_LEN = nLenIn;
	const unsigned char *SRC_END = pSrc + SRC_LEN;
	const unsigned char *SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

	unsigned char *pStart = strOut;
	unsigned char *pEnd = pStart;
	unsigned long ulGet = 0;

	while (pSrc < SRC_LAST_DEC)
	{
		++ulGet;
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
				&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}
		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
	{
		++ulGet;
		*pEnd++ = *pSrc++;
	}
	return ulGet;
}

CHttpPeerBase::CHttpPeerBase()
{
	m_qQueue = new CInternalUQueue;
	m_qHeader = new CInternalUQueue;	
	m_HttpPush = gcnew CHttpPushImpl();
	m_HttpPush->m_HttpPeer = this;
}

CHttpPeerBase::~CHttpPeerBase()
{
	delete m_qQueue;
	delete m_qHeader;
}

}
#endif
}
