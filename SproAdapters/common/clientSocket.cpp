#include "StdAfx.h"

//#include "clientsocket.h"

namespace SocketProAdapter
{
namespace ClientSide
{

	CClientSocket::CClientSocket(): m_ulBatchBalance(0)
	{
		m_pPush = gcnew CUPushClientImpl();
		m_pPush->m_ClientSocket = this;
		m_pIUChat = NULL;
		m_pIUFast = NULL;
		m_pIUSocket = NULL;
		m_pIJobContext = nullptr;
		m_pIJobManager = nullptr;
		m_pIProcess = nullptr;

		if(IsWeb())
		{
			m_pCSEvent = NULL;
		}
		else
		{
			m_pCSEvent = new CCSEvent();
			m_pCSEvent->m_pClientSocket = this;
		}

		m_lstAsynHandler = gcnew List<CAsyncServiceHandler^>();
		Init();
		m_pCS = new CComAutoCriticalSection();
	}

	CClientSocket::CClientSocket(bool bCreateOne): m_ulBatchBalance(0)
	{
		m_pPush = gcnew CUPushClientImpl();
		m_pPush->m_ClientSocket = this;
		m_pIUChat = NULL;
		m_pIUFast = NULL;
		m_pIUSocket = NULL;
		m_pIJobContext = nullptr;
		m_pIJobManager = nullptr;
		m_pIProcess = nullptr;
		
		if(IsWeb())
		{
			m_pCSEvent = NULL;
		}
		else
		{
			m_pCSEvent = new CCSEvent();
			m_pCSEvent->m_pClientSocket = this;
		}

		m_lstAsynHandler = gcnew List<CAsyncServiceHandler^>();
		if(bCreateOne)
		{
			Init();
		}
		else
		{
			m_nCurSvsID = sidStartup;
		}
		m_pCS = new CComAutoCriticalSection();
	}

	//construct this class instance with a valid existing USocket object
	CClientSocket::CClientSocket(IntPtr pIUnknownToUSocket) : m_ulBatchBalance(0)
	{
		m_pPush = gcnew CUPushClientImpl();
		m_pPush->m_ClientSocket = this;
		m_pIUChat = NULL;
		m_pIUFast = NULL;
		m_pIUSocket = NULL;
		m_pIJobContext = nullptr;
		m_pIJobManager = nullptr;
		m_pIProcess = nullptr;
		
		if(IsWeb())
		{
			m_pCSEvent = NULL;
		}
		else
		{
			m_pCSEvent = new CCSEvent();
			m_pCSEvent->m_pClientSocket = this;
		}

		m_lstAsynHandler = gcnew List<CAsyncServiceHandler^>();
		InitEx(pIUnknownToUSocket);
		m_pCS = new CComAutoCriticalSection();
		if(m_pIUSocket != NULL)
		{
			Advise();
		}
	}

	CClientSocket::~CClientSocket()
	{
		DetachAll();

		//make sure that all of attached handlers are detached first
		Uninit();

		if(m_pIJobManager != nullptr && m_pIJobContext != nullptr && m_pIJobContext->JobStatus == tagJobStatus::jsCreating)
		{
			m_pIJobManager->DestroyJob(m_pIJobContext);
		}

		if(m_pCSEvent != NULL)
		{
			m_pCSEvent->m_pClientSocket = nullptr;
			delete m_pCSEvent;
			m_pCSEvent = NULL;
		}

		if(m_lstAsynHandler != nullptr)
		{
			m_lstAsynHandler->Clear();
			m_lstAsynHandler = nullptr;
		}

		if(m_pCS != NULL)
		{
			delete m_pCS;
			m_pCS = NULL;
		}
	}

	bool CClientSocket::CUPushClientImpl::Enter(array<int> ^Groups)
	{
		if(Groups == nullptr || Groups->Length == 0)
			return Exit();
		CComVariant vtGroups;
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Groups, IntPtr(&vtGroups));
		HRESULT hr = m_ClientSocket->m_pIUChat->XEnter(vtGroups);
		return (hr == S_OK);
	}

