
#include "stdafx.h"
#include "masterpool.h"

extern CTableCache g_cache;

CMasterPool::CMasterPool() {

}

void CMasterPool::OnSocketPoolEvent(tagSocketPoolEvent spe, const PHandler &asyncSQL) {
    switch (spe) {
        case speConnected:
            if (asyncSQL == GetAsyncHandlers()[0]) {
                asyncSQL->GetAttachedClientSocket()->GetPush().OnPublish = [](CClientSocket*cs, const CMessageSender& sender, const unsigned int* groups, unsigned int count, const SPA::UVariant & vtMsg) {
					VARIANT *vData;
					size_t res;
					unsigned int elements = vtMsg.parray->rgsabound->cElements;
					::SafeArrayAccessData(vtMsg.parray, (void**)&vData);
					tagUpdateEvent eventType = (tagUpdateEvent)(vData[0].intVal);
					std::wstring dbName;
					if (vData[3].vt == (VT_I1 | VT_ARRAY)) {
						char *s;
						::SafeArrayAccessData(vData[3].parray, (void**)&s);
						dbName = SPA::Utilities::ToWide(s, vData[3].parray->rgsabound->cElements);
						::SafeArrayUnaccessData(vData[3].parray);
					}
					else if (vData[3].vt == VT_BSTR)
						dbName = vData[3].bstrVal;
					else {
						assert(false);
					}
					std::wstring tblName;
					if (vData[4].vt == (VT_I1 | VT_ARRAY)) {
						char *s;
						::SafeArrayAccessData(vData[4].parray, (void**)&s);
						tblName = SPA::Utilities::ToWide(s, vData[4].parray->rgsabound->cElements);
						::SafeArrayUnaccessData(vData[4].parray);
					}
					else if (vData[3].vt == VT_BSTR)
						tblName = vData[4].bstrVal;
					else {
						assert(false);
					}
					switch (eventType)
					{
					case SPA::UDB::ueInsert:
						res = g_cache.AddRows(dbName.c_str(), tblName.c_str(), vData + 5, vtMsg.parray->rgsabound->cElements - 5);
						assert(res == 1);
						break;
					case SPA::UDB::ueUpdate:
					{
						unsigned int count = vtMsg.parray->rgsabound->cElements - 5;
#ifndef NDEBUG
						CTableCache::CKeyMap map = g_cache.FindKeys(dbName.c_str(), tblName.c_str());

						//there must be one or two key columns. For other cases, you must implement them
						assert(map.size() > 0 && map.size() <= 2);

						size_t cols = g_cache.GetColumnCount(dbName.c_str(), tblName.c_str());
						assert((size_t)count == cols * 2);
#endif
						
						res = g_cache.UpdateARow(dbName.c_str(), tblName.c_str(), vData + 5, count);
					}
						break;
					case SPA::UDB::ueDelete:
					{			
						unsigned int keys = vtMsg.parray->rgsabound->cElements - 5;
						//there must be one or two key columns. For other cases, you must implement them
						assert(keys <= 2 && keys > 0);
#ifndef NDEBUG
						CTableCache::CKeyMap map = g_cache.FindKeys(dbName.c_str(), tblName.c_str());
						assert(map.size() == keys);
#endif
						if (keys == 1)
							res = g_cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5]);
						else
							res = g_cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5], vData[6]);
						assert(res == 1 || res == 0);
					}
						break;
					default:
						//not implemented
						assert(false);
						break;
					}
					::SafeArrayUnaccessData(vtMsg.parray);
                };

                bool ok = asyncSQL->Open(L"", [this](CAsyncSQLHandler &h, int res, const std::wstring & errMsg) {
                    this->m_cache.Empty();
					unsigned int port;
					std::string ip = h.GetAttachedClientSocket()->GetPeerName(&port);
					ip += ":";
					ip += std::to_string(port);
					this->m_cache.SetDBServerIp(ip.c_str());
                }, ENABLE_TABLE_UPDATE_MESSAGES);
                ok = asyncSQL->Execute(L"", [this](CAsyncSQLHandler &h, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
                    if (res == 0) {
                        g_cache.Swap(this->m_cache);
                    }
                }, [this](CAsyncSQLHandler &h, CDBVariantArray & vData) {
                    auto meta = h.GetColumnInfo();
                    const CDBColumnInfo &info = meta.front();
                    this->m_cache.AddRows(info.DBPath.c_str(), info.TablePath.c_str(), vData);
                }, [this](CAsyncSQLHandler & h) {
                    this->m_cache.AddEmptyRowset(h.GetColumnInfo());
                });
            }
            break;
        default:
            break;
    }
}