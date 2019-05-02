#ifndef _SSL_CONTEXT_WINDOW_H__
#define _SSL_CONTEXT_WINDOW_H__

#include "../../include/membuffer.h"
#include "../../include/ucomm.h"
#ifdef WINCE
#include <schnlsp.h>
#else
#include <schannel.h>
#endif
#define SECURITY_WIN32
#include <security.h>
#include <sspi.h>

namespace SPA {

	enum tagSSLState
	{
		ssUnknown = 0,
		ssClientHello,
		ssHandshaking,
		ssFinished,
	};

	class CSslContext
	{
	public:
		CSslContext();
		~CSslContext();

	public:
		bool LoadSecurityLibrary();
		bool OpenStore(bool bMachine = false, bool bRoot = true);
#ifndef WINCE
		bool OpenStore(const wchar_t* strPfxFile, const wchar_t* strPassword);
#endif
		void CloseStoreAndUnloadSecurityLibrary();
		bool IsLoaded() const;
		SECURITY_STATUS Open(DWORD dwProtocol, const char* strUserName);
		void FreeCredentials();
		PSecurityFunctionTableW GetSspi() const;
		CredHandle* GetCredHandle();

	private:
		CSslContext(const CSslContext &sc);
		CSslContext& operator=(const CSslContext &sc);

	private:
		CredHandle				m_hCreds;
		HMODULE					m_hSecurity;
		PSecurityFunctionTableW	m_pSSPI;
		HCERTSTORE				m_hMyCertStore;
	};

	class CMsSsl
	{
	public:
		CMsSsl(CSslContext &sslContext);
		~CMsSsl();

	public:
		tagSSLState	GetSSLState();
		unsigned int Encrypt(CUQueue &out);
		unsigned int Decrypt(CUQueue &out);
		unsigned int Close(CUQueue &out);
		SECURITY_STATUS HandshakeServer(CUQueue &out, bool bImpersonateLoggedOnUser = true);
		void* GetHandle();

	private:
		CMsSsl(const CMsSsl &ssl);
		CMsSsl& operator=(const CMsSsl &ssl);
		void FreeResource();
		void GetMaxSize();

	private:
		CSslContext& m_sslContext;
		tagSSLState	m_SSLState;
		CScopeUQueue m_suEncrypt;
		CScopeUQueue m_suDecrypt;
		bool m_bImpersonateLoggedOnUser;
		CtxtHandle m_hContext;
		SecPkgContext_StreamSizes m_Sizes;
		HANDLE m_hUserToken;
		unsigned long m_nMaxSize;

	public:
		CUQueue &m_qEncrypted;
		CUQueue &m_qDecrypted;
	};
};

#endif

