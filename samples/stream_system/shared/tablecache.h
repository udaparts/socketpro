
#pragma once

#include "../../../include/udatabase.h"
#include <map>

class CTableCache
{
public:
	CTableCache();

	typedef std::pair<SPA::UDB::CDBColumnInfoArray, SPA::UDB::CDBVariantArray> CPColumnRowset;
	typedef std::vector<CPColumnRowset> CRowsetArray;
	typedef std::pair<std::wstring, std::wstring> CPDbTable; //DB and Table name pair
	typedef std::map<unsigned int, SPA::UDB::CDBColumnInfo> CKeyMap;

public:
	void Empty();
	void Swap(CTableCache &tc);
	SPA::UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
	SPA::UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);
	SPA::UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
	SPA::UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);
	size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key);
	size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1);
	size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key);
	size_t DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1);
	size_t UpdateARow(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count);
	void AddEmptyRowset(const SPA::UDB::CDBColumnInfoArray &meta);
	size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, SPA::UDB::CDBVariantArray &vData);
	size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count);
	std::vector<CPDbTable> GetDbTablePair();
	CKeyMap FindKeys(const wchar_t *dbName, const wchar_t *tblName);
	SPA::UDB::CDBColumnInfoArray GetColumMeta(const wchar_t *dbName, const wchar_t *tblName);
	size_t GetRowCount(const wchar_t *dbName, const wchar_t *tblName);
	size_t GetColumnCount(const wchar_t *dbName, const wchar_t *tblName);
	std::string GetDBServerIp();
	void SetDBServerIp(const char *ip);

private:
	SPA::UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key);
	SPA::UDB::CDBVariant* FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT &key1);
	
	static size_t FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta);
	static size_t FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta, size_t &key1);

private:
	SPA::CUCriticalSection m_cs;
	CRowsetArray m_ds;
	std::string m_strIp;

private:
	CTableCache(const CTableCache &tc);
	CTableCache& operator=(const CTableCache &tc);
};

