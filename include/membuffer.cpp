
#include "membuffer.h"
#include <assert.h>

namespace SPA
{

    void CUQueue::ChangeArrayInt(void *p, unsigned char sizePerInt, unsigned int count) {
        unsigned int n;
        unsigned char *pData = (unsigned char*) p;
        for (n = 0; n < count; ++n) {
            ChangeEndian(pData, sizePerInt);
            pData += sizePerInt;
        }
    }

    void CUQueue::Insert(const wchar_t* str, unsigned int chars, unsigned int position) {
        if (!str) {
            return;
        }
        if (chars == UQUEUE_NULL_LENGTH) {
            chars = (unsigned int) ::wcslen(str);
        }
        if (chars == 0) {
            return;
        }
#if defined(__ANDROID__) || defined(ANDROID)
        std::basic_string<UTF16> s = Utilities::ToUTF16(str, chars);
        Insert((const unsigned char*) s.c_str(), (unsigned int) (s.size() * sizeof (UTF16)), position);
#elif defined(WCHAR32)
        if (position >= GetSize()) {
            Utilities::ToUTF16(str, chars, *this, true);
        } else {
            CScopeUQueue sb;
            Utilities::ToUTF16(str, chars, *sb);
            Insert(sb->GetBuffer(), sb->GetSize(), position);
        }
#else
        Insert((const unsigned char*) str, chars * sizeof (UTF16), position);
#endif
    }

    CUQueue & CUQueue::operator >> (std::wstring & str) {
        unsigned int size;
        Pop((unsigned char*) &size, sizeof (unsigned int));
        switch (size) {
            case UQUEUE_NULL_LENGTH:
            case 0:
                str.clear();
                break;
            default:
                if (size > GetSize()) {
                    throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                }
#if defined(__ANDROID__) || defined(ANDROID)
                const UTF16 *utf16 = (const UTF16*) GetBuffer();
                unsigned int chars = size / sizeof (UTF16);
                str = Utilities::ToWide(utf16, chars);
#elif defined(WIN32_64)
                str.assign((const wchar_t*) GetBuffer(), size / sizeof (wchar_t));
#else
                CScopeUQueue su;
                const UTF16 *utf16 = (const UTF16*) GetBuffer();
                unsigned int chars = size / sizeof (UTF16);
                Utilities::ToWide(utf16, chars, *su);
                const wchar_t *wstr = (const wchar_t*) su->GetBuffer();
                str.assign(wstr, wstr + su->GetSize() / sizeof (wchar_t));
#endif
                Pop(size); //discard 
                break;
        }
        return *this;
    }

