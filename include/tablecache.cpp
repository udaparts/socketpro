
#include "tablecache.h"

namespace SPA
{
	CTable& CTable::operator = (const CTable &tbl) {
		if (this == &tbl)
			return *this;
		CPColumnRowset &base = *this;
		base = tbl;
		m_bFieldNameCaseSensitive = tbl.m_bFieldNameCaseSensitive;
		m_bDataCaseSensitive = tbl.m_bDataCaseSensitive;
		return *this;
	}

	int CTable::Find(size_t ordinal, CTable::Operator op, const CComVariant &vt, CTable &tbl) const {
		return Find(ordinal, op, (const VARIANT &)vt, tbl);
	}

	int CTable::Find(size_t ordinal, CTable::Operator op, const VARIANT &vt, CTable &tbl) const {
		tbl.second.clear();
		tbl.first = first;
		tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
		tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
		return -1;
	}

	int CTable::Between(size_t ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable &tbl) const {
		return Between(ordinal, (const VARIANT&)vt0, (const VARIANT&)vt1, tbl);
	}

	int CTable::Between(size_t ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable &tbl) const {
		tbl.second.clear();
		tbl.first = first;
		tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
		tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
		if (ordinal >= tbl.first.size())
			return BAD_ORDINAL;


		return -1;
	}

	int CTable::Append(const CTable &tbl) {
		return -1;
	}

	CDataSet::CDataSet() 
		: m_ms(UDB::msUnknown),
		m_bDBNameCaseSensitive(false),
		m_bTableNameCaseSensitive(false),
		m_bFieldNameCaseSensitive(false),
		m_bDataCaseSensitive(false) {
    }

    void CDataSet::Swap(CDataSet & tc) {
        CAutoLock al(m_cs);
        m_ds.swap(tc.m_ds);
        m_strIp.swap(tc.m_strIp);
        m_strHostName.swap(tc.m_strHostName);
        m_strUpdater.swap(tc.m_strUpdater);
        UDB::tagManagementSystem ms = m_ms;
        m_ms = tc.m_ms;
        tc.m_ms = ms;
    }

    void CDataSet::AddEmptyRowset(const UDB::CDBColumnInfoArray & meta) {
        if (!meta.size())
            return;
        CAutoLock al(m_cs);
		m_ds.push_back(CTable(meta, m_bFieldNameCaseSensitive, m_bDataCaseSensitive));
    }

