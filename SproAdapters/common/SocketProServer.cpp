#include "StdAfx.h"


namespace SocketProAdapter
{
#ifndef _WIN32_WCE
namespace ServerSide
{


CSocketProServer::CSocketProServer(void) 
	: m_ulEvents(0), m_Thread(nullptr), m_nPort(0), m_unMaxBacklog(64)
{
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	Init();
	g_SocketProLoader.InitSocketProServer(0);
}

CSocketProServer::CSocketProServer(int nParam)
	: m_ulEvents(0), m_Thread(nullptr), m_nPort(0), m_unMaxBacklog(64)
{
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	Init();
	g_SocketProLoader.InitSocketProServer(nParam);
}

void CSocketProServer::Init()
{
	if(!g_SocketProLoader.IsLoaded())
	{
		if(!g_SocketProLoader.LoadSocketProServer())
		{
#ifdef _WIN64
			throw gcnew System::InvalidProgramException("SocketPro core server library usktpror.dll (x64) is not available!");
#else
			throw gcnew System::InvalidProgramException("SocketPro core server library usktpror.dll (x32) is not available!");
#endif
		}
	}
	if(m_pSocketProServer == nullptr)
	{
		m_pSocketProServer = this;
	}
	else
	{
		throw gcnew Exception("SocketPro supports one instance of CSocketProServer only!");
	}

	m_OnAccept = gcnew DOnAccept(this, &CSocketProServer::OnAccept);
	m_OnChatRequestComing = gcnew DOnChatRequestComing(this, &CSocketProServer::OnCRComing);
	m_OnChatRequestCame = gcnew DOnChatRequestCame(this, &CSocketProServer::OnCRCame);
	m_OnClose = gcnew DOnClose(this, &CSocketProServer::OnClose);
	m_OnIsPermitted = gcnew DOnIsPermitted(this, &CSocketProServer::OnIsPermitted);
	m_OnSwitchTo = gcnew DOnSwitchTo(this, &CSocketProServer::OnSwitchTo);
	m_OnSend = gcnew DOnSend(this, &CSocketProServer::OnSend);
	m_OnSSLEvent = gcnew DOnSSLEvent(this, &CSocketProServer::OnSSLEvent);
	m_OnWinMessage = gcnew DOnWinMessage(this, &CSocketProServer::OnWinMessage);
}

void CSocketProServer::RemoveDelegates()
{
	if(m_pOnWinMessage != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnWinMessage(NULL);
	}

	if(m_pOnSSLEvent != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnSSLEvent(NULL);
	}

	if(m_pOnSend != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnSend(NULL);
	}

	if(m_pOnSwitchTo != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnSwitchTo(NULL);
	}

	if(m_pOnIsPermitted != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnIsPermitted(NULL);
	}

	if(m_pOnClose != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnClose(NULL);
	}

	if(m_pOnChatRequestCame != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnChatRequestCame(NULL);
	}

	if(m_pOnChatRequestComing != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnChatRequestComing(NULL);
	}

	if(m_pOnAccept != IntPtr::Zero)
	{
		g_SocketProLoader.SetOnAccept(NULL);
	}

	m_pOnWinMessage = IntPtr::Zero;
	m_pOnSSLEvent = IntPtr::Zero;
	m_pOnSend = IntPtr::Zero;
	m_pOnSwitchTo = IntPtr::Zero;
	m_pOnIsPermitted = IntPtr::Zero;
	m_pOnClose = IntPtr::Zero;
	m_pOnChatRequestCame = IntPtr::Zero;
	m_pOnChatRequestComing = IntPtr::Zero;
	m_pOnAccept = IntPtr::Zero;
}

CSocketProServer::~CSocketProServer(void)
{
	::Sleep(250);
	m_OnWinMessage = nullptr;
	m_OnSSLEvent = nullptr;
	m_OnSend = nullptr;
	m_OnSwitchTo = nullptr;
	m_OnIsPermitted = nullptr;
	m_OnClose = nullptr;
	m_OnChatRequestCame = nullptr;
	m_OnChatRequestComing = nullptr;
	m_OnAccept = nullptr;
	RemoveDelegates();
	StopSocketProServer();
	m_pSocketProServer = nullptr;
	g_SocketProLoader.UninitSocketProServer();
	if(m_hEvent != NULL)
		::CloseHandle(m_hEvent);
}

void CSocketProServer::OnChatRequestComing(int hSocketSource, USOCKETLib::tagChatRequestID sRequestID, Object ^Param0, Object ^Param1)
{

}

void CSocketProServer::OnChatRequestCame(int hSocketSource, USOCKETLib::tagChatRequestID sRequestID)
{

}

void CSocketProServer::OnCRComing(int hSocketSource, short sRequestID, int nLen)
{
	CComVariant vtP0, vtP1;
	Object ^Param0, ^Param1;
	CClientPeer ^p = CBaseService::SeekClientPeerGlobally(hSocketSource);
	if(p != nullptr)
	{
		VARTYPE vt = (VT_ARRAY|VT_UI1);
		CScopeUQueue sq;
		if(sq.UQueue->GetInternalUQueue()->GetMaxSize() < (unsigned long)nLen)
			sq.UQueue->ReallocBuffer((unsigned long)nLen + 10);
		long nGet = p->RetrieveBuffer(sq.UQueue->GetBuffer(), nLen, true);
		ATLASSERT(nGet == nLen);
		sq.UQueue->SetSize(nGet);
		switch(sRequestID)
		{
		case idExit:
			break;
		case idEnter:
			sq.UQueue->GetInternalUQueue()->Pop(&vtP0.ulVal);
			vtP0.vt = VT_I4;
			break;
		case idXEnter:
			sq.UQueue->GetInternalUQueue()->PopVT(vtP0);
			::VariantChangeType(&vtP0, &vtP0, VARIANT_NOVALUEPROP, (VT_I4|VT_ARRAY));
			break;
		case idSpeak:
			sq.UQueue->GetInternalUQueue()->Pop(&vtP1.ulVal);
			vtP1.vt = VT_I4;
			sq.UQueue->GetInternalUQueue()->PopVT(vtP0);
			break;
		case idSpeakEx:
			sq.UQueue->GetInternalUQueue()->Pop(&vtP1.ulVal);
			vtP1.vt = VT_I4;
			nGet = sq.UQueue->GetSize();
			sq.UQueue->GetInternalUQueue()->Insert(&nGet);
			sq.UQueue->GetInternalUQueue()->Insert((BYTE*)&vt, sizeof(vt));
			sq.UQueue->GetInternalUQueue()->PopVT(vtP0);
			break;
		case idXSpeakEx:
			sq.UQueue->GetInternalUQueue()->Insert((BYTE*)&vt, sizeof(vt));
			sq.UQueue->GetInternalUQueue()->PopVT(vtP0);
			nGet = sq.UQueue->GetInternalUQueue()->GetSize()/sizeof(unsigned long);
			sq.UQueue->GetInternalUQueue()->Insert(&nGet);
			vt = (VT_ARRAY|VT_I4);
			sq.UQueue->GetInternalUQueue()->Insert((BYTE*)&vt, sizeof(vt));
			sq.UQueue->GetInternalUQueue()->PopVT(vtP1);
			break;
		case idXSpeak:
			sq.UQueue->GetInternalUQueue()->PopVT(vtP1);
			sq.UQueue->GetInternalUQueue()->PopVT(vtP0);
			::VariantChangeType(&vtP0, &vtP0, VARIANT_NOVALUEPROP, (VT_I4|VT_ARRAY));
			break;
		case idSpeakTo:
			{
				unsigned int nPort;
				wchar_t strBuffer[41] = {0};
				wchar_t strIpAddr[21] = {0};
				sq.UQueue->Discard(8);
				g_SocketProLoader.GetPeerName(p->Socket, &nPort, strIpAddr, 20);
				swprintf(strBuffer, L"%s:%d", strIpAddr, nPort);
				vtP0.vt = VT_BSTR;
				vtP0.bstrVal = CComBSTR(strBuffer).Detach();
			}
			sq.UQueue->GetInternalUQueue()->PopVT(vtP1);
			break;
		case idSpeakToEx:
			{
				unsigned int nPort;
				wchar_t strBuffer[41] = {0};
				wchar_t strIpAddr[21] = {0};
				sq.UQueue->Discard(8);
				g_SocketProLoader.GetPeerName(p->Socket, &nPort, strIpAddr, 20);
				swprintf(strBuffer, L"%s:%d", strIpAddr, nPort);
				vtP0.vt = VT_BSTR;
				vtP0.bstrVal = CComBSTR(strBuffer).Detach();
			}
			nGet = sq.UQueue->GetSize();
			sq.UQueue->GetInternalUQueue()->Insert(&nGet);
			sq.UQueue->GetInternalUQueue()->Insert((BYTE*)&vt, sizeof(vt));
			sq.UQueue->GetInternalUQueue()->PopVT(vtP1);
			break;
		case idSendUserMessage:
			{
				CComBSTR bstrUserId;
				*(sq.UQueue->GetInternalUQueue()) >> bstrUserId;
				vtP0.vt = VT_BSTR;
				vtP0.bstrVal = bstrUserId.Detach();
			}
			sq.UQueue->GetInternalUQueue()->PopVT(vtP1);
			break;
		case idSendUserMessageEx:
			{
				CComBSTR bstrUserId;
				*(sq.UQueue->GetInternalUQueue()) >> bstrUserId;
				vtP0.vt = VT_BSTR;
				vtP0.bstrVal = bstrUserId.Detach();
			}
			nGet = sq.UQueue->GetSize();
			sq.UQueue->GetInternalUQueue()->Insert(&nGet);
			sq.UQueue->GetInternalUQueue()->Insert((BYTE*)&vt, sizeof(vt));
			sq.UQueue->GetInternalUQueue()->PopVT(vtP1);
			break;
		case idGetAllClients:
			break;
		case idGetAllGroups:
			break;
		case idGetAllListeners:
			if(sq.UQueue->GetSize() == sizeof(unsigned long))
			{
				sq.UQueue->GetInternalUQueue()->Pop(&vtP0.ulVal);
				vtP0.vt = VT_I4;
			}
			else
			{
				sq.UQueue->GetInternalUQueue()->PopVT(vtP0);
				::VariantChangeType(&vtP0, &vtP0, VARIANT_NOVALUEPROP, (VT_I4|VT_ARRAY));
			}
			break;
		default:
			break;
		}
		ATLASSERT(sq.UQueue->GetSize() == 0);
		Param0 = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtP0));
		Param1 = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtP1));
		p->OnCRComing((USOCKETLib::tagChatRequestID)sRequestID, Param0, Param1);
	}
	OnChatRequestComing(hSocketSource, (USOCKETLib::tagChatRequestID)sRequestID, Param0, Param1);
}

