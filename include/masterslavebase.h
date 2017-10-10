
#ifndef _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_
#define _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_

#include "tablecache.h"

namespace SPA {
	template<typename THandler, typename TCS = ClientSide::CClientSocket>
    class CMasterSlaveBase : public ClientSide::CSocketPool < THandler, TCS> {
    public:
        typedef ClientSide::CSocketPool < THandler, TCS> CBase;

        CMasterSlaveBase(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT)
        : CBase(true, recvTimeout), m_dbDefalut(defaultDb ? defaultDb : L""), m_nRecvTimeout(recvTimeout) {
        }

        typedef std::function<void() > DOnClosed;

    public:

        void Subscribe(UINT64 key, DOnClosed c) {
            CAutoLock al(this->m_cs);
            m_mapClose[key] = c;
        }

        void Remove(UINT64 key) {
            CAutoLock al(this->m_cs);
            m_mapClose.erase(key);
        }

        const wchar_t* GetDefaultDBName() const {
            return m_dbDefalut.c_str();
        }

        unsigned int GetRecvTimeout() const {
            return m_nRecvTimeout;
        }

    protected:

        virtual void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const std::shared_ptr<THandler> &asyncSQL) {
            switch (spe) {
                case ClientSide::speSocketClosed:
                    if (!asyncSQL->GetAttachedClientSocket()->Sendable()) {
                        CAutoLock al(this->m_cs);
                        for (auto it = m_mapClose.begin(), end = m_mapClose.end(); it != end; ++it) {
                            DOnClosed closed = it->second;
                            if (closed)
                                closed();
                        }
                    }
                    break;
                default:
                    break;
            }
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
        std::unordered_map<UINT64, DOnClosed> m_mapClose; //protected by base class m_cs
        std::wstring m_dbDefalut;
        unsigned int m_nRecvTimeout;
    };
} //namespace SPA

#endif