
#ifndef _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_
#define _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_

#include "tablecache.h"

namespace SPA {

	template<typename THandler, typename TCS = ClientSide::CClientSocket>
	class CMasterSlaveBase : public ClientSide::CSocketPool < THandler, TCS > {
	public:
		typedef ClientSide::CSocketPool < THandler, TCS> CBase;

		CMasterSlaveBase(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT)
			: CBase(true, recvTimeout),
			m_dbDefalut(defaultDb ? defaultDb : L""),
			m_nRecvTimeout(recvTimeout) {
		}

		typedef std::function<void() > DOnClosed;

	public:
		inline const std::wstring& GetDefaultDBName() const {
			return m_dbDefalut;
		}

		inline unsigned int GetRecvTimeout() const {
			return m_nRecvTimeout;
		}

		static std::wstring ToWide(const VARIANT &data) {
			const char *s;
			assert(data.vt == (VT_ARRAY | VT_I1));
			::SafeArrayAccessData(data.parray, (void**)&s);
			std::wstring ws = Utilities::ToWide(s, data.parray->rgsabound->cElements);
			::SafeArrayUnaccessData(data.parray);
			return ws;
		}

	private:

		int GetIndex(std::shared_ptr<THandler> h) {
			int index = 0;
			CAutoLock al(this->m_cs);
			for (auto it = this->m_mapSocketHandler.begin(), end = this->m_mapSocketHandler.end(); it != end; ++it, ++index) {
				if (it->second == h)
					return index;
			}
			assert(false);
			return -1;
		}

	private:
		std::wstring m_dbDefalut;
		unsigned int m_nRecvTimeout;
	};
} //namespace SPA

#endif