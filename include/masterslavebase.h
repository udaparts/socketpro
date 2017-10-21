
#ifndef _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_
#define _UDAPARTS_MASTER_SLAVE_POOL_BASE_H_

#include "tablecache.h"

namespace SPA {

    template<typename THandler, typename TCS = ClientSide::CClientSocket>
    class CMasterSlaveBase : public ClientSide::CSocketPool < THandler, TCS > {
    public:
        typedef ClientSide::CSocketPool < THandler, TCS> CBase;

        CMasterSlaveBase(const wchar_t *defaultDb, const char* qname, bool auto_merge = true, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT)
        : CBase(true, recvTimeout),
        m_dbDefalut(defaultDb ? defaultDb : L""),
        m_qname(qname ? qname : ""),
        m_nRecvTimeout(recvTimeout),
        m_bAutoMerge(auto_merge) {
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
                case ClientSide::speConnected:
                    if (m_bAutoMerge && !this->GetQueueAutoMerge()) {
                        this->SetQueueAutoMerge(m_bAutoMerge);
                    }
                    if (m_qname.size()) {
                        std::string qname = m_qname + "_pool_" + std::to_string(GetIndex(asyncSQL));
                        bool ok = asyncSQL->GetAttachedClientSocket()->GetClientQueue().StartQueue(qname.c_str(), 3600, false);
                        assert(ok);
                    }
                    break;
                default:
                    break;
            }
        }

        static std::wstring ToWide(const VARIANT &data) {
            const char *s;
            assert(data.vt == (VT_ARRAY | VT_I1));
            ::SafeArrayAccessData(data.parray, (void**) &s);
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
        std::unordered_map<UINT64, DOnClosed> m_mapClose; //protected by base class m_cs
        std::wstring m_dbDefalut;
        std::string m_qname;
        unsigned int m_nRecvTimeout;
        bool m_bAutoMerge;
    };
} //namespace SPA

#endif