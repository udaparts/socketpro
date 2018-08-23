
#ifndef _UDAPARTS_GENERAL_CACHE_CLIENT_HANDLER_H_
#define _UDAPARTS_GENERAL_CACHE_CLIENT_HANDLER_H_

#include "udatabase.h"
#include "aclientw.h"

namespace SPA {
    namespace ClientSide {
        using namespace UDB;

        template<unsigned int serviceId>
        class CCachedBaseHandler : public CAsyncServiceHandler {
            static const unsigned int ONE_MEGA_BYTES = 0x100000;
            static const unsigned int BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0;

        public:

            CCachedBaseHandler(CClientSocket *cs)
            : CAsyncServiceHandler(serviceId, cs), m_indexRowset(0), m_ms(UDB::msUnknown) {
                m_Blob.Utf8ToW(true);
            }

            static const unsigned int CachedServiceId = serviceId;

        public:
            typedef std::function<void(int res, const std::wstring &errMsg) > DResult;
            typedef std::function<void(CDBColumnInfoArray &meta) > DRowsetHeader;
            typedef std::function<void(CDBVariantArray &vData) > DRows;

        public:

            virtual unsigned int CleanCallbacks() {
                {
                    CAutoLock al(m_csCache);
                    m_mapRowset.clear();
                }
                return CAsyncServiceHandler::CleanCallbacks();
            }

            inline tagManagementSystem GetDBManagementSystem() {
                CAutoLock al(m_csCache);
                return m_ms;
            }

            virtual bool GetCachedTables(const wchar_t *defaultDb, DResult handler, DRows row, DRowsetHeader rh, unsigned int flags = SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES) {
                UINT64 index = GetCallIndex();
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                //in case a client asynchronously sends lots of requests without use of client side queue.
                m_csCache.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csCache.unlock();

                if (!SendRequest(idGetCachedTables, defaultDb, flags, index, [index, handler, this](CAsyncResult & ar) {
                        int res, dbMS;
                        std::wstring errMsg;
                                ar >> dbMS >> res >> errMsg;
                                this->m_csCache.lock();
                                this->m_ms = (UDB::tagManagementSystem) dbMS;
                                auto it = this->m_mapRowset.find(index);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        this->m_csCache.unlock();
                        if (handler) {
                            handler(res, errMsg);
                        }
                    })) {
                CAutoLock al(m_csCache);
                m_mapRowset.erase(index);
                return false;
            }
                return true;
            }

        protected:

            virtual void OnMergeTo(CAsyncServiceHandler & to) {
                CCachedBaseHandler &dbTo = (CCachedBaseHandler&) to;
                CAutoLock al0(dbTo.m_csCache);
                {
                    CAutoLock al1(m_csCache);
                    for (auto it = m_mapRowset.begin(), end = m_mapRowset.end(); it != end; ++it) {
                        dbTo.m_mapRowset[it->first] = it->second;
                    }
                    m_mapRowset.clear();
                }
            }

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case idRowsetHeader:
                    {
                        m_Blob.SetSize(0);
                        if (m_Blob.GetMaxSize() > ONE_MEGA_BYTES) {
                            m_Blob.ReallocBuffer(ONE_MEGA_BYTES);
                        }
                        CDBColumnInfoArray vColInfo;
                        mc >> vColInfo >> m_indexRowset;
                        CAutoLock al(m_csCache);
                        m_vData.clear();
                        auto &p = m_mapRowset[m_indexRowset];
                        if (p.first) {
                            p.first(vColInfo);
                        }
                    }
                    case idBeginRows:
                        m_Blob.SetSize(0);
                        m_vData.clear();
                        break;
                    case idTransferring:
                        if (mc.GetSize()) {
                            m_csCache.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
                            m_csCache.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
                            while (mc.GetSize()) {
                                m_vData.push_back(CDBVariant());
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
                        }
                        break;
                    case idEndRows:
                        if (mc.GetSize() || m_vData.size()) {
                            m_csCache.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
                            m_csCache.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
                            CDBVariant vtOne;
                            while (mc.GetSize()) {
                                m_vData.push_back(vtOne);
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
                            DRows row;
                            {
                                CAutoLock al(m_csCache);
                                auto &p = m_mapRowset[m_indexRowset];
                                row = p.second;
                            }
                            if (row) {
                                row(m_vData);
                            }
                        }
                        m_vData.clear();
                        break;
                    case idStartBLOB:
                    {
                        m_Blob.SetSize(0);
                        unsigned int len;
                        mc >> len;
                        if (len != UQUEUE_END_POSTION && len > m_Blob.GetMaxSize()) {
                            m_Blob.ReallocBuffer(len);
                        }
                        m_Blob.Push(mc.GetBuffer(), mc.GetSize());
                        mc.SetSize(0);
                    }
                        break;
                    case idChunk:
                        m_Blob.Push(mc.GetBuffer(), mc.GetSize());
                        mc.SetSize(0);
                        break;
                    case idEndBLOB:
                        if (mc.GetSize() || m_Blob.GetSize()) {
                            m_Blob.Push(mc.GetBuffer(), mc.GetSize());
                            mc.SetSize(0);
                            unsigned int *len = (unsigned int*) m_Blob.GetBuffer(sizeof (VARTYPE));
                            if (*len >= BLOB_LENGTH_NOT_AVAILABLE) {
                                //legth should be reset if BLOB length not available from server side at beginning
                                *len = (m_Blob.GetSize() - sizeof (VARTYPE) - sizeof (unsigned int));
                            }
                            m_vData.push_back(CDBVariant());
                            CDBVariant &vt = m_vData.back();
                            m_Blob >> vt;
                            assert(m_Blob.GetSize() == 0);
                        }
                        break;
                    default:
                        CAsyncServiceHandler::OnResultReturned(reqId, mc);
                        break;
                }
            }

        protected:
            CUCriticalSection m_csCache;

        private:
            typedef std::pair<DRowsetHeader, DRows> CRowsetHandler;
            std::unordered_map<UINT64, CRowsetHandler> m_mapRowset;
            CDBVariantArray m_vData;
            CUQueue m_Blob;
            UINT64 m_indexRowset;
            UDB::tagManagementSystem m_ms;
        };
    } //namespace ClientSide
} //namespace SPA
#endif