    void CUQueue::Insert(const VARIANT &vtData, unsigned int position) {
        if (position > GetSize()) {
            position = GetSize();
        }
        Insert((const unsigned char*) &(vtData.vt), sizeof (vtData.vt), position);
        position += sizeof (vtData.vt);
        switch (vtData.vt) {
            case VT_NULL:
            case VT_EMPTY:
                break;
            case VT_I1:
                Insert((const unsigned char*) &(vtData.cVal), sizeof (vtData.cVal), position);
                break;
            case VT_UI1:
                Insert((const unsigned char*) &(vtData.bVal), sizeof (vtData.bVal), position);
                break;
            case VT_I2:
                Insert((const unsigned char*) &(vtData.iVal), sizeof (vtData.iVal), position);
                break;
            case VT_UI2:
                Insert((const unsigned char*) &(vtData.uiVal), sizeof (vtData.uiVal), position);
                break;
            case VT_I4:
                Insert((const unsigned char*) &(vtData.lVal), sizeof (vtData.lVal), position);
                break;
            case VT_UI4:
                Insert((const unsigned char*) &(vtData.ulVal), sizeof (vtData.ulVal), position);
                break;
            case VT_R4:
                Insert((const unsigned char*) &(vtData.fltVal), sizeof (vtData.fltVal), position);
                break;
            case VT_R8:
                Insert((const unsigned char*) &(vtData.dblVal), sizeof (vtData.dblVal), position);
                break;
            case VT_CY:
                Insert((const unsigned char*) &(vtData.cyVal), sizeof (vtData.cyVal), position);
                break;
            case VT_DATE:
#ifdef WIN32_64
                if (m_TimeEx) {
#ifndef _WIN32_WCE
                    Insert((const unsigned char*) &(vtData.ullVal), sizeof (vtData.ullVal), position);
#else
#endif
                } else {
                    UDateTime time = UDateTime(vtData.date);
                    Insert((const unsigned char*) &time.time, sizeof (time.time), position);
                }
#else
                Insert((const unsigned char*) &(vtData.ullVal), sizeof (vtData.ullVal), position);
#endif
                break;
            case VT_BOOL:
                Insert((const unsigned char*) &(vtData.boolVal), sizeof (vtData.boolVal), position);
                break;
            case VT_BSTR:
                (*this) << vtData.bstrVal;
                break;
            case VT_DECIMAL:
                Insert((const unsigned char*) &(vtData.decVal), sizeof (vtData.decVal), position);
                break;
            case VT_INT:
                Insert((const unsigned char*) &(vtData.intVal), sizeof (vtData.intVal), position);
                break;
            case VT_UINT:
                Insert((const unsigned char*) &(vtData.uintVal), sizeof (vtData.uintVal), position);
                break;
#ifndef _WIN32_WCE
            case VT_I8:
                Insert((const unsigned char*) &(vtData.llVal), sizeof (vtData.llVal), position);
                break;
            case VT_UI8:
                Insert((const unsigned char*) &(vtData.ullVal), sizeof (vtData.ullVal), position);
                break;
#else
            case VT_I8:
                Insert((const unsigned char*) &(vtData.lVal), sizeof (LONGLONG), position);
                break;
            case VT_UI8:
                Insert((const unsigned char*) &(vtData.ulVal), sizeof (ULONGLONG), position);
                break;
#endif
            default:
                if ((vtData.vt & VT_ARRAY) == VT_ARRAY) {
                    assert(vtData.parray->cDims == 1);
                    assert(vtData.parray->rgsabound[0].lLbound == 0);
                    const unsigned char *pBuffer = nullptr;
                    unsigned int size = vtData.parray->rgsabound->cElements;
                    assert(size);
                    Insert((const unsigned char*) &size, sizeof (size), position);
                    position += sizeof (size);
                    SafeArrayAccessData(vtData.parray, (void**) &pBuffer);
                    assert(pBuffer);
                    if (!pBuffer) {
                        throw CUSEx("Bad pointer to variant array data");
                    }
                    switch (vtData.vt) {
                        case (VT_UI1 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (char), position);
                            break;
                        case (VT_I1 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (unsigned char), position);
                            break;
                        case (VT_I2 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (short), position);
                            break;
                        case (VT_UI2 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (unsigned short), position);
                            break;
                        case (VT_I4 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (int), position);
                            break;
                        case (VT_UI4 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (unsigned int), position);
                            break;
                        case (VT_R4 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (float), position);
                            break;
                        case (VT_R8 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (double), position);
                            break;
                        case (VT_CY | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (CY), position);
                            break;
                        case (VT_DATE | VT_ARRAY):
                        {
                            CScopeUQueue su;
#ifndef _WIN32_WCE
                            static_assert(sizeof (UDateTime) == 8, "Bad UDateTime size");
#endif
                            unsigned int max = size * sizeof (UDateTime);
                            if (max > su->GetMaxSize()) {
                                su->ReallocBuffer(max);
                            }
                            UDateTime *udt = (UDateTime*) su->GetBuffer();
#ifdef WIN32_64
                            if (m_TimeEx) {
                                memcpy(udt, pBuffer, max);
                            } else {
                                DATE *vd = (DATE*) pBuffer;
                                for (unsigned int n = 0; n < size; ++n) {
                                    udt[n].Set(vd[n]);
                                }
                            }
#else
                            memcpy(udt, pBuffer, max);
#endif
                            Insert(su->GetBuffer(), size * sizeof (UDateTime), position);
                        }
                            break;
                        case (VT_BOOL | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (VARIANT_BOOL), position);
                            break;
                        case (VT_VARIANT | VT_ARRAY):
                        {
                            unsigned int ul;
                            VARIANT *pvt = (VARIANT *) pBuffer;
                            if (position >= GetSize()) {
                                for (ul = 0; ul < size; ++ul) {
                                    Push(pvt[ul]);
                                }
                            } else {
                                CScopeUQueue su(m_os, IsBigEndian());
                                CUQueue &UQueue = *su;
                                for (ul = 0; ul < size; ++ul) {
                                    UQueue.Push(pvt[ul]);
                                }
                                Insert(UQueue.GetBuffer(), UQueue.GetSize(), position);
                            }
                        }
                            break;
                        case (VT_BSTR | VT_ARRAY):
                        {
                            unsigned int ul;
                            BSTR *pbstr = (BSTR *) pBuffer;
                            if (position >= GetSize()) {
                                for (ul = 0; ul < size; ++ul) {
                                    (*this) << pbstr[ul];
                                }
                            } else {
                                CScopeUQueue su(m_os, IsBigEndian());
                                CUQueue &q = *su;
                                for (ul = 0; ul < size; ++ul) {
                                    q << pbstr[ul];
                                }
                                Insert(q.GetBuffer(), q.GetSize(), position);
                            }
                        }
                            break;
                        case (VT_DECIMAL | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (DECIMAL), position);
                            break;
                        case (VT_INT | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (int), position);
                            break;
                        case (VT_UINT | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (unsigned int), position);
                            break;
                        case (VT_I8 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (INT64), position);
                            break;
                        case (VT_UI8 | VT_ARRAY):
                            Insert(pBuffer, size * sizeof (UINT64), position);
                            break;
                        default:
                            SafeArrayUnaccessData(vtData.parray);
                            throw CUExCode("Unsupported data type for serialization", MB_SERIALIZATION_NOT_SUPPORTED);
                            break;
                    }
                    SafeArrayUnaccessData(vtData.parray);
                } else {
                    throw CUExCode("Unsupported data type for serialization", MB_SERIALIZATION_NOT_SUPPORTED);
                }
                break;
        }
    }