void CSocketProServer::OnCRCame(int hSocketSource, short sRequestID, int nLen)
{
	CClientPeer ^p = CBaseService::SeekClientPeerGlobally(hSocketSource);
	if(p != nullptr)
		p->OnCRCame((USOCKETLib::tagChatRequestID)sRequestID);
	OnChatRequestCame(hSocketSource, (USOCKETLib::tagChatRequestID)sRequestID);
}

//track all of window messages by overriding this function
bool CSocketProServer::OnWinMessage(int hWnd, int nMessage, int wParam, int lParam, int dwTime, int nPointX, int nPointY)
{
	return false;	
}

void CSocketProServer::OnWinMessage(int hWnd, int nMessage, int wParam, int lParam)
{

}

void CSocketProServer::OnAccept(int hSocket, int nError)
{

}

void CSocketProServer::OnClose(int hSocket, int nError)
{

}

void CSocketProServer::OnSend(int hSocket, int nError)
{

}

void CSocketProServer::OnSwitchTo(int hSocket, int nPrevSvsID, int nCurrSvsID)
{

}

bool CSocketProServer::OnIsPermitted(int hSocket, int nSvsID)
{
	return true;
}

void CSocketProServer::OnSSLEvent(int hSocket, int nWhere, int nRtn)
{

}

