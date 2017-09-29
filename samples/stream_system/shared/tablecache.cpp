
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

size_t CTableCache::AddRows(const wchar_t *dbName, const wchar_t *tblName, VARIANT *pvt, size_t count) {
    if (!pvt || !count)
        return 0;
    if (!dbName || !tblName)
        return INVALID_NUMBER;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
        CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            if (count % col_count)
                return INVALID_NUMBER;
            SPA::UDB::CDBVariantArray &row_data = pr.second;
            for (size_t n = 0; n < count; ++n) {
                row_data.push_back(SPA::UDB::CDBVariant(pvt[n])); //avoid memory repeatedly allocation/de-allocation for better performance
            }
            return (count / col_count);
        }
    }
    return INVALID_NUMBER; //not found
}

size_t CTableCache::AddRows(const wchar_t *dbName, const wchar_t *tblName, SPA::UDB::CDBVariantArray &vData) {
    size_t count = vData.size();
    if (!count)
        return 0;
    if (!dbName || !tblName)
        return INVALID_NUMBER;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
        CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            if (count % col_count)
                return INVALID_NUMBER;
            SPA::UDB::CDBVariantArray &row_data = pr.second;
            for (size_t n = 0; n < count; ++n) {
                row_data.push_back(std::move(vData[n])); //avoid memory repeatedly allocation/de-allocation for better performance
            }
            return (count / col_count);
        }
    }
    return INVALID_NUMBER; //not found
}

SPA::UDB::CDBVariant* CTableCache::FindARowInternal(CTableCache::CPColumnRowset &pcr, const VARIANT &key) {
    size_t col_count = pcr.first.size();
    size_t keyIndex = FindKeyColIndex(pcr.first);
    if (keyIndex == INVALID_NUMBER)
        return nullptr;
    SPA::UDB::CDBVariantArray &vData = pcr.second;
    size_t rows = pcr.second.size() / col_count;
    for (size_t r = 0; r < rows; ++r) {
        const SPA::UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
        if (vtKey == key)
            return vData.data() + r * col_count;
    }
    return nullptr;
}

SPA::UDB::CDBVariant* CTableCache::FindARowInternal(CTableCache::CPColumnRowset &pcr, const VARIANT &key0, const VARIANT &key1) {
    size_t col_count = pcr.first.size();
    size_t key;
    size_t keyIndex = FindKeyColIndex(pcr.first, key);
    if (keyIndex == INVALID_NUMBER || key == INVALID_NUMBER)
        return nullptr;
    SPA::UDB::CDBVariantArray &vData = pcr.second;
    size_t rows = pcr.second.size() / col_count;
    for (size_t r = 0; r < rows; ++r) {
        const SPA::UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
        const SPA::UDB::CDBVariant &vtKey1 = vData[r * col_count + key];
        if (vtKey == key0 && vtKey1 == key1)
            return vData.data() + r * col_count;
    }
    return nullptr;
}

SPA::UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key) {
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            size_t keyIndex = FindKeyColIndex(meta);
            if (keyIndex == INVALID_NUMBER)
                return SPA::UDB::CDBVariantArray();
            const SPA::UDB::CDBVariantArray &vData = pr.second;
            size_t rows = pr.second.size() / col_count;
            for (size_t r = 0; r < rows; ++r) {
                const SPA::UDB::CDBVariant &vtKey = vData[r * col_count + keyIndex];
                if (vtKey == key)
                    return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
            }
            break;
        }
    }
    return SPA::UDB::CDBVariantArray();
}

size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &vtKey) {
    size_t deleted = 0;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
        CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            size_t key = FindKeyColIndex(meta);
            if (key == INVALID_NUMBER)
                return INVALID_NUMBER;
            SPA::UDB::CDBVariantArray &vData = pr.second;
            size_t rows = vData.size() / col_count;
            for (size_t r = 0; r < rows; ++r) {
                const SPA::UDB::CDBVariant &vtKey0 = vData[r * col_count + key];
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
        return INVALID_NUMBER;
    size_t updated = 0;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
        CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            if (count % col_count || 2 * col_count != count)
                return INVALID_NUMBER;
            size_t key1;
            SPA::UDB::CDBVariant *row;
            size_t key0 = FindKeyColIndex(meta, key1);
            if (key0 == INVALID_NUMBER && key1 == INVALID_NUMBER)
                return INVALID_NUMBER;
            else if (key1 == INVALID_NUMBER) {
                row = FindARowInternal(*it, pvt[key0 * 2]);
            } else {
                row = FindARowInternal(*it, pvt[key0 * 2], pvt[key1 * 2]);
            }
            if (row) {
                for (unsigned int n = 0; n < col_count; ++n) {
                    row[n] = pvt[2 * n + 1]; //??? date time in string
                }
                updated = 1;
            }
            break;
        }
    }
    return updated;
}

