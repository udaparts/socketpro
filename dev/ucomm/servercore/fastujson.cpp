#include "../core_shared/pinc/fastujson.h"

#ifdef	___U_VARIANT_TO_RAPID_JSON___H___

SPA::CUCriticalSection rapidjson::CrtAllocator::_cs;
std::vector<void*> rapidjson::CrtAllocator::_vBuffer;
size_t rapidjson::CrtAllocator::_allocatedSize = 0;

#endif

namespace SPA{

    UJsonValue MakeJsonValue(const char *str, UMemoryPoolAllocator & allocator) {
        return UJsonValue(str, allocator);
    }

    UJsonValue MakeJsonValue(const wchar_t *str, UMemoryPoolAllocator & allocator) {
        if (!str)
            str = L"";
#if defined(__ANDROID__) || defined(ANDROID)
        return MakeJsonValue(Utilities::ToUTF8(str, ::wcslen(str)).c_str(), allocator);
#else
        CScopeUQueue su;
        Utilities::ToUTF8(str, ::wcslen(str), *su);
        return MakeJsonValue((const char*) su->GetBuffer(), allocator);
#endif
    }

#ifndef WIN32_64

    UJsonValue MakeJsonValue(const char16_t *str, UMemoryPoolAllocator & allocator) {
        if (!str)
            str = u"";
#if defined(__ANDROID__) || defined(ANDROID)
        return MakeJsonValue(Utilities::ToUTF8(str, ::wcslen(str)).c_str(), allocator);
#else
        CScopeUQueue su;
        Utilities::ToUTF8(str, GetLen(str), *su);
        return MakeJsonValue((const char*) su->GetBuffer(), allocator);
#endif
    }
#endif