	bool CClientSocket::CUPushClientImpl::Broadcast(Object ^Message, array<int> ^Groups)
	{
		if(Groups == nullptr || Groups->Length == 0)
			return false;
		CComVariant vtGroups;
		CComVariant vtMsg;
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Groups, IntPtr(&vtGroups));
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Message, IntPtr(&vtMsg));
		HRESULT hr = m_ClientSocket->m_pIUChat->XSpeak(vtMsg, vtGroups);
		return (hr == S_OK);
	}

	bool CClientSocket::CUPushClientImpl::Broadcast(array<unsigned char> ^Message, array<int> ^Groups)
	{
		if(Groups == nullptr || Groups->Length == 0)
			return false;
		pin_ptr<int> pGroup = &Groups[0];
		pin_ptr<unsigned char> wch;
		unsigned long ulMessageSize = 0;
		if(Message != nullptr)
		{
			ulMessageSize = Message->Length;
			if(ulMessageSize)
				wch = &Message[0];
		}
		HRESULT hr = m_ClientSocket->m_pIUFast->XSpeakEx(ulMessageSize, wch, Groups->Length, (unsigned long*)pGroup);
		return (hr == S_OK);
	}

	bool CClientSocket::CUPushClientImpl::SendUserMessage(Object ^Message, String ^UserId)
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars(UserId);
		CComBSTR bstrUserId(wch);
		CComVariant vtMsg;
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(Message, IntPtr(&vtMsg));
		HRESULT hr = m_ClientSocket->m_pIUChat->SendUserMessage(bstrUserId, vtMsg);
		return( hr == S_OK);
	}

	bool CClientSocket::CUPushClientImpl::SendUserMessage(String ^UserId, array<unsigned char> ^Message)
	{
		pin_ptr<const wchar_t> uid = PtrToStringChars(UserId);
		CComBSTR bstrUserId(uid);
		pin_ptr<unsigned char> wch;
		unsigned long ulMessageSize = 0;
		if(Message != nullptr)
		{
			ulMessageSize = Message->Length;
			if(ulMessageSize)
				wch = &Message[0];
		}
		HRESULT hr = m_ClientSocket->m_pIUFast->SendUserMessageEx(bstrUserId, ulMessageSize, wch);
		return(hr == S_OK);
	}

	bool CClientSocket::CUPushClientImpl::Exit()
	{
		return (m_ClientSocket->m_pIUChat->Exit() == S_OK);
	}

	IntPtr CClientSocket::GetIUFast()
	{
		return IntPtr(m_pIUFast);
	}

	IntPtr CClientSocket::GetIUChat()
	{
		return IntPtr(m_pIUChat);
	}

	IntPtr CClientSocket::GetIUSocket()
	{
		return IntPtr(m_pIUSocket);
	}

	int CClientSocket::GetErrorCode()
	{
		long lRtn = S_OK;
		if(m_pIUSocket != NULL)
		{
			m_pIUSocket->get_Rtn(&lRtn);
		}
		return lRtn;
	}

	String^ CClientSocket::GetErrorMsg()
	{
		CComBSTR bstrErrorMsg;
		if(m_pIUSocket != NULL)
		{
			m_pIUSocket->get_ErrorMsg(&bstrErrorMsg);
		}
		return gcnew String(bstrErrorMsg.m_str);
	}

	int CClientSocket::GetCurrentServiceID()
	{
		return m_nCurSvsID;
	}

	void CClientSocket::SetUSocket(IntPtr pIUnknownToUSocket)
	{
		if(m_ulBatchBalance)
			throw gcnew System::InvalidOperationException("Make sure method call balance between BeginBatching and Commit/Rollback before calling the method SetUSocket!");
		Uninit();
		InitEx(pIUnknownToUSocket);
		if(m_pIUSocket != NULL)
		{
			Advise();
		}
	}

	USOCKETLib::USocketClass^ CClientSocket::GetUSocket()
	{
		if(m_USocketClass == nullptr && m_pIUSocket != NULL)
		{
			m_USocketClass = (USOCKETLib::USocketClass^)Marshal::GetTypedObjectForIUnknown(IntPtr(m_pIUSocket), USOCKETLib::USocketClass::typeid);
		}
		return m_USocketClass;
	}

	bool CClientSocket::IsConnected()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		long hSocket = 0;
		m_pIUSocket->get_Socket(&hSocket);
		return (hSocket != 0 && hSocket != -1);
	}
