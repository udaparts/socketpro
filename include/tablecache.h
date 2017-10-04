
#ifndef __SOCKETPRO_REAL_TIME_CACHE_H_
#define __SOCKETPRO_REAL_TIME_CACHE_H_

#include <map>
#include "udatabase.h"

namespace SPA {
    typedef std::pair<UDB::CDBColumnInfoArray, UDB::CDBVariantArray> CPColumnRowset; //meta and data array for a rowset
	
	class CDataSet;

	class CTable : protected CPColumnRowset {
	public:

		enum Operator {
			equal = 0,
			great,
			less,
			great_equal,
			less_equal
		};

		static const int BAD_ORDINAL = -1;
		static const int BAD_DATA_TYPE = -2;


		CTable() 
			: m_bFieldNameCaseSensitive(false),
			m_bDataCaseSensitive(false) {
		}

		CTable(const UDB::CDBColumnInfoArray &meta, bool bFieldNameCaseSensitive, bool bDataCaseSensitive)
			: CPColumnRowset(meta, UDB::CDBVariantArray()),
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
		const UDB::CDBColumnInfoArray& GetMeta() const { return first; }
		const UDB::CDBVariantArray& GetData() const { return second; }

		int Find(size_t ordinal, Operator op, const CComVariant &vt, CTable &tbl) const;
		int Find(size_t ordinal, Operator op, const VARIANT &vt, CTable &tbl) const;
		int Between(size_t ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl) const;
		int Between(size_t ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl) const;
		int Append(const CTable &tbl);

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
        void Swap(CDataSet &tc);
        void AddEmptyRowset(const UDB::CDBColumnInfoArray &meta);
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

        size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count);
        size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, UDB::CDBVariantArray &vData);
        size_t UpdateARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
        size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);

		static std::string ToDate(const VARIANT &vtDate);

    private:
        static UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key);
        static UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT &key1);
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