
#ifndef __SOCKETPRO_REAL_TIME_CACHE_H_
#define __SOCKETPRO_REAL_TIME_CACHE_H_

#include <map>
#include "udatabase.h"

namespace SPA {

    namespace ServerSide {
        template<typename THandler, typename TCache>
        class CMasterPool;
    };

    typedef std::pair<UDB::CDBColumnInfoArray, UDB::CDBVariantArray> CPColumnRowset; //meta and data array for a rowset
    typedef std::vector<CPColumnRowset> CRowsetArray;
    typedef std::pair<std::wstring, std::wstring> CPDbTable; //DB and Table name pair
    typedef std::map<unsigned int, UDB::CDBColumnInfo> CKeyMap; //ordinal and column info map

    class CTableCache {
    public:
        CTableCache();

        static const size_t INVALID_VALUE = ((size_t) (~0));

    public:
        std::vector<CPDbTable> GetDbTablePair();
        UDB::CDBColumnInfoArray GetColumMeta(const wchar_t *dbName, const wchar_t *tblName);
        size_t GetRowCount(const wchar_t *dbName, const wchar_t *tblName);
        size_t GetColumnCount(const wchar_t *dbName, const wchar_t *tblName);
        std::string GetDBServerIp();
        std::wstring GetDBServerName();
        std::wstring GetUpdater();
        bool IsEmpty();
        bool Utf8ToW();
        bool HighPrecsionTime();
        UDB::tagManagementSystem GetManagementSystem();

        void Swap(CTableCache &tc);
        void AddEmptyRowset(const UDB::CDBColumnInfoArray &meta);

        CKeyMap FindKeys(const wchar_t *dbName, const wchar_t *tblName);

        //find a row based on one or two keys and equal operation
        UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
        UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);
        UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
        UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);
        size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count);
        size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, UDB::CDBVariantArray &vData);

        size_t UpdateARow(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);

        //you may define other methods for >, >=, < and <= to search for rows

    private:
        UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key);
        UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT &key1);
        UDB::CDBVariant Convert(const VARIANT &data, VARTYPE vtTarget);
        static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta);
        static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta, size_t &key1);

    protected:
        CUCriticalSection m_cs;
        CRowsetArray m_ds;

    private:
        std::string m_strIp;
        bool m_bWide;
        std::wstring m_strHostName;
        std::wstring m_strUpdater;
        UDB::tagManagementSystem m_ms;
#ifdef WIN32_64
        bool m_bTimeEx;
#endif
    private:
        CTableCache(const CTableCache &tc);
        CTableCache& operator=(const CTableCache &tc);

        template<typename THandler, typename TCache>
        friend class ServerSide::CMasterPool;
    };
}; //namespace SPA

#endif