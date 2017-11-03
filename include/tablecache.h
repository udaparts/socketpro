
#ifndef __SOCKETPRO_REAL_TIME_CACHE_H_
#define __SOCKETPRO_REAL_TIME_CACHE_H_

#include <map>
#include <memory>
#include "udatabase.h"

namespace SPA {

    typedef UDB::CDBVariantArray CRow;
    typedef std::shared_ptr<CRow> CPRow;
    typedef std::vector<CPRow> CDataMatrix;

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
            is_null
        };

        static const int BAD_ORDINAL = -1;
        static const int BAD_DATA_TYPE = -2;
        static const int OPERATION_NOT_SUPPORTED = -3;
        static const int COMPARISON_NOT_SUPPORTED = -4;
        static const int NO_TABLE_NAME_GIVEN = -5;
        static const int NO_TABLE_FOUND = -6;

        static const unsigned int INVALID_ORDINAL = (unsigned int) (-1);

        CTable()
        : m_bFieldNameCaseSensitive(false),
        m_bDataCaseSensitive(false) {
        }

        CTable(const UDB::CDBColumnInfoArray &meta, bool bFieldNameCaseSensitive, bool bDataCaseSensitive)
        : CPColumnRowset(meta, CDataMatrix()),
        m_bFieldNameCaseSensitive(bFieldNameCaseSensitive),
        m_bDataCaseSensitive(bDataCaseSensitive) {
        }

        CTable(const CTable &tbl)
        : CPColumnRowset(tbl),
        m_bFieldNameCaseSensitive(tbl.m_bFieldNameCaseSensitive),
        m_bDataCaseSensitive(tbl.m_bDataCaseSensitive) {
        }

    public:
        CTable& operator=(const CTable &tbl);

        const UDB::CDBColumnInfoArray& GetMeta() const {
            return first;
        }

        const CDataMatrix& GetDataMatrix() const {
            return second;
        }

        int Find(unsigned int ordinal, Operator op, const CComVariant &vt, CTable &tbl, bool copyData = false) const;
        int Find(unsigned int ordinal, Operator op, const VARIANT &vt, CTable &tbl, bool copyData = false) const;
        int Between(unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl, bool copyData = false) const;
        int Between(unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl, bool copyData = false) const;
        int Append(const CTable &tbl);
        int Sort(unsigned int ordinal, bool desc = false);
        unsigned int FindOrdinal(const wchar_t *colName) const;
        unsigned int FindOrdinal(const char *colName) const;

    protected:
        int gt(const VARIANT &vt0, const VARIANT &vt1) const;
        int ge(const VARIANT &vt0, const VARIANT &vt1) const;
        int lt(const VARIANT &vt0, const VARIANT &vt1) const;
        int le(const VARIANT &vt0, const VARIANT &vt1) const;
        int eq(const VARIANT &vt0, const VARIANT &vt1) const;
        static HRESULT ChangeType(const VARIANT &vtSrc, VARTYPE vtTarget, VARIANT &vtDes);

    protected:
        bool m_bFieldNameCaseSensitive;
        bool m_bDataCaseSensitive;

    private:
        friend class CDataSet;
    };

    typedef std::vector<CTable> CRowsetArray;
    typedef std::pair<std::wstring, std::wstring> CPDbTable; //DB and Table name pair
    typedef std::map<unsigned int, UDB::CDBColumnInfo> CKeyMap; //ordinal and column info map

    class CDataSet {
    public:
        CDataSet();

        static const size_t INVALID_VALUE = ((size_t) (~0));

    public:
        std::vector<CPDbTable> GetDBTablePair();
        UDB::CDBColumnInfoArray GetColumMeta(const wchar_t *dbName, const wchar_t *tblName);
        size_t GetRowCount(const wchar_t *dbName, const wchar_t *tblName);
        size_t GetColumnCount(const wchar_t *dbName, const wchar_t *tblName);
        std::string GetDBServerIp();
        std::wstring GetDBServerName();
        std::wstring GetUpdater();
        bool IsEmpty();
        UDB::tagManagementSystem GetDBManagementSystem();
        void Set(const char *strIp, UDB::tagManagementSystem ms);
        void SetDBServerName(const wchar_t *strDBServerName);
        void SetUpdater(const wchar_t *strUpdater);
        void Empty();
        CKeyMap FindKeys(const wchar_t *dbName, const wchar_t *tblName);
        void SetDBNameCaseSensitive(bool bCaseSensitive);
        void SetTableNameCaseSensitive(bool bCaseSensitive);
        void SetFieldNameCaseSensitive(bool bCaseSensitive);
        void SetDataCaseSensitive(bool bCaseSensitive);
        bool GetDBNameCaseSensitive();
        bool GetTableNameCaseSensitive();
        bool GetFieldNameCaseSensitive();
        bool GetDataCaseSensitive();
        unsigned int FindOrdinal(const wchar_t *dbName, const wchar_t *tblName, const wchar_t *colName);
        unsigned int FindOrdinal(const char *dbName, const char *tblName, const char *colName);
        int Find(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable::Operator op, const CComVariant &vt, CTable &tbl);
        int Find(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable::Operator op, const VARIANT &vt, CTable &tbl);
        int Between(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl);
        int Between(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);
        static std::string ToDate(const VARIANT &vtDate);

		virtual void Swap(CDataSet &tc);
		virtual void AddEmptyRowset(const UDB::CDBColumnInfoArray &meta);
		virtual size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count);
		virtual size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, const UDB::CDBVariantArray &vData);
		virtual size_t UpdateARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count);
		virtual size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
		virtual size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);

    private:
        static CPRow FindARowInternal(CPColumnRowset &pcr, const VARIANT &key);
        static CPRow FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT &key1);
        static UDB::CDBVariant Convert(const VARIANT &data, VARTYPE vtTarget);
        static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta);
        static size_t FindKeyColIndex(const UDB::CDBColumnInfoArray &meta, size_t &key1);

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