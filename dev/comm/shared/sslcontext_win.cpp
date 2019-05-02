
#include "sslcontext_win.h"
#include <assert.h>

namespace SPA {

	CSslContext::CSslContext()
		: m_hSecurity(NULL), m_pSSPI(NULL), m_hMyCertStore(NULL)
	{
		::memset(&m_hCreds, 0, sizeof(m_hCreds));
	}

	CSslContext::~CSslContext() {
		CloseStoreAndUnloadSecurityLibrary();
	}

	PSecurityFunctionTableW CSslContext::GetSspi() const {
		return m_pSSPI;
	}

	CredHandle* CSslContext::GetCredHandle() {
		return &m_hCreds;
	}

	bool CSslContext::IsLoaded() const
	{
		return (m_hSecurity != NULL);
	}

	void CSslContext::FreeCredentials()
	{
		if(m_hCreds.dwLower != 0 || m_hCreds.dwUpper != 0)
		{
			assert(IsLoaded());
			SECURITY_STATUS ss = m_pSSPI->FreeCredentialsHandle(&m_hCreds);
			::memset(&m_hCreds, 0, sizeof(m_hCreds));
		}
	}

#ifndef WINCE
	bool CSslContext::OpenStore(const wchar_t *strPfxFile, const wchar_t *strPassword)
	{
		DWORD ulRead = 0;
		assert(m_hMyCertStore == NULL);

		HANDLE hfile = INVALID_HANDLE_VALUE;
	
		// get the handle to the file...
		hfile = ::CreateFileW(strPfxFile, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(INVALID_HANDLE_VALUE == hfile)
			return false;
		CRYPT_DATA_BLOB blob;
		blob.cbData = GetFileSize(hfile, 0);
		blob.pbData = (BYTE*)::malloc(blob.cbData);
		::ReadFile(hfile, blob.pbData, blob.cbData, &ulRead, NULL);
		bool bSuc = true;
		do
		{
			bSuc = ::PFXIsPFXBlob(&blob) ? true : false;
			if(!bSuc)
				break;
			m_hMyCertStore = PFXImportCertStore(&blob, strPassword, 0/*bMachine ? CRYPT_MACHINE_KEYSET : CRYPT_USER_KEYSET*/);
			bSuc = (m_hMyCertStore != NULL);
		}while(false);
		::free(blob.pbData);
		::CloseHandle(hfile);
		return bSuc;
	}
#endif
	bool CSslContext::OpenStore(bool bMachine, bool bRoot)
	{
		assert(m_hMyCertStore == NULL);
		DWORD dwStoreOpenFlag = (CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG);
		if(bMachine)
		{
			if(bRoot)
				dwStoreOpenFlag |= CERT_SYSTEM_STORE_CURRENT_USER;
			else
				dwStoreOpenFlag |= CERT_SYSTEM_STORE_LOCAL_MACHINE;

			m_hMyCertStore = CertOpenStore( CERT_STORE_PROV_SYSTEM,
											0,
											X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
											dwStoreOpenFlag,
											bRoot ? L"ROOT" : L"MY");
		}
		else
		{
			m_hMyCertStore = ::CertOpenSystemStore(0, bRoot ? L"ROOT" : L"MY"); 
		}
		
		if(m_hMyCertStore == NULL)
		{
			CloseStoreAndUnloadSecurityLibrary();
			return false;
		}
		return true;
	}

	bool CSslContext::LoadSecurityLibrary()
	{
		if(IsLoaded())
			return true;
		m_hSecurity = ::LoadLibraryW(L"Secur32.dll");
		if (m_hSecurity == NULL)
			return false;
#ifdef WINCE
		INIT_SECURITY_INTERFACE_W pInitSecurityInterface = (INIT_SECURITY_INTERFACE_W)GetProcAddressW(m_hSecurity, L"InitSecurityInterfaceW");
#else
		INIT_SECURITY_INTERFACE_W pInitSecurityInterface = (INIT_SECURITY_INTERFACE_W)GetProcAddress(m_hSecurity, "InitSecurityInterfaceW");
#endif
		if (pInitSecurityInterface == NULL)
		{
			CloseStoreAndUnloadSecurityLibrary();
			return false;
		}

		m_pSSPI = pInitSecurityInterface();
		if(m_pSSPI == NULL)
		{
			CloseStoreAndUnloadSecurityLibrary();
			return false;
		}
		return true;
	}

	void CSslContext::CloseStoreAndUnloadSecurityLibrary()
	{
		BOOL ok;
		if(m_hCreds.dwLower != 0 || m_hCreds.dwUpper != 0)
		{
			SECURITY_STATUS ss = m_pSSPI->FreeCredentialsHandle(&m_hCreds);
			::memset(&m_hCreds, 0, sizeof(m_hCreds));
		}

		if(m_hMyCertStore != NULL)
		{
			ok = ::CertCloseStore(m_hMyCertStore, 0);
			m_hMyCertStore = NULL;
		}

		if(m_hSecurity != NULL)
		{
			ok = ::FreeLibrary(m_hSecurity);
			m_hSecurity = NULL;
		}

		m_pSSPI = NULL;
	}

	SECURITY_STATUS CSslContext::Open(DWORD dwProtocol, const char* strUserName)
	{
		assert(IsLoaded());
		BOOL ok;
		TimeStamp       tsExpiry;
		SECURITY_STATUS Status = SEC_E_OK;
		PCCERT_CONTEXT  pCertContext = NULL;

		if(strUserName == NULL || ::strlen(strUserName) == 0)
		{
			return SEC_E_NO_CREDENTIALS;
		}

	
		pCertContext = ::CertFindCertificateInStore(m_hMyCertStore, 
												  X509_ASN_ENCODING, 
												  0,
												  CERT_FIND_SUBJECT_STR_A,
												  strUserName,
												  NULL);
		if(pCertContext == NULL)
		{
			DWORD dwErrorCode = ::GetLastError();
			return dwErrorCode;
		}

		SCHANNEL_CRED					SchannelCred;
		::memset(&SchannelCred, 0, sizeof(SchannelCred));

		SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
		if (pCertContext)
		{
			SchannelCred.cCreds = 1;
			SchannelCred.paCred = &pCertContext;
		}

		SchannelCred.grbitEnabledProtocols = dwProtocol;
	
		if (m_hCreds.dwLower != 0 || m_hCreds.dwUpper != 0)
		{
			Status = m_pSSPI->FreeCredentialsHandle(&m_hCreds);
			::memset(&m_hCreds, 0, sizeof(m_hCreds));
		}

		Status = m_pSSPI->AcquireCredentialsHandleW(
							NULL,                   // Name of principal    
							UNISP_NAME_W,			// Name of package UNISP_NAME for the Schannel SSP
							SECPKG_CRED_INBOUND,	// Flags indicating use
							NULL,                   // Pointer to logon ID
							&SchannelCred,			// Package specific data
							NULL,                   // Pointer to GetKey() func
							NULL,                   // Value to pass to GetKey()
							&m_hCreds,              // (out) Cred Handle
							&tsExpiry);
		if (pCertContext)
		{
			ok = ::CertFreeCertificateContext(pCertContext);
		}
		return Status;
	}

	CMsSsl::CMsSsl(CSslContext &sslContext)
		: m_sslContext(sslContext),
		m_SSLState(ssUnknown),
		m_bImpersonateLoggedOnUser(false),
		m_qEncrypted(*m_suEncrypt),
		m_qDecrypted(*m_suDecrypt),
		m_hUserToken(NULL),
		m_nMaxSize(0)
	{
		::memset(&m_Sizes, 0, sizeof(m_Sizes));
		::memset(&m_hContext, 0, sizeof(m_hContext));
	}

	CMsSsl::~CMsSsl()
	{
		FreeResource();
	}

	SECURITY_STATUS CMsSsl::HandshakeServer(CUQueue &out, bool bImpersonateLoggedOnUser) 
	{
		SECURITY_STATUS scRet = SEC_I_CONTINUE_NEEDED;
		SecBufferDesc   InBuffer;
		SecBuffer       InBuffers[2];
		SecBufferDesc   OutBuffer;
		SecBuffer       OutBuffers[1];
		TimeStamp       tsExpiry;
		DWORD dwSSPIOutFlags = 0;
		DWORD dwSSPIFlags = ASC_REQ_SEQUENCE_DETECT |
						ASC_REQ_REPLAY_DETECT |
						ASC_REQ_CONFIDENTIALITY |
						ASC_REQ_EXTENDED_ERROR |
						ASC_REQ_ALLOCATE_MEMORY |
						ASC_REQ_STREAM;
		
		bool bBreak = false;
		while (true)
		{
			m_SSLState = ssHandshaking;
			if (m_qEncrypted.GetSize() == 0)
				return scRet;
		
			// Set up the input buffers. Buffer 0 is used to pass in data
			// received from the server. Schannel will consume some or all
			// of this. Leftover data (if any) will be placed in buffer 1 and
			// given a buffer type of SECBUFFER_EXTRA.
			InBuffers[0].pvBuffer   = (void*)m_qEncrypted.GetBuffer();
			InBuffers[0].cbBuffer   = m_qEncrypted.GetSize();
			InBuffers[0].BufferType = SECBUFFER_TOKEN;

			InBuffers[1].pvBuffer   = NULL;
			InBuffers[1].cbBuffer   = 0;
			InBuffers[1].BufferType = SECBUFFER_EMPTY;

			InBuffer.cBuffers       = 2;
			InBuffer.pBuffers       = InBuffers;
			InBuffer.ulVersion      = SECBUFFER_VERSION;

			// Set up the output buffers. These are initialized to NULL
			// so as to make it less likely we'll attempt to free random
			// garbage later.
			OutBuffers[0].pvBuffer  = NULL;
			OutBuffers[0].BufferType= SECBUFFER_TOKEN;
			OutBuffers[0].cbBuffer  = 0;

			OutBuffer.cBuffers      = 1;
			OutBuffer.pBuffers      = OutBuffers;
			OutBuffer.ulVersion     = SECBUFFER_VERSION;
		
			bool bInit = (m_hContext.dwLower == 0 && m_hContext.dwUpper == 0);
			scRet = m_sslContext.GetSspi()->AcceptSecurityContext(
							m_sslContext.GetCredHandle(),
							(bInit ? NULL : &m_hContext),
							&InBuffer,
							dwSSPIFlags,
							0,
							&m_hContext,
							&OutBuffer,
							&dwSSPIOutFlags,
							&tsExpiry);

		
			// If InitializeSecurityContext was successful (or if the error was 
			// one of the special extended ones), send the contends of the output
			// buffer to the server.
			if (scRet == SEC_E_OK || scRet == SEC_I_CONTINUE_NEEDED ||
				FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
			{
				if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
				{
					out.Push((const unsigned char*)OutBuffers[0].pvBuffer, (unsigned int)OutBuffers[0].cbBuffer);
					m_sslContext.GetSspi()->FreeContextBuffer(OutBuffers[0].pvBuffer);
					OutBuffers[0].pvBuffer = NULL;
				}
				bBreak = true;
				if (FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
					scRet = SEC_E_INCOMPLETE_MESSAGE;
			}

			if (scRet == SEC_E_INCOMPLETE_MESSAGE || scRet == SEC_I_INCOMPLETE_CREDENTIALS || scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
				bBreak = true;
				scRet = SEC_I_CONTINUE_NEEDED;
			}

			// If InitializeSecurityContext returned SEC_E_OK, then the 
			// handshake completed successfully.
			if (scRet == SEC_E_OK)
			{
				m_bImpersonateLoggedOnUser = false;
				m_SSLState = ssFinished;
				if (bImpersonateLoggedOnUser)
				{
					scRet = m_sslContext.GetSspi()->QuerySecurityContextToken(&m_hContext, &m_hUserToken);
					if (SUCCEEDED(scRet))
					{
						if (ImpersonateLoggedOnUser(m_hUserToken))
						{
							m_bImpersonateLoggedOnUser = true;
						}
					}
					else if (scRet == SEC_E_NO_IMPERSONATION)
						scRet = SEC_E_OK;
				}
			}

			if (InBuffers[1].BufferType == SECBUFFER_EXTRA )
			{
				assert(m_qEncrypted.GetSize() > InBuffers[1].cbBuffer);
				unsigned int ulSent = m_qEncrypted.GetSize() - InBuffers[1].cbBuffer;
				m_qEncrypted.Pop(ulSent);
			}
			else
			{
				m_qEncrypted.SetSize(0);
			}
			if (bBreak)
				break;
		}
		if (FAILED(scRet))
			FreeResource();
		return scRet;
	}

	unsigned int CMsSsl::Encrypt(CUQueue &out) {
	

		// ????
		return 0;
	}

	unsigned int CMsSsl::Decrypt(CUQueue &out) {
		assert(m_SSLState == ssFinished);
		
		SecBufferDesc   Message;
		SecBuffer       Buffers[4];
		
		memset(Buffers, 0, sizeof(Buffers));

        // Attempt to decrypt the received data.
        Buffers[0].pvBuffer     = (void*)m_qEncrypted.GetBuffer();
        Buffers[0].cbBuffer     = m_qEncrypted.GetSize();
        Buffers[0].BufferType   = SECBUFFER_DATA;
		Buffers[1].BufferType   = SECBUFFER_EMPTY;
        Buffers[2].BufferType   = SECBUFFER_EMPTY;
        Buffers[3].BufferType   = SECBUFFER_EMPTY;

        Message.ulVersion       = SECBUFFER_VERSION;
        Message.cBuffers        = 4;
        Message.pBuffers        = Buffers;

		m_qEncrypted.SetSize(0);
		SECURITY_STATUS scRet = m_sslContext.GetSspi()->DecryptMessage(&m_hContext, &Message, 0, NULL);
        for (unsigned long i = 1; i < Message.cBuffers; i++) {
            if (Buffers[i].BufferType == SECBUFFER_DATA)
                out.Push((const unsigned char*)Buffers[i].pvBuffer, (unsigned int)Buffers[i].cbBuffer);
            else if(Buffers[i].BufferType == SECBUFFER_EXTRA) {
				m_qEncrypted.Push((const unsigned char*)Buffers[i].pvBuffer, (unsigned int)Buffers[i].cbBuffer);
			}
        }
		return scRet;
	}

	unsigned int CMsSsl::Close(CUQueue &out) 
	{
		unsigned int start = out.GetSize();
		if (m_hContext.dwLower != 0 || m_hContext.dwUpper != 0) {
			assert(m_sslContext.IsLoaded());
			do
			{
				DWORD           dwType;
				char			*pbMessage;
				DWORD           cbMessage;
				SecBufferDesc   OutBuffer;
				SecBuffer       OutBuffers[1];
				DWORD           dwSSPIFlags;
				DWORD           dwSSPIOutFlags;
				TimeStamp       tsExpiry;
				DWORD           Status;
				dwType = SCHANNEL_SHUTDOWN;
				OutBuffers[0].pvBuffer   = &dwType;
				OutBuffers[0].BufferType = SECBUFFER_TOKEN;
				OutBuffers[0].cbBuffer   = sizeof(dwType);

				OutBuffer.cBuffers  = 1;
				OutBuffer.pBuffers  = OutBuffers;
				OutBuffer.ulVersion = SECBUFFER_VERSION;
				Status = m_sslContext.GetSspi()->ApplyControlToken(&m_hContext, &OutBuffer);
				if (FAILED(Status))
					break;
				dwSSPIFlags =   ASC_REQ_SEQUENCE_DETECT     |
						ASC_REQ_REPLAY_DETECT       |
						ASC_REQ_CONFIDENTIALITY     |
						ASC_REQ_EXTENDED_ERROR      |
						ASC_REQ_ALLOCATE_MEMORY     |
						ASC_REQ_STREAM;

				OutBuffers[0].pvBuffer   = NULL;
				OutBuffers[0].BufferType = SECBUFFER_TOKEN;
				OutBuffers[0].cbBuffer   = 0;

				OutBuffer.cBuffers  = 1;
				OutBuffer.pBuffers  = OutBuffers;
				OutBuffer.ulVersion = SECBUFFER_VERSION;

				Status = m_sslContext.GetSspi()->AcceptSecurityContext(
								m_sslContext.GetCredHandle(),
								&m_hContext,
								NULL,
								dwSSPIFlags,
								SECURITY_NATIVE_DREP,
								NULL,
								&OutBuffer,
								&dwSSPIOutFlags,
								&tsExpiry);
				if(FAILED(Status))
					break;
				pbMessage = (char*)OutBuffers[0].pvBuffer;
				cbMessage = OutBuffers[0].cbBuffer;
				if(pbMessage != NULL && cbMessage != 0)
				{
					out.Push((const unsigned char*)pbMessage, cbMessage);
					// Free output buffer.
					Status = m_sslContext.GetSspi()->FreeContextBuffer(pbMessage);
				}
			}while(false);
		}
		FreeResource();
		return (out.GetSize() - start);
	}

	void* CMsSsl::GetHandle() {
		return &m_hContext;
	}

	void CMsSsl::FreeResource()
	{
		if (m_bImpersonateLoggedOnUser)
		{
#ifndef WINCE
			BOOL bSuc = RevertToSelf();
			assert(bSuc);
#endif
			m_bImpersonateLoggedOnUser = false;
		}
		if(m_hContext.dwLower != 0 || m_hContext.dwUpper != 0)
		{
			assert(m_sslContext.IsLoaded());
			SECURITY_STATUS ss = m_sslContext.GetSspi()->DeleteSecurityContext(&m_hContext);
			::memset(&m_hContext, 0, sizeof(m_hContext));
		}
		::memset(&m_Sizes, 0, sizeof(m_Sizes));
		m_SSLState = SPA::ssUnknown;
		m_qDecrypted.SetSize(0);
		m_qEncrypted.SetSize(0);
		m_nMaxSize = 0;
	}

	void CMsSsl::GetMaxSize()
	{
		assert(m_sslContext.IsLoaded());
		if (m_Sizes.cbMaximumMessage == 0) {
			SECURITY_STATUS ss = m_sslContext.GetSspi()->QueryContextAttributesW(&m_hContext, SECPKG_ATTR_STREAM_SIZES, &m_Sizes);
			if (ss != SEC_E_OK) {
				assert(false);
				return;
			}
		}
		m_nMaxSize = m_Sizes.cbMaximumMessage + m_Sizes.cbHeader + m_Sizes.cbTrailer;
	}

	tagSSLState	CMsSsl::GetSSLState()
	{
		return m_SSLState;
	}
} //namespace SPA