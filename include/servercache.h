

#ifndef ___SOCKETPRO_SERVER_SIDE_CACHE_I_H__
#define ___SOCKETPRO_SERVER_SIDE_CACHE_I_H__

#include "tablecache.h"
#include "udb_client.h"

namespace SPA {
    namespace ServerSide {

        template<typename THandler, typename TCache = CTableCache>
        class CMasterPool : public ClientSide::CSocketPool < THandler > {
        public:

            CMasterPool(bool autoConn = true, unsigned int recvTimeout = ClientSide::DEFAULT_RECV_TIMEOUT, unsigned int connTimeout = ClientSide::DEFAULT_CONN_TIMEOUT)
            : ClientSide::CSocketPool<THandler>(autoConn, recvTimeout, connTimeout) {
            }

            static TCache Cache; //real-time cache accessable from your code

            typedef ClientSide::CAsyncDBHandler<THandler::SQLStreamServiceId> CSQLHandler;
            typedef ClientSide::CSocketPool<THandler> CSlavePool;
            typedef TCache CTableCache;

        protected:

            void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const std::shared_ptr<THandler> &asyncSQL) {
                switch (spe) {
                    case ClientSide::speConnected:
                        //use the first socket session for table events only, update, delete and insert
                        if (asyncSQL == this->GetAsyncHandlers()[0]) {
                            asyncSQL->GetAttachedClientSocket()->GetPush().OnPublish = [](ClientSide::CClientSocket*cs, const ClientSide::CMessageSender& sender, const unsigned int* groups, unsigned int count, const UVariant & vtMsg) {
                                VARIANT *vData;
                                size_t res;

                                assert(count == 1);
                                assert(groups != nullptr);
                                assert(groups[0] == UDB::STREAMING_SQL_CHAT_GROUP_ID);

                                //vData[0] == event type; vData[1] == host; vData[2] = database user; vData[3] == db name; vData[4] == table name
                                ::SafeArrayAccessData(vtMsg.parray, (void**) &vData);
                                ClientSide::tagUpdateEvent eventType = (ClientSide::tagUpdateEvent)(vData[0].intVal);

                                if (!Cache.GetDBServerName().size()) {
                                    if (vData[1].vt == (VT_ARRAY | VT_I1))
                                        Cache.SetDBServerName(ToWide(vData[1]).c_str());
                                    else if (vData[1].vt == VT_BSTR)
                                        Cache.SetDBServerName(vData[1].bstrVal);
                                }
                                if (vData[2].vt == (VT_ARRAY | VT_I1))
                                    Cache.SetUpdater(ToWide(vData[2]).c_str());
                                else if (vData[2].vt == VT_BSTR)
                                    Cache.SetUpdater(vData[2].bstrVal);
                                else
                                    Cache.SetUpdater(nullptr);

                                std::wstring dbName;
                                if (vData[3].vt == (VT_I1 | VT_ARRAY)) {
                                    dbName = ToWide(vData[3]);
                                } else if (vData[3].vt == VT_BSTR)
                                    dbName = vData[3].bstrVal;
                                else {
                                    assert(false); //shouldn't come here
                                }
                                std::wstring tblName;
                                if (vData[4].vt == (VT_I1 | VT_ARRAY)) {
                                    tblName = ToWide(vData[4]);
                                } else if (vData[3].vt == VT_BSTR)
                                    tblName = vData[4].bstrVal;
                                else {
                                    assert(false); //shouldn't come here
                                }
                                switch (eventType) {
                                    case UDB::ueInsert:
                                        res = Cache.AddRows(dbName.c_str(), tblName.c_str(), vData + 5, vtMsg.parray->rgsabound->cElements - 5);
                                        assert(res == 1);
                                        break;
                                    case UDB::ueUpdate:
                                    {
                                        unsigned int count = vtMsg.parray->rgsabound->cElements - 5;
#ifndef NDEBUG
                                        CKeyMap map = Cache.FindKeys(dbName.c_str(), tblName.c_str());

                                        //there must be one or two key columns. For other cases, you must implement them
                                        assert(map.size() > 0 && map.size() <= 2);

                                        size_t cols = Cache.GetColumnCount(dbName.c_str(), tblName.c_str());
                                        assert((size_t) count == cols * 2);
#endif

                                        res = Cache.UpdateARow(dbName.c_str(), tblName.c_str(), vData + 5, count);
                                        assert(res != CTableCache::INVALID_VALUE);
                                    }
                                        break;
                                    case UDB::ueDelete:
                                    {
                                        unsigned int keys = vtMsg.parray->rgsabound->cElements - 5;
                                        //there must be one or two key columns. For other cases, you must implement them
                                        assert(keys <= 2 && keys > 0);
#ifndef NDEBUG
                                        CKeyMap map = Cache.FindKeys(dbName.c_str(), tblName.c_str());
                                        assert(map.size() == keys);
#endif
                                        if (keys == 1)
                                            res = Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5]);
                                        else
                                            res = Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5], vData[6]);
                                        assert(res != CTableCache::INVALID_VALUE);
                                    }
                                        break;
                                    default:
                                        //not implemented
                                        assert(false);
                                        break;
                                }
                                ::SafeArrayUnaccessData(vtMsg.parray);
                            };

