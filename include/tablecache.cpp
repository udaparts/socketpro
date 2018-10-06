
#include "tablecache.h"
#include <algorithm>

namespace SPA {

    CTable::CTable()
    : m_bFieldNameCaseSensitive(false),
    m_bDataCaseSensitive(false) {
    }

    CTable::CTable(const UDB::CDBColumnInfoArray &meta, bool bFieldNameCaseSensitive, bool bDataCaseSensitive)
    : CPColumnRowset(meta, CDataMatrix()),
    m_bFieldNameCaseSensitive(bFieldNameCaseSensitive),
    m_bDataCaseSensitive(bDataCaseSensitive) {
    }

    CTable::CTable(const CTable & tbl)
    : CPColumnRowset(tbl),
    m_bFieldNameCaseSensitive(tbl.m_bFieldNameCaseSensitive),
    m_bDataCaseSensitive(tbl.m_bDataCaseSensitive) {
    }

    const UDB::CDBColumnInfoArray & CTable::GetMeta() const {
        return first;
    }

    const CDataMatrix & CTable::GetDataMatrix() const {
        return second;
    }

    CKeyMap CTable::GetKeys() const {
        CKeyMap map;
        const UDB::CDBColumnInfoArray &meta = first;
        unsigned int col = 0;
        for (auto m = meta.cbegin(), mend = meta.cend(); m != mend; ++m, ++col) {
            if ((m->Flags & (UDB::CDBColumnInfo::FLAG_PRIMARY_KEY | UDB::CDBColumnInfo::FLAG_AUTOINCREMENT)))
                map[col] = *m;
        }
        return map;
    }

    CTable & CTable::operator=(const CTable & tbl) {
        if (this == &tbl)
            return *this;
        CPColumnRowset &base = *this;
        base = tbl;
        m_bFieldNameCaseSensitive = tbl.m_bFieldNameCaseSensitive;
        m_bDataCaseSensitive = tbl.m_bDataCaseSensitive;
        return *this;
    }

    int CTable::Find(unsigned int ordinal, CTable::Operator op, const CComVariant &vt, CTable & tbl, bool copyData) const {
        return Find(ordinal, op, (const VARIANT &) vt, tbl, copyData);
    }

    HRESULT CTable::ChangeType(const VARIANT &vtSrc, VARTYPE vtTarget, VARIANT & vtDes) {
        if (vtSrc.vt == (VT_ARRAY | VT_I1) && vtTarget == VT_BSTR) {
            const char *s;
            ::SafeArrayAccessData(vtSrc.parray, (void**) &s);
            vtDes.vt = VT_BSTR;
            vtDes.bstrVal = Utilities::ToBSTR(s, vtSrc.parray->rgsabound->cElements);
            ::SafeArrayUnaccessData(vtSrc.parray);
            return S_OK;
        }
        HRESULT hr = ::VariantChangeType(&vtDes, &vtSrc, 0, vtTarget);
        if (S_OK != hr)
            return BAD_DATA_TYPE;
#ifdef WIN32_64
        if (vtTarget == VT_DATE && vtDes.date > UDB::MIN_WIN_DATETIME) {
            UDateTime dt(vtDes.date);
            vtDes.ullVal = dt.time;
        }
#endif
        return hr;
    }

    int CTable::FindNull(unsigned int ordinal, CTable &tbl, bool copyData) const {
        VARIANT vt;
        ::memset(&vt, 0, sizeof (vt));
        return Find(ordinal, CTable::is_null, vt, tbl, copyData);
    }

    int CTable::In(unsigned int ordinal, const UDB::CDBVariantArray &vArray, CTable &tbl, bool copyData) const {
        tbl.second.clear();
        tbl.first = first;
        tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
        tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
        if (!vArray.size())
            return 1;
        if (ordinal >= first.size())
            return BAD_ORDINAL;
        VARTYPE type = first[ordinal].DataType;
        if (type == (VT_I1 | VT_ARRAY))
            type = VT_BSTR; //Table string is always unicode string
        UDB::CDBVariantArray v;
        for (auto it = vArray.cbegin(), end = vArray.cend(); it != end; ++it) {
            UDB::CDBVariant vt;
            HRESULT hr = ChangeType(*it, type, vt);
            if (S_OK != hr)
                return hr;
            v.push_back((UDB::CDBVariant&&)vt);
        }
        size_t cols = first.size();
        size_t rows = second.size();
        for (size_t r = 0; r < rows; ++r) {
            CPRow prow = second[r];
            const VARIANT &v0 = prow->at(ordinal);
            bool ok = (std::find_if(v.cbegin(), v.cend(), [&v0, this](const UDB::CDBVariant & v1) -> bool {
                return (this->eq(v0, v1) > 0);
            }) != v.cend());
            if (ok) {
                if (copyData) {
                    CPRow p(new CRow);
                    for (size_t n = 0; n < cols; ++n) {
                        p->push_back(prow->at(n));
                    }
                    tbl.second.push_back(p);
                } else
                    tbl.second.push_back(prow);
            }
        }
        return 1;
    }

