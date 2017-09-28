
#include "tablecache.h"

CTableCache::CTableCache() {
}

void CTableCache::Empty() {
	SPA::CAutoLock al(m_cs);
	m_ds.clear();
}

void CTableCache::Swap(CTableCache &tc) {
	SPA::CAutoLock al(m_cs);
	m_ds.swap(tc.m_ds);
}

void CTableCache::AddEmptyRowset(const SPA::UDB::CDBColumnInfoArray &meta) {
	if (!meta.size())
		return;
	SPA::CAutoLock al(m_cs);
	m_ds.push_back(CPColumnRowset(meta, SPA::UDB::CDBVariantArray()));
}

size_t CTableCache::AddRows(const wchar_t *dbName, const wchar_t *tblName, SPA::UDB::CDBVariantArray &vData) {
	if (!dbName || !tblName || !vData.size())
		return (~0);
	SPA::CAutoLock al(m_cs);
	for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
		CPColumnRowset &pr = *it;
		const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
		const SPA::UDB::CDBColumnInfo &col = meta.front();
		if (col.DBPath == dbName && col.OriginalName == tblName) {
			size_t col_count = meta.size();
			if (vData.size() % col_count)
				return (~0);
			SPA::UDB::CDBVariantArray &row_data = pr.second;
			for (size_t n = 0; n < vData.size(); ++n) {
				auto &d = vData[n];
				row_data.push_back(std::move(d)); //avoid memory repeatedly allocation/de-allocation for better performance
			}
			return (row_data.size() / col_count);
		}
	}
	return (~0); //not found
}

SPA::UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const SPA::UDB::CDBVariant &key, size_t *cols) {
	if (cols)
		*cols = 0;
	SPA::CAutoLock al(m_cs);
	for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
		const CPColumnRowset &pr = *it;
		const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
		const SPA::UDB::CDBColumnInfo &col = meta.front();
		if (col.DBPath == dbName && col.OriginalName == tblName) {
			size_t col_count = meta.size();
			if (cols)
				*cols = col_count;
			size_t keyIndex = FindKeyColIndex(meta);
			if (keyIndex == (~0))
				return SPA::UDB::CDBVariantArray();
			const SPA::UDB::CDBVariantArray &vData = pr.second;
			size_t rows = pr.second.size() / col_count;
			for (size_t r = 0; r < rows; ++r) {
				const SPA::UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
				if (key.vt == VT_BSTR && key.vt == vtKey.vt) {
					//case-insensitive compare
					int res = ::_wcsicmp(vtKey.bstrVal, key.bstrVal);
					if (res == 0)
						return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
				}
				else if (key.vt == (VT_I1 | VT_ARRAY) && key.vt == vtKey.vt) {
					if (key.parray->rgsabound->cElements == vtKey.parray->rgsabound->cElements) {
						const char *s0;
						const char *s1;
						::SafeArrayAccessData(key.parray, (void**)&s0);
						::SafeArrayAccessData(vtKey.parray, (void**)&s1);
						//case-insensitive compare
						bool equal = (::_strnicmp(s0, s1, key.parray->rgsabound->cElements) == 0);
						::SafeArrayUnaccessData(vtKey.parray);
						::SafeArrayUnaccessData(key.parray);
						if (equal)
							return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
					}
				}
				else if (SPA::IsEqual(key, vtKey)) {
					return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
				}
			}
			break;
		}
	}
	return SPA::UDB::CDBVariantArray();
}

size_t CTableCache::FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta) {
	size_t index = 0;
	for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it, ++index) {
		if ((it->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
			return index;
	}
	return (~0);
}

size_t CTableCache::FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta, size_t &key1) {
	size_t index = (~0);
	key1 = (~0);
	for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it) {
		if (index == (~0)) {
			if ((it->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
				index = it - meta.cbegin();
		}
		else {
			if ((it->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT))) {
				key1 = it - meta.cbegin();
				break;
			}
		}
	}
	return index;
}

SPA::UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const SPA::UDB::CDBVariant &key0, const SPA::UDB::CDBVariant &key1, size_t *cols) {
	if (cols)
		*cols = 0;
	SPA::CAutoLock al(m_cs);
	for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
		const CPColumnRowset &pr = *it;
		const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
		const SPA::UDB::CDBColumnInfo &col = meta.front();
		if (col.DBPath == dbName && col.OriginalName == tblName) {
			size_t col_count = meta.size();
			if (cols)
				*cols = col_count;
			size_t nKey1;
			size_t key = FindKeyColIndex(meta, nKey1);
			if (key == (~0) || nKey1 == (~0))
				return SPA::UDB::CDBVariantArray();
			const SPA::UDB::CDBVariantArray &vData = pr.second;
			size_t rows = pr.second.size() / col_count;
			for (size_t r = 0; r < rows; ++r) {
				const SPA::UDB::CDBVariant &vtKey0 = vData[r * col_count + key];
				if (key0.vt == VT_BSTR && key0.vt == vtKey0.vt) {
					//case-insensitive compare
					int res = ::_wcsicmp(vtKey0.bstrVal, key0.bstrVal);
					if (res != 0)
						continue;
				}
				else if (key0.vt == (VT_I1 | VT_ARRAY) && key0.vt == vtKey0.vt) {
					if (key0.parray->rgsabound->cElements == vtKey0.parray->rgsabound->cElements) {
						const char *s0;
						const char *s1;
						::SafeArrayAccessData(key0.parray, (void**)&s0);
						::SafeArrayAccessData(vtKey0.parray, (void**)&s1);
						//case-insensitive compare
						bool equal = (::_strnicmp(s0, s1, key0.parray->rgsabound->cElements) == 0);
						::SafeArrayUnaccessData(vtKey0.parray);
						::SafeArrayUnaccessData(key0.parray);
						if (!equal)
							continue;
					}
				}
				else if (!SPA::IsEqual(key0, vtKey0)) {
					continue;
				}

				const SPA::UDB::CDBVariant &vtKey1 = vData[r * col_count + nKey1];
				if (key1.vt == VT_BSTR && key1.vt == vtKey1.vt) {
					//case-insensitive compare
					int res = ::_wcsicmp(vtKey0.bstrVal, key1.bstrVal);
					if (res == 0)
						return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
				}
				else if (key1.vt == (VT_I1 | VT_ARRAY) && key1.vt == vtKey1.vt) {
					if (key1.parray->rgsabound->cElements == vtKey1.parray->rgsabound->cElements) {
						const char *s0;
						const char *s1;
						::SafeArrayAccessData(key1.parray, (void**)&s0);
						::SafeArrayAccessData(vtKey1.parray, (void**)&s1);
						//case-insensitive compare
						bool equal = (::_strnicmp(s0, s1, key1.parray->rgsabound->cElements) == 0);
						::SafeArrayUnaccessData(vtKey1.parray);
						::SafeArrayUnaccessData(key1.parray);
						if (equal)
							return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
					}
				}
				else if (SPA::IsEqual(key1, vtKey1)) {
					return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
				}
			}
			break;
		}
	}
	return SPA::UDB::CDBVariantArray();
}
