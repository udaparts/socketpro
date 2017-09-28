
#pragma once

#include "../../../include/udatabase.h"
//using namespace SPA::UDB;

class CTableCache
{
public:
	CTableCache();

	typedef std::pair<SPA::UDB::CDBColumnInfoArray, SPA::UDB::CDBVariantArray> CPColumnRowset;
	typedef std::vector<CPColumnRowset> CRowsetArray;

public:
	void Empty();
	void Swap(CTableCache &tc);
	SPA::UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const SPA::UDB::CDBVariant &key, size_t *cols = nullptr);
	SPA::UDB::CDBVariantArray FindARow(const wchar_t *dbName, const wchar_t *tblName, const SPA::UDB::CDBVariant &key0, const SPA::UDB::CDBVariant &key1, size_t *cols = nullptr);
	void AddEmptyRowset(const SPA::UDB::CDBColumnInfoArray &meta);
	size_t AddRows(const wchar_t *dbName, const wchar_t *tblName, SPA::UDB::CDBVariantArray &vData);

private:
	static size_t FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta);
	static size_t FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta, size_t &key1);

private:
	SPA::CUCriticalSection m_cs;
	CRowsetArray m_ds;

private:
	CTableCache(const CTableCache &tc);
	CTableCache& operator=(const CTableCache &tc);
};

