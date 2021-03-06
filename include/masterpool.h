
#ifndef _UDAPARTS_MASTER_POOL_H_
#define _UDAPARTS_MASTER_POOL_H_

#include "generalcache.h"
#include "masterslavebase.h"

#ifndef NO_MIDDLE_TIER
#include "aserverw.h" //don't need to distribute server code library if the below template midTier is false, even though the header file is required here
#endif

namespace SPA {

    template<bool midTier, typename THandler, typename TCache = CDataSet, typename TCS = ClientSide::CClientSocket>
    class CMasterPool : public CMasterSlaveBase < THandler, TCS > {
    public:
        typedef CMasterSlaveBase < THandler, TCS > CBase;

        CMasterPool(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT, unsigned int svsId = 0)
        : CBase(defaultDb, recvTimeout, svsId) {
        }
        typedef TCache CDataSet;
        TCache Cache; //real-time cache accessible from your code
        typedef CMasterSlaveBase < THandler, TCS > CSlavePool;

    protected:

        virtual void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const std::shared_ptr<THandler> &pHandler) {
            switch (spe) {
                case ClientSide::tagSocketPoolEvent::speConnected:
                    if (pHandler->GetSocket()->GetErrorCode() != 0)
                        break;
                    //use the first socket session for table events only, update, delete and insert
                    if (pHandler == this->GetAsyncHandlers()[0]) {
                        pHandler->GetSocket()->GetPush().OnPublish = [this, pHandler](ClientSide::CClientSocket*cs, const ClientSide::CMessageSender& sender, const unsigned int* groups, unsigned int count, const UVariant & vtMsg) {
                            assert(count == 1);
                            assert(groups != nullptr);
                            assert(groups[0] == UDB::STREAMING_SQL_CHAT_GROUP_ID || groups[0] == UDB::CACHE_UPDATE_CHAT_GROUP_ID);

                            if (groups[0] == UDB::CACHE_UPDATE_CHAT_GROUP_ID) {
#ifndef NO_MIDDLE_TIER
                                if (midTier) {
                                    UVariant vtMessage;
                                    //notify front clients to re-initialize cache
                                    ServerSide::CSocketProServer::PushManager::Publish(vtMessage, &UDB::CACHE_UPDATE_CHAT_GROUP_ID, 1);
                                }
#endif
                                this->SetInitialCache(pHandler);
                                return;
                            }
#ifndef NO_MIDDLE_TIER
                            if (midTier) {
                                //push message onto front clients which may be interested in the message
                                ServerSide::CSocketProServer::PushManager::Publish(vtMsg, &UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
                            }
#endif
                            VARIANT *vData;
                            size_t res;
                            //vData[0] == event type; vData[1] == host; vData[2] = database user; vData[3] == db name; vData[4] == table name
                            ::SafeArrayAccessData(vtMsg.parray, (void**) &vData);
                            ClientSide::tagUpdateEvent eventType = (ClientSide::tagUpdateEvent)(vData[0].intVal);

                            if (!this->Cache.GetDBServerName().size()) {
                                if (vData[1].vt == (VT_ARRAY | VT_I1) || vData[1].vt == VT_BSTR) {
                                    this->Cache.SetDBServerName(this->ToWide(vData[1]).c_str());
                                } else {
                                    this->Cache.SetDBServerName(L"");
                                }
                            }
                            if (vData[2].vt == (VT_ARRAY | VT_I1) || vData[2].vt == VT_BSTR) {
                                this->Cache.SetUpdater(this->ToWide(vData[2]).c_str());
                            } else {
                                this->Cache.SetUpdater(L"");
                            }
                            SPA::CDBString dbName = this->ToUTF16(vData[3]);
                            SPA::CDBString tblName = this->ToUTF16(vData[4]);
                            switch (eventType) {
                                case UDB::tagUpdateEvent::ueInsert:
                                    res = this->Cache.AddRows(dbName.c_str(), tblName.c_str(), vData + 5, vtMsg.parray->rgsabound->cElements - 5);
                                    assert(res != CDataSet::INVALID_VALUE);
                                    break;
                                case UDB::tagUpdateEvent::ueUpdate:
                                {
                                    unsigned int count = vtMsg.parray->rgsabound->cElements - 5;
                                    res = this->Cache.UpdateARow(dbName.c_str(), tblName.c_str(), vData + 5, count);
                                    assert(res != CDataSet::INVALID_VALUE);
                                }
                                    break;
                                case UDB::tagUpdateEvent::ueDelete:
                                {
                                    unsigned int keys = vtMsg.parray->rgsabound->cElements - 5;
                                    //there must be one or two key columns. For other cases, you must implement them
                                    if (keys == 1)
                                        res = this->Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5]);
                                    else
                                        res = this->Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData + 5, keys);
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
#ifndef NO_MIDDLE_TIER
                        if (midTier) {
                            UVariant vtMessage;
                            ServerSide::CSocketProServer::PushManager::Publish(vtMessage, &UDB::CACHE_UPDATE_CHAT_GROUP_ID, 1);
                        }
#endif
                        this->m_cache.SetUpdater(nullptr);
                        this->m_cache.Empty();
                        SetInitialCache(pHandler);
                    } else {
                        pHandler->GetCachedTables(this->GetDefaultDBName().c_str(), nullptr, nullptr, nullptr, (unsigned int) 0);
                    }
                    break;
                default:
                    break;
            }
            CBase::OnSocketPoolEvent(spe, pHandler);
        }

    private:

        void SetInitialCache(const std::shared_ptr<THandler> &pHandler) {
            //open default database and subscribe for table update events (update, delete and insert) by setting flag UDB::ENABLE_TABLE_UPDATE_MESSAGES
            bool ok = pHandler->GetCachedTables(this->GetDefaultDBName().c_str(), [this, pHandler](int res, const std::wstring & errMsg) {
                unsigned int port;
                std::string ip = pHandler->GetSocket()->GetPeerName(&port);
                ip += ":";
                ip += std::to_string((UINT64) port);
                this->m_cache.Set(ip.c_str(), pHandler->GetDBManagementSystem());
                std::string host = pHandler->GetSocket()->GetConnectionContext().Host;
                std::wstring s = Utilities::ToWide(host.c_str(), host.size());
                this->m_cache.SetDBServerName(s.c_str());
                if (res == 0) {
                    this->Cache.Swap(this->m_cache); //exchange between master Cache and this m_cache
                    this->m_cache.Set(ip.c_str(), pHandler->GetDBManagementSystem());
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
} //namespace SPA

#endif