    unsigned int CUQueue::Pop(VARIANT& vtData, unsigned int position) {
        unsigned int total = 0;
#ifndef _WIN32_WCE
        try{
#endif
            if (vtData.vt == VT_BSTR) {
                VariantClear(&vtData);
            } else if ((vtData.vt & VT_ARRAY) == VT_ARRAY) {
                VariantClear(&vtData);
            }
#ifndef _WIN32_WCE
        }

        catch(...) {
        }
#endif
        total = Pop(&(vtData.vt), position);
        switch (vtData.vt) {
            case VT_NULL:
                break;
            case VT_EMPTY:
                break;
            case VT_I1:
                total += Pop((unsigned char*) &(vtData.cVal), sizeof (vtData.cVal), position);
                break;
            case VT_UI1:
                total += Pop((unsigned char*) &(vtData.bVal), sizeof (vtData.bVal), position);
                break;
            case VT_I2:
                total += Pop((unsigned char*) &(vtData.iVal), sizeof (vtData.iVal), position);
                break;
            case VT_UI2:
                total += Pop((unsigned char*) &(vtData.uiVal), sizeof (vtData.uiVal), position);
                break;
            case VT_I4:
                total += Pop((unsigned char*) &(vtData.lVal), sizeof (vtData.lVal), position);
                break;
            case VT_UI4:
                total += Pop((unsigned char*) &(vtData.ulVal), sizeof (vtData.ulVal), position);
                break;
            case VT_R4:
                total += Pop((unsigned char*) &(vtData.fltVal), sizeof (vtData.fltVal), position);
                break;
            case VT_R8:
                total += Pop((unsigned char*) &(vtData.dblVal), sizeof (vtData.dblVal), position);
                break;
            case VT_DATE:
#ifdef WIN32_64
                if (m_TimeEx) {
#ifndef _WIN32_WCE
                    //UDateTime has accuracy to microsecond
                    total += Pop((unsigned char*) &vtData.ullVal, sizeof (vtData.ullVal), position);
#endif
                } else {
                    UDateTime dt;
                    total += Pop((unsigned char*) &dt.time, sizeof (dt.time), position);
                    //variant date has accuracy to millisecond
                    vtData.date = dt.GetVariantDate();
                }
#else
                //UDateTime has accuracy to microsecond
                total += Pop((unsigned char*) &vtData.ullVal, sizeof (vtData.ullVal), position);
#endif
                break;
            case VT_BOOL:
                total += Pop((unsigned char*) &(vtData.boolVal), sizeof (vtData.boolVal), position);
                break;
            case VT_CY:
                total += Pop((unsigned char*) &(vtData.cyVal), sizeof (vtData.cyVal), position);
                break;
            case VT_BSTR:
                if (m_ToUtf8) {
                    unsigned int len;
                    *this >> len;
                    total += sizeof (len);
                    if (len == SPA::UQUEUE_NULL_LENGTH) {
                        vtData.vt = VT_NULL;
                    } else {
                        assert((len % sizeof (UTF16)) == 0);
                        if (len > GetSize()) {
                            throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                        }
                        SPA::CScopeUQueue sb;
                        unsigned int wchars = (len >> 1);
                        const UTF16 *str = (const UTF16 *) GetBuffer();
                        vtData.vt = (VT_ARRAY | VT_I1);
                        Utilities::ToUTF8(str, wchars, *sb);
                        Pop(len);
                        total += len;
                        SAFEARRAYBOUND sab[1] = {sb->GetSize(), 0};
                        SAFEARRAY *psa = SafeArrayCreate(VT_I1, 1, sab);
                        unsigned char *pBuffer;
                        SafeArrayAccessData(psa, (void**) &pBuffer);
                        memcpy(pBuffer, sb->GetBuffer(), sb->GetSize());
                        SafeArrayUnaccessData(psa);
                        vtData.parray = psa;
                    }
                } else {
                    unsigned int ulLen = GetSize();
#ifdef WIN32_64
                    CComBSTR bstr;
                    *this >> bstr;
                    vtData.bstrVal = bstr.Detach();
#else
                    unsigned int len;
                    vtData.vt = VT_BSTR;
                    *this >> len;
                    if (len == (unsigned int) (~0))
                        vtData.bstrVal = nullptr;
                    else {
                        if (len > GetSize()) {
                            throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                        }
                        const UTF16 *str = (const UTF16 *) GetBuffer();
                        vtData.bstrVal = Utilities::SysAllocString(str, len >> 1);
                        Pop(len);
                    }
#endif
                    total += (ulLen - GetSize());
                }
                break;
            case VT_INT:
                total += Pop((unsigned char*) &(vtData.intVal), sizeof (vtData.intVal), position);
                break;
            case VT_UINT:
                total += Pop((unsigned char*) &(vtData.uintVal), sizeof (vtData.uintVal), position);
                break;
#ifndef _WIN32_WCE
            case VT_I8:
                total += Pop((unsigned char*) &(vtData.llVal), sizeof (vtData.llVal), position);
                break;
            case VT_UI8:
                total += Pop((unsigned char*) &(vtData.ullVal), sizeof (vtData.ullVal), position);
                break;
#else
            case VT_I8:
                total += Pop((unsigned char*) &(vtData.lVal), sizeof (LONGLONG), position);
                break;
            case VT_UI8:
                total += Pop((unsigned char*) &(vtData.ulVal), sizeof (ULONGLONG), position);
                break;
#endif
            case VT_DECIMAL:
                total += Pop((unsigned char*) &(vtData.decVal), sizeof (vtData.decVal), position);
                vtData.vt = VT_DECIMAL;
                break;
            default:
                if ((vtData.vt & VT_ARRAY) == VT_ARRAY) {
                    unsigned int ulSize = 0;
                    SAFEARRAY *psa;
                    void *pBuffer = nullptr;
                    total += Pop(&ulSize, position);
                    SAFEARRAYBOUND sab[1] = {ulSize, 0};
                    switch (vtData.vt) {
                        case (VT_I1 | VT_ARRAY):
                            if (m_8ToW) {
                                const char *utf8 = (const char*) GetBuffer();
                                vtData.vt = VT_BSTR;
                                vtData.bstrVal = Utilities::ToBSTR(utf8, ulSize);
                                Pop(ulSize);
                                total += ulSize;
                                return total;
                            } else {
                                psa = SafeArrayCreate(VT_I1, 1, sab);
                                SafeArrayAccessData(psa, &pBuffer);
                                total += Pop((unsigned char*) pBuffer, ulSize * sizeof (char), position);
                                SafeArrayUnaccessData(psa);
                            }
                            break;
                        case (VT_UI1 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_UI1, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (unsigned char), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_I2 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_I2, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (short), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_UI2 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_UI2, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (unsigned short), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_I4 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_I4, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (int), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_UI4 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_UI4, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (unsigned int), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_R4 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_R4, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (float), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_R8 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_R8, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (double), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_BOOL | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_BOOL, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (VARIANT_BOOL), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_DATE | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_DATE, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (UDateTime), position); //sizeof(DATE) == sizeof(INT64) == sizeof(UDateTime)
#ifdef WIN32_64
                            if (!m_TimeEx) {
                                //inline conversion
                                UDateTime *udt = (UDateTime*) pBuffer;
                                DATE *vd = (DATE*) pBuffer;
                                for (unsigned int n = 0; n < ulSize; ++n) {
                                    vd[n] = udt[n].GetVariantDate();
                                }
                            }
#endif
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_CY | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_CY, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (CY), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_VARIANT | VT_ARRAY):
                        {
                            unsigned int ul;
                            psa = SafeArrayCreate(VT_VARIANT, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            for (ul = 0; ul < ulSize; ul++) {
                                total += Pop(((VARIANT*) pBuffer)[ul], position);
                            }
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_BSTR | VT_ARRAY):
                        {
                            unsigned int ulLen;
                            unsigned int ulIndex;
                            BSTR *pbstr;
                            psa = SafeArrayCreate(VT_BSTR, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            pbstr = (BSTR*) pBuffer;
                            for (ulIndex = 0; ulIndex < ulSize; ulIndex++) {
                                ulLen = GetSize();
#ifdef WIN32_64
                                CComBSTR bstr;
                                *this >> bstr;
                                pbstr[ulIndex] = bstr.Detach();
#else
                                unsigned int len;
                                *this >> len;
                                if (len == (unsigned int) (~0))
                                    pbstr[ulIndex] = nullptr;
                                else {
                                    if (len > GetSize()) {
                                        throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                                    }
                                    pbstr[ulIndex] = Utilities::SysAllocString((const UTF16*) GetBuffer(), (len >> 1));
                                    Pop(len);
                                }
#endif
                                total += (ulLen - GetSize());
                            }
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_INT | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_INT, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (int), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_UINT | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_UINT, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (unsigned int), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_I8 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_I8, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (INT64), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_UI8 | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_UI8, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (UINT64), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        case (VT_DECIMAL | VT_ARRAY):
                        {
                            psa = SafeArrayCreate(VT_DECIMAL, 1, sab);
                            SafeArrayAccessData(psa, &pBuffer);
                            total += Pop((unsigned char*) pBuffer, ulSize * sizeof (DECIMAL), position);
                            SafeArrayUnaccessData(psa);
                        }
                            break;
                        default:
                            vtData.vt = VT_EMPTY;
                            throw CUException("Unsupported data type for deserialization", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                            break;
                    }
                    vtData.parray = psa;
                } else {
                    vtData.vt = VT_EMPTY;
                    throw CUException("Unsupported data type for deserialization", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                }
                break;
        }
        return total;
    }

    namespace Utilities{

        unsigned int GetLen(const UTF16 * chars) {
            if (!chars) {
                return 0;
            }
            unsigned int len = 0;
            while (*chars) {
                ++len;
                ++chars;
            }
            return len;
        }

#if defined(__ANDROID__) || defined(ANDROID)

#else

        void ToWide(const char *str, size_t src_len, CUQueue & q, bool append) {
            if (!append) {
                q.SetSize(0);
            }
            if (!src_len || !str) {
                q.SetNull();
                return;
            }
#if 0
            size_t req_len = ((size_t)::MultiByteToWideChar(CP_UTF8, 0, str, (int) src_len, nullptr, 0) + 1) * sizeof (wchar_t);
#else
            size_t req_len = (src_len + 1) * sizeof (wchar_t);
#endif
            if (req_len > q.GetTailSize()) {
                q.ReallocBuffer((unsigned int) req_len + q.GetSize());
            }

#ifdef WIN32_64
            req_len = (size_t)::MultiByteToWideChar(CP_UTF8, 0, str, (int) src_len, (wchar_t*) q.GetBuffer(q.GetSize()), q.GetTailSize() / sizeof (wchar_t));
            q.SetSize((unsigned int) (req_len * sizeof (wchar_t)) + q.GetSize());
#else
            char *src = (char*) str;
            char *pos = (char*) q.GetBuffer(q.GetSize());
            size_t size_output = q.GetTailSize();
            size_t max_size = size_output;
            iconv_t UTF82W = ::iconv_open("wchar_t", "UTF-8");
            req_len = ::iconv(UTF82W, &src, &src_len, &pos, &size_output);
            assert(req_len != (size_t) (~0));
            ::iconv_close(UTF82W);
            if (req_len != (size_t) (-1)) {
                q.SetSize((max_size - size_output) + q.GetSize());
            }
#endif
            q.SetNull();
        }

        BSTR ToBSTR(const char *utf8, size_t chars) {
            if (!utf8)
                return nullptr;
            if ((size_t) (~0) == chars)
                chars = ::strlen(utf8);
#ifdef WIN32_64
            int nConvertedLen = ::MultiByteToWideChar(CP_UTF8, 0, utf8, (int) chars, nullptr, 0);
            BSTR bstr = ::SysAllocStringLen(nullptr, nConvertedLen);
            int nResult = ::MultiByteToWideChar(CP_UTF8, 0, utf8, (int) chars, bstr, nConvertedLen);
            assert(nConvertedLen == nResult);
            return bstr;
#else
            CScopeUQueue sb;
            ToWide(utf8, chars, *sb);
            return SysAllocStringLen((const wchar_t*) sb->GetBuffer(), sb->GetSize() / sizeof (wchar_t));
#endif
        }

        std::wstring ToWide(const char *utf8, size_t chars) {
            if (!utf8)
                return L"";
            if ((size_t) (~0) == chars)
                chars = ::strlen(utf8);
            CScopeUQueue sb;
            Utilities::ToWide(utf8, chars, *sb);
            return (const wchar_t*) sb->GetBuffer();
        }

        std::string ToUTF8(const wchar_t *str, size_t wchars) {
            if (!str)
                return "";
            if ((size_t) (~0) == wchars)
                wchars = ::wcslen(str);
            CScopeUQueue sb;
            Utilities::ToUTF8(str, wchars, *sb);
            return (const char*) sb->GetBuffer();
        }

        void ToUTF8(const wchar_t *str, size_t len, CUQueue & q, bool append) {
            if (!append) {
                q.SetSize(0);
            }
            if (!str || !len) {
                q.SetNull();
                return;
            }
            size_t req_len;
#ifdef WIN32_64
            req_len = (size_t)::WideCharToMultiByte(CP_UTF8, 0, str, (int) len, nullptr, 0, nullptr, nullptr) + sizeof (wchar_t);
#else
            req_len = (len + 1) * sizeof (wchar_t);
#endif
            if ((unsigned int) req_len > q.GetTailSize())
                q.ReallocBuffer((unsigned int) req_len + q.GetSize());
#ifdef WIN32_64
            req_len = (size_t)::WideCharToMultiByte(CP_UTF8, 0, str, (int) len, (char*) q.GetBuffer(q.GetSize()), (int) q.GetTailSize(), nullptr, nullptr);
            q.SetSize((unsigned int) req_len + q.GetSize());
#else
            char *src = (char*) str;
            char *pos = (char*) q.GetBuffer(q.GetSize());
            size_t size_output = q.GetTailSize();
            size_t max_size = size_output;
            len *= sizeof (wchar_t);
            iconv_t W2UTF8 = ::iconv_open("UTF-8", "wchar_t");
            req_len = ::iconv(W2UTF8, &src, &len, &pos, &size_output);
            assert(req_len != (size_t) (~0));
            ::iconv_close(W2UTF8);
            if (req_len != (size_t) (-1)) {
                q.SetSize((max_size - size_output) + q.GetSize());
            }
#endif
            q.SetNull();
        }

#ifdef WCHAR32

        void ToWide(const UTF16 *str, size_t chars, CUQueue & q, bool append) {
            if (!append) {
                q.SetSize(0);
            }
            if (!str || !chars) {
                q.SetNull();
                return;
            }
            char *input = (char*) str;
            size_t sizeInput = chars * sizeof (UTF16);
            if (q.GetTailSize() < (chars + 1) * sizeof (wchar_t)) {
                q.ReallocBuffer((chars + 1) * sizeof (wchar_t) + q.GetSize());
            }
            char *pos = (char*) q.GetBuffer(q.GetSize());
            size_t size_output = q.GetTailSize();
            size_t max_size = size_output;
            iconv_t UTF162W = ::iconv_open("wchar_t", "UTF-16LE");
            size_t res = ::iconv(UTF162W, &input, &sizeInput, &pos, &size_output);
            assert(res != (size_t) (~0));
            ::iconv_close(UTF162W);
            q.SetSize((max_size - size_output) + q.GetSize());
            q.SetNull();
        }

        BSTR SysAllocString(const SPA::UTF16 *sz, unsigned int wchars) {
            if (!sz && wchars == (unsigned int) (~0))
                return nullptr;
            else if (sz && wchars == (unsigned int) (~0)) {
                wchars = GetLen(sz);
            }
            CScopeUQueue sb;
            ToWide(sz, wchars, *sb);
            return SysAllocStringLen((const wchar_t*) sb->GetBuffer(), sb->GetSize() / sizeof (wchar_t));
        }

        void ToUTF8(const UTF16 *str, size_t chars, CUQueue & q, bool append) {
            if (!append) {
                q.SetSize(0);
            }
            if (!str || !chars) {
                q.SetNull();
                return;
            }
            char *input = (char*) str;
            size_t sizeInput = chars * sizeof (SPA::UTF16);
            if (q.GetTailSize() < (chars * 3 + sizeof (wchar_t))) {
                q.ReallocBuffer(chars * 3 + sizeof (wchar_t) + q.GetSize());
            }
            char *pos = (char*) q.GetBuffer(q.GetSize());
            size_t size_output = q.GetTailSize();
            size_t max_size = size_output;
            iconv_t UTF162UTF8 = ::iconv_open("UTF-8", "UTF-16LE");
            size_t res = ::iconv(UTF162UTF8, &input, &sizeInput, &pos, &size_output);
            assert(res != (size_t) (~0));
            ::iconv_close(UTF162UTF8);
            res = max_size - size_output;
            q.SetSize(res + q.GetSize());
            q.SetNull();
        }

        /**
         * 
         * @param str
         * @param q
         */
        void ToUTF16(const wchar_t *str, size_t wchars, CUQueue & q, bool append) {
            if (!append) {
                q.SetSize(0);
            }
            if (!str || !wchars) {
                q.SetNull();
                return;
            }
            char *input = (char*) str;
            size_t sizeInput = wchars * sizeof (wchar_t);
            if (q.GetTailSize() < sizeInput + sizeof (wchar_t)) {
                q.ReallocBuffer(sizeInput + sizeof (wchar_t) + q.GetSize());
            }
            char *pos = (char*) q.GetBuffer(q.GetSize());
            size_t size_output = q.GetTailSize();
            size_t max_size = size_output;
            iconv_t W2UTF16 = ::iconv_open("UTF-16LE", "wchar_t");
            size_t res = ::iconv(W2UTF16, &input, &sizeInput, &pos, &size_output);
            assert(res != (size_t) (~0));
            ::iconv_close(W2UTF16);
            q.SetSize((max_size - size_output) + q.GetSize());
            q.SetNull();
        }

        void ToUTF16(const char *str, size_t chars, CUQueue &q, bool append) {
            if (!append) {
                q.SetSize(0);
            }
            if (!str || !chars) {
                q.SetNull();
                return;
            }
            char *input = (char*) str;
            size_t sizeInput = chars * sizeof (UTF16);
            if (q.GetTailSize() < sizeInput + sizeof (wchar_t)) {
                q.ReallocBuffer(sizeInput + sizeof (wchar_t) + q.GetSize());
            }
            char *pos = (char*) q.GetBuffer(q.GetSize());
            size_t size_output = q.GetTailSize();
            size_t max_size = size_output;
            iconv_t UTF82UTF16 = ::iconv_open("UTF-16LE", "UTF-8");
            size_t res = ::iconv(UTF82UTF16, &input, &sizeInput, &pos, &size_output);
            assert(res != (size_t) (~0));
            ::iconv_close(UTF82UTF16);
            q.SetSize((max_size - size_output) + q.GetSize());
            q.SetNull();
        }
#endif
#endif
    }
}

