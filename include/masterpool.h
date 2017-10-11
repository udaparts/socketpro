
#ifndef _UDAPARTS_MASTER_POOL_H_
#define _UDAPARTS_MASTER_POOL_H_

#include "masterslavebase.h"
#include "generalcache.h"

namespace SPA {

    template<bool midTier, typename THandler, typename TCache = CDataSet>
    class CMasterPool : public CMasterSlaveBase < THandler > {
    public:
        typedef CMasterSlaveBase < THandler > CBase;

        CMasterPool(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT)
        : CBase(defaultDb, recvTimeout) {
        }
        typedef TCache CDataSet;
        static TCache Cache; //real-time cache accessible from your code
        typedef ClientSide::CCachedBaseHandler<THandler::CachedServiceId> CHandler;
        typedef CMasterSlaveBase < THandler > CSlavePool;

    protected:

        virtual void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const std::shared_ptr<THandler> &pHandler) {
            switch (spe) {
                case ClientSide::speConnected:
                    if (pHandler->GetAttachedClientSocket()->GetErrorCode() != 0)
                        break;
                    //use the first socket session for table events only, update, delete and insert
                    if (pHandler == this->GetAsyncHandlers()[0]) {
                        pHandler->GetAttachedClientSocket()->GetPush().OnPublish = [this, pHandler](ClientSide::CClientSocket*cs, const ClientSide::CMessageSender& sender, const unsigned int* groups, unsigned int count, const UVariant & vtMsg) {
                            assert(count == 1);
                            assert(groups != nullptr);
                            assert(groups[0] == UDB::STREAMING_SQL_CHAT_GROUP_ID || groups[0] == UDB::CACHE_UPDATE_CHAT_GROUP_ID);

                            if (groups[0] == UDB::CACHE_UPDATE_CHAT_GROUP_ID) {
                                if (midTier) {
                                    UVariant vtMessage;
                                    //notify front clients to re-initialize cache
                                    ServerSide::CSocketProServer::PushManager::Publish(vtMessage, &UDB::CACHE_UPDATE_CHAT_GROUP_ID, 1);
                                }
                                this->SetInitialCache(pHandler);
                                return;
                            }
                            if (midTier) {
                                //push message onto front clients which may be interested in the message
                                ServerSide::CSocketProServer::PushManager::Publish(vtMsg, &UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
                            }

                            VARIANT *vData;
                            size_t res;
                            //vData[0] == event type; vData[1] == host; vData[2] = database user; vData[3] == db name; vData[4] == table name
                            ::SafeArrayAccessData(vtMsg.parray, (void**) &vData);
                            ClientSide::tagUpdateEvent eventType = (ClientSide::tagUpdateEvent)(vData[0].intVal);

                            if (!Cache.GetDBServerName().size()) {
                                if (vData[1].vt == (VT_ARRAY | VT_I1))
                                    Cache.SetDBServerName(this->ToWide(vData[1]).c_str());
                                else if (vData[1].vt == VT_BSTR)
                                    Cache.SetDBServerName(vData[1].bstrVal);
                            }
                            if (vData[2].vt == (VT_ARRAY | VT_I1))
                                Cache.SetUpdater(this->ToWide(vData[2]).c_str());
                            else if (vData[2].vt == VT_BSTR)
                                Cache.SetUpdater(vData[2].bstrVal);
                            else
                                Cache.SetUpdater(nullptr);

                            std::wstring dbName;
                            if (vData[3].vt == (VT_I1 | VT_ARRAY)) {
                                dbName = this->ToWide(vData[3]);
                            } else if (vData[3].vt == VT_BSTR)
                                dbName = vData[3].bstrVal;
                            else {
                                assert(false); //shouldn't come here
                            }
                            std::wstring tblName;
                            if (vData[4].vt == (VT_I1 | VT_ARRAY)) {
                                tblName = this->ToWide(vData[4]);
                            } else if (vData[3].vt == VT_BSTR)
                                tblName = vData[4].bstrVal;
                            else {
                                assert(false); //shouldn't come here
                            }
                            switch (eventType) {
                                case UDB::ueInsert:
                                    res = Cache.AddRows(dbName.c_str(), tblName.c_str(), vData + 5, vtMsg.parray->rgsabound->cElements - 5);
                                    assert(res != CDataSet::INVALID_VALUE);
                                    break;
                                case UDB::ueUpdate:
                                {
                                    unsigned int count = vtMsg.parray->rgsabound->cElements - 5;
                                    res = Cache.UpdateARow(dbName.c_str(), tblName.c_str(), vData + 5, count);
                                    assert(res != CDataSet::INVALID_VALUE);
                                }
                                    break;
                                case UDB::ueDelete:
                                {
                                    unsigned int keys = vtMsg.parray->rgsabound->cElements - 5;
                                    //there must be one or two key columns. For other cases, you must implement them
                                    if (keys == 1)
                                        res = Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5]);
                                    else
                                        res = Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5], vData[6]);
                                    assert(res != CDataSet::INVALID_VALUE);
                                }
                                    break;
                                default:
                                    //not implemented
                                    assert(false);
                                    break;
                            }
                            ::SafeArrayUnaccessData(vtMsg.parray);
                        };

                        if (midTier) {
                            UVariant vtMessage;
                            ServerSide::CSocketProServer::PushManager::Publish(vtMessage, &UDB::CACHE_UPDATE_CHAT_GROUP_ID, 1);
                        }
                        this->m_cache.SetDBServerName(nullptr);
                        this->m_cache.SetUpdater(nullptr);
                        this->m_cache.Empty();
                        unsigned int port;
                        std::string ip = pHandler->GetAttachedClientSocket()->GetPeerName(&port);
                        ip += ":";
                        ip += std::to_string(port);
                        this->m_cache.Set(ip.c_str(), UDB::msUnknown);
                        SetInitialCache(pHandler);
                    } else {
                        pHandler->GetCachedTables(nullptr, nullptr, nullptr, (unsigned int) 0);
                    }
                    break;
                default:
                    CBase::OnSocketPoolEvent(spe, pHandler);
                    break;
            }
        }

    private:

        void SetInitialCache(const std::shared_ptr<THandler> &pHandler) {
            //open default database and subscribe for table update events (update, delete and insert) by setting flag UDB::ENABLE_TABLE_UPDATE_MESSAGES
            bool ok = pHandler->GetCachedTables([this](int res, const std::wstring & errMsg) {
                if (res == 0) {
                    Cache.Swap(this->m_cache); //exchange between master Cache and this m_cache
                }
            }, [this](UDB::CDBVariantArray & vData) {
                UDB::CDBColumnInfoArray &vCol = this->m_meta;
                this->m_cache.AddRows(vCol.front().DBPath.c_str(), vCol.front().TablePath.c_str(), vData);
            }, [this](UDB::CDBColumnInfoArray & meta) {
                this->m_cache.AddEmptyRowset(meta);
                this->m_meta = meta;
            }, UDB::ENABLE_TABLE_UPDATE_MESSAGES);
        }

    protected:
        TCache m_cache;

    private:
        UDB::CDBColumnInfoArray m_meta;
    };
    template<bool midTier, typename THandler, typename TCache>
    TCache CMasterPool<midTier, THandler, TCache>::Cache;
} //namespace SPA

#endif
