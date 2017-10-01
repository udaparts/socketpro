
#include "tablecache.h"

namespace SPA
{

    CTableCache::CTableCache() : m_bWide(false), m_ms(UDB::msUnknown)
#ifdef WIN32_64
            , m_bTimeEx(true)
#endif
    {

    }

    void CTableCache::Swap(CTableCache & tc) {
        CAutoLock al(m_cs);
        m_ds.swap(tc.m_ds);
        m_strIp.swap(tc.m_strIp);
        m_strHostName.swap(tc.m_strHostName);
        m_strUpdater.swap(tc.m_strUpdater);
        bool b = m_bWide;
        m_bWide = tc.m_bWide;
        tc.m_bWide = b;
        UDB::tagManagementSystem ms = m_ms;
        m_ms = tc.m_ms;
        tc.m_ms = ms;

#ifdef WIN32_64
        b = m_bTimeEx;
        m_bTimeEx = tc.m_bTimeEx;
        tc.m_bTimeEx = b;
#endif
    }

    void CTableCache::AddEmptyRowset(const UDB::CDBColumnInfoArray & meta) {
        if (!meta.size())
            return;
        CAutoLock al(m_cs);
        m_ds.push_back(CPColumnRowset(meta, UDB::CDBVariantArray()));
    }

    size_t CTableCache::AddRows(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count) {
        if (!pvt || !count)
            return 0;
        if (!dbName || !tblName)
            return INVALID_VALUE;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                if (count % col_count)
                    return INVALID_VALUE;
                UDB::CDBVariantArray &row_data = pr.second;
                for (size_t n = 0; n < count; ++n) {
                    row_data.push_back(Convert(pvt[n], meta[n % col_count].DataType));
                }
                return (count / col_count);
            }
        }
        return INVALID_VALUE; //not found
    }

    size_t CTableCache::AddRows(const wchar_t *dbName, const wchar_t *tblName, UDB::CDBVariantArray & vData) {
        size_t count = vData.size();
        if (!count)
            return 0;
        if (!dbName || !tblName)
            return INVALID_VALUE;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                if (count % col_count)
                    return INVALID_VALUE;
                UDB::CDBVariantArray &row_data = pr.second;
                for (size_t n = 0; n < count; ++n) {
                    if (m_bWide && vData[n].vt == VT_BSTR)
                        row_data.push_back(std::move(vData[n]));
                    else
                        row_data.push_back(Convert(vData[n], meta[n % col_count].DataType));
                }
                return (count / col_count);
            }
        }
        return INVALID_VALUE; //not found
    }

    UDB::CDBVariant * CTableCache::FindARowInternal(CPColumnRowset &pcr, const VARIANT & key) {
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

    UDB::CDBVariant * CTableCache::FindARowInternal(CPColumnRowset &pcr, const VARIANT &key0, const VARIANT & key1) {
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

    UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT & key) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                size_t keyIndex = FindKeyColIndex(meta);
                if (keyIndex == INVALID_VALUE)
                    return UDB::CDBVariantArray();
                const UDB::CDBVariantArray &vData = pr.second;
                size_t rows = pr.second.size() / col_count;
                for (size_t r = 0; r < rows; ++r) {
                    const UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
                    if (vtKey == key)
                        return UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
                }
                break;
            }
        }
        return UDB::CDBVariantArray();
    }

    size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT & vtKey) {
        size_t deleted = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                size_t key = FindKeyColIndex(meta);
                if (key == INVALID_VALUE)
                    return INVALID_VALUE;
                UDB::CDBVariantArray &vData = pr.second;
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

    size_t CTableCache::UpdateARow(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count) {
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
                        row[n] = Convert(pvt[2 * n + 1], meta[n].DataType); //??? date time in string
                    }
                    updated = 1;
                }
                break;
            }
        }
        return updated;
    }

    size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &vtKey0, const VARIANT & vtKey1) {
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

    size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant & key) {
        return DeleteARow(dbName, tblName, (const VARIANT&) key);
    }

    size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant & key1) {
        return DeleteARow(dbName, tblName, (const VARIANT&) key0, (const VARIANT&) key1);
    }

    size_t CTableCache::FindKeyColIndex(const UDB::CDBColumnInfoArray & meta) {
        size_t index = 0;
        for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it, ++index) {
            if ((it->Flags & (UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                return index;
        }
        return INVALID_VALUE;
    }

    UDB::CDBVariant CTableCache::Convert(const VARIANT &data, VARTYPE vtTarget) {
        switch (data.vt) {
            case VT_NULL:
            case VT_EMPTY:
                return UDB::CDBVariant();
            case VT_BOOL:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.boolVal ? true : false);
                    case VT_I1:
                        return (char) (data.boolVal ? 1 : 0);
                    case VT_UI1:
                        return (unsigned char) (data.boolVal ? 1 : 0);
                    case VT_I2:
                        return (short) (data.boolVal ? 1 : 0);
                    case VT_UI2:
                        return (unsigned short) (data.boolVal ? 1 : 0);
                    case VT_INT:
                    case VT_I4:
                        return (int) (data.boolVal ? 1 : 0);
                    case VT_UINT:
                    case VT_UI4:
                        return (unsigned int) (data.boolVal ? 1 : 0);
                    case VT_I8:
                        return (INT64) (data.boolVal ? 1 : 0);
                    case VT_UI8:
                        return (UINT64) (data.boolVal ? 1 : 0);
                    case VT_R4:
                        return (float) (data.boolVal ? 1 : 0);
                    case VT_R8:
                        return (double) (data.boolVal ? 1 : 0);
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_UI1:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.bVal ? true : false);
                    case VT_UI1:
                        return data.bVal;
                    case VT_UI2:
                        return (unsigned short) data.bVal;
                    case VT_UINT:
                    case VT_UI4:
                        return (unsigned int) data.bVal;
                    case VT_UI8:
                        return (UINT64) data.bVal;
                    case VT_R4:
                        return (float) data.bVal;
                    case VT_R8:
                        return (double) data.bVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_I1:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.cVal ? true : false);
                    case VT_I1:
                        return data.cVal;
                    case VT_I2:
                        return (short) data.cVal;
                    case VT_INT:
                    case VT_I4:
                        return (int) data.cVal;
                    case VT_I8:
                        return (INT64) data.cVal;
                    case VT_R4:
                        return (float) data.cVal;
                    case VT_R8:
                        return (double) data.cVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_UI2:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.uiVal ? true : false);
                        break;
                    case VT_UI1:
                        return (unsigned char) data.uiVal;
                    case VT_UI2:
                        return data.uiVal;
                    case VT_UINT:
                    case VT_UI4:
                        return (unsigned int) data.uiVal;
                    case VT_UI8:
                        return (UINT64) data.uiVal;
                    case VT_R4:
                        return (float) data.uiVal;
                    case VT_R8:
                        return (double) data.uiVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_I2:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.iVal ? true : false);
                        break;
                    case VT_I1:
                        return (char) data.iVal;
                    case VT_I2:
                        return data.iVal;
                    case VT_INT:
                    case VT_I4:
                        return (int) data.iVal;
                    case VT_I8:
                        return (INT64) data.iVal;
                    case VT_R4:
                        return (float) data.iVal;
                    case VT_R8:
                        return (double) data.iVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_INT:
            case VT_I4:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.lVal ? true : false);
                    case VT_I1:
                        return (char) data.lVal;
                    case VT_I2:
                        return (short) data.lVal;
                    case VT_INT:
                    case VT_I4:
                        return (int) data.lVal;
                    case VT_I8:
                        return (INT64) data.lVal;
                    case VT_R8:
                        return (double) data.lVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_UINT:
            case VT_UI4:
                switch (vtTarget) {
                    case VT_BOOL:
                        return (data.ulVal ? true : false);
                    case VT_UI1:
                        return (unsigned char) data.ulVal;
                    case VT_UI2:
                        return (unsigned short) data.ulVal;
                    case VT_UINT:
                    case VT_UI4:
                        return (unsigned int) data.ulVal;
                    case VT_UI8:
                        return (UINT64) data.ulVal;
                    case VT_R8:
                        return (double) data.ulVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_UI8:
                switch (vtTarget) {
                    case VT_BOOL:
                        return UDB::CDBVariant((data.ullVal ? true : false));
                    case VT_UI1:
                        return (unsigned char) data.ullVal;
                    case VT_UI2:
                        return (unsigned short) data.ullVal;
                    case VT_UINT:
                    case VT_UI4:
                        return (unsigned int) data.ullVal;
                    case VT_UI8:
                        return data;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_I8:
                if (m_ms == UDB::msMysql) {
                    switch (vtTarget) {
                        case VT_BOOL:
                            return (data.llVal ? true : false);
                        case VT_I1:
                            return (char) data.llVal;
                        case VT_UI1:
                            return (unsigned char) data.llVal;
                        case VT_I2:
                            return (short) data.llVal;
                        case VT_UI2:
                            return (unsigned short) data.llVal;
                        case VT_INT:
                        case VT_I4:
                            return (int) data.llVal;
                        case VT_UINT:
                        case VT_UI4:
                            return (unsigned int) data.llVal;
                        case VT_I8:
                            return data.llVal;
                        case VT_UI8:
                            return (UINT64) data.llVal;
                        default:
                            //shoudn't come here
                            assert(false); //you may need to implement these cases
                            break;
                    }
                } else {
                    switch (vtTarget) {
                        case VT_BOOL:
                            return (data.llVal ? true : false);
                        case VT_I1:
                            return (char) data.llVal;
                        case VT_I2:
                            return (short) data.llVal;
                        case VT_INT:
                        case VT_I4:
                            return (int) data.llVal;
                        case VT_I8:
                            return data.llVal;
                        default:
                            //shoudn't come here
                            assert(false); //you may need to implement these cases
                            break;
                    }
                }
                break;
            case VT_R8:
                switch (vtTarget) {
                    case VT_R8:
                        return data.dblVal;
                    case VT_R4:
                        return (float) data.dblVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case VT_R4:
                switch (vtTarget) {
                    case VT_R4:
                        return data.fltVal;
                    case VT_R8:
                        return (double) data.fltVal;
                    default:
                        //shoudn't come here
                        assert(false); //you may need to implement these cases
                        break;
                }
                break;
            case (VT_ARRAY | VT_I1):
                if (vtTarget == data.vt || vtTarget == VT_BSTR) {
                    if (m_bWide || vtTarget == VT_BSTR) {
                        const char *s;
                        CScopeUQueue sb;
                        ::SafeArrayAccessData(data.parray, (void**) &s);
                        Utilities::ToWide(s, data.parray->rgsabound->cElements, *sb);
                        ::SafeArrayUnaccessData(data.parray);
                        return (const wchar_t *) sb->GetBuffer();
                    }
                    return data; //
                } else if (vtTarget == VT_DATE) {
                    const char *s;
                    ::SafeArrayAccessData(data.parray, (void**) &s);
                    std::string str(s, s + data.parray->rgsabound->cElements);
                    ::SafeArrayUnaccessData(data.parray);
                    UDateTime dt(str.c_str());
#ifdef WIN32_64
                    UDB::CDBVariant vtDate(dt);
                    if (!m_bTimeEx) {
                        vtDate.date = dt.GetVariantDate();
                    }
                    return vtDate;
#else
                    return dt;
#endif
                } else {
                    //shoudn't come here
                    assert(false); //you may need to implement these cases
                }
                break;
            case VT_BSTR:
                if (!m_bWide && vtTarget == (VT_ARRAY | VT_I1)) {
                    CScopeUQueue sb;
                    Utilities::ToUTF8(data.bstrVal, ::wcslen(data.bstrVal), *sb);
                    return (const char*) sb->GetBuffer();
                }
                break;
            default:
                break;
        }
        assert(data.vt == vtTarget);
        return UDB::CDBVariant(data);
    }

    size_t CTableCache::FindKeyColIndex(const UDB::CDBColumnInfoArray &meta, size_t & key1) {
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

    std::vector<CPDbTable> CTableCache::GetDBTablePair() {
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

    UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant & key) {
        return FindARow(dbName, tblName, (const VARIANT &) key);
    }

    UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant & key1) {
        return FindARow(dbName, tblName, (const VARIANT &) key0, (const VARIANT &) key1);
    }

    CKeyMap CTableCache::FindKeys(const wchar_t *dbName, const wchar_t * tblName) {
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

    UDB::CDBColumnInfoArray CTableCache::GetColumMeta(const wchar_t *dbName, const wchar_t * tblName) {
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

    bool CTableCache::IsEmpty() {
        CAutoLock al(m_cs);
        return (m_ds.size() == 0);
    }

	void CTableCache::Empty() {
		CAutoLock al(m_cs);
		return m_ds.clear();
	}

    bool CTableCache::Utf8ToW() {
        CAutoLock al(m_cs);
        return m_bWide;
    }

    UDB::tagManagementSystem CTableCache::GetDBManagementSystem() {
        CAutoLock al(m_cs);
        return m_ms;
    }

    bool CTableCache::HighPrecsionTime() {
#ifdef WIN32_64
        CAutoLock al(m_cs);
        return m_bTimeEx;
#else
        return true;
#endif
    }

    std::string CTableCache::GetDBServerIp() {
        CAutoLock al(m_cs);
        return m_strIp;
    }

    std::wstring CTableCache::GetDBServerName() {
        CAutoLock al(m_cs);
        return m_strHostName;
    }

    std::wstring CTableCache::GetUpdater() {
        CAutoLock al(m_cs);
        return m_strUpdater;
    }

	void CTableCache::Set(const char *strIp, bool bWide, bool bHighPrecision, UDB::tagManagementSystem ms) {
		CAutoLock al(m_cs);
		if (strIp)
			m_strIp = strIp;
		else
			m_strIp.clear();
		m_bWide = bWide;
		m_ms = ms;
#ifdef WIN32_64
		m_bTimeEx = bHighPrecision;
#endif
	}

	void CTableCache::SetDBServerName(const wchar_t *strDBServerName) {
		CAutoLock al(m_cs);
		if (strDBServerName)
			m_strHostName = strDBServerName;
		else
			m_strHostName.clear();
	}
	void CTableCache::SetUpdater(const wchar_t *strUpdater) {
		CAutoLock al(m_cs);
		if (strUpdater)
			m_strUpdater = strUpdater;
		else
			m_strUpdater.clear();
	}

    size_t CTableCache::GetColumnCount(const wchar_t *dbName, const wchar_t * tblName) {
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

    size_t CTableCache::GetRowCount(const wchar_t *dbName, const wchar_t * tblName) {
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

    UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT & key1) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CPColumnRowset &pr = *it;
            const UDB::CDBColumnInfoArray &meta = pr.first;
            const UDB::CDBColumnInfo &col = meta.front();
            if (col.DBPath == dbName && col.TablePath == tblName) {
                size_t col_count = meta.size();
                size_t nKey1;
                size_t key = FindKeyColIndex(meta, nKey1);
                if (key == INVALID_VALUE || nKey1 == INVALID_VALUE)
                    return UDB::CDBVariantArray();
                const UDB::CDBVariantArray &vData = pr.second;
                size_t rows = pr.second.size() / col_count;
                for (size_t r = 0; r < rows; ++r) {
                    const UDB::CDBVariant &vtKey0 = vData[r * col_count + key];
                    const UDB::CDBVariant &vtKey1 = vData[r * col_count + nKey1];
                    if (vtKey0 == key0 && vtKey1 == key1)
                        return UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
                }
                break;
            }
        }
        return UDB::CDBVariantArray();
    }

} //namespace SPA