                            //open default database and subscribe for table update events (update, delete and insert) by setting flag UDB::ENABLE_TABLE_UPDATE_MESSAGES
                            bool ok = asyncSQL->Open(L"", [this](CSQLHandler &h, int res, const std::wstring & errMsg) {
                                this->m_cache.SetDBServerName(nullptr);
                                this->m_cache.SetUpdater(nullptr);
                                        this->m_cache.Empty();
                                        unsigned int port;
                                        std::string ip = h.GetAttachedClientSocket()->GetPeerName(&port);
                                        ip += ":";
                                        ip += std::to_string(port);
										h.Utf8ToW(true);
#ifdef WIN32_64
										h.TimeEx(true);
#endif
                                        this->m_cache.Set(ip.c_str(), h.GetDBManagementSystem());
                            }, UDB::ENABLE_TABLE_UPDATE_MESSAGES);

                            //bring all cached table data into m_cache first for initial cache, and exchange it with Cache if there is no error
                            ok = asyncSQL->Execute(L"", [this](CSQLHandler &h, int res, const std::wstring &errMsg, INT64 affected, UINT64 fail_ok, UDB::CDBVariant & vtId) {
                                if (res == 0) {
                                    Cache.Swap(this->m_cache); //exchange between master Cache and this m_cache
                                } else {
                                    std::cout << "Error code: " << res << ", error message: ";
                                    std::wcout << errMsg.c_str() << std::endl;
                                }
                            }, [this](CSQLHandler &h, UDB::CDBVariantArray & vData) {
                                auto meta = h.GetColumnInfo();
                                const UDB::CDBColumnInfo &info = meta.front();
                                        //populate vData into m_cache container
                                        this->m_cache.AddRows(info.DBPath.c_str(), info.TablePath.c_str(), vData);
                            }, [this](CSQLHandler & h) {
                                //a rowset column meta comes
                                this->m_cache.AddEmptyRowset(h.GetColumnInfo());
                            });
                        }
                        break;
                    default:
                        break;
                }
            }

        protected:

            static std::wstring ToWide(const VARIANT &data) {
                const char *s;
                assert(data.vt == (VT_ARRAY | VT_I1));
                ::SafeArrayAccessData(data.parray, (void**) &s);
                std::wstring ws = Utilities::ToWide(s, data.parray->rgsabound->cElements);
                ::SafeArrayUnaccessData(data.parray);
                return ws;
            }

        protected:
            TCache m_cache;
        };

        template<typename THandler, typename TCache>
        TCache CMasterPool<THandler, TCache>::Cache;
    }; //namespace ServerSide
}; //namespace SPA

#endif