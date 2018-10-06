
#include "membuffer.h"
#include <assert.h>

namespace SPA {
#ifdef NODE_JS_ADAPTER_PROJECT
    template<unsigned int InitSize, unsigned int BlockSize, typename mb>
    CUCriticalSection CScopeUQueueEx<InitSize, BlockSize, mb>::m_cs;

    template<unsigned int InitSize, unsigned int BlockSize, typename mb>
    std::vector<mb*> CScopeUQueueEx<InitSize, BlockSize, mb>::m_aUQueue;
#endif

    unsigned int SHARED_BUFFER_CLEAN_SIZE = 32 * 1024;

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

    CUQueue & CUQueue::operator>>(std::wstring & str) {
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
        try {
#endif
            if (vtData.vt == VT_BSTR) {
                VariantClear(&vtData);
            } else if ((vtData.vt & VT_ARRAY) == VT_ARRAY) {
                VariantClear(&vtData);
            }
#ifndef _WIN32_WCE
        } catch (...) {
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
                    total += Pop((unsigned char*) &len, sizeof (len), position);
                    if (len == SPA::UQUEUE_NULL_LENGTH) {
                        vtData.vt = VT_NULL;
                    } else {
                        assert((len % sizeof (UTF16)) == 0);
                        if ((len + position) > GetSize()) {
                            throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                        }
                        SPA::CScopeUQueue sb;
                        unsigned int wchars = (len >> 1);
                        const UTF16 *str = (const UTF16 *) GetBuffer(position);
                        vtData.vt = (VT_ARRAY | VT_I1);
                        Utilities::ToUTF8(str, wchars, *sb);
                        Pop(len, position);
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
                    unsigned int len;
                    unsigned int ulLen = GetSize();
                    Pop((unsigned char*) &len, sizeof (len), position);
                    if (len == (unsigned int) (~0))
                        vtData.bstrVal = nullptr;
                    else {
                        if ((len + position) > GetSize()) {
                            throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                        }
                        const UTF16 *str = (const UTF16 *) GetBuffer(position);
#ifdef WIN32_64
                        vtData.bstrVal = ::SysAllocStringLen(str, len >> 1);
#else
                        vtData.bstrVal = Utilities::SysAllocString(str, len >> 1);
#endif
                        Pop(len, position);
                    }
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
#ifdef WIN32_64
                vtData.vt = VT_DECIMAL;
#endif
                break;
            case VT_CLSID:
            {
                unsigned char *p = nullptr;
                SAFEARRAYBOUND sab[1] = {sizeof (GUID), 0};
                SAFEARRAY *psa = ::SafeArrayCreate(VT_UI1, 1, sab);
                ::SafeArrayAccessData(psa, (void**) &p);
                total += Pop(p, sizeof (GUID), position);
                ::SafeArrayUnaccessData(psa);
                vtData.parray = psa;
            }
                vtData.vt = (VT_ARRAY | VT_UI1);
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
                                const char *utf8 = (const char*) GetBuffer(position);
                                vtData.vt = VT_BSTR;
                                vtData.bstrVal = Utilities::ToBSTR(utf8, ulSize);
                                Pop(ulSize, position);
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
                                unsigned int len;
                                Pop((unsigned char*) &len, sizeof (len), position);
                                if (len == (unsigned int) (~0))
                                    pbstr[ulIndex] = nullptr;
                                else {
                                    if ((len + position) > GetSize()) {
                                        throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                                    }
#ifdef WIN32_64
                                    pbstr[ulIndex] = ::SysAllocStringLen((const UTF16*) GetBuffer(position), (len >> 1));
#else
                                    pbstr[ulIndex] = Utilities::SysAllocString((const UTF16*) GetBuffer(position), (len >> 1));
#endif
                                    Pop(len, position);
                                }

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

    namespace Utilities {
#ifdef WIN32_64

        std::wstring GetErrorMessage(DWORD dwError) {
            wchar_t *lpMsgBuf = nullptr;
            DWORD res = ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPWSTR) & lpMsgBuf,
                    0,
                    nullptr);
            std::wstring s(lpMsgBuf ? lpMsgBuf : L"");
            if (lpMsgBuf)
                LocalFree(lpMsgBuf);
            return s;
        }
#endif

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

#ifndef WIN32_64

HRESULT VariantChangeType(VARIANT *pvargDest, const VARIANT *pvarSrc, unsigned short wFlags, VARTYPE vt) {
    if (!pvargDest || !pvarSrc)
        return E_INVALIDARG;
    ::VariantClear(pvargDest);
    if ((pvarSrc->vt == VT_NULL || pvarSrc->vt == VT_EMPTY) && (vt == VT_NULL || vt == VT_EMPTY)) {
        pvargDest->vt = pvarSrc->vt;
        return S_OK;
    }
    pvargDest->vt = vt;
    switch (vt) {
        case VT_I1:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->cVal = pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->cVal = (char) pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->cVal = (char) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->cVal = (char) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->cVal = (char) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->cVal = (char) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->cVal = (char) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->cVal = (char) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->cVal = (char) pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->cVal = (char) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->cVal = (char) pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->cVal = (char) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::INT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
                    if (res)
                        pvargDest->cVal = (char) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_I2:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->iVal = pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->iVal = pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->iVal = (short) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->iVal = (short) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->iVal = (short) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->iVal = (short) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->iVal = (short) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->iVal = (short) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->iVal = (short) pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->iVal = (short) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->iVal = pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->iVal = (short) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::INT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
                    if (res)
                        pvargDest->iVal = (short) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_INT:
        case VT_I4:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->intVal = pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->intVal = pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->intVal = pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->intVal = (int) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->intVal = (int) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->intVal = (int) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->intVal = (int) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->intVal = (int) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->intVal = (int) pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->intVal = (int) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->intVal = pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->intVal = (int) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::INT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
                    if (res)
                        pvargDest->intVal = (int) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_I8:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->llVal = pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->llVal = pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->llVal = pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->llVal = pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->llVal = pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->llVal = (SPA::INT64)pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->llVal = (SPA::INT64)pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->llVal = (SPA::INT64)pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->llVal = (SPA::INT64)pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->llVal = (SPA::INT64)pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->llVal = pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->llVal = (SPA::INT64)SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::INT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
                    if (res)
                        pvargDest->llVal = (char) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_R4:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->fltVal = (float) pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->fltVal = (float) pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->fltVal = (float) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->fltVal = (float) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->fltVal = (float) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->fltVal = (float) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->fltVal = (float) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->fltVal = (float) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->fltVal = pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->fltVal = (float) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->fltVal = (float) pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->fltVal = (float) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    float data;
                    int res = swscanf(pvarSrc->bstrVal, L"%f", &data);
                    if (res)
                        pvargDest->fltVal = data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_R8:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->dblVal = (double) pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->dblVal = (double) pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->dblVal = (double) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->dblVal = (double) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->dblVal = (double) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->dblVal = (double) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->dblVal = (double) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->dblVal = (double) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->dblVal = pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->dblVal = pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->dblVal = pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->dblVal = SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    double data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lf", &data);
                    if (res)
                        pvargDest->dblVal = data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_UI1:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->bVal = (unsigned char) pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->bVal = (unsigned char) pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->bVal = (unsigned char) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->bVal = (unsigned char) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->bVal = (unsigned char) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->bVal = (unsigned char) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->bVal = (unsigned char) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->bVal = (unsigned char) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->bVal = (unsigned char) pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->bVal = (unsigned char) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->bVal = (unsigned char) pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->bVal = (unsigned char) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::UINT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
                    if (res)
                        pvargDest->bVal = (char) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return E_UNEXPECTED;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_UI2:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->uiVal = (unsigned short) pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->uiVal = (unsigned short) pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->uiVal = (unsigned short) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->uiVal = (unsigned short) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->uiVal = (unsigned short) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->uiVal = (unsigned short) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->uiVal = (unsigned short) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->uiVal = (unsigned short) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->uiVal = (unsigned short) pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->uiVal = (unsigned short) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->uiVal = (unsigned short) pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->uiVal = (unsigned short) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::UINT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
                    if (res)
                        pvargDest->uiVal = (unsigned short) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return E_UNEXPECTED;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_UINT:
        case VT_UI4:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->uintVal = (unsigned int) pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->uintVal = (unsigned int) pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->uintVal = (unsigned int) pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->uintVal = (unsigned int) pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->uintVal = (unsigned int) pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->uintVal = (unsigned int) pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->uintVal = (unsigned int) pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->uintVal = (unsigned int) pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->uintVal = (unsigned int) pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->uintVal = (unsigned int) pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->uintVal = (unsigned int) pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->uintVal = (unsigned int) SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::UINT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
                    if (res)
                        pvargDest->uintVal = (unsigned int) data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_UI8:
            switch (pvarSrc->vt) {
                case VT_I1:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->cVal;
                    break;
                case VT_I2:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->iVal;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->intVal;
                    break;
                case VT_I8:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->llVal;
                    break;
                case VT_UI1:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->ulVal;
                    break;
                case VT_UI8:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->ullVal;
                    break;
                case VT_R4:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->fltVal;
                    break;
                case VT_R8:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->dblVal;
                    break;
                case VT_BOOL:
                    pvargDest->ullVal = (SPA::UINT64)pvarSrc->boolVal;
                    break;
                case VT_DECIMAL:
                    pvargDest->ullVal = (SPA::UINT64)SPA::ToDouble(pvarSrc->decVal);
                    break;
                case VT_BSTR:
                {
                    SPA::UINT64 data;
                    int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
                    if (res)
                        pvargDest->ullVal = data;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_BSTR:
            if (pvarSrc->vt == VT_BSTR) {
                pvargDest->bstrVal = ::SysAllocString(pvarSrc->bstrVal);
            } else if (pvarSrc->vt == VT_DECIMAL) {
                std::string s = SPA::ToString_long(pvarSrc->decVal);
                pvargDest->bstrVal = SPA::Utilities::ToBSTR(s.c_str(), s.size());
            } else {
                wchar_t buffer[64] = {0};
                switch (pvarSrc->vt) {
                    case VT_I1:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%d", pvarSrc->cVal);
                        break;
                    case VT_I2:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%d", pvarSrc->iVal);
                        break;
                    case VT_INT:
                    case VT_I4:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%d", pvarSrc->intVal);
                        break;
                    case VT_I8:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%ld", pvarSrc->cVal);
                        break;
                    case VT_UI1:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%u", pvarSrc->bVal);
                        break;
                    case VT_UI2:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%u", pvarSrc->uiVal);
                        break;
                    case VT_UINT:
                    case VT_UI4:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%u", pvarSrc->uintVal);
                        break;
                    case VT_UI8:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%lu", pvarSrc->ullVal);
                        break;
                    case VT_R4:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%f", pvarSrc->fltVal);
                        break;
                    case VT_R8:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%lf", pvarSrc->dblVal);
                        break;
                    case VT_BOOL:
                        swprintf(buffer, sizeof (buffer) / sizeof (wchar_t), L"%d", pvarSrc->boolVal);
                        break;
                    default:
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                }
                pvargDest->bstrVal = ::SysAllocString(buffer);
            }
            break;
        case VT_BOOL:
            switch (pvarSrc->vt) {
                case VT_BOOL:
                    pvargDest->boolVal = pvarSrc->boolVal;
                    break;
                case VT_I1:
                    pvargDest->boolVal = pvarSrc->cVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_I2:
                    pvargDest->boolVal = pvarSrc->iVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_INT:
                case VT_I4:
                    pvargDest->boolVal = pvarSrc->intVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_I8:
                    pvargDest->boolVal = pvarSrc->llVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_UI1:
                    pvargDest->boolVal = pvarSrc->bVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_UI2:
                    pvargDest->boolVal = pvarSrc->uiVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->boolVal = pvarSrc->uintVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_UI8:
                    pvargDest->boolVal = pvarSrc->ullVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_R4:
                    pvargDest->boolVal = pvarSrc->fltVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_R8:
                    pvargDest->boolVal = pvarSrc->dblVal ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_DECIMAL:
                    pvargDest->boolVal = (pvarSrc->decVal.Lo64 || pvarSrc->decVal.Hi32) ? VARIANT_TRUE : VARIANT_FALSE;
                    break;
                case VT_BSTR:
                {
                    double data;
                    int res = swscanf(pvarSrc->bstrVal, L"%lf", &data);
                    if (res)
                        pvargDest->boolVal = data ? VARIANT_TRUE : VARIANT_FALSE;
                    else {
                        pvargDest->vt = VT_EMPTY;
                        return DISP_E_TYPEMISMATCH;
                    }
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_DATE:
            switch (pvarSrc->vt) {
                case VT_DATE:
                    pvargDest->ullVal = pvarSrc->ullVal;
                    break;
                case VT_BSTR:
                {
                    SPA::CScopeUQueue sb;
                    SPA::Utilities::ToUTF8(pvarSrc->bstrVal, ::SysStringLen(pvarSrc->bstrVal), *sb);
                    //assuming string is correct!
                    pvargDest->ullVal = SPA::UDateTime((const char*) sb->GetBuffer()).time;
                }
                    break;
                default:
                    pvargDest->vt = VT_EMPTY;
                    return DISP_E_TYPEMISMATCH;
            }
            break;
        case VT_DECIMAL:
            ::memset(&pvargDest->decVal, 0, sizeof (DECIMAL));
            switch (pvarSrc->vt) {
                case VT_DECIMAL:
                    pvargDest->decVal = pvarSrc->decVal;
                    break;
                case VT_BOOL:
                    if (pvarSrc->boolVal) {
                        pvargDest->decVal.Lo64 = 1;
                        pvargDest->decVal.sign = 0x80;
                    }
                    break;
                case VT_UI1:
                    pvargDest->decVal.Lo64 = pvarSrc->bVal;
                    break;
                case VT_UI2:
                    pvargDest->decVal.Lo64 = pvarSrc->uiVal;
                    break;
                case VT_UINT:
                case VT_UI4:
                    pvargDest->decVal.Lo64 = pvarSrc->uintVal;
                    break;
                case VT_UI8:
                    pvargDest->decVal.Lo64 = pvarSrc->ullVal;
                    break;
                case VT_I1:
                {
                    char str[32] = {0};
                    ::sprintf(str, "%d", pvarSrc->cVal);
                    SPA::ParseDec(str, pvargDest->decVal);
                }
                    break;
                case VT_I2:
                {
                    char str[32] = {0};
                    ::sprintf(str, "%d", pvarSrc->iVal);
                    SPA::ParseDec(str, pvargDest->decVal);
                }
                    break;
                case VT_I4:
                case VT_INT:
                {
                    char str[32] = {0};
                    ::sprintf(str, "%d", pvarSrc->intVal);
                    SPA::ParseDec(str, pvargDest->decVal);
                }
                    break;
                case VT_I8:
                {
                    char str[32] = {0};
                    ::sprintf(str, "%ld", pvarSrc->llVal);
                    SPA::ParseDec(str, pvargDest->decVal);
                }
                    break;
                case VT_R4:
                {
                    char str[64] = {0};
                    ::sprintf(str, "%f", pvarSrc->fltVal);
                    SPA::ParseDec(str, pvargDest->decVal);
                }
                    break;
                case VT_R8:
                {
                    char str[64] = {0};
                    ::sprintf(str, "%lf", pvarSrc->dblVal);
                    SPA::ParseDec(str, pvargDest->decVal);
                }
                    break;
                case VT_BSTR:
                {
                    SPA::CScopeUQueue sb;
                    SPA::Utilities::ToUTF8(pvarSrc->bstrVal, ::SysStringLen(pvarSrc->bstrVal), *sb);
                    //assuming parse correct!
                    SPA::ParseDec_long((const char*) sb->GetBuffer(), pvargDest->decVal);
                }
                    break;
                default:
                    break;
            }
            break;
        case VT_NULL:
        case VT_EMPTY:
            break;
        case VT_CLSID:
        case VT_SAFEARRAY:
        case VT_PTR:
        case VT_HRESULT:
        case VT_RESERVED:
        case VT_VOID:
            pvargDest->vt = VT_EMPTY;
            return DISP_E_BADVARTYPE;
        default:
            pvargDest->vt = VT_EMPTY;
            return DISP_E_TYPEMISMATCH;
    }
    return S_OK;
}

#endif