    int CTable::NotIn(unsigned int ordinal, const UDB::CDBVariantArray &vArray, CTable &tbl, bool copyData) const {
        tbl.second.clear();
        tbl.first = first;
        tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
        tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
        if (ordinal >= first.size())
            return BAD_ORDINAL;
        VARTYPE type = first[ordinal].DataType;
        if (type == (VT_I1 | VT_ARRAY))
            type = VT_BSTR; //Table string is always unicode string
        UDB::CDBVariantArray v;
        for (auto it = vArray.cbegin(), end = vArray.cend(); it != end; ++it) {
            UDB::CDBVariant vt;
            HRESULT hr = ChangeType(*it, type, vt);
            if (S_OK != hr)
                return hr;
            v.push_back((UDB::CDBVariant&&)vt);
        }
        size_t cols = first.size();
        size_t rows = second.size();
        for (size_t r = 0; r < rows; ++r) {
            CPRow prow = second[r];
            const VARIANT &v0 = prow->at(ordinal);
            if (v0.vt > VT_NULL && !In(v, v0)) {
                if (copyData) {
                    CPRow p(new CRow);
                    for (size_t n = 0; n < cols; ++n) {
                        p->push_back(prow->at(n));
                    }
                    tbl.second.push_back(p);
                } else
                    tbl.second.push_back(prow);
            }
        }
        return 1;
    }

    int CTable::Find(unsigned int ordinal, CTable::Operator op, const VARIANT &vt, CTable & tbl, bool copyData) const {
        tbl.second.clear();
        tbl.first = first;
        tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
        tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
        if (ordinal >= first.size())
            return BAD_ORDINAL;
        if (vt.vt <= VT_NULL && op != is_null)
            return COMPARISON_NOT_SUPPORTED;
        VARTYPE type = first[ordinal].DataType;
        if (type == (VT_I1 | VT_ARRAY))
            type = VT_BSTR; //Table string is always unicode string
        CComVariant v;
        if (op != is_null) {
            HRESULT hr = ChangeType(vt, type, v);
            if (S_OK != hr)
                return hr;
        }
        size_t cols = first.size();
        size_t rows = second.size();
        for (size_t r = 0; r < rows; ++r) {
            bool ok;
            CPRow prow = second[r];
            if (op == is_null) {
                VARTYPE d = prow->at(ordinal).vt;
                ok = (d == VT_NULL || d == VT_EMPTY);
            } else {
                int res;
                const VARIANT &v0 = prow->at(ordinal);
                switch (op) {
                    case equal:
                        res = eq(v0, v);
                        break;
                    case not_equal:
                        res = neq(v0, v);
                        break;
                    case great:
                        res = gt(v0, v);
                        break;
                    case great_equal:
                        res = ge(v0, v);
                        break;
                    case less:
                        res = lt(v0, v);
                        break;
                    case less_equal:
                        res = le(v0, v);
                        break;
                    default:
                        return OPERATION_NOT_SUPPORTED;
                }
                if (res < 0)
                    return res;
                ok = (res > 0);
            }
            if (ok) {
                if (copyData) {
                    CPRow p(new CRow);
                    for (size_t n = 0; n < cols; ++n) {
                        p->push_back(prow->at(n));
                    }
                    tbl.second.push_back(p);
                } else
                    tbl.second.push_back(prow);
            }
        }
        return 1;
    }

    int CTable::Between(unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable & tbl, bool copyData) const {
        return Between(ordinal, (const VARIANT&) vt0, (const VARIANT&) vt1, tbl, copyData);
    }

    int CTable::Between(unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable & tbl, bool copyData) const {
        tbl.second.clear();
        tbl.first = first;
        tbl.m_bDataCaseSensitive = m_bDataCaseSensitive;
        tbl.m_bFieldNameCaseSensitive = m_bFieldNameCaseSensitive;
        if (ordinal >= first.size())
            return BAD_ORDINAL;
        if (vt0.vt <= VT_NULL || vt1.vt <= VT_NULL)
            return COMPARISON_NOT_SUPPORTED;
        VARTYPE type = first[ordinal].DataType;
        if (type == (VT_I1 | VT_ARRAY))
            type = VT_BSTR; //Table string is always unicode string 
        CComVariant v0, v1;
        HRESULT hr = ChangeType(vt0, type, v0);
        if (S_OK != hr)
            return hr;
        hr = ChangeType(vt1, type, v1);
        if (S_OK != hr)
            return BAD_DATA_TYPE;
        const VARIANT *small_vt = &v0;
        const VARIANT *large_vt = &v1;
        int res = gt(v0, v1);
        if (res < 0)
            return res;
        else if (res) {
            small_vt = &v1;
            large_vt = &v0;
        }
        size_t cols = first.size();
        size_t rows = second.size();
        for (size_t r = 0; r < rows; ++r) {
            CPRow prow = second[r];
            const VARIANT &vt = prow->at(ordinal);
            res = ge(vt, *small_vt);
            if (res == 0)
                continue;
            if (res < 0)
                return res;
            res = le(vt, *large_vt);
            if (res == 0)
                continue;
            if (res < 0)
                return res;
            if (copyData) {
                CPRow p(new CRow);
                for (size_t n = 0; n < cols; ++n) {
                    p->push_back(prow->at(n));
                }
                tbl.second.push_back(p);
            } else
                tbl.second.push_back(prow);
        }
        return 1;
    }