    size_t CDataSet::AddRows(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count) {
        if (!pvt || !count)
            return 0;
        if (!dbName || !tblName)
            return INVALID_VALUE;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            const UDB::CDBColumnInfoArray &meta = it->GetMeta();
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                if (count % col_count)
                    return INVALID_VALUE;
				UDB::CDBVariantArray &row_data = it->second;
                for (size_t n = 0; n < count; ++n) {
                    const VARIANT &vt = pvt[n];
                    if (vt.vt == (VT_ARRAY | VT_I1)) {
                        const char *s;
                        CComVariant vtNew;
                        ::SafeArrayAccessData(vt.parray, (void**) &s);
                        vtNew.bstrVal = SPA::Utilities::ToBSTR(s, vt.parray->rgsabound->cElements);
                        vtNew.vt = VT_BSTR;
                        ::SafeArrayUnaccessData(vt.parray);
                        VARTYPE vtTarget = meta[n % col_count].DataType;
                        if (vtTarget == (VT_I1 | VT_ARRAY))
                            vtTarget = VT_BSTR;
                        row_data.push_back(Convert(vtNew, vtTarget));
                    } else {
                        row_data.push_back(Convert(vt, meta[n % col_count].DataType));
                    }
                }
                return (count / col_count);
            }
        }
        return INVALID_VALUE; //not found
    }

    size_t CDataSet::AddRows(const wchar_t *dbName, const wchar_t *tblName, UDB::CDBVariantArray & vData) {
        size_t count = vData.size();
        if (!count)
            return 0;
        if (!dbName || !tblName)
            return INVALID_VALUE;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            const UDB::CDBColumnInfoArray &meta = it->GetMeta();
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                if (count % col_count)
                    return INVALID_VALUE;
                UDB::CDBVariantArray &row_data = it->second;
                for (size_t n = 0; n < count; ++n) {
                    VARTYPE vtTarget = meta[n % col_count].DataType;
                    if (vtTarget == (VT_I1 | VT_ARRAY))
                        vtTarget = VT_BSTR;
                    row_data.push_back(Convert(vData[n], vtTarget));
                }
                return (count / col_count);
            }
        }
        return INVALID_VALUE; //not found
    }

    UDB::CDBVariant * CDataSet::FindARowInternal(CPColumnRowset &pcr, const VARIANT & key) {
        size_t col_count = pcr.first.size();
        size_t keyIndex = FindKeyColIndex(pcr.first);
        if (keyIndex == INVALID_VALUE)
            return nullptr;
        UDB::CDBVariantArray &vData = pcr.second;
        size_t rows = pcr.second.size() / col_count;
        for (size_t r = 0; r < rows; ++r) {
            const UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
            if (vtKey == key)
                return vData.data() + r * col_count;
        }
        return nullptr;
    }

    UDB::CDBVariant * CDataSet::FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT & key1) {
        size_t col_count = pcr.first.size();
        size_t key;
        size_t keyIndex = FindKeyColIndex(pcr.first, key);
        if (keyIndex == INVALID_VALUE || key == INVALID_VALUE)
            return nullptr;
        UDB::CDBVariantArray &vData = pcr.second;
        size_t rows = pcr.second.size() / col_count;
        for (size_t r = 0; r < rows; ++r) {
            const UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
            const UDB::CDBVariant &vtKey1 = vData[r * col_count + key];
            if (vtKey == key0 && vtKey1 == key1)
                return vData.data() + r * col_count;
        }
        return nullptr;
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT & vtKey) {
        size_t deleted = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            const UDB::CDBColumnInfoArray &meta = it->GetMeta();
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                size_t key = FindKeyColIndex(meta);
                if (key == INVALID_VALUE)
                    return INVALID_VALUE;
                UDB::CDBVariantArray &vData = it->second;
                size_t rows = vData.size() / col_count;
                for (size_t r = 0; r < rows; ++r) {
                    const UDB::CDBVariant &vtKey0 = vData[r * col_count + key];
                    if (vtKey0 == vtKey) {
                        vData.erase(vData.begin() + r * col_count, vData.begin() + (r + 1) * col_count);
                        deleted = col_count;
                        break;
                    }
                }
                break;
            }
        }
        return deleted;
    }

    size_t CDataSet::UpdateARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count) {
        if (!pvt || !count || count % 2)
            return INVALID_VALUE;
        size_t updated = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                if (count % col_count || 2 * col_count != count)
                    return INVALID_VALUE;
                size_t key1;
                UDB::CDBVariant *row;
                size_t key0 = FindKeyColIndex(meta, key1);
                if (key0 == INVALID_VALUE && key1 == INVALID_VALUE)
                    return INVALID_VALUE;
                else if (key1 == INVALID_VALUE) {
                    row = FindARowInternal(*it, pvt[key0 * 2]);
                } else {
                    row = FindARowInternal(*it, pvt[key0 * 2], pvt[key1 * 2]);
                }
                if (row) {
                    for (unsigned int n = 0; n < col_count; ++n) {
                        const VARIANT &vt = pvt[2 * n + 1];
                        if (vt.vt == (VT_ARRAY | VT_I1)) {
                            const char *s;
                            CComVariant vtNew;
                            ::SafeArrayAccessData(vt.parray, (void**) &s);
                            vtNew.bstrVal = SPA::Utilities::ToBSTR(s, vt.parray->rgsabound->cElements);
                            vtNew.vt = VT_BSTR;
                            ::SafeArrayUnaccessData(vt.parray);
                            VARTYPE vtTarget = meta[n].DataType;
                            if (vtTarget == (VT_I1 | VT_ARRAY))
                                vtTarget = VT_BSTR;
                            row[n] = Convert(vtNew, vtTarget);
                        } else {
                            row[n] = Convert(vt, meta[n].DataType);
                        }
                    }
                    updated = 1;
                }
                break;
            }
        }
        return updated;
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &vtKey0, const VARIANT & vtKey1) {
        size_t deleted = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                size_t key1;
                size_t key = FindKeyColIndex(meta, key1);
                if (key == INVALID_VALUE || key1 == INVALID_VALUE)
                    return INVALID_VALUE;
                UDB::CDBVariantArray &vData = pr.second;
                size_t rows = vData.size() / col_count;
                for (size_t r = 0; r < rows; ++r) {
                    const UDB::CDBVariant &vtKey = vData[r * col_count + key];
                    const UDB::CDBVariant &vt2 = vData[r * col_count + key1];
                    if (vtKey == vtKey0 && vt2 == vtKey1) {
                        vData.erase(vData.begin() + r * col_count, vData.begin() + (r + 1) * col_count);
                        deleted = col_count;
                        break;
                    }
                }
                break;
            }
        }
        return deleted;
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant & key) {
        return DeleteARow(dbName, tblName, (const VARIANT&) key);
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant & key1) {
        return DeleteARow(dbName, tblName, (const VARIANT&) key0, (const VARIANT&) key1);
    }

    size_t CDataSet::FindKeyColIndex(const UDB::CDBColumnInfoArray & meta) {
        size_t index = 0;
        for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it, ++index) {
            if ((it->Flags & (UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                return index;
        }
        return INVALID_VALUE;
    }

    UDB::CDBVariant CDataSet::Convert(const VARIANT &data, VARTYPE vtTarget) {
        assert(vtTarget != (VT_ARRAY | VT_I1)); //no ASCII string!
        if (data.vt == vtTarget)
            return data;
        switch (data.vt) {
            case VT_NULL:
            case VT_EMPTY:
                return UDB::CDBVariant();
            default:
                break;
        }
        UDB::CDBVariant vt;
        HRESULT hr = ::VariantChangeType(&vt, &data, 0, vtTarget);
        assert(S_OK == hr);
        return vt;
    }

    size_t CDataSet::FindKeyColIndex(const UDB::CDBColumnInfoArray &meta, size_t & key1) {
        size_t index = INVALID_VALUE;
        key1 = INVALID_VALUE;
        for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it) {
            if (index == INVALID_VALUE) {
                if ((it->Flags & (UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                    index = it - meta.cbegin();
            } else {
                if ((it->Flags & (UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | UDB::CDBColumnInfo::FLAG_AUTOINCREMENT))) {
                    key1 = it - meta.cbegin();
                    break;
                }
            }
        }
        return index;
    }

    std::vector<CPDbTable> CDataSet::GetDBTablePair() {
        std::vector<CPDbTable> v;
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            v.push_back(CPDbTable(col.DBPath, col.TablePath));
        }
        return v;
    }

    std::string CDataSet::ToDate(const VARIANT & vtDate) {
        assert(vtDate.vt == VT_DATE);
#ifdef WIN32_64
        assert(vtDate.dblVal < 1.0 / (3600000 * 24)); //must be in high precision time format
#endif
        UDateTime dt(vtDate.ullVal);
        return dt.ToDBString();
    }

    CKeyMap CDataSet::FindKeys(const wchar_t *dbName, const wchar_t * tblName) {
        CKeyMap map;
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                unsigned int col = 0;
                for (auto m = meta.cbegin(), mend = meta.cend(); m != mend; ++m, ++col) {
                    if ((m->Flags & (UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                        map[col] = *m;
                }
                break;
            }
        }
        return map;
    }

    UDB::CDBColumnInfoArray CDataSet::GetColumMeta(const wchar_t *dbName, const wchar_t * tblName) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                return meta;
            }
        }
        return UDB::CDBColumnInfoArray();
    }

    bool CDataSet::IsEmpty() {
        CAutoLock al(m_cs);
        return (m_ds.size() == 0);
    }

    void CDataSet::Empty() {
        CAutoLock al(m_cs);
        return m_ds.clear();
    }

    UDB::tagManagementSystem CDataSet::GetDBManagementSystem() {
        CAutoLock al(m_cs);
        return m_ms;
    }

    std::string CDataSet::GetDBServerIp() {
        CAutoLock al(m_cs);
        return m_strIp;
    }

    std::wstring CDataSet::GetDBServerName() {
        CAutoLock al(m_cs);
        return m_strHostName;
    }

    std::wstring CDataSet::GetUpdater() {
        CAutoLock al(m_cs);
        return m_strUpdater;
    }

	void CDataSet::SetDBNameCaseSensitive(bool bCaseSensitive) {
		CAutoLock al(m_cs);
		m_bDBNameCaseSensitive = bCaseSensitive;
	}

	void CDataSet::SetTableNameCaseSensitive(bool bCaseSensitive) {
		CAutoLock al(m_cs);
		m_bTableNameCaseSensitive = bCaseSensitive;
	}

	void CDataSet::SetFieldNameCaseSensitive(bool bCaseSensitive) {
		CAutoLock al(m_cs);
		m_bFieldNameCaseSensitive = bCaseSensitive;
	}

	void CDataSet::SetDataCaseSensitive(bool bCaseSensitive) {
		CAutoLock al(m_cs);
		m_bDataCaseSensitive = bCaseSensitive;
	}

	bool CDataSet::GetDBNameCaseSensitive() {
		CAutoLock al(m_cs);
		return m_bDBNameCaseSensitive;
	}

	bool CDataSet::GetTableNameCaseSensitive() {
		CAutoLock al(m_cs);
		return m_bTableNameCaseSensitive;
	}

	bool CDataSet::GetFieldNameCaseSensitive() {
		CAutoLock al(m_cs);
		return m_bFieldNameCaseSensitive;
	}

	bool CDataSet::GetDataCaseSensitive() {
		CAutoLock al(m_cs);
		return m_bDataCaseSensitive;
	}

    void CDataSet::Set(const char *strIp, UDB::tagManagementSystem ms) {
        CAutoLock al(m_cs);
        if (strIp)
            m_strIp = strIp;
        else
            m_strIp.clear();
        m_ms = ms;
    }

    void CDataSet::SetDBServerName(const wchar_t * strDBServerName) {
        CAutoLock al(m_cs);
        if (strDBServerName)
            m_strHostName = strDBServerName;
        else
            m_strHostName.clear();
    }

    void CDataSet::SetUpdater(const wchar_t * strUpdater) {
        CAutoLock al(m_cs);
        if (strUpdater)
            m_strUpdater = strUpdater;
        else
            m_strUpdater.clear();
    }

    size_t CDataSet::GetColumnCount(const wchar_t *dbName, const wchar_t * tblName) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                return meta.size();
            }
        }
        return INVALID_VALUE;
    }

    size_t CDataSet::GetRowCount(const wchar_t *dbName, const wchar_t * tblName) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                return pr.second.size() / meta.size();
            }
        }
        return INVALID_VALUE;
    }

} //namespace SPA