size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &vtKey0, const VARIANT &vtKey1) {
    size_t deleted = 0;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
        CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            size_t key1;
            size_t key = FindKeyColIndex(meta, key1);
            if (key == INVALID_NUMBER || key1 == INVALID_NUMBER)
                return INVALID_NUMBER;
            SPA::UDB::CDBVariantArray &vData = pr.second;
            size_t rows = vData.size() / col_count;
            for (size_t r = 0; r < rows; ++r) {
                const SPA::UDB::CDBVariant &vtKey = vData[r * col_count + key];
                const SPA::UDB::CDBVariant &vt2 = vData[r * col_count + key1];
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

size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key) {
    return DeleteARow(dbName, tblName, (const VARIANT&) key);
}

size_t CTableCache::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1) {
    return DeleteARow(dbName, tblName, (const VARIANT&) key0, (const VARIANT&) key1);
}

size_t CTableCache::FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta) {
    size_t index = 0;
    for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it, ++index) {
        if ((it->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
            return index;
    }
    return INVALID_NUMBER;
}

size_t CTableCache::FindKeyColIndex(const SPA::UDB::CDBColumnInfoArray &meta, size_t &key1) {
    size_t index = INVALID_NUMBER;
    key1 = INVALID_NUMBER;
    for (auto it = meta.cbegin(), end = meta.cend(); it != end; ++it) {
        if (index == INVALID_NUMBER) {
            if ((it->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                index = it - meta.cbegin();
        } else {
            if ((it->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT))) {
                key1 = it - meta.cbegin();
                break;
            }
        }
    }
    return index;
}

std::vector<CTableCache::CPDbTable> CTableCache::GetDbTablePair() {
    std::vector<CPDbTable> v;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        v.push_back(CPDbTable(col.DBPath, col.TablePath));
    }
    return v;
}

SPA::UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key) {
    return FindARow(dbName, tblName, (const VARIANT &) key);
}

SPA::UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant &key0, const CComVariant &key1) {
    return FindARow(dbName, tblName, (const VARIANT &) key0, (const VARIANT &) key1);
}

CTableCache::CKeyMap CTableCache::FindKeys(const wchar_t *dbName, const wchar_t *tblName) {
    CKeyMap map;
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            unsigned int col = 0;
            for (auto m = meta.cbegin(), mend = meta.cend(); m != mend; ++m, ++col) {
                if ((m->Flags & (SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                    map[col] = *m;
            }
            break;
        }
    }
    return map;
}

SPA::UDB::CDBColumnInfoArray CTableCache::GetColumMeta(const wchar_t *dbName, const wchar_t *tblName) {
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            return meta;
        }
    }
    return SPA::UDB::CDBColumnInfoArray();
}

std::string CTableCache::GetDBServerIp() {
    SPA::CAutoLock al(m_cs);
    return m_strIp;
}

void CTableCache::SetDBServerIp(const char *ip) {
    SPA::CAutoLock al(m_cs);
    if (ip)
        m_strIp = ip;
    else
        m_strIp.clear();
}

size_t CTableCache::GetColumnCount(const wchar_t *dbName, const wchar_t *tblName) {
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            return meta.size();
        }
    }
    return INVALID_NUMBER;
}

size_t CTableCache::GetRowCount(const wchar_t *dbName, const wchar_t *tblName) {
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            return pr.second.size() / meta.size();
        }
    }
    return INVALID_NUMBER;
}

SPA::UDB::CDBVariantArray CTableCache::FindARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT &key0, const VARIANT &key1) {
    SPA::CAutoLock al(m_cs);
    for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
        const CPColumnRowset &pr = *it;
        const SPA::UDB::CDBColumnInfoArray &meta = pr.first;
        const SPA::UDB::CDBColumnInfo &col = meta.front();
        if (col.DBPath == dbName && col.TablePath == tblName) {
            size_t col_count = meta.size();
            size_t nKey1;
            size_t key = FindKeyColIndex(meta, nKey1);
            if (key == INVALID_NUMBER || nKey1 == INVALID_NUMBER)
                return SPA::UDB::CDBVariantArray();
            const SPA::UDB::CDBVariantArray &vData = pr.second;
            size_t rows = pr.second.size() / col_count;
            for (size_t r = 0; r < rows; ++r) {
                const SPA::UDB::CDBVariant &vtKey0 = vData[r * col_count + key];
                const SPA::UDB::CDBVariant &vtKey1 = vData[r * col_count + nKey1];
                if (vtKey0 == key0 && vtKey1 == key1)
                    return SPA::UDB::CDBVariantArray(vData.data() + r * col_count, vData.data() + (r + 1) * col_count);
            }
            break;
        }
    }
    return SPA::UDB::CDBVariantArray();
}