bool CSocketProServer::PushManager::AddAChatGroup(int nGroupID, String ^strDescription)
{
	pin_ptr<const wchar_t> wch;
	if(strDescription != nullptr)
		wch = PtrToStringChars(strDescription);
	else
		wch = nullptr;
	return g_SocketProLoader.AddAChatGroup(nGroupID, wch);
}

void CSocketProServer::AskForEvents(int nEvents)
{
	if(m_pOnAccept == IntPtr::Zero)
	{
		m_pOnAccept = Marshal::GetFunctionPointerForDelegate(m_OnAccept);
		g_SocketProLoader.SetOnAccept((POnAccept)m_pOnAccept.ToPointer());
	}

	if(m_pOnClose == IntPtr::Zero)
	{
		m_pOnClose = Marshal::GetFunctionPointerForDelegate(m_OnClose);
		g_SocketProLoader.SetOnClose((POnClose)m_pOnClose.ToPointer());
	}

	if(m_pOnIsPermitted == IntPtr::Zero)
	{
		m_pOnIsPermitted = Marshal::GetFunctionPointerForDelegate(m_OnIsPermitted);
		g_SocketProLoader.SetOnIsPermitted((POnIsPermitted)m_pOnIsPermitted.ToPointer());
	}

	if((nEvents & (int)tagEvent::eOnSend))
	{
		m_pOnSend = Marshal::GetFunctionPointerForDelegate(m_OnSend);
		g_SocketProLoader.SetOnSend((POnSend)m_pOnSend.ToPointer());
	}
	else
	{
		g_SocketProLoader.SetOnSend(NULL);
	}

	if((nEvents & (int)tagEvent::eOnChatRequestCame))
	{
		m_pOnChatRequestCame = Marshal::GetFunctionPointerForDelegate(m_OnChatRequestCame);
		g_SocketProLoader.SetOnChatRequestCame((POnChatRequestCame)m_pOnChatRequestCame.ToPointer());
	}
	else
	{
		g_SocketProLoader.SetOnChatRequestCame(NULL);
	}

	if((nEvents & (int)tagEvent::eOnChatRequestComing))
	{
		m_pOnChatRequestComing = Marshal::GetFunctionPointerForDelegate(m_OnChatRequestComing);
		g_SocketProLoader.SetOnChatRequestComing((POnChatRequestComing)m_pOnChatRequestComing.ToPointer());
	}
	else
	{
		g_SocketProLoader.SetOnChatRequestComing(NULL);
	}
	
	if(nEvents & (int)tagEvent::eOnSSLEvent)
	{
		m_pOnSSLEvent = Marshal::GetFunctionPointerForDelegate(m_OnSSLEvent);
		g_SocketProLoader.SetOnSSLEvent((POnSSLEvent)m_pOnSSLEvent.ToPointer());
	}
	else
	{
		g_SocketProLoader.SetOnSSLEvent(NULL);
	}
	
	if((nEvents & (int)tagEvent::eOnSwitchTo))
	{
		m_pOnSwitchTo = Marshal::GetFunctionPointerForDelegate(m_OnSwitchTo);
		g_SocketProLoader.SetOnSwitchTo((POnSwitchTo)m_pOnSwitchTo.ToPointer());
	}
	else
	{
		g_SocketProLoader.SetOnSwitchTo(NULL);
	}

	if((nEvents & (int)tagEvent::eOnWinMessage))
	{
		m_pOnWinMessage = Marshal::GetFunctionPointerForDelegate(m_OnWinMessage);
		g_SocketProLoader.SetOnWinMessage((POnWinMessage)m_pOnWinMessage.ToPointer());
	}
	else
	{
		g_SocketProLoader.SetOnWinMessage(NULL);
	}
}