    int CTable::Sort(unsigned int ordinal, bool desc) {
        if (ordinal >= first.size())
            return BAD_ORDINAL;
        std::sort(second.begin(), second.end(), [this, ordinal, desc](const CPRow& a, const CPRow & b) -> bool {
            const UDB::CDBVariant &va = a->at(ordinal);
            const UDB::CDBVariant &vb = b->at(ordinal);
            if (vb.vt <= VT_NULL && va.vt <= VT_NULL)
                return false;
                if (desc) {
                    if (vb.vt <= VT_NULL)
                        return true;
                    else if (va.vt <= VT_NULL)
                        return false;
                        return (this->gt(va, vb) > 0);
                    }
            if (va.vt <= VT_NULL)
                return true;
            else if (vb.vt <= VT_NULL)
                return false;
                return (this->lt(va, vb) > 0);
            });
        return 1;
    }

    int CTable::Append(const CTable & tbl) {
        if (first.size() != tbl.first.size())
            return OPERATION_NOT_SUPPORTED;
        size_t index = 0;
        for (auto it = first.cbegin(), end = first.cend(); it != end; ++it, ++index) {
            if (*it != tbl.first[index])
                return OPERATION_NOT_SUPPORTED;
        }
        for (auto it = tbl.second.cbegin(), end = tbl.second.cend(); it != end; ++it) {
            second.push_back(*it);
        }
        return 1;
    }

    unsigned int CTable::FindOrdinal(const wchar_t * str) const {
        if (!str)
            return INVALID_ORDINAL;
        unsigned int ordinal = 0;
        for (auto it = first.cbegin(), end = first.cend(); it != end; ++it, ++ordinal) {
            if (m_bDataCaseSensitive) {
                if (::wcscmp(it->OriginalName.c_str(), str) == 0)
                    return ordinal;
            } else {
#ifdef WIN32_64
                if (::_wcsicmp(it->OriginalName.c_str(), str) == 0)
#else
                if (::wcscasecmp(it->OriginalName.c_str(), str) == 0)
#endif
                {
                    return ordinal;
                }
            }
        }
        return INVALID_ORDINAL;
    }

    unsigned int CTable::FindOrdinal(const char *str) const {
        if (!str)
            return INVALID_ORDINAL;
        CScopeUQueue sb;
        Utilities::ToWide(str, ::strlen(str), *sb);
        return FindOrdinal((const wchar_t*) sb->GetBuffer());
    }

