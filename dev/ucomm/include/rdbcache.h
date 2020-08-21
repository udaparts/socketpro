
#ifndef ___SOCKETPRO_SERVER_SIDE_CACHE_I_H__
#define ___SOCKETPRO_SERVER_SIDE_CACHE_I_H__

#include "udb_client.h"
#include "masterslavebase.h"

#ifndef NO_MIDDLE_TIER
#include "aserverw.h" //don't need to distribute server code library if the below template midTier is false, even though the header file is required here
#endif

namespace SPA {

    template<bool midTier, typename THandler, typename TCache = CDataSet, typename TCS = ClientSide::CClientSocket>
    class CSQLMasterPool : public CMasterSlaveBase < THandler, TCS > {
    public:
        typedef CMasterSlaveBase < THandler, TCS > CBase;

        CSQLMasterPool(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT, unsigned int svsId = 0)
        : CBase(defaultDb, recvTimeout, svsId) {
        }

        typedef TCache CDataSet;
        TCache Cache; //real-time cache accessible from your code
        typedef ClientSide::CAsyncDBHandler<THandler::SQLStreamServiceId> CSQLHandler;

        class CSlavePool : public CMasterSlaveBase < THandler, TCS > {
        public:

            CSlavePool(const wchar_t *defaultDb, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT, unsigned int svsId = 0)
            : CMasterSlaveBase<THandler>(defaultDb, recvTimeout, svsId) {
            }

        protected:

            virtual void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const std::shared_ptr<THandler> &asyncSQL) {
                switch (spe) {
                    case SPA::ClientSide::speConnected:
                        if (asyncSQL->GetSocket()->GetErrorCode() != 0)
                            break;
                        asyncSQL->Utf8ToW(true);
                        asyncSQL->Open(this->GetDefaultDBName().c_str(), nullptr); //open a session to backend database by default 
                        break;
                    default:
                        break;
                }
                CMasterSlaveBase < THandler >::OnSocketPoolEvent(spe, asyncSQL);
            }
        };

    protected:

        virtual void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const std::shared_ptr<THandler> &asyncSQL) {
            switch (spe) {
                case ClientSide::speConnected:
                    if (asyncSQL->GetSocket()->GetErrorCode() != 0)
                        break;
                    //use the first socket session for table events only, update, delete and insert
                    if (asyncSQL == this->GetAsyncHandlers()[0]) {
                        asyncSQL->GetSocket()->GetPush().OnPublish = [this, asyncSQL](ClientSide::CClientSocket*cs, const ClientSide::CMessageSender& sender, const unsigned int* groups, unsigned int count, const UVariant & vtMsg) {
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
                                this->SetInitialCache(asyncSQL);
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
                            CDBString dbName = this->ToUTF16(vData[3]);
                            CDBString tblName = this->ToUTF16(vData[4]);
                            switch (eventType) {
                                case UDB::ueInsert:
                                    res = this->Cache.AddRows(dbName.c_str(), tblName.c_str(), vData + 5, vtMsg.parray->rgsabound->cElements - 5);
                                    assert(res != CDataSet::INVALID_VALUE);
                                    break;
                                case UDB::ueUpdate:
                                {
                                    unsigned int count = vtMsg.parray->rgsabound->cElements - 5;
                                    res = this->Cache.UpdateARow(dbName.c_str(), tblName.c_str(), vData + 5, count);
                                    assert(res != CDataSet::INVALID_VALUE);
                                }
                                    break;
                                case UDB::ueDelete:
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
                        SetInitialCache(asyncSQL);
                    } else {
                        asyncSQL->Utf8ToW(true);
                        asyncSQL->Open(this->GetDefaultDBName().c_str(), nullptr);
                    }
                    break;
                default:
                    break;
            }
            CBase::OnSocketPoolEvent(spe, asyncSQL);
        }

    private:

        void SetInitialCache(const std::shared_ptr<THandler> &asyncSQL) {
            //open default database and subscribe for table update events (update, delete and insert) by setting flag UDB::ENABLE_TABLE_UPDATE_MESSAGES
            bool ok = asyncSQL->Open(this->GetDefaultDBName().c_str(), [this](CSQLHandler &h, int res, const std::wstring & errMsg) {
                if (!res) {
                    this->m_cache.SetUpdater(nullptr);
                    this->m_cache.Empty();
                    h.Utf8ToW(true);
                    unsigned int port;
                    std::string ip = h.GetSocket()->GetPeerName(&port);
                    ip += ":";
                    ip += std::to_string((UINT64) port);
                    std::string host = h.GetSocket()->GetConnectionContext().Host;
                    std::wstring s = Utilities::ToWide(host.c_str(), host.size());
                    this->m_cache.SetDBServerName(s.c_str());
                    this->m_cache.Set(ip.c_str(), h.GetDBManagementSystem());
                }
            }, UDB::ENABLE_TABLE_UPDATE_MESSAGES);

            //bring all cached table data into m_cache first for initial cache, and exchange it with Cache if there is no error
            ok = asyncSQL->Execute(L"", [this](CSQLHandler &h, int res, const std::wstring &errMsg, INT64 affected, UINT64 fail_ok, UDB::CDBVariant & vtId) {
                if (res == 0) {
                    this->Cache.Swap(this->m_cache); //exchange between master Cache and this m_cache
                }
            },
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
            [this](CSQLHandler &h, CUQueue & v) {
                auto &meta = h.GetColumnInfo();
                const UDB::CDBColumnInfo &info = meta.front();
                UDB::CDBVariantArray vData;
                while (v.GetSize()) {
                    UDB::CDBVariant vt;
                    v >> vt;
                    vData.push_back(std::move(vt));
                }
                //populate vData into m_cache container
                this->m_cache.AddRows(info.DBPath.c_str(), info.TablePath.c_str(), vData);
            }, [this](CSQLHandler & h, const unsigned char *start, unsigned int bytes) {
                //a rowset column meta comes
                this->m_cache.AddEmptyRowset(h.GetColumnInfo());
            });
#else
            [this](CSQLHandler &h, UDB::CDBVariantArray & vData) {
                auto &meta = h.GetColumnInfo();
                const UDB::CDBColumnInfo &info = meta.front();
                //populate vData into m_cache container
                this->m_cache.AddRows(info.DBPath.c_str(), info.TablePath.c_str(), vData);
            }, [this](CSQLHandler & h) {
                //a rowset column meta comes
                this->m_cache.AddEmptyRowset(h.GetColumnInfo());
            });
#endif
        }

    protected:
        TCache m_cache;
    };
}; //namespace SPA

#endif