void CSocketProServer::StartMessagePump()
{
	if(g_SocketProLoader.GetUseWindowMessagePump != NULL && g_SocketProLoader.StartIOPump != NULL 
		&& (!g_SocketProLoader.GetUseWindowMessagePump()))
	{
		::SetEvent(m_hEvent);
		g_SocketProLoader.StartIOPump(); //support IO completion port pump from SocketPro version 4.8.0.1
	}
	else
	{
		MSG msg;
		::PeekMessage(&msg, 0, 0, 0, 0); 
		::SetEvent(m_hEvent);
		while(::GetMessage(&msg, 0, 0, 0))
		{
			::TranslateMessage(&msg);
			if(OnWinMessage((int)msg.hwnd, msg.message, msg.wParam, msg.lParam, msg.time, msg.pt.x, msg.pt.y))
			{
				continue;
			}
			::DispatchMessage(&msg);
		}
	}
}

String^ CSocketProServer::GetPassword(int hSocket)
{
	WCHAR strPassword[256] = {0};
	ULONG ulLen = sizeof(strPassword)/sizeof(WCHAR);
	ulLen = g_SocketProLoader.GetPassword(hSocket, strPassword, ulLen);
	return gcnew String(strPassword);
}

String^ CSocketProServer::GetUserID(int hSocket)
{
	WCHAR strUserID[256] = {0};
	ULONG ulLen = sizeof(strUserID)/sizeof(WCHAR);
	ulLen = g_SocketProLoader.GetUID(hSocket, strUserID, ulLen);
	return gcnew String(strUserID);
}