    UJsonValue MakeJsonValue(const UVariant &vtData, UMemoryPoolAllocator & allocator) {
        UJsonValue jv;
        VARTYPE vt = vtData.vt;
        switch (vt) {
            case VT_NULL:
            case VT_EMPTY:
                return jv;
            case VT_I1:
                return MakeJsonValue((int) vtData.cVal);
            case VT_I2:
                return MakeJsonValue((int) vtData.iVal);
            case VT_INT:
            case VT_I4:
                return MakeJsonValue((int) vtData.lVal);
            case VT_UI1:
                return MakeJsonValue((unsigned int) vtData.bVal);
            case VT_UI2:
                return MakeJsonValue((unsigned int) vtData.uiVal);
            case VT_UINT:
            case VT_UI4:
                return MakeJsonValue((unsigned int) vtData.ulVal);
#ifdef WINCE
            case VT_I8:
                return MakeJsonValue(vtData.cyVal.int64);
            case VT_UI8:
                return MakeJsonValue((UINT64) (vtData.cyVal.int64));
#else
            case VT_I8:
                return MakeJsonValue(vtData.llVal);
            case VT_UI8:
                return MakeJsonValue(vtData.ullVal);
#endif
            case VT_BOOL:
                return MakeJsonValue(vtData.boolVal ? true : false);
            case VT_R8:
                return MakeJsonValue(vtData.dblVal);
            case VT_BSTR:
#ifdef WIN32_64
                return MakeJsonValue(vtData.bstrVal, allocator);
#else
            {
                const char16_t *s = (const char16_t *) vtData.bstrVal;
                return MakeJsonValue(s, allocator);
            }
#endif
            case VT_DATE:
            {
                char str[32] = {0};
#ifdef WINCE
                SYSTEMTIME st;
                ::VariantTimeToSystemTime(vtData.date, &st);
                ::sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#elif defined(WIN32_64)
                SYSTEMTIME st;
                ::VariantTimeToSystemTime(vtData.date, &st);
                ::sprintf_s(str, sizeof (str), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
                SPA::UDateTime dt(vtData.ullVal);
                dt.ToWebString(str, sizeof (str));
#endif
                return MakeJsonValue(str, allocator);
            }
            case VT_R4:
            {
                double d = vtData.fltVal;
                return MakeJsonValue(d);
            }
            case VT_CY:
            {
                double d = (double) vtData.cyVal.int64;
                d /= 10000;
                return MakeJsonValue(d);
            }
            case VT_DECIMAL:
                return MakeJsonValue(SPA::ToDouble(vtData.decVal));
            default:
                if ((vt & VT_ARRAY) == VT_ARRAY) {
                    bool ok = true;
                    vt ^= VT_ARRAY;
                    UJsonValue jv;
                    assert(vtData.parray->cDims == 1);
                    assert(vtData.parray->rgsabound[0].lLbound == 0);
                    const unsigned char *pBuffer = nullptr;
                    unsigned int size = vtData.parray->rgsabound[0].cElements;
                    ::SafeArrayAccessData(vtData.parray, (void**) &pBuffer);
                    assert(pBuffer != nullptr);
                    switch (vt) {
                        case VT_I1:
                        {
                            CScopeUQueue su;
                            su->Push((const char*) pBuffer, size);
                            su->SetNull();
                            jv = MakeJsonValue((const char*) su->GetBuffer(), allocator);
                        }
                            break;
                        case VT_I2:
                            jv = MakeJsonValue((const short*) pBuffer, size, allocator);
                            break;
                        case VT_I4:
                        case VT_INT:
                            jv = MakeJsonValue((const int*) pBuffer, size, allocator);
                            break;
                        case VT_UI1:
                            jv = MakeJsonValue((const unsigned char*) pBuffer, size, allocator);
                            break;
                        case VT_UI2:
                            jv = MakeJsonValue((const unsigned short*) pBuffer, size, allocator);
                            break;
                        case VT_UI4:
                        case VT_UINT:
                            jv = MakeJsonValue((const unsigned int*) pBuffer, size, allocator);
                            break;
                        case VT_I8:
                            jv = MakeJsonValue((const SPA::INT64*) pBuffer, size, allocator);
                            break;
                        case VT_UI8:
                            jv = MakeJsonValue((const SPA::UINT64*) pBuffer, size, allocator);
                            break;
                        case VT_R8:
                            jv = MakeJsonValue((const double*) pBuffer, size, allocator);
                            break;
                        case VT_R4:
                            jv = MakeJsonValue((const float*) pBuffer, size, allocator);
                            break;
                        case VT_CY:
                        {
                            SPA::CScopeUQueue su;
                            const CY *p = (const CY *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                double d = p[n].int64 / 10000.0;
                                su << d;
                            }
                            jv = MakeJsonValue((const double*) su->GetBuffer(), su->GetSize() / sizeof (double), allocator);
                        }
                            break;
                        case VT_DECIMAL:
                        {
                            SPA::CScopeUQueue su;
                            const DECIMAL *p = (const DECIMAL *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                su << SPA::ToDouble(p[n]);
                            }
                            jv = MakeJsonValue((const double*) su->GetBuffer(), su->GetSize() / sizeof (double), allocator);
                        }
                            break;
                        case VT_BOOL:
                        {
                            SPA::CScopeUQueue su;
                            VARIANT_BOOL *p = (VARIANT_BOOL *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                bool b = p[n] ? true : false;
                                su->Push((const unsigned char*) &b, sizeof (b));
                            }
                            jv = MakeJsonValue((const bool*) su->GetBuffer(), size, allocator);
                        }
                            break;
                        case VT_BSTR:
                        {
                            jv.SetArray();
                            const BSTR *p = (const BSTR *) pBuffer;
#ifdef WIN32_64
                            for (unsigned int n = 0; n < size; ++n) {
                                jv.PushBack(MakeJsonValue(p[n], allocator), allocator);
                            }
#else
                            for (unsigned int n = 0; n < size; ++n) {
                                const char16_t *s = p[n];
                                jv.PushBack(MakeJsonValue(s, allocator), allocator);
                            }
#endif
                        }
                            break;
                        case VT_VARIANT:
                        {
                            jv.SetArray();
                            const UVariant *p = (const UVariant *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                jv.PushBack(MakeJsonValue(p[n], allocator), allocator);
                            }
                        }
                            break;
                        case VT_DATE:
                        {
                            jv.SetArray();
#ifndef WIN32_64
                            const SPA::UINT64 *p = (const SPA::UINT64 *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                char str[32] = {0};
                                SPA::UDateTime dt(p[n]);
                                dt.ToWebString(str, sizeof (str));
                                jv.PushBack(MakeJsonValue(str, allocator), allocator);
                            }
#else
                            const DATE *p = (const DATE *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                SYSTEMTIME st;
                                ::VariantTimeToSystemTime(p[n], &st);
                                char str[32] = {0};
#ifdef WINCE
                                ::sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
#else
                                ::sprintf_s(str, sizeof (str), "%04d-%02d-%02dT%02d:%02d:%02dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
#endif
                                jv.PushBack(MakeJsonValue(str, allocator), allocator);
                            }
#endif
                        }
                            break;
                        default:
                            ok = false;
                            break;
                    }
                    ::SafeArrayUnaccessData(vtData.parray);
                    if (ok)
                        return jv;
                }
                break;
        }
        throw CUExCode("Data type not supported for JSON value conversion.", MB_NOT_SUPPORTED);

    }
}