    int CTable::gt(const VARIANT &vt0, const VARIANT & vt1) const {
        if (vt0.vt <= VT_NULL)
            return 0;
        assert(vt0.vt == vt1.vt);
        switch (vt0.vt) {
            case VT_I1:
                return (vt0.cVal > vt1.cVal);
            case VT_I2:
                return (vt0.iVal > vt1.iVal);
            case VT_INT:
            case VT_I4:
                return (vt0.intVal > vt1.intVal);
            case VT_I8:
                return (vt0.llVal > vt1.llVal);
            case VT_R4:
                return (vt0.fltVal > vt1.fltVal);
            case VT_R8:
                return (vt0.dblVal > vt1.dblVal);
            case VT_UI8:
            case VT_DATE:
                return (vt0.ullVal > vt1.ullVal);
            case VT_UI1:
                return (vt0.bVal > vt1.bVal);
                break;
            case VT_UI2:
                return (vt0.uiVal > vt1.uiVal);
            case VT_UINT:
            case VT_UI4:
                return (vt0.uintVal > vt1.uintVal);
            case VT_DECIMAL:
                return (ToDouble(vt0.decVal) > ToDouble(vt1.decVal));
            case VT_BSTR:
                if (m_bDataCaseSensitive) {
                    return (::wcscmp(vt0.bstrVal, vt1.bstrVal) > 0);
                } else {
#ifdef WIN32_64
                    return (::_wcsicmp(vt0.bstrVal, vt1.bstrVal) > 0);
#else
                    return (::wcscasecmp(vt0.bstrVal, vt1.bstrVal) > 0);
#endif
                }
            case VT_BOOL:
                return (vt0.boolVal < vt1.boolVal);
                break;
            case (VT_ARRAY | VT_UI1):
                if (vt0.parray->rgsabound->cElements == vt1.parray->rgsabound->cElements && vt0.parray->rgsabound->cElements == sizeof (UUID)) {
                    unsigned char *s0, *s1;
                    ::SafeArrayAccessData(vt0.parray, (void**) &s0);
                    ::SafeArrayAccessData(vt1.parray, (void**) &s1);
                    int ret = ::memcmp(s0, s1, sizeof (UUID));
                    ::SafeArrayUnaccessData(vt1.parray);
                    ::SafeArrayUnaccessData(vt0.parray);
                    return ret > 0;
                }
                break;
            default:
                break;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    int CTable::ge(const VARIANT &vt0, const VARIANT & vt1) const {
        if (vt0.vt <= VT_NULL)
            return 0;
        assert(vt0.vt == vt1.vt);
        switch (vt0.vt) {
            case VT_I1:
                return (vt0.cVal >= vt1.cVal);
            case VT_I2:
                return (vt0.iVal >= vt1.iVal);
            case VT_INT:
            case VT_I4:
                return (vt0.intVal >= vt1.intVal);
            case VT_I8:
                return (vt0.llVal >= vt1.llVal);
            case VT_R4:
                return (vt0.fltVal >= vt1.fltVal);
            case VT_R8:
                return (vt0.dblVal >= vt1.dblVal);
            case VT_UI8:
            case VT_DATE:
                return (vt0.ullVal >= vt1.ullVal);
            case VT_UI1:
                return (vt0.bVal >= vt1.bVal);
                break;
            case VT_UI2:
                return (vt0.uiVal >= vt1.uiVal);
            case VT_UINT:
            case VT_UI4:
                return (vt0.uintVal >= vt1.uintVal);
            case VT_DECIMAL:
                return (ToDouble(vt0.decVal) >= ToDouble(vt1.decVal));
            case VT_BSTR:
                if (m_bDataCaseSensitive) {
                    return (::wcscmp(vt0.bstrVal, vt1.bstrVal) >= 0);
                } else {
#ifdef WIN32_64
                    return (::_wcsicmp(vt0.bstrVal, vt1.bstrVal) >= 0);
#else
                    return (::wcscasecmp(vt0.bstrVal, vt1.bstrVal) >= 0);
#endif
                }
            case VT_BOOL:
                return (vt0.boolVal <= vt1.boolVal);
                break;
            case (VT_ARRAY | VT_UI1):
                if (vt0.parray->rgsabound->cElements == vt1.parray->rgsabound->cElements && vt0.parray->rgsabound->cElements == sizeof (UUID)) {
                    unsigned char *s0, *s1;
                    ::SafeArrayAccessData(vt0.parray, (void**) &s0);
                    ::SafeArrayAccessData(vt1.parray, (void**) &s1);
                    int ret = ::memcmp(s0, s1, sizeof (UUID));
                    ::SafeArrayUnaccessData(vt1.parray);
                    ::SafeArrayUnaccessData(vt0.parray);
                    return ret >= 0;
                }
                break;
            default:
                break;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    int CTable::lt(const VARIANT &vt0, const VARIANT & vt1) const {
        if (vt0.vt <= VT_NULL)
            return 0;
        assert(vt0.vt == vt1.vt);
        switch (vt0.vt) {
            case VT_I1:
                return (vt0.cVal < vt1.cVal);
            case VT_I2:
                return (vt0.iVal < vt1.iVal);
            case VT_INT:
            case VT_I4:
                return (vt0.intVal < vt1.intVal);
            case VT_I8:
                return (vt0.llVal < vt1.llVal);
            case VT_R4:
                return (vt0.fltVal < vt1.fltVal);
            case VT_R8:
                return (vt0.dblVal < vt1.dblVal);
            case VT_UI8:
            case VT_DATE:
                return (vt0.ullVal < vt1.ullVal);
            case VT_UI1:
                return (vt0.bVal < vt1.bVal);
                break;
            case VT_UI2:
                return (vt0.uiVal < vt1.uiVal);
            case VT_UINT:
            case VT_UI4:
                return (vt0.uintVal < vt1.uintVal);
            case VT_DECIMAL:
                return (ToDouble(vt0.decVal) < ToDouble(vt1.decVal));
            case VT_BSTR:
                if (m_bDataCaseSensitive) {
                    return (::wcscmp(vt0.bstrVal, vt1.bstrVal) < 0);
                } else {
#ifdef WIN32_64
                    return (::_wcsicmp(vt0.bstrVal, vt1.bstrVal) < 0);
#else
                    return (::wcscasecmp(vt0.bstrVal, vt1.bstrVal) < 0);
#endif
                }
            case VT_BOOL:
                return (vt0.boolVal > vt1.boolVal);
                break;
            case (VT_ARRAY | VT_UI1):
                if (vt0.parray->rgsabound->cElements == vt1.parray->rgsabound->cElements && vt0.parray->rgsabound->cElements == sizeof (UUID)) {
                    unsigned char *s0, *s1;
                    ::SafeArrayAccessData(vt0.parray, (void**) &s0);
                    ::SafeArrayAccessData(vt1.parray, (void**) &s1);
                    int ret = ::memcmp(s0, s1, sizeof (UUID));
                    ::SafeArrayUnaccessData(vt1.parray);
                    ::SafeArrayUnaccessData(vt0.parray);
                    return ret < 0;
                }
                break;
            default:
                break;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    int CTable::le(const VARIANT &vt0, const VARIANT & vt1) const {
        if (vt0.vt <= VT_NULL)
            return 0;
        assert(vt0.vt == vt1.vt);
        switch (vt0.vt) {
            case VT_I1:
                return (vt0.cVal <= vt1.cVal);
            case VT_I2:
                return (vt0.iVal <= vt1.iVal);
            case VT_INT:
            case VT_I4:
                return (vt0.intVal <= vt1.intVal);
            case VT_I8:
                return (vt0.llVal <= vt1.llVal);
            case VT_R4:
                return (vt0.fltVal <= vt1.fltVal);
            case VT_R8:
                return (vt0.dblVal <= vt1.dblVal);
            case VT_UI8:
            case VT_DATE:
                return (vt0.ullVal <= vt1.ullVal);
            case VT_UI1:
                return (vt0.bVal <= vt1.bVal);
                break;
            case VT_UI2:
                return (vt0.uiVal <= vt1.uiVal);
            case VT_UINT:
            case VT_UI4:
                return (vt0.uintVal <= vt1.uintVal);
            case VT_DECIMAL:
                return (ToDouble(vt0.decVal) <= ToDouble(vt1.decVal));
            case VT_BSTR:
                if (m_bDataCaseSensitive) {
                    return (::wcscmp(vt0.bstrVal, vt1.bstrVal) <= 0);
                } else {
#ifdef WIN32_64
                    return (::_wcsicmp(vt0.bstrVal, vt1.bstrVal) <= 0);
#else
                    return (::wcscasecmp(vt0.bstrVal, vt1.bstrVal) <= 0);
#endif
                }
            case VT_BOOL:
                return (vt0.boolVal >= vt1.boolVal);
                break;
            case (VT_ARRAY | VT_UI1):
                if (vt0.parray->rgsabound->cElements == vt1.parray->rgsabound->cElements && vt0.parray->rgsabound->cElements == sizeof (UUID)) {
                    unsigned char *s0, *s1;
                    ::SafeArrayAccessData(vt0.parray, (void**) &s0);
                    ::SafeArrayAccessData(vt1.parray, (void**) &s1);
                    int ret = ::memcmp(s0, s1, sizeof (UUID));
                    ::SafeArrayUnaccessData(vt1.parray);
                    ::SafeArrayUnaccessData(vt0.parray);
                    return ret <= 0;
                }
                break;
            default:
                break;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    bool CTable::In(const UDB::CDBVariantArray &v, const VARIANT & v0) const {
        for (auto it = v.cbegin(), end = v.cend(); it != end; ++it) {
            if (eq(*it, v0) > 0)
                return true;
        }
        return false;
    }

    int CTable::eq(const VARIANT &vt0, const VARIANT & vt1) const {
        if (vt0.vt <= VT_NULL)
            return 0;
        assert(vt0.vt == vt1.vt);
        switch (vt0.vt) {
            case VT_I1:
                return (vt0.cVal == vt1.cVal);
            case VT_I2:
                return (vt0.iVal == vt1.iVal);
            case VT_INT:
            case VT_I4:
                return (vt0.intVal == vt1.intVal);
            case VT_I8:
                return (vt0.llVal == vt1.llVal);
            case VT_R4:
                return (vt0.fltVal == vt1.fltVal);
            case VT_R8:
                return (vt0.dblVal == vt1.dblVal);
            case VT_UI8:
            case VT_DATE:
                return (vt0.ullVal == vt1.ullVal);
            case VT_UI1:
                return (vt0.bVal == vt1.bVal);
                break;
            case VT_UI2:
                return (vt0.uiVal == vt1.uiVal);
            case VT_UINT:
            case VT_UI4:
                return (vt0.uintVal == vt1.uintVal);
            case VT_DECIMAL:
                return (memcmp(&vt0.decVal, &vt1.decVal, sizeof (DECIMAL)) == 0);
            case VT_BSTR:
                if (m_bDataCaseSensitive) {
                    return (::wcscmp(vt0.bstrVal, vt1.bstrVal) == 0);
                } else {
#ifdef WIN32_64
                    return (::_wcsicmp(vt0.bstrVal, vt1.bstrVal) == 0);
#else
                    return (::wcscasecmp(vt0.bstrVal, vt1.bstrVal) == 0);
#endif
                }
            case VT_BOOL:
                return ((vt0.boolVal ? true : false) == (vt1.boolVal ? true : false));
                break;
            case (VT_ARRAY | VT_UI1):
                if (vt0.parray->rgsabound->cElements == vt1.parray->rgsabound->cElements && vt0.parray->rgsabound->cElements == sizeof (UUID)) {
                    unsigned char *s0, *s1;
                    ::SafeArrayAccessData(vt0.parray, (void**) &s0);
                    ::SafeArrayAccessData(vt1.parray, (void**) &s1);
                    int ret = ::memcmp(s0, s1, sizeof (UUID));
                    ::SafeArrayUnaccessData(vt1.parray);
                    ::SafeArrayUnaccessData(vt0.parray);
                    return ret == 0;
                }
                break;
            default:
                break;
        }
        return COMPARISON_NOT_SUPPORTED;
    }

    int CTable::neq(const VARIANT &vt0, const VARIANT & vt1) const {
        if (vt0.vt <= VT_NULL)
            return 0;
        assert(vt0.vt == vt1.vt);
        switch (vt0.vt) {
            case VT_I1:
                return (vt0.cVal != vt1.cVal);
            case VT_I2:
                return (vt0.iVal != vt1.iVal);
            case VT_INT:
            case VT_I4:
                return (vt0.intVal != vt1.intVal);
            case VT_I8:
                return (vt0.llVal != vt1.llVal);
            case VT_R4:
                return (vt0.fltVal != vt1.fltVal);
            case VT_R8:
                return (vt0.dblVal != vt1.dblVal);
            case VT_UI8:
            case VT_DATE:
                return (vt0.ullVal != vt1.ullVal);
            case VT_UI1:
                return (vt0.bVal != vt1.bVal);
                break;
            case VT_UI2:
                return (vt0.uiVal != vt1.uiVal);
            case VT_UINT:
            case VT_UI4:
                return (vt0.uintVal != vt1.uintVal);
            case VT_DECIMAL:
                return (memcmp(&vt0.decVal, &vt1.decVal, sizeof (DECIMAL)) != 0);
            case VT_BSTR:
                if (m_bDataCaseSensitive) {
                    return (::wcscmp(vt0.bstrVal, vt1.bstrVal) != 0);
                } else {
#ifdef WIN32_64
                    return (::_wcsicmp(vt0.bstrVal, vt1.bstrVal) != 0);
#else
                    return (::wcscasecmp(vt0.bstrVal, vt1.bstrVal) != 0);
#endif
                }
            case VT_BOOL:
                return (vt0.boolVal != vt1.boolVal);
                break;
            case (VT_ARRAY | VT_UI1):
                if (vt0.parray->rgsabound->cElements == vt1.parray->rgsabound->cElements && vt0.parray->rgsabound->cElements == sizeof (UUID)) {
                    unsigned char *s0, *s1;
                    ::SafeArrayAccessData(vt0.parray, (void**) &s0);
                    ::SafeArrayAccessData(vt1.parray, (void**) &s1);
                    int ret = ::memcmp(s0, s1, sizeof (UUID));
                    ::SafeArrayUnaccessData(vt1.parray);
                    ::SafeArrayUnaccessData(vt0.parray);
                    return ret != 0;
                }
                break;
            default:
                break;
        }
        return COMPARISON_NOT_SUPPORTED;
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
            if (!Is(*it, dbName, tblName))
                continue;
            const UDB::CDBColumnInfoArray &meta = it->GetMeta();
            size_t col_count = meta.size();
            if (count % col_count)
                return INVALID_VALUE;
            auto &vRow = it->second;
            CPRow prow;
            for (size_t n = 0; n < count; ++n) {
                if ((n % col_count) == 0) {
                    prow.reset(new CRow());
                    vRow.push_back(prow);
                }
                const VARIANT &vt = pvt[n];
                if (vt.vt == (VT_ARRAY | VT_I1)) {
                    const char *s;
                    CComVariant vtNew;
                    ::SafeArrayAccessData(vt.parray, (void**) &s);
                    vtNew.bstrVal = Utilities::ToBSTR(s, vt.parray->rgsabound->cElements);
                    vtNew.vt = VT_BSTR;
                    ::SafeArrayUnaccessData(vt.parray);
                    VARTYPE vtTarget = meta[n % col_count].DataType;
                    if (vtTarget == (VT_I1 | VT_ARRAY))
                        vtTarget = VT_BSTR;
                    prow->push_back(Convert(vtNew, vtTarget));
                } else {
                    prow->push_back(Convert(vt, meta[n % col_count].DataType));
                }
            }
            return (count / col_count);
        }
        return 0; //not found
    }

    size_t CDataSet::AddRows(const wchar_t *dbName, const wchar_t *tblName, const UDB::CDBVariantArray & vData) {
        size_t count = vData.size();
        if (!count)
            return 0;
        if (!dbName || !tblName)
            return INVALID_VALUE;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            if (!Is(*it, dbName, tblName))
                continue;
            const UDB::CDBColumnInfoArray &meta = it->GetMeta();
            size_t col_count = meta.size();
            if (count % col_count)
                return INVALID_VALUE;
            auto &vRow = it->second;
            CPRow prow;
            for (size_t n = 0; n < count; ++n) {
                if ((n % col_count) == 0) {
                    prow.reset(new CRow());
                    vRow.push_back(prow);
                }
                VARTYPE vtTarget = meta[n % col_count].DataType;
                if (vtTarget == (VT_I1 | VT_ARRAY))
                    vtTarget = VT_BSTR;
                prow->push_back(Convert(vData[n], vtTarget));
            }
            return (count / col_count);
        }
        return 0; //not found
    }

    CPRow CDataSet::FindARowInternal(const CTable &tbl, size_t f, const VARIANT & key) {
        auto &vRow = tbl.second;
        size_t rows = tbl.second.size();
        for (size_t r = 0; r < rows; ++r) {
            const UDB::CDBVariant &vtKey = vRow[r]->at(f);
            if (tbl.eq(vtKey, key) > 0)
                return vRow[r];
        }
        return nullptr;
    }

    CPRow CDataSet::FindARowInternal(const CTable &tbl, size_t f0, size_t f1, const VARIANT &key0, const VARIANT & key1) {
        auto &vRow = tbl.second;
        size_t rows = tbl.second.size();
        for (size_t r = 0; r < rows; ++r) {
            const UDB::CDBVariant &vtKey = vRow[r]->at(f0);
            const UDB::CDBVariant &vtKey1 = vRow[r]->at(f1);
            if (tbl.eq(vtKey, key0) > 0 && tbl.eq(vtKey1, key1) > 0)
                return vRow[r];
        }
        return nullptr;
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT & vtKey) {
        if (!dbName || !tblName)
            return INVALID_VALUE;
        size_t deleted = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            if (!Is(*it, dbName, tblName))
                continue;
            const UDB::CDBColumnInfoArray &meta = it->GetMeta();
            size_t key = FindKeyColIndex(meta);
            if (key == INVALID_VALUE)
                return INVALID_VALUE;
            VARTYPE type = meta[key].DataType;
            if (type == (VT_I1 | VT_ARRAY))
                type = VT_BSTR; //Table string is always unicode string
            UDB::CDBVariant vt = Convert(vtKey, type);
            auto &vRow = it->second;
            size_t rows = vRow.size();
            for (size_t r = 0; r < rows; ++r) {
                const UDB::CDBVariant &vtKey0 = vRow[r]->at(key);
                if (it->eq(vtKey0, vt) > 0) {
                    vRow.erase(vRow.begin() + r);
                    deleted = 1;
                    break;
                }
            }
            break;
        }
        return deleted;
    }

    size_t CDataSet::UpdateARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pvt, size_t count) {
        if (!dbName || !tblName)
            return INVALID_VALUE;
        if (!pvt || !count || count % 2)
            return INVALID_VALUE;
        size_t updated = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            if (!Is(*it, dbName, tblName))
                continue;
            const UDB::CDBColumnInfoArray &meta = it->first;
            size_t col_count = meta.size();
            if (count % col_count || 2 * col_count != count)
                return INVALID_VALUE;
            size_t key1;
            CPRow row;
            size_t key0 = FindKeyColIndex(meta, key1);
            if (key0 == INVALID_VALUE && key1 == INVALID_VALUE)
                return INVALID_VALUE;
            else if (key1 == INVALID_VALUE) {
                VARTYPE type = meta[key0].DataType;
                if (type == (VT_I1 | VT_ARRAY))
                    type = VT_BSTR; //Table string is always unicode string
                UDB::CDBVariant vt = Convert(pvt[key0 * 2], type);
                row = FindARowInternal(*it, key0, vt);
            } else {
                VARTYPE type = meta[key0].DataType;
                if (type == (VT_I1 | VT_ARRAY))
                    type = VT_BSTR; //Table string is always unicode string
                UDB::CDBVariant vt0 = Convert(pvt[key0 * 2], type);
                type = meta[key1].DataType;
                if (type == (VT_I1 | VT_ARRAY))
                    type = VT_BSTR; //Table string is always unicode string
                UDB::CDBVariant vt1 = Convert(pvt[key1 * 2], type);
                row = FindARowInternal(*it, key0, key1, vt0, vt1);
            }
            if (row) {
                for (unsigned int n = 0; n < col_count; ++n) {
                    const VARIANT &vt = pvt[2 * n + 1];
                    if (vt.vt == (VT_ARRAY | VT_I1)) {
                        const char *s;
                        CComVariant vtNew;
                        ::SafeArrayAccessData(vt.parray, (void**) &s);
                        vtNew.bstrVal = Utilities::ToBSTR(s, vt.parray->rgsabound->cElements);
                        vtNew.vt = VT_BSTR;
                        ::SafeArrayUnaccessData(vt.parray);
                        VARTYPE vtTarget = meta[n].DataType;
                        if (vtTarget == (VT_I1 | VT_ARRAY))
                            vtTarget = VT_BSTR;
                        row->at(n) = Convert(vtNew, vtTarget);
                    } else {
                        row->at(n) = Convert(vt, meta[n].DataType);
                    }
                }
                updated = 1;
            }
            break;
        }
        return updated;
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const VARIANT *pRow, unsigned int cols) {
        if (!dbName || !tblName)
            return INVALID_VALUE;
        size_t deleted = 0;
        CAutoLock al(m_cs);
        for (auto it = m_ds.begin(), end = m_ds.end(); it != end; ++it) {
            if (!Is(*it, dbName, tblName))
                continue;
            const UDB::CDBColumnInfoArray &meta = it->first;
            size_t key1;
            size_t key = FindKeyColIndex(meta, key1);
            if (key == INVALID_VALUE && key1 == INVALID_VALUE)
                return INVALID_VALUE;
            VARTYPE type = meta[key].DataType;
            if (type == (VT_I1 | VT_ARRAY))
                type = VT_BSTR; //Table string is always unicode string
            UDB::CDBVariant vt0 = Convert(pRow[key], type);
            UDB::CDBVariant vt1;
            if (key1 != INVALID_VALUE) {
                type = meta[key1].DataType;
                if (type == (VT_I1 | VT_ARRAY))
                    type = VT_BSTR; //Table string is always unicode string
                if (cols == 2)
                    vt1 = Convert(pRow[1], type);
                else
                    vt1 = Convert(pRow[key1], type);
            }
            auto &vRow = it->second;
            size_t rows = vRow.size();
            for (size_t r = 0; r < rows; ++r) {
                const UDB::CDBVariant &vtKey = vRow[r]->at(key);
                if (it->eq(vtKey, vt0) <= 0)
                    continue;
                if (key1 != INVALID_VALUE) {
                    const UDB::CDBVariant &vt2 = vRow[r]->at(key1);
                    if (it->eq(vt2, vt1) <= 0)
                        continue;
                }
                vRow.erase(vRow.begin() + r);
                deleted = 1;
                break;
            }
            break;
        }
        return deleted;
    }

    size_t CDataSet::DeleteARow(const wchar_t *dbName, const wchar_t *tblName, const CComVariant & key) {
        return DeleteARow(dbName, tblName, (const VARIANT&) key);
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
        if (vtTarget == (VT_UI1 | VT_ARRAY) && data.vt == (VT_ARRAY | VT_I1)) {
            vt = data;
            vt.vt = (VT_ARRAY | VT_UI1);
            return vt;
        } else if (vtTarget == VT_DATE && data.vt == VT_BSTR) {
            std::string s = Utilities::ToUTF8(data.bstrVal);
            UDateTime dt(s.c_str());
            vt = dt;
            return vt;
        }
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
        assert(vtDate.dblVal < UDB::MIN_WIN_DATETIME); //must be in high precision time format
#endif
        UDateTime dt(vtDate.ullVal);
        return dt.ToDBString();
    }

    CKeyMap CDataSet::FindKeys(const wchar_t *dbName, const wchar_t * tblName) {
        {
            CAutoLock al(m_cs);
            for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
                if (!Is(*it, dbName, tblName))
                    continue;
                return it->GetKeys();
            }
        }
        CKeyMap map;
        return map;
    }

    UDB::CDBColumnInfoArray CDataSet::GetColumMeta(const wchar_t *dbName, const wchar_t * tblName) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            if (!Is(*it, dbName, tblName))
                continue;
            return it->first;
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

    int CDataSet::Find(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable::Operator op, const CComVariant &vt, CTable & tbl) {
        return Find(dbName, tblName, ordinal, op, (const VARIANT&) vt, tbl);
    }

    int CDataSet::FindNull(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable & tbl) {
        VARIANT vt;
        ::memset(&vt, 0, sizeof (vt));
        return Find(dbName, tblName, ordinal, CTable::is_null, vt, tbl);
    }

    bool CDataSet::Is(const CTable &tbl, const wchar_t *dbName, const wchar_t * tblName) {
        bool eq;
        if (m_bDBNameCaseSensitive) {
            eq = (::wcscmp(tbl.first[0].DBPath.c_str(), dbName) == 0);
        } else {
#ifdef WIN32_64
            eq = (::_wcsicmp(tbl.first[0].DBPath.c_str(), dbName) == 0);
#else
            eq = (::wcscasecmp(tbl.first[0].DBPath.c_str(), dbName) == 0);
#endif
        }
        if (!eq)
            return false;
        if (m_bTableNameCaseSensitive) {
            eq = (::wcscmp(tbl.first[0].TablePath.c_str(), tblName) == 0);
        } else {
#ifdef WIN32_64
            eq = (::_wcsicmp(tbl.first[0].TablePath.c_str(), tblName) == 0);
#else
            eq = (::wcscasecmp(tbl.first[0].TablePath.c_str(), tblName) == 0);
#endif
        }
        return eq;
    }

    int CDataSet::In(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const UDB::CDBVariantArray &v, CTable & t) {
        if (!dbName)
            dbName = L"";
        if (!tblName || !::wcslen(tblName))
            return CTable::NO_TABLE_NAME_GIVEN;
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CTable &tbl = *it;
            if (!Is(tbl, dbName, tblName))
                continue;
            return tbl.In(ordinal, v, t, true);
        }
        return CTable::NO_TABLE_FOUND;
    }

    int CDataSet::NotIn(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const UDB::CDBVariantArray &v, CTable & t) {
        if (!dbName)
            dbName = L"";
        if (!tblName || !::wcslen(tblName))
            return CTable::NO_TABLE_NAME_GIVEN;
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CTable &tbl = *it;
            if (!Is(tbl, dbName, tblName))
                continue;
            return tbl.NotIn(ordinal, v, t, true);
        }
        return CTable::NO_TABLE_FOUND;
    }