bool CSocketProServer::SetPassword(int hSocket, String^ strPassword)
{
	if(strPassword == nullptr || strPassword->Length == 0)
		return false;
	if(g_SocketProLoader.SetPassword == NULL)
		return false;
	pin_ptr<const wchar_t> wch = PtrToStringChars(strPassword);
	return g_SocketProLoader.SetPassword(hSocket, wch);
}

int CSocketProServer::FindClient(int hSocket)
{
	return g_SocketProLoader.FindClient(hSocket);
}

int CSocketProServer::GetClient(int nIndex)
{
	return g_SocketProLoader.GetClient(nIndex);
}

String^ CSocketProServer::PushManager::GetAChatGroupDiscription (int nGroupID)
{
	WCHAR str[2049] = {0};
	g_SocketProLoader.GetAChatGroup(nGroupID, str, sizeof(str)/sizeof(WCHAR));
	return gcnew String(str);
}

int CSocketProServer::PushManager::GetChatterSocket(int nGroupID, int nIndex)
{
	return g_SocketProLoader.GetChatterSocket(nGroupID, nIndex);
}

int CSocketProServer::PushManager::GetCountOfChatters(int nGroupID)
{
	return g_SocketProLoader.GetCountOfChatters(nGroupID);
}

bool CSocketProServer::PostQuit()
{
	CAutoLock al(&m_pCS->m_sec);
	if(m_pSocketProServer == nullptr)
		return true;
	if(m_pSocketProServer->m_Thread != nullptr)
	{
		if(!g_SocketProLoader.GetUseWindowMessagePump())
			//Quit IO completion port or window message pump
			g_SocketProLoader.PostQuitPump(g_SocketProLoader.GetMainThreadID()); 
		else
			::PostThreadMessage(g_SocketProLoader.GetMainThreadID(), WM_QUIT, 0, 0) ? true : false;
		m_pSocketProServer->m_Thread = nullptr;
		if(CBaseService::m_aService != nullptr)
			CBaseService::m_aService->Clear();
		m_pSocketProServer->RemoveDelegates();
	}
	return true;
}

int CSocketProServer::PushManager::GetGroupID(int nIndex)
{
	unsigned long ulIndex = (unsigned long)nIndex;
	if(g_SocketProLoader.XGetGroupID)
		return (int)g_SocketProLoader.XGetGroupID(ulIndex);
	if(ulIndex > 255)
		return 0;
	return (int)g_SocketProLoader.GetGroupID((BYTE)ulIndex);
}

