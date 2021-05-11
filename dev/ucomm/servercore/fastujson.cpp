#include "../core_shared/pinc/fastujson.h"

namespace SPA
{

    UJsonValue MakeJsonValue(const wchar_t * str) {
        if (!str)
            str = L"";
#if defined(__ANDROID__) || defined(ANDROID)
        return Utilities::ToUTF8(str, ::wcslen(str)).c_str();
#else
        CScopeUQueue su;
        Utilities::ToUTF8(str, ::wcslen(str), *su);
        return (const char*) su->GetBuffer();
#endif
    }

#ifndef WIN32_64

    UJsonValue MakeJsonValue(const char16_t * str) {
        if (!str)
            str = u"";
#if defined(__ANDROID__) || defined(ANDROID)
        return Utilities::ToUTF8(str, ::wcslen(str)).c_str();
#else
        CScopeUQueue su;
        Utilities::ToUTF8(str, GetLen(str), *su);
        return (const char*) su->GetBuffer();
#endif
    }
#endif

    UJsonValue MakeJsonValue(const UVariant & vtData) {
        VARTYPE vt = vtData.vt;
        switch (vt) {
            case VT_NULL:
            case VT_EMPTY:
                return nullptr;
            case VT_I1:
                return vtData.cVal;
            case VT_I2:
                return vtData.iVal;
            case VT_INT:
            case VT_I4:
                return (int) vtData.lVal;
            case VT_UI1:
                return vtData.bVal;
            case VT_UI2:
                return vtData.uiVal;
            case VT_UINT:
            case VT_UI4:
                return (unsigned int) vtData.ulVal;
#ifdef WINCE
            case VT_I8:
                return MakeJsonValue(vtData.cyVal.int64);
            case VT_UI8:
                return MakeJsonValue((UINT64) (vtData.cyVal.int64));
#else
            case VT_I8:
                return vtData.llVal;
            case VT_UI8:
                return vtData.ullVal;
#endif
            case VT_BOOL:
                return (vtData.boolVal ? true : false);
            case VT_R8:
                return vtData.dblVal;
            case VT_BSTR:
#ifdef WIN32_64
                return MakeJsonValue((const wchar_t *) vtData.bstrVal);
#else
            {
                const char16_t *s = (const char16_t *) vtData.bstrVal;
                return MakeJsonValue(s);
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
                return str;
            }
            case VT_R4:
                return vtData.fltVal;
            case VT_CY:
            {
                double d = (double) vtData.cyVal.int64;
                d /= 10000;
                return d;
            }
            case VT_DECIMAL:
                return SPA::ToDouble(vtData.decVal);
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
                            jv = (const char*) su->GetBuffer();
                        }
                            break;
                        case VT_I2:
                            jv = MakeJsonValue((const short*) pBuffer, size);
                            break;
                        case VT_I4:
                        case VT_INT:
                            jv = MakeJsonValue((const int*) pBuffer, size);
                            break;
                        case VT_UI1:
                            jv = MakeJsonValue((const unsigned char*) pBuffer, size);
                            break;
                        case VT_UI2:
                            jv = MakeJsonValue((const unsigned short*) pBuffer, size);
                            break;
                        case VT_UI4:
                        case VT_UINT:
                            jv = MakeJsonValue((const unsigned int*) pBuffer, size);
                            break;
                        case VT_I8:
                            jv = MakeJsonValue((const SPA::INT64*) pBuffer, size);
                            break;
                        case VT_UI8:
                            jv = MakeJsonValue((const SPA::UINT64*) pBuffer, size);
                            break;
                        case VT_R8:
                            jv = MakeJsonValue((const double*) pBuffer, size);
                            break;
                        case VT_R4:
                            jv = MakeJsonValue((const float*) pBuffer, size);
                            break;
                        case VT_CY:
                        {
                            SPA::CScopeUQueue su;
                            const CY *p = (const CY *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                double d = p[n].int64 / 10000.0;
                                su << d;
                            }
                            jv = MakeJsonValue((const double*) su->GetBuffer(), su->GetSize() / sizeof (double));
                        }
                            break;
                        case VT_DECIMAL:
                        {
                            SPA::CScopeUQueue su;
                            const DECIMAL *p = (const DECIMAL *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                su << SPA::ToDouble(p[n]);
                            }
                            jv = MakeJsonValue((const double*) su->GetBuffer(), su->GetSize() / sizeof (double));
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
                            jv = MakeJsonValue((const bool*) su->GetBuffer(), size);
                        }
                            break;
                        case VT_BSTR:
                        {
                            UJsonArray v;
                            const BSTR *p = (const BSTR *) pBuffer;
#ifdef WIN32_64
                            for (unsigned int n = 0; n < size; ++n) {
                                v.push_back(MakeJsonValue((const wchar_t *) (p[n])));
                            }
#else
                            for (unsigned int n = 0; n < size; ++n) {
                                const char16_t *s = p[n];
                                jv.push_back(MakeJsonValue(s));
                            }
#endif
                            jv = std::move(v);
                        }
                            break;
                        case VT_VARIANT:
                        {
                            UJsonArray v;
                            const UVariant *p = (const UVariant *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                v.push_back(MakeJsonValue(p[n]));
                            }
                            jv = std::move(v);
                        }
                            break;
                        case VT_DATE:
                        {
                            UJsonArray v;
#ifndef WIN32_64
                            const SPA::UINT64 *p = (const SPA::UINT64 *) pBuffer;
                            for (unsigned int n = 0; n < size; ++n) {
                                char str[32] = {0};
                                SPA::UDateTime dt(p[n]);
                                dt.ToWebString(str, sizeof (str));
                                v.push_back(str);
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
                                v.push_back(str);
                            }
#endif
                            jv = std::move(v);
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

    CUQueue & operator << (CUQueue& q, const UJsonValue & jv) {
        boost::json::serializer sr;
        sr.reset(&jv);
        while (!sr.done()) {
            if (q.GetTailSize() <= (unsigned int) 256) {
                q.SetHeadPosition();
                q.ReallocBuffer(q.GetMaxSize() + q.GetBlockSize());
            }
            auto sv = sr.read((char*) q.GetBuffer(q.GetSize()), q.GetTailSize());
            q.SetSize((unsigned int) (q.GetSize() + sv.size()));
        }
        q.SetNull();
        return q;
    }
}