    int CDataSet::Find(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, CTable::Operator op, const VARIANT &vt, CTable & t) {
        if (!dbName)
            dbName = L"";
        if (!tblName || !::wcslen(tblName))
            return CTable::NO_TABLE_NAME_GIVEN;
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CTable &tbl = *it;
            if (!Is(tbl, dbName, tblName))
                continue;
            return tbl.Find(ordinal, op, vt, t, true);
        }
        return CTable::NO_TABLE_FOUND;
    }

    int CDataSet::Between(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const CComVariant &vt0, const CComVariant &vt1, CTable & tbl) {
        return Between(dbName, tblName, ordinal, (const VARIANT&) vt0, (const VARIANT&) vt1, tbl);
    }

    int CDataSet::Between(const wchar_t *dbName, const wchar_t *tblName, unsigned int ordinal, const VARIANT &vt0, const VARIANT &vt1, CTable & t) {
        if (!dbName)
            dbName = L"";
        if (!tblName || !::wcslen(tblName))
            return CTable::NO_TABLE_NAME_GIVEN;
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CTable &tbl = *it;
            if (!Is(tbl, dbName, tblName))
                continue;
            return tbl.Between(ordinal, vt0, vt1, t, true);
        }
        return CTable::NO_TABLE_FOUND;
    }

    unsigned int CDataSet::FindOrdinal(const wchar_t *dbName, const wchar_t *tblName, const wchar_t * str) {
        if (!tblName || !str)
            return CTable::INVALID_ORDINAL;
        if (!dbName)
            dbName = L"";
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            const CTable &tbl = *it;
            if (!Is(tbl, dbName, tblName))
                continue;
            return tbl.FindOrdinal(str);
        }
        return CTable::INVALID_ORDINAL;
    }

    unsigned int CDataSet::FindOrdinal(const char *dbName, const char *tblName, const char *str) {
        if (!tblName || !str)
            return CTable::INVALID_ORDINAL;
        if (!dbName)
            dbName = "";
        std::wstring wdbName = Utilities::ToWide(dbName);
        std::wstring wtblName = Utilities::ToWide(tblName);
        std::wstring wstr = Utilities::ToWide(str);
        return FindOrdinal(wdbName.c_str(), wtblName.c_str(), wstr.c_str());
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
            if (!Is(*it, dbName, tblName))
                continue;
            const UDB::CDBColumnInfoArray &meta = it->first;
            return meta.size();
        }
        return 0;
    }

    size_t CDataSet::GetRowCount(const wchar_t *dbName, const wchar_t * tblName) {
        CAutoLock al(m_cs);
        for (auto it = m_ds.cbegin(), end = m_ds.cend(); it != end; ++it) {
            if (!Is(*it, dbName, tblName))
                continue;
            return it->second.size();
        }
        return INVALID_VALUE;
    }

} //namespace SPA
