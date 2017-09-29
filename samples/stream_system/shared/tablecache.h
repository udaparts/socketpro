
#ifndef __SOCKETPRO_REAL_TIME_CACHE_H_
#define __SOCKETPRO_REAL_TIME_CACHE_H_

#include <map>
#include "../../../include/udb_client.h"

namespace SPA {

	class CTableCache {
	public:
		CTableCache();

		typedef std::pair<UDB::CDBColumnInfoArray, UDB::CDBVariantArray> CPColumnRowset; //meta and data array for a rowset
		typedef std::vector<CPColumnRowset> CRowsetArray;
		typedef std::pair<std::wstring, std::wstring> CPDbTable; //DB and Table name pair
		typedef std::map<unsigned int, UDB::CDBColumnInfo> CKeyMap; //ordinal and colunm info map

		static const size_t INVALID_VALUE = ((size_t)(~0));

	public:
		std::vector<CPDbTable> GetDbTablePair();
		CKeyMap FindKeys(const wchar_t *dbName, const wchar_t *tblName);
		UDB::CDBColumnInfoArray GetColumMeta(const wchar_t *dbName, const wchar_t *tblName);
		size_t GetRowCount(const wchar_t *dbName, const wchar_t *tblName);
		size_t GetColumnCount(const wchar_t *dbName, const wchar_t *tblName);
		std::string GetDBServerIp();
		bool IsEmpty();



		//find a row based on one or two keys and equal operation
		UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
		UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);
		UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
		UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);

		
		

		//you may define other methods for >, >=, < and <= to search for rows

	private:
		void Swap(CTableCache &tc);
		size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
		size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);
		size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
		size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);
		size_t UpdateARow(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count);
		void AddEmptyRowset(const UDB::CDBColumnInfoArray &meta);
		size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, UDB::CDBVariantArray &vData);
		size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count);

	private:
		UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key);
		UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT &key1);
		static UDB::CDBVariant Convert(const VARIANT &data, VARTYPE vtTarget);
		static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta);
		static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta, size_t &key1);

	protected:
		CUCriticalSection m_cs;
		CRowsetArray m_ds;
		std::string m_strIp;
		bool m_bWide;

#ifdef WIN32_64
		bool m_bTimeEx;
#endif

	private:
		CTableCache(const CTableCache &tc);
		CTableCache& operator=(const CTableCache &tc);

		template<unsigned int serviceId, typename TCache>
		friend class CMasterPool;
	};

	template<unsigned int serviceId, typename TCache = CTableCache>
	class CMasterPool : public ClientSide::CSocketPool<ClientSide::CAsyncDBHandler<serviceId> > {
	public:
		CMasterPool() {
		}

		static CTableCache Cache;

		typedef ClientSide::CAsyncDBHandler<serviceId> CAsyncSQLHandler;
		typedef ClientSide::CSocketPool<CAsyncSQLHandler> CAsyncSQLPool;

	protected:
		virtual void OnSocketPoolEvent(ClientSide::tagSocketPoolEvent spe, const PHandler &asyncSQL) {
			switch (spe) {
			case ClientSide::speConnected:
				if (asyncSQL == GetAsyncHandlers()[0]) {
					asyncSQL->GetAttachedClientSocket()->GetPush().OnPublish = [](ClientSide::CClientSocket*cs, const ClientSide::CMessageSender& sender, const unsigned int* groups, unsigned int count, const UVariant & vtMsg) {
						VARIANT *vData;
						size_t res;
						::SafeArrayAccessData(vtMsg.parray, (void**)&vData);
						ClientSide::tagUpdateEvent eventType = (ClientSide::tagUpdateEvent)(vData[0].intVal);
						std::wstring dbName;
						if (vData[3].vt == (VT_I1 | VT_ARRAY)) {
							dbName = ToWide(vData[3]);
						}
						else if (vData[3].vt == VT_BSTR)
							dbName = vData[3].bstrVal;
						else {
							assert(false);
						}
						std::wstring tblName;
						if (vData[4].vt == (VT_I1 | VT_ARRAY)) {
							tblName = ToWide(vData[4]);
						}
						else if (vData[3].vt == VT_BSTR)
							tblName = vData[4].bstrVal;
						else {
							assert(false);
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
							CTableCache::CKeyMap map = Cache.FindKeys(dbName.c_str(), tblName.c_str());

							//there must be one or two key columns. For other cases, you must implement them
							assert(map.size() > 0 && map.size() <= 2);

							size_t cols = Cache.GetColumnCount(dbName.c_str(), tblName.c_str());
							assert((size_t)count == cols * 2);
#endif

							res = Cache.UpdateARow(dbName.c_str(), tblName.c_str(), vData + 5, count);
						}
						break;
						case UDB::ueDelete:
						{
							unsigned int keys = vtMsg.parray->rgsabound->cElements - 5;
							//there must be one or two key columns. For other cases, you must implement them
							assert(keys <= 2 && keys > 0);
#ifndef NDEBUG
							CTableCache::CKeyMap map = Cache.FindKeys(dbName.c_str(), tblName.c_str());
							assert(map.size() == keys);
#endif
							if (keys == 1)
								res = Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5]);
							else
								res = Cache.DeleteARow(dbName.c_str(), tblName.c_str(), vData[5], vData[6]);
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
						this->m_cache.m_ds.clear();
						unsigned int port;
						std::string ip = h.GetAttachedClientSocket()->GetPeerName(&port);
						ip += ":";
						ip += std::to_string(port);
						this->m_cache.m_strIp = ip;
						this->m_cache.m_bWide = h.Utf8ToW();
#ifdef WIN32_64
						this->m_cache.m_bTimeEx = h.TimeEx();
#endif
					}, UDB::ENABLE_TABLE_UPDATE_MESSAGES);
					ok = asyncSQL->Execute(L"", [this](CAsyncSQLHandler &h, int res, const std::wstring &errMsg, INT64 affected, UINT64 fail_ok, UDB::CDBVariant & vtId) {
						if (res == 0) {
							Cache.Swap(this->m_cache);
						}
					}, [this](CAsyncSQLHandler &h, UDB::CDBVariantArray & vData) {
						auto meta = h.GetColumnInfo();
						const UDB::CDBColumnInfo &info = meta.front();
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

	private:
		static std::wstring ToWide(const VARIANT &data) {
			const char *s;
			assert(data.vt == (VT_ARRAY | VT_I1));
			::SafeArrayAccessData(data.parray, (void**)&s);
			std::wstring ws = Utilities::ToWide(s, data.parray->rgsabound->cElements);
			::SafeArrayUnaccessData(data.parray);
			return ws;
		}

	private:
		CTableCache m_cache;
	};

	template<unsigned int serviceId, typename TCache>
	TCache CMasterPool<serviceId, TCache>::Cache;

}; //namespace SPA

#endif