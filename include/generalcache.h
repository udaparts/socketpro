
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

            CCachedBaseHandler(CClientSocket *cs = nullptr)
            : CAsyncServiceHandler(serviceId, cs), m_nCall(0), m_indexRowset(0) {
                m_Blob.Utf8ToW(true);
#ifdef WIN32_64
                m_Blob.TimeEx(true);
#endif
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

            virtual bool GetCachedTables(DResult handler, DRows row, DRowsetHeader rh, unsigned int flags = SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES) {
                bool rowset = (rh || row);
                UINT64 index;
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csCache);
                    ++m_nCall;
                    index = m_nCall;
                    if (rowset) {
                        m_mapRowset[index] = CRowsetHandler(rh, row);
                    }
                }

                if (!SendRequest(idGetCachedTables, flags, rowset, index, [index, handler, this](CAsyncResult & ar) {
                        int res;
                        std::wstring errMsg;
                                ar >> res >> errMsg;
                                this->m_csCache.lock();
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
#ifdef WIN32_64
                            bool timeEx = m_Blob.TimeEx();
#endif
                            m_csCache.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(true);
#endif
                            while (mc.GetSize()) {
                                m_vData.push_back(CDBVariant());
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(false);
#endif
                        }
                        break;
                    case idEndRows:
                        if (mc.GetSize() || m_vData.size()) {
                            m_csCache.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
#ifdef WIN32_64
                            bool timeEx = m_Blob.TimeEx();
#endif
                            m_csCache.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(true);
#endif
                            CDBVariant vtOne;
                            while (mc.GetSize()) {
                                m_vData.push_back(vtOne);
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(false);
#endif
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
            UINT64 m_nCall;
            UINT64 m_indexRowset;
        };
    } //namespace ClientSide
} //namespace SPA
#endif