//bool CSocketProServer::StartSocketProServer(int nPort)
//{
//	return StartSocketProServer(nPort, 64);
//}

array<String^>^ CSocketProServer::HttpPush::GetHTTPChatIds(int nGroupId)
{
	unsigned long n;
	List<String^> list;
	unsigned long nSize = g_SocketProLoader.GetCountOfHTTPChatters((unsigned long)nGroupId);
	for(n=0; n<nSize; n++)
	{
		const wchar_t *strChatId = g_SocketProLoader.GetHTTPChatId(nGroupId, n);
		if(strChatId != NULL)
		{
			list.Add(gcnew String(strChatId));
		}
	}
	return list.ToArray();
}

bool CSocketProServer::HttpPush::GetHTTPChatContext(String ^strChatId, String ^%strUserID, String ^%strIpAddr, int %nLeaseTime, array<long> ^%GroupIds, int %nTimeout, int %nCountOfMessages)
{
	if(strChatId != nullptr)
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars(strChatId);
		unsigned long lt = 0;
		unsigned long timeout = 0;
		unsigned long messages = 0;
		wchar_t *strUID = NULL;
		wchar_t *strIp = NULL;
		CComVariant vt;
		if(g_SocketProLoader.GetHTTPChatContext(wch, &strUID, &strIp, &lt, &vt, &timeout, &messages))
		{
			strUserID = gcnew String(strUID);
			strIpAddr = gcnew String(strIp);
			nLeaseTime = lt;
			CComVariant vtGroups;
			::VariantChangeType(&vtGroups, &vt, VARIANT_NOVALUEPROP, (VT_ARRAY|VT_I4));
			if(vtGroups.vt == (VT_ARRAY|VT_I4))
			{
				Object ^obj = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtGroups));
				GroupIds = (array<long>^)obj;
			}
			nTimeout = timeout;
			nCountOfMessages = messages;
			return true;
		}
	}
	strUserID = nullptr; 
	strIpAddr = nullptr; 
	GroupIds = nullptr;
	nLeaseTime = 0; 
	nTimeout = 0; 
	nCountOfMessages = 0;
	return false;
}

//bool CSocketProServer::StartSocketProServer(int nPort, int nMaxBlacklog)
//{
//	if(Events == 0)
//	{
//		AskForEvents((int)tagEvent::eOnClose + (int)tagEvent::eOnIsPermitted + (int)tagEvent::eOnAccept + (int)tagEvent::eOnChatRequestComing);
//	}
//	return g_SocketProLoader.StartSocketProServer(nPort, nMaxBlacklog);
//}

void CSocketProServer::ThreadProc()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	do
	{
		if(!g_SocketProLoader.StartSocketProServer(m_pSocketProServer->m_nPort, m_pSocketProServer->m_unMaxBacklog))
		{
			::SetEvent(m_pSocketProServer->m_hEvent);
			break;
		}
		m_pSocketProServer->StartMessagePump();
		g_SocketProLoader.StopSocketProServer();
		g_SocketProLoader.UninitSocketProServer();
		g_SocketProLoader.InitSocketProServer(0);
	}while(false);
	CoUninitialize();
}

bool CSocketProServer::Run(int nPort)
{
	return Run(nPort, 0, 64);
}

bool CSocketProServer::Run(int nPort, long lEvents)
{
	return Run(nPort, lEvents, 64);
}

