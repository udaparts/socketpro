
#ifndef __SOCKETPRO_REAL_TIME_CACHE_H_
#define __SOCKETPRO_REAL_TIME_CACHE_H_

#include <map>
#include <memory>
#include "udatabase.h"

namespace SPA {

    typedef UDB::CDBVariantArray CRow;
    typedef std::shared_ptr<CRow> CPRow;
    typedef std::vector<CPRow> CDataMatrix;

    typedef std::map<unsigned int, UDB::CDBColumnInfo> CKeyMap; //ordinal and column info map
    typedef std::pair<UDB::CDBColumnInfoArray, CDataMatrix> CPColumnRowset; //meta and data array for a rowset

    class CDataSet;

    class CTable : protected CPColumnRowset {
    public:

        enum Operator {
            equal = 0,
            great,
            less,
            great_equal,
            less_equal,
            not_equal,
            is_null
        };

        static const int BAD_ORDINAL = -1;
        static const int BAD_DATA_TYPE = -2;
        static const int OPERATION_NOT_SUPPORTED = -3;
        static const int COMPARISON_NOT_SUPPORTED = -4;
        static const int NO_TABLE_NAME_GIVEN = -5;
        static const int NO_TABLE_FOUND = -6;

        static const unsigned int INVALID_ORDINAL = (unsigned int) (-1);

        CTable();
        CTable(const UDB::CDBColumnInfoArray &meta, bool bFieldNameCaseSensitive, bool bDataCaseSensitive);
        CTable(const CTable &tbl);

    public:
        CTable& operator=(const CTable &tbl);
        const UDB::CDBColumnInfoArray& GetMeta() const;
        const CDataMatrix& GetDataMatrix() const;
        CKeyMap GetKeys() const;

        int Find(unsigned int ordinal, Operator op, const CComVariant &vt, CTable &tbl, bool copyData = false) const;
        int Find(unsigned int ordinal, Operator op, const VARIANT &vt, CTable &tbl, bool copyData = false) const;
        int FindNull(unsigned int ordinal, CTable &tbl, bool copyData = false) const;
        int In(unsigned int ordinal, const UDB::CDBVariantArray &v, CTable &tbl, bool copyData = false) const;
        int NotIn(unsigned int ordinal, const UDB::CDBVariantArray &v, CTable &tbl, bool copyData = false) const;
        int Between(unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl, bool copyData = false) const;
        int Between(unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl, bool copyData = false) const;
        int Append(const CTable &tbl);
        int Sort(unsigned int ordinal, bool desc = false);
        unsigned int FindOrdinal(const UTF16 *colName) const;
        unsigned int FindOrdinal(const char *colName) const;

    private:
        int gt(const VARIANT &vt0, const VARIANT &vt1) const;
        int ge(const VARIANT &vt0, const VARIANT &vt1) const;
        int lt(const VARIANT &vt0, const VARIANT &vt1) const;
        int le(const VARIANT &vt0, const VARIANT &vt1) const;
        int eq(const VARIANT &vt0, const VARIANT &vt1) const;
        int neq(const VARIANT &vt0, const VARIANT &vt1) const;
        bool In(const UDB::CDBVariantArray &v, const VARIANT &v0) const;
        static HRESULT ChangeType(const VARIANT &vtSrc, VARTYPE vtTarget, VARIANT &vtDes);

    private:
        bool m_bFieldNameCaseSensitive;
        bool m_bDataCaseSensitive;

    private:
        friend class CDataSet;
    };

    typedef std::vector<CTable> CRowsetArray;
    typedef std::pair<CDBString, CDBString> CPDbTable; //DB and Table name pair

    class CDataSet {
    public:
        CDataSet();

        static const size_t INVALID_VALUE = ((size_t) (~0));

    public:
        std::vector<CPDbTable> GetDBTablePair();
        UDB::CDBColumnInfoArray GetColumMeta(const UTF16 *dbName, const UTF16 *tblName);
        size_t GetRowCount(const UTF16 *dbName, const UTF16 *tblName);
        size_t GetColumnCount(const UTF16 *dbName, const UTF16 *tblName);
        std::string GetDBServerIp();
        std::wstring GetDBServerName();
        std::wstring GetUpdater();
        bool IsEmpty();
        UDB::tagManagementSystem GetDBManagementSystem();
        void Set(const char *strIp, UDB::tagManagementSystem ms);
        void SetDBServerName(const wchar_t *strDBServerName);
        void SetUpdater(const wchar_t *strUpdater);
        void Empty();
        CKeyMap FindKeys(const UTF16 *dbName, const UTF16 *tblName);
        void SetDBNameCaseSensitive(bool bCaseSensitive);
        void SetTableNameCaseSensitive(bool bCaseSensitive);
        void SetFieldNameCaseSensitive(bool bCaseSensitive);
        void SetDataCaseSensitive(bool bCaseSensitive);
        bool GetDBNameCaseSensitive();
        bool GetTableNameCaseSensitive();
        bool GetFieldNameCaseSensitive();
        bool GetDataCaseSensitive();
        unsigned int FindOrdinal(const UTF16 *dbName, const UTF16 *tblName, const UTF16 *colName);
        unsigned int FindOrdinal(const char *dbName, const char *tblName, const char *colName);
        int Find(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, CTable::Operator op, const CComVariant &vt, CTable &tbl);
        int Find(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, CTable::Operator op, const VARIANT &vt, CTable &tbl);
        int FindNull(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, CTable &tbl);
        int In(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, const UDB::CDBVariantArray &v, CTable &tbl);
        int NotIn(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, const UDB::CDBVariantArray &v, CTable &tbl);
        int Between(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl);
        int Between(const UTF16 *dbName, const UTF16 *tblName, unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl);
        size_t DeleteARow(const UTF16 *dbName, const UTF16 *tblName, const CComVariant &key);


