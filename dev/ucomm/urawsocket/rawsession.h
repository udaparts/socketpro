#ifndef _U_RAW_SESSION_H_
#define _U_RAW_SESSION_H_


#include <atomic>
#include "rawclient.h"
#include "../core_shared/shared/includes.h"

#ifdef WIN32_64
#include "../core_shared/shared/certificateimpl.h"
#else

#endif

namespace SPA {

	using ms = std::chrono::milliseconds;

	class CRawThread;

	class CRawSession : public USessionBase {

	public:
		CRawSession(CIoService &IoService, CRawThread &rt, PDataArrive da);
		CRawSession(const CRawSession &rs) = delete;
		~CRawSession();

	public:
		CRawSession& operator=(const CRawSession &rs) = delete;
		void PostProcessing(unsigned int hint, UINT64 data);
		bool Connect(const char *strHost, unsigned int nPort, tagEncryptionMethod secure, bool b6, bool bSync, unsigned int timeout);
		bool Shutdown(tagShutdownType st);
		int GetErrorCode(char *em, unsigned int len);
		bool IsConnected();
		void Close();
		int Send(const unsigned char *data, unsigned int bytes);
		unsigned int GetSendBufferSize();
		IUcert* GetUCert();

	private:
		int SendInternal(const unsigned char *data, unsigned int bytes);
		bool IsSameThread();
		void OnPostProcessing(unsigned int hint, UINT64 data);
		void OnWriteCompleted(const CErrorCode& Error, size_t bytes_transferred);
		void Read();
		void OnReadCompleted(const CErrorCode& Error, size_t nLen);

		static unsigned char* GetIoBuffer();
		static void ReleaseIoBuffer(unsigned char *buffer);

		static const unsigned int HINT_CONNECT = 1;
		static const unsigned int HINT_CLOSE = 2;

	private:
		static CSpinLock m_csBuffer;
		static std::vector<unsigned char*> m_aBuffer;

		CScopeUQueue m_sbWrite;
		CUQueue &m_qWrite;

		CIoService &m_io;
		CRawThread &m_rt;
		PDataArrive m_da;
		CConditionVariable m_cv;
		typedef CSpinAutoLock CAutoLock;
		CSpinLock m_cs;
		std::mutex m_mutex;
		CErrorCode m_ec;
		CSocket m_socket;
		unsigned int m_nPort;
		std::string m_strhost;
		bool m_b6;
		bool m_bSync;
		std::atomic<tagSessionState> m_ss;
		tagEncryptionMethod m_secure;
		unsigned char *m_ReadBuffer;
		bool m_bRBLocked;
		unsigned char *m_WriteBuffer;
		size_t m_bWBLocked;
		bool m_bWaiting;

#ifdef WIN32_64
		void FreeCredHandle();
		SECURITY_STATUS OpenCred();
		CredHandle m_hCreds;
		CSspiPtr m_pSspi;
		CCertificateImplPtr m_pCert;
#else


#endif

	};

	typedef CRawSession *PRawSession;

}; //namespace SPA;

#endif