bool CSocketProServer::Run(int nPort, long lEvents, int nMaxBacklog)
{
	bool b = true;
	{
		CAutoLock al(&m_pCS->m_sec);
		if(m_Thread != nullptr)
		{
			//bad operation because SocketPro server already started.
			ATLASSERT(false);
			return false;
		}
		m_nPort = (UINT)nPort;
		m_unMaxBacklog = (unsigned int)nMaxBacklog;
	}
	do
	{
		AskForEvents(lEvents|(int)tagEvent::eOnClose|(int)tagEvent::eOnIsPermitted|(int)tagEvent::eOnAccept|(int)tagEvent::eOnChatRequestComing);
		b = m_pSocketProServer->OnSettingServer();
		if(!b)
			break;
		m_Thread = gcnew Thread(gcnew ThreadStart(ThreadProc));
		b = (m_Thread != nullptr);
		if(!b)
			break;
		m_Thread->Start();
		::WaitForSingleObject(m_hEvent, INFINITE);

		//delay 150 ms so that pump is fully started for safety
		b = (!m_Thread->Join(150));
		if(!b)
			m_Thread = nullptr;
	}while(false);
	return b;
}

void CSocketProServer::StopSocketProServer()
{
	CAutoLock al(&m_pCS->m_sec);
	if(m_pSocketProServer == nullptr)
		return;
	if(m_pSocketProServer->m_Thread != nullptr && ::GetCurrentThreadId() != g_SocketProLoader.GetMainThreadID())
	{
		if(!g_SocketProLoader.GetUseWindowMessagePump())
			//Quit IO completion port or window message pump
			g_SocketProLoader.PostQuitPump(g_SocketProLoader.GetMainThreadID()); 
		else
			::PostThreadMessage(g_SocketProLoader.GetMainThreadID(), WM_QUIT, 0, 0) ? true : false;
		m_Thread->Join();
		m_Thread = nullptr;
		if(CBaseService::m_aService != nullptr)
			CBaseService::m_aService->Clear();
		RemoveDelegates();
	}
}

void CSocketProServer::UseSSL(USOCKETLib::tagEncryptionMethod EncryptionMethod, String ^strSubject, bool bMachine, bool bRoot)
{
	pin_ptr<const wchar_t> wchSubject = nullptr;
	if(strSubject != nullptr)
		wchSubject = PtrToStringChars(strSubject);
	g_SocketProLoader.UseMSCert(bMachine, bRoot, wchSubject);
	g_SocketProLoader.SetDefaultEncryptionMethod((tagEncryptionMethod)EncryptionMethod);
}

void CSocketProServer::UseSSL(String ^strPfxFile, String ^strPassword, String ^strSubject, USOCKETLib::tagEncryptionMethod EncryptionMethod)
{
	pin_ptr<const wchar_t> wchSubject = nullptr;
	if(strSubject != nullptr)
		wchSubject = PtrToStringChars(strSubject);
	g_SocketProLoader.UseMSCert(false, true, wchSubject);
	pin_ptr<const wchar_t> wchPfxFile = nullptr;
	if(strPfxFile != nullptr)
		wchPfxFile = PtrToStringChars(strPfxFile);
	pin_ptr<const wchar_t> wchPassword = nullptr;
	if(strPassword != nullptr)
		wchPassword = PtrToStringChars(strPassword);
	g_SocketProLoader.SetPfxFile(wchPfxFile, wchPassword);
	g_SocketProLoader.SetDefaultEncryptionMethod((tagEncryptionMethod)EncryptionMethod);
}

void CSocketProServer::UseSSL(USOCKETLib::tagEncryptionMethod EncryptionMethod, String^ strCertFile, String^ strPrivateKeyFile)
{
	pin_ptr<const wchar_t> wchCertFile;
	pin_ptr<const wchar_t> wchPrivateKeyFile;
	if(strCertFile != nullptr)
		wchCertFile = PtrToStringChars(strCertFile);
	else
		wchCertFile = nullptr;

	if(strPrivateKeyFile != nullptr)
		wchPrivateKeyFile = PtrToStringChars(strPrivateKeyFile);
	else
		wchPrivateKeyFile = nullptr;
	g_SocketProLoader.SetCertFile(wchCertFile);
	g_SocketProLoader.SetPrivateKeyFile(wchPrivateKeyFile);
	g_SocketProLoader.SetDefaultEncryptionMethod((tagEncryptionMethod)EncryptionMethod);
}


} //namespace ServerSide
#endif
} //namespace SocketProAdapter