/*
	bool CClientSocket::SpeakEx(int nGroups, CUQueue ^UQueue)
	{
		if(UQueue == nullptr)
			return SpeakEx(nGroups, IntPtr::Zero, 0);
		return SpeakEx(nGroups, UQueue->GetBuffer(), UQueue->GetSize());
	}

	bool CClientSocket::SpeakEx(array<BYTE> ^MessageBuffer, int nGroups)
	{
		if(m_pIUFast == NULL)
			throw gcnew Exception("No IUFast interface available!");
		if(MessageBuffer == nullptr || MessageBuffer->Length == 0)
			return (m_pIUFast->SpeakEx(0, NULL, nGroups) == S_OK);
		pin_ptr<BYTE> pBuffer = &MessageBuffer[0];
		return (m_pIUFast->SpeakEx(MessageBuffer->Length, pBuffer, nGroups) == S_OK);
	}

	bool CClientSocket::SpeakEx(int nGroups, IntPtr pBuffer, int nLen)
	{
		if(m_pIUFast == NULL)
			throw gcnew Exception("No IUFast interface available!");
		return (m_pIUFast->SpeakEx(nLen, (BYTE*)pBuffer.ToPointer(), nGroups) == S_OK);
	}

	bool CClientSocket::SpeakToEx(String^ strIPAddr, int nPort, CUQueue ^UQueue)
	{
		return SpeakToEx(strIPAddr, nPort, UQueue->GetBuffer(), UQueue->GetSize());
	}

	bool CClientSocket::SpeakToEx(String^ strIPAddr, int nPort, array<BYTE> ^MessageBuffer)
	{
		if(m_pIUFast == NULL)
			throw gcnew Exception("No IUFast interface available!");
		if(strIPAddr == nullptr || strIPAddr->Length == 0)
			throw gcnew ArgumentException("Host address can't be empty!");
		if(nPort == 0)
			throw gcnew ArgumentException("Host port can't be zero!");
		pin_ptr<const wchar_t> wch = PtrToStringChars(strIPAddr);
		if(MessageBuffer == nullptr || MessageBuffer->Length == 0)
			return (m_pIUFast->SpeakToEx(CComBSTR(wch), nPort, 0, NULL) == S_OK);
		pin_ptr<BYTE> pBuffer = &MessageBuffer[0];
		return (m_pIUFast->SpeakToEx(CComBSTR(wch), nPort, MessageBuffer->Length, pBuffer) == S_OK);
	}

	bool CClientSocket::SpeakToEx(String^ strIPAddr, int nPort, IntPtr pBuffer, int nLen)
	{
		if(m_pIUFast == NULL)
			throw gcnew Exception("No IUFast interface available!");
		if(strIPAddr == nullptr || strIPAddr->Length == 0)
			throw gcnew ArgumentException("Host address can't be empty!");
		if(nPort == 0)
			throw gcnew ArgumentException("Host port can't be zero!");
		pin_ptr<const wchar_t> wch = PtrToStringChars(strIPAddr);
		return (m_pIUFast->SpeakToEx(CComBSTR(wch), nPort, nLen, (BYTE*)pBuffer.ToPointer()) == S_OK);
	}*/

	bool CClientSocket::IsWeb()
	{
		System::AppDomain ^app = System::AppDomain::CurrentDomain;
		array<System::Reflection::Assembly^> ^assemblies = app->GetAssemblies();
		for each (System::Reflection::Assembly ^ass in assemblies)
		{
			String ^strCodeBase = ass->CodeBase->ToLower();
			if(strCodeBase->Contains(".web.dll"))
				return true;
		}
		return false;
	}

	bool CClientSocket::BeginBatching()
	{
		{
			CAutoLock al(&m_pCS->m_sec);
			m_ulBatchBalance++;
			if(m_ulBatchBalance > 1)
				return true;
		}

		if(m_pIJobManager != nullptr)
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_pIJobContext == nullptr)
				return false;
			if(m_pIJobContext->JobStatus != tagJobStatus::jsCreating)
				return false;
			if(!IsBatchingBalanced(m_pIJobContext->Tasks))
				return false;
			return (m_pIJobContext->AddTask(idStartBatching, IntPtr::Zero, 0) != 0);
		}
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		return (m_pIUSocket->StartBatching() == S_OK);
	}

	bool CClientSocket::Commit()
	{
		return Commit(false);
	}

	bool CClientSocket::Commit(bool bBatchingAtServer)
	{
		{
			CAutoLock al(&m_pCS->m_sec);
			m_ulBatchBalance--;
			if(m_ulBatchBalance > 0)
			{
				//we return here because the methods BeginBatching and Commit/Rollback not balanced yet
				return true;
			}
		}

		if(m_pIJobManager != nullptr)
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_pIJobContext == nullptr)
				return false;
			if(m_pIJobContext->JobStatus != tagJobStatus::jsCreating)
				return false;
			if(!IsBatchingBalanced(m_pIJobContext->Tasks))
				return false;
			VARIANT_BOOL vb = bBatchingAtServer ? VARIANT_TRUE : VARIANT_FALSE;
			return (m_pIJobContext->AddTask(idCommitBatching, IntPtr(&vb), sizeof(vb)) != 0);
		}
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		return (m_pIUSocket->CommitBatching(bBatchingAtServer ? VARIANT_TRUE : VARIANT_FALSE) == S_OK);
	}

	bool CClientSocket::Rollback()
	{
		if(m_ulBatchBalance != 1)
			throw gcnew System::InvalidOperationException("Must call the method Rollback only after calling the method BeginBatching and Calling balance must be 1.");
		{
			CAutoLock al(&m_pCS->m_sec);
			for each(CAsyncServiceHandler ^p in m_lstAsynHandler)
			{
				p->RemoveAsyncHandlers((unsigned int)(~0));
			}
			m_ulBatchBalance--;
		}

		if(m_pIJobManager != nullptr)
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_pIJobContext == nullptr)
				return false;
			if(m_pIJobContext->JobStatus != tagJobStatus::jsCreating)
				return false;
			Dictionary<int, CTaskContext^> ^aTasks = m_pIJobContext->Tasks;
			if(IsBatchingBalanced(aTasks))
				return true;
			for each (int idTask in aTasks->Keys)
			{
				CTaskContext ^tc = aTasks[idTask];
				m_pIJobContext->RemoveTask(idTask);
				if(tc->m_sRequestId == idStartBatching)
					break;
			}
			return true;
		}

		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		return (m_pIUSocket->AbortBatching() == S_OK);
	}

	bool CClientSocket::IsBatching()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		VARIANT_BOOL bBatching = VARIANT_FALSE;
		m_pIUSocket->get_IsBatching(&bBatching);
		return (!(bBatching == VARIANT_FALSE));
	}

	int CClientSocket::GetBytesBatched()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		long lBytes = 0;
		m_pIUSocket->get_BytesBatched(&lBytes);
		return lBytes;
	}

	bool CClientSocket::Connect(String ^strHost, int nPort)
	{
		return Connect(strHost, nPort, false);
	}

	bool CClientSocket::Connect(String ^strHost, int nPort, bool bSyn)
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		if(strHost == nullptr || strHost->Length == 0)
			throw gcnew ArgumentException("Host address can't be empty!");
		if(nPort == 0)
			throw gcnew ArgumentException("Host port can't be zero!");
		pin_ptr<const wchar_t> wch = PtrToStringChars(strHost);
		Advise();
		return (m_pIUSocket->Connect(CComBSTR(wch), nPort, bSyn ? VARIANT_TRUE : VARIANT_FALSE) == S_OK);
		return false;
	}
	
	//abort a socket connection
	void CClientSocket::Disconnect()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		m_pIUSocket->Disconnect();
	}
	
	//gracefully close a socket connection
	void CClientSocket::Shutdown()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		m_pIUSocket->Shutdown();
	}

	bool CClientSocket::SwitchTo(int nSvsID, bool bAutoTransferServerException)
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		Advise();
		HRESULT hr;
		ULONG *pulBuffer;
		ULONG pulParameters[3];
		CComVariant vtParameters;
		hr = m_pIUSocket->get_ClientParams(&vtParameters);
		::SafeArrayAccessData(vtParameters.parray, (void**)&pulBuffer);
		memcpy(pulParameters, pulBuffer + 2, sizeof(pulParameters));
		::SafeArrayUnaccessData(vtParameters.parray);
		if(bAutoTransferServerException)
		{
			pulParameters[2] |= TRANSFER_SERVER_EXCEPTION;
		}
		else
		{
			pulParameters[2] &= (~TRANSFER_SERVER_EXCEPTION);
		}
		vtParameters.Clear();
		SAFEARRAYBOUND sab[1] = {3, 0};
		vtParameters.vt = (VT_ARRAY | VT_I4);
		vtParameters.parray = ::SafeArrayCreate(VT_I4, 1, sab);
		::SafeArrayAccessData(vtParameters.parray, (void**)&pulBuffer);
		memcpy(pulBuffer, pulParameters, sizeof(pulParameters));
		::SafeArrayUnaccessData(vtParameters.parray);
		hr = m_pIUSocket->put_ClientParams(vtParameters);
		
		hr = m_pIUSocket->SwitchTo(nSvsID);

		bool bOk = (hr == S_OK);
		
		//clean password right after calling IUSocket::SwitchTo
		//so that there is no possibility for other codes to peek a password
		hr = m_pIUSocket->put_Password(NULL); 
		return bOk;
	}

	//switch for a service
	bool CClientSocket::SwitchTo(int nSvsID)
	{
		return SwitchTo(nSvsID, false);
	}

	bool CClientSocket::SwitchTo(CAsyncServiceHandler ^pAsynHandler, bool bAutoTransferServerException)
	{
		if(pAsynHandler != nullptr)
		{
			return SwitchTo(pAsynHandler->GetSvsID(), bAutoTransferServerException);
		}
		return false;
	}

	bool CClientSocket::SwitchTo(CAsyncServiceHandler ^pAsynHandler)
	{
		return SwitchTo(pAsynHandler, false);
	}

	//freeze GUI
	void CClientSocket::DisableUI()
	{
		DisableUI(true);
	}

	void CClientSocket::DisableUI(bool bDisable)
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		m_pIUSocket->put_Frozen(bDisable ? VARIANT_TRUE : VARIANT_FALSE);
	}

	//if ulSvsID = 0, it waits a request for the current service id
	bool CClientSocket::Wait(short sRequestID)
	{
		return Wait(sRequestID, -1, 0);
	}

	bool CClientSocket::Wait(short sRequestID, int nTimeout)
	{
		return Wait(sRequestID, nTimeout, 0);
	}

	bool CClientSocket::Wait(short sRequestID, int nTimeout, int nSvsID)
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_ulBatchBalance != 0)
				throw gcnew System::InvalidOperationException("Bad operation. Make sure that methods BeginBatching and Commit/Rollback are balanced first!");
		}
		VARIANT_BOOL bTimeout = VARIANT_FALSE;
		if(nSvsID == 0)
			nSvsID = GetCurrentServiceID();
		HRESULT hr = m_pIUSocket->Wait(sRequestID, nTimeout, nSvsID, &bTimeout);
		bool bWait = (hr == S_OK && bTimeout == VARIANT_FALSE && IsConnected());
		return bWait;
	}

	bool CClientSocket::WaitAll()
	{
		return WaitAll(-1);
	}

	bool CClientSocket::WaitAll(int nTimeout)
	{
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_ulBatchBalance != 0)
				throw gcnew System::InvalidOperationException("Bad operation. Make sure that methods BeginBatching and Commit/Rollback are balanced first!");
		}
		VARIANT_BOOL bTimeout = VARIANT_FALSE;
		HRESULT hr = m_pIUSocket->WaitAll(nTimeout, &bTimeout);
		bool bWait = (hr == S_OK && bTimeout == VARIANT_FALSE && IsConnected());
		return bWait;
	}
	
	//client credentials
	void CClientSocket::SetUID(String ^strUID)
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		if(strUID != nullptr)
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars(strUID);
			m_pIUSocket->put_UserID(CComBSTR(wch));
		}
		else
		{
			m_pIUSocket->put_UserID(NULL);
		}
	}

	void CClientSocket::SetPassword(String ^strPassword)
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");

		if(strPassword != nullptr)
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars(strPassword);
			m_pIUSocket->put_Password(CComBSTR(wch));
		}
		else
		{
			m_pIUSocket->put_Password(NULL);
		}
	}

	int CClientSocket::GetCountOfRequestsInQueue()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		long lCount = 0;
		m_pIUSocket->get_CountOfRequestsInQueue(&lCount);
		return lCount;
	}
			
	int CClientSocket::GetCountOfAttachedServiceHandlers()
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		return m_lstAsynHandler->Count;
	}

	List<CAsyncServiceHandler^>^ CClientSocket::GetRequestAsynHandlers()
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		return m_lstAsynHandler;
	}

	void CClientSocket::SetUSocket(USOCKETLib::USocketClass ^USocket)
	{
		if(m_ulBatchBalance)
			throw gcnew System::InvalidOperationException("Make sure method call balance between BeginBatching and Commit/Rollback before calling the method SetUSocket!");
		if(USocket == nullptr)
		{
			SetUSocket(IntPtr::Zero);
		}
		IntPtr pIUnknown = System::Runtime::InteropServices::Marshal::GetIUnknownForObject(USocket);
		if(pIUnknown != IntPtr::Zero)
		{
			SetUSocket(pIUnknown);
			System::Runtime::InteropServices::Marshal::Release(pIUnknown);
		}
	}
	
	void CClientSocket::OnRequestProcessed(int hSocket, short nRequestID, int lLen, int lLenInBuffer, USOCKETLib::tagReturnFlag sFlag)
	{
		if(sFlag == USOCKETLib::tagReturnFlag::rfCompleted)
		{
			if(nRequestID == idSwitchTo)
			{
				long lSvsID = 0;
				m_pIUSocket->get_CurrentSvsID(&lSvsID);
				m_nCurSvsID = lSvsID;

				ULONG *pulParameters;
				CComVariant vtServerParameters;
				HRESULT hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
				::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
				m_bServerException = ((pulParameters[4] & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
				::SafeArrayUnaccessData(vtServerParameters.parray);
			}
			
			if(nRequestID >= 0 && nRequestID <= SOCKETPRO_MAX_BASE_REQUEST_ID) 
			{
				if(m_OnBaseRequestProcessed != nullptr)
				{
					m_OnBaseRequestProcessed->Invoke(nRequestID);
				}
				if(m_OnRequestProcessed != nullptr)
				{
					m_OnRequestProcessed->Invoke(hSocket, nRequestID, 0, 0, sFlag);
				}
				return;
			}
			
			CAsyncServiceHandler ^p;
			CAutoLock AutoLock(&m_pCS->m_sec);
			p = Lookup(m_nCurSvsID);
			if(p != nullptr)
			{
				ATLASSERT(lLen == lLenInBuffer);
				p->OnRR(hSocket, nRequestID, lLenInBuffer, sFlag);
			}
			else
			{
				if(m_OnRequestProcessed != nullptr)
					m_OnRequestProcessed->Invoke(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
			}
		}
		else
		{
			if(m_OnRequestProcessed != nullptr)
				m_OnRequestProcessed->Invoke(hSocket, nRequestID, lLen, lLenInBuffer, sFlag);
		}
	}

	void CClientSocket::InitEx(IntPtr pIUnknownToUSocket)
	{
		IUnknown *pIUnknown = (IUnknown *)pIUnknownToUSocket.ToPointer();
		if(pIUnknown != NULL)
		{
			IUSocket *pIUSocket = NULL;
			HRESULT hr = pIUnknown->QueryInterface(__uuidof(IUSocket), (void**)&pIUSocket);
			m_pIUSocket = pIUSocket;

			//an interface to a valid USocket object expected
			ATLASSERT(hr == S_OK && m_pIUSocket != NULL);
			if(hr == S_OK)
			{
				ULONG *pulParameters;
				CComVariant vtServerParameters;
				hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
				::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
				m_bServerException = ((pulParameters[4] & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
				::SafeArrayUnaccessData(vtServerParameters.parray);

#ifdef _DEBUG
				//1000 second. 
				//If too short, client will automatically disconnect socket connection when you debug at server side.
				hr = m_pIUSocket->put_RecvTimeout(1000000);
#endif
				IUFast *pIUFast = NULL;

				hr = m_pIUSocket->QueryInterface(__uuidof(IUFast), (void**)&pIUFast);
				m_pIUFast = pIUFast;

				IUChat		*pIUChat = NULL;
				hr = m_pIUSocket->QueryInterface(__uuidof(IUChat), (void**)&pIUChat);
				m_pIUChat = pIUChat;
				ATLASSERT(m_pIUChat != NULL);
				
				long lSvsID;
				m_pIUSocket->get_CurrentSvsID(&lSvsID);
				m_nCurSvsID = lSvsID;

				Advise();
			}
		}
	}

	void CClientSocket::Disadvise()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		if(m_bAdvised)
		{
			if(m_pCSEvent)
			{
				m_pCSEvent->DispEventUnadvise(m_pIUSocket);
			}
			else
			{
				USOCKETLib::USocketClass ^usc = GetUSocket();
				usc->OnClosing -= gcnew USOCKETLib::_IUSocketEvent_OnClosingEventHandler(this, &CClientSocket::OnClosingEventHandler);
				usc->OnConnecting -= gcnew USOCKETLib::_IUSocketEvent_OnConnectingEventHandler(this, &CClientSocket::OnConnectingEventHandler);
				usc->OnDataAvailable -= gcnew USOCKETLib::_IUSocketEvent_OnDataAvailableEventHandler(this, &CClientSocket::OnDataAvailableEventHandler);
				usc->OnGetHostByAddr -= gcnew USOCKETLib::_IUSocketEvent_OnGetHostByAddrEventHandler(this, &CClientSocket::OnGetHostByAddrEventHandler);
				usc->OnGetHostByName -= gcnew USOCKETLib::_IUSocketEvent_OnGetHostByNameEventHandler(this, &CClientSocket::OnGetHostByNameEventHandler);
				usc->OnOtherMessage -= gcnew USOCKETLib::_IUSocketEvent_OnOtherMessageEventHandler(this, &CClientSocket::OnOtherMessageEventHandler);
				usc->OnRequestProcessed -= gcnew USOCKETLib::_IUSocketEvent_OnRequestProcessedEventHandler(this, &CClientSocket::OnRequestProcessedEventHandler);
				usc->OnSendingData -= gcnew USOCKETLib::_IUSocketEvent_OnSendingDataEventHandler(this, &CClientSocket::OnSendingDataEventHandler);
				usc->OnSocketClosed -= gcnew USOCKETLib::_IUSocketEvent_OnSocketClosedEventHandler(this, &CClientSocket::OnSocketClosedEventHandler);
				usc->OnSocketConnected -= gcnew USOCKETLib::_IUSocketEvent_OnSocketConnectedEventHandler(this, &CClientSocket::OnSocketConnectedEventHandler);
				System::Runtime::InteropServices::Marshal::ReleaseComObject(m_USocketClass);
				m_USocketClass = nullptr;
			}
			m_bAdvised = false;
		}
	}

	void CClientSocket::Advise()
	{
		if(m_pIUSocket == NULL)
			throw gcnew Exception("No USocket object available!");
		if(!m_bAdvised)
		{
			if(m_pCSEvent != NULL)
			{
				m_bAdvised = (!FAILED(m_pCSEvent->DispEventAdvise(m_pIUSocket)));
			}
			else
			{
				USOCKETLib::USocketClass ^usc = GetUSocket();
				if(m_OnClosing != nullptr)
					usc->OnClosing += gcnew USOCKETLib::_IUSocketEvent_OnClosingEventHandler(this, &CClientSocket::OnClosingEventHandler);
				if(m_OnConnecting != nullptr)
					usc->OnConnecting += gcnew USOCKETLib::_IUSocketEvent_OnConnectingEventHandler(this, &CClientSocket::OnConnectingEventHandler);
				if(m_OnDataAvailable != nullptr)
					usc->OnDataAvailable += gcnew USOCKETLib::_IUSocketEvent_OnDataAvailableEventHandler(this, &CClientSocket::OnDataAvailableEventHandler);
				if(m_OnSendingData != nullptr)
					usc->OnSendingData += gcnew USOCKETLib::_IUSocketEvent_OnSendingDataEventHandler(this, &CClientSocket::OnSendingDataEventHandler);
				if(m_OnGetHostByAddr != nullptr)
					usc->OnGetHostByAddr += gcnew USOCKETLib::_IUSocketEvent_OnGetHostByAddrEventHandler(this, &CClientSocket::OnGetHostByAddrEventHandler);
				if(m_OnGetHostByName != nullptr)
					usc->OnGetHostByName -= gcnew USOCKETLib::_IUSocketEvent_OnGetHostByNameEventHandler(this, &CClientSocket::OnGetHostByNameEventHandler);
				if(m_OnSocketClosed != nullptr)
					usc->OnSocketClosed += gcnew USOCKETLib::_IUSocketEvent_OnSocketClosedEventHandler(this, &CClientSocket::OnSocketClosedEventHandler);
				if(m_OnSocketConnected != nullptr)
					usc->OnSocketConnected += gcnew USOCKETLib::_IUSocketEvent_OnSocketConnectedEventHandler(this, &CClientSocket::OnSocketConnectedEventHandler);

				usc->OnOtherMessage += gcnew USOCKETLib::_IUSocketEvent_OnOtherMessageEventHandler(this, &CClientSocket::OnOtherMessageEventHandler);
				usc->OnRequestProcessed += gcnew USOCKETLib::_IUSocketEvent_OnRequestProcessedEventHandler(this, &CClientSocket::OnRequestProcessedEventHandler);
				m_bAdvised = true;
			}
		}
	}

	void CClientSocket::Uninit()
	{
		if(m_pIUSocket != NULL)
		{
			Disadvise();
		}
		if(m_USocketClass != nullptr)
		{
			System::Runtime::InteropServices::Marshal::ReleaseComObject(m_USocketClass);
			m_USocketClass = nullptr;
		}
		if(m_pIUChat != NULL)
		{
			m_pIUChat->Release();
			m_pIUChat = NULL;
		}
		if(m_pIUFast != NULL)
		{
			m_pIUFast->Release();
			m_pIUFast = NULL;
		}
		if(m_pIUSocket != NULL)
		{
			m_pIUSocket->Release();
			m_pIUSocket = NULL;
		}
		m_nCurSvsID = sidStartup;
	}

	void CClientSocket::DestroyUSocket()
	{
		if(m_pIUSocket != NULL)
		{
			m_pIUSocket->Disconnect();
		}
		Uninit();
	}

	void CClientSocket::CleanTrack()
	{
		if(m_pIUSocket != NULL)
		{
			m_pIUSocket->CleanTrack();
		}
	}

	void CClientSocket::OnClosingEventHandler(int hSocket, int hWnd)
	{
		if(m_OnClosing != nullptr)
		{
			m_OnClosing->Invoke(hSocket, hWnd);
		}
	}

	void CClientSocket::OnConnectingEventHandler(int hSocket, int hWnd)
	{
		if(m_OnConnecting != nullptr)
		{
			m_OnConnecting->Invoke(hSocket, hWnd);
		}
		CAutoLock AutoLock(&m_pCS->m_sec);
		for each(CAsyncServiceHandler ^p in m_lstAsynHandler)
		{
			p->RemoveAsyncHandlers((unsigned int)(~0));
		}
	}

	void CClientSocket::OnDataAvailableEventHandler(int hSocket, int lBytes, int lError)
	{
		if(m_OnDataAvailable != nullptr)
		{
			m_OnDataAvailable->Invoke(hSocket, lBytes, lError);
		}
	}
	void CClientSocket::OnGetHostByAddrEventHandler(int hHandle, String ^strHostName, String ^strHostAlias, int lError)
	{
		if(m_OnGetHostByAddr != nullptr)
		{
			m_OnGetHostByAddr->Invoke(hHandle, strHostName, strHostAlias, lError);
		}
	}

	void CClientSocket::OnGetHostByNameEventHandler(int hHandle, String ^strHostName, String ^strAlias, String ^strIPAddr, int lError)
	{
		if(m_OnGetHostByName != nullptr)
		{
			m_OnGetHostByName->Invoke(hHandle, strHostName, strHostName, strIPAddr, lError);
		}
	}

	void CClientSocket::OnOtherMessageEventHandler(int hSocket, int nMsg, int wParam, int lParam)
	{
		if(nMsg == msgAllRequestsProcessed && m_OnAllRequestsProcessed != nullptr)
		{
			m_OnAllRequestsProcessed->Invoke(hSocket, (short)wParam);
		}
		if(m_OnOtherMessage != nullptr)
		{
			m_OnOtherMessage->Invoke(hSocket, nMsg, wParam, lParam);
		}
		if(nMsg == msgAllRequestsProcessed && m_cb != nullptr)
		{
			m_cb->Invoke(this);
			m_cb = nullptr;
		}
	}

	void CClientSocket::OnRequestProcessedEventHandler(int hSocket, short nRequestID, int lLen, int lLenInBuffer, short sFlag)
	{
		USOCKETLib::tagReturnFlag rf = (USOCKETLib::tagReturnFlag)sFlag;
		OnRequestProcessed(hSocket, nRequestID, lLen, lLenInBuffer, rf);
	}

	void CClientSocket::OnSendingDataEventHandler(int hSocket, int lError, int lSent)
	{
		if(m_OnSendingData != nullptr)
		{
			m_OnSendingData->Invoke(hSocket, lError, lSent);
		}
	}

	void CClientSocket::OnSocketClosedEventHandler(int hSocket, int lError)
	{
		if(m_OnSocketClosed != nullptr)
		{
			m_OnSocketClosed->Invoke(hSocket, lError);
		}
		CAutoLock AutoLock(&m_pCS->m_sec);
		for each(CAsyncServiceHandler ^p in m_lstAsynHandler)
		{
			p->RemoveAsyncHandlers((unsigned int)(~0));
		}
	}

	void CClientSocket::OnSocketConnectedEventHandler(int hSocket, int lError)
	{
		if(m_OnSocketConnected != nullptr)
		{
			m_OnSocketConnected->Invoke(hSocket, lError);
		}
	}

	bool CClientSocket::StartJob()
	{
		if(m_pIJobManager != nullptr)
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_pIJobContext != nullptr && m_pIJobContext->JobStatus == tagJobStatus::jsCreating)
			{
				//job context not enqueued or destroyed yet
				return false;
			}
			CAsyncServiceHandler ^async = Lookup(m_nCurSvsID);
			m_pIJobContext = m_pIJobManager->CreateJob(async);
			return (m_pIJobContext != nullptr);
		}

		ATLASSERT(m_pIUSocket != NULL);
		if(m_pIUSocket != NULL)
		{
			return (m_pIUSocket->StartJob() == S_OK);
		}
		return false;
	}

	bool CClientSocket::IsBatchingBalanced(Dictionary<int, CTaskContext^>^ aTasks)
	{
		bool bBalanced = true;
		for each (CTaskContext ^tc in aTasks->Values)
		{
			if(tc->m_sRequestId == idStartBatching)
				bBalanced = false;
			else if(tc->m_sRequestId == idCommitBatching)
				bBalanced = true;
		}
		return bBalanced;
	}

	bool CClientSocket::EndJob()
	{
		if(m_pIJobManager != nullptr)
		{
			CAutoLock al(&m_pCS->m_sec);
			if(m_pIJobContext== nullptr)
				return false;
			if(!IsBatchingBalanced(m_pIJobContext->Tasks))
			{
				//Batching not balanced!!!
				return false;
			}
			if(m_pIJobContext->JobStatus != tagJobStatus::jsCreating || !m_pIJobManager->EnqueueJob(m_pIJobContext))
			{
				//job context has already been enqueued or destroyed.
				return false;
			}
			m_pIProcess->Process();
			return true;
		}

		ATLASSERT(m_pIUSocket != NULL);
		if(m_pIUSocket != NULL)
		{
			return (m_pIUSocket->EndJob() == S_OK);
		}
		return false;
	}

	void CClientSocket::Cancel()
	{
		Cancel(-1);
	}
		
	void CClientSocket::Cancel(int nRequests)
	{
		if(m_pIUSocket != NULL)
		{
			HRESULT hr = m_pIUSocket->Cancel(nRequests);
			if(hr == S_OK && nRequests)
			{
				unsigned long nTotal = (unsigned long)nRequests;
				CAutoLock AutoLock(&m_pCS->m_sec);
				for each(CAsyncServiceHandler ^p in m_lstAsynHandler)
				{
					nTotal -= p->RemoveAsyncHandlers(nTotal);
					if(!nTotal)
						break;
				}
			}
		}
	}

	void CClientSocket::DetachAll()
	{
		CAsyncServiceHandler ^p;
		CAutoLock AutoLock(&m_pCS->m_sec);
		int nSize = m_lstAsynHandler->Count;
		while(nSize > 0)
		{
			p = m_lstAsynHandler[0];
			ATLASSERT(p != nullptr);
			p->Detach();
			nSize--;
		}
	}

	CAsyncServiceHandler^ CClientSocket::Lookup(int nSvsID)
	{
		int n;
		CAsyncServiceHandler ^p = nullptr;
		CAutoLock AutoLock(&m_pCS->m_sec);
		int nSize = m_lstAsynHandler->Count;
		for(n=0; n<nSize; n++)
		{
			p = m_lstAsynHandler[n];
			ATLASSERT(p != nullptr);
			if(p->GetSvsID() == nSvsID)
				return p;
		}
		return nullptr;
	}

	void CClientSocket::Init()
	{
		IUSocket *pIUSocket = NULL;
		HRESULT hr = ::CoCreateInstance(__uuidof(USocket), NULL, CLSCTX_ALL, __uuidof(IUSocket), (void**)&pIUSocket);
		//make sure either CoInitialize or CoInitializeEx called
		//make sure usocket.dll registered properly
		ATLASSERT(hr == S_OK);

		if(hr == S_OK)
		{
			m_pIUSocket = pIUSocket;

			ULONG *pulParameters;
			CComVariant vtServerParameters;
			hr = m_pIUSocket->get_ServerParams(&vtServerParameters);
			::SafeArrayAccessData(vtServerParameters.parray, (void**)&pulParameters);
			m_bServerException = ((pulParameters[4] & TRANSFER_SERVER_EXCEPTION) == TRANSFER_SERVER_EXCEPTION);
			::SafeArrayUnaccessData(vtServerParameters.parray);

//			m_USocketClass = (USOCKETLib::USocketClass^)Marshal::GetTypedObjectForIUnknown(IntPtr(m_pIUSocket), USOCKETLib::USocketClass::typeid);

#ifdef _DEBUG
			//1000 second. 
			//If too short, client will automatically disconnect socket connection when you debug at server side.
			hr = m_pIUSocket->put_RecvTimeout(1000000);
#endif
			IUFast *pIUFast = NULL;
			hr = m_pIUSocket->QueryInterface(__uuidof(IUFast), (void**)&pIUFast);
			m_pIUFast = pIUFast;

			IUChat		*pIUChat = NULL;
			hr = m_pIUSocket->QueryInterface(__uuidof(IUChat), (void**)&pIUChat);
			m_pIUChat = pIUChat;
			ATLASSERT(m_pIUChat != NULL);

			long lSvsID;
			m_pIUSocket->get_CurrentSvsID(&lSvsID);
			m_nCurSvsID = lSvsID;

//			Advise();
		}
		else
		{
#ifdef _WIN64
				throw gcnew System::InvalidProgramException("ATL COM object can not be created! This may happen because the core library npUSocket.dll (64bit) is not registerred (regsvr32 npUSocket.dll) properly or COM environment is not initialized (MTAThread or STAThread) yet!");
#else
				throw gcnew System::InvalidProgramException("ATL COM object can not be created! This may happen because the core library npUSocket.dll (32bit) is not registerred (regsvr32 npUSocket.dll) properly or COM environment is not initialized (MTAThread or STAThread) yet!");
#endif
		}
	}
} //ClientSide
} //SocketProAdapter