        static std::string ToDate(const VARIANT &vtDate);

        /**
         * Swap internal data structure with tc. Track cache data initialization event by overriding this method
         * @param tc A valid Dataset object
         */
        virtual void Swap(CDataSet &tc);

        /**
         * Add an empty rowset from a given column meta data for cache. Track the event that a new rowset is added into a cache by overriding this method
         * @param meta A meta data for a rowset
         */
        virtual void AddEmptyRowset(const UDB::CDBColumnInfoArray &meta);

        /**
         * Add one or more rows into cache. Track the event of adding rows into cache by overriding this method
         * @param dbName A database name string
         * @param tblName A table name string
         * @param pvt A pointer to a data array
         * @param count The number of data in array
         * @return The number of rows added into cache. It could also be 0 and INVALID_VALUE
         */
        virtual size_t AddRows(const UTF16 *dbName, const UTF16 *tblName, const VARIANT *pvt, size_t count);

        /**
         * Add one or more rows into cache. Track the event of adding rows into cache by overriding this method
         * @param dbName A database name string
         * @param tblName A table name string
         * @param vData A data array
         * @return The number of rows added into cache. It could also be 0 and INVALID_VALUE
         */
        virtual size_t AddRows(const UTF16 *dbName, const UTF16 *tblName, const UDB::CDBVariantArray &vData);

        /**
         * Update a row data into cache. Track update event by overriding this method
         * @param dbName A database name string
         * @param tblName A table name string
         * @param pvt A pointer to an array of data containing both old and new values (old,new,old,new, ......) for one row
         * @param count
         * @return The number of updated rows, which could be 0, 1 or INVALID_VALUE
         */
        virtual size_t UpdateARow(const UTF16 *dbName, const UTF16 *tblName, const VARIANT *pvt, size_t count);

        /**
         * Delete one row from cache from one given key value. Track delete event by overriding this method
         * @param dbName A database name string
         * @param tblName A table name string
         * @param key One given key value
         * @return The number of deleted rows, which could be 0, 1 or INVALID_VALUE
         */
        virtual size_t DeleteARow(const UTF16 *dbName, const UTF16 *tblName, const VARIANT &key);

        /**
         * Delete one row from cache from one given row of data. Track delete event by overriding this method
         * @param dbName A database name string
         * @param tblName A table name string
         * @param pRow a pointer to a row of data
         * @param cols the column number
         * @return The number of deleted rows, which could be 0, 1 or INVALID_VALUE
         */
        virtual size_t DeleteARow(const UTF16 *dbName, const UTF16 *tblName, const VARIANT *pRow, unsigned int cols);

#if defined(WCHAR32) || _MSC_VER >= 1900
        UDB::CDBColumnInfoArray GetColumMeta(const wchar_t *dbName, const wchar_t *tblName);
        size_t GetRowCount(const wchar_t *dbName, const wchar_t *tblName);
        size_t GetColumnCount(const wchar_t *dbName, const wchar_t *tblName);
        unsigned int FindOrdinal(const wchar_t *dbName, const wchar_t *tblName, const wchar_t *colName);
        int Find(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable::Operator op, const CComVariant &vt, CTable &tbl);
        int Find(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable::Operator op, const VARIANT &vt, CTable &tbl);
        int FindNull(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable &tbl);
        int In(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const UDB::CDBVariantArray &v, CTable &tbl);
        int NotIn(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const UDB::CDBVariantArray &v, CTable &tbl);
        int Between(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl);
        int Between(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
        size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count);
        size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, const UDB::CDBVariantArray &vData);
        size_t UpdateARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pRow, unsigned int cols);
#endif

    private:
        static CPRow FindARowInternal(const CTable &tbl, size_t f, const VARIANT &key);
        static CPRow FindARowInternal(const CTable &tbl, size_t f0, size_t f1, const VARIANT &key0, const VARIANT &key1);
        static UDB::CDBVariant Convert(const VARIANT &data, VARTYPE vtTarget);
        static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta);
        static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta, size_t &key1);
        bool Is(const CTable &tbl, const UTF16 *dbName, const UTF16 *tblName);

    protected:
        CUCriticalSection m_cs;
        CRowsetArray m_ds;

    private:
        std::string m_strIp;
        std::wstring m_strHostName;
        std::wstring m_strUpdater;
        UDB::tagManagementSystem m_ms;
        bool m_bDBNameCaseSensitive;
        bool m_bTableNameCaseSensitive;
        bool m_bFieldNameCaseSensitive;
        bool m_bDataCaseSensitive;

    private:
        CDataSet(const CDataSet &tc);
        CDataSet& operator=(const CDataSet &tc);
    };
}; //namespace SPA

#endif