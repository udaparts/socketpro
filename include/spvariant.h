#ifndef	___SOCKETPRO_VARIANT___H___
#define ___SOCKETPRO_VARIANT___H___

#include "definebase.h"

#ifdef WIN32_64
#include <atlcomcli.h>
#else
#include "bvariant.h"
#endif

namespace SPA {

    static const VARTYPE VT_XML = 35;
    static const VARTYPE VT_DATETIMEOFFSET = 2816;
    static const VARTYPE VT_TIMESPAN = 3072;
    static const VARTYPE VT_USERIALIZER_OBJECT = 3328;
    static const VARTYPE VT_NETObject = 3584;

    typedef CComVariant UVariant;

    static bool IsEqual(const tagVARIANT &vt0, const tagVARIANT &vt1) {
        if (vt0.vt == VT_EMPTY && vt1.vt == VT_EMPTY) {
            return true;
        } else if (vt0.vt == VT_NULL && vt1.vt == VT_NULL) {
            return true;
        }
        VARTYPE vtMe = vt0.vt;
        VARTYPE vtSrc = vt1.vt;
        if (vtMe != vtSrc) {
            if (vtMe == VT_INT) {
                vtMe = VT_I4;
            } else if (vtMe == VT_UINT) {
                vtMe = VT_UI4;
            } else if (vtMe == (VT_INT | VT_ARRAY)) {
                vtMe = (VT_I4 | VT_ARRAY);
            } else if (vtMe == (VT_UINT | VT_ARRAY)) {
                vtMe = (VT_UI4 | VT_ARRAY);
            }

            if (vtSrc == VT_INT) {
                vtSrc = VT_I4;
            } else if (vtSrc == VT_UINT) {
                vtSrc = VT_UI4;
            } else if (vtSrc == (VT_INT | VT_ARRAY)) {
                vtSrc = (VT_I4 | VT_ARRAY);
            } else if (vtSrc == (VT_UINT | VT_ARRAY)) {
                vtSrc = (VT_UI4 | VT_ARRAY);
            }
        }
        if (vtMe != vtSrc)
            return false;
        if ((vtMe & VT_ARRAY) == VT_ARRAY) {
            if (vt0.parray == vt1.parray)
                return true;
            if (!vt0.parray || !vt1.parray)
                return false;
            unsigned int len = vt0.parray->rgsabound->cElements;
            unsigned int lenSrc = vt1.parray->rgsabound->cElements;
            if (len != lenSrc)
                return false;
            PVOID p, pSrc;
            bool equal = true, fixed = false;
            SafeArrayAccessData(vt1.parray, &pSrc);
            SafeArrayAccessData(vt0.parray, &p);
            for (unsigned int n = 0; n < len; ++n) {
                switch (vtMe & (~VT_ARRAY)) {
                    case VT_BSTR:
                    {
                        BSTR *pp0 = (BSTR*) p;
                        BSTR *pp1 = (BSTR*) pSrc;
                        BSTR bs0 = pp0[n];
                        BSTR bs1 = pp1[n];
                        if (bs0 == bs1 && nullptr == bs0) {

                        } else if (!bs0 || !bs1) {
                            equal = false;
                        } else {
                            unsigned int chars = SysStringLen(bs0);
                            unsigned int charsSrc = SysStringLen(bs1);
                            if (chars == charsSrc) {
                                equal = (::memcmp(bs0, bs1, chars * sizeof (UTF16)) == 0);
                            } else {
                                equal = false;
                            }
                        }
                    }
                        break;
                    case VT_DECIMAL:
                    {
                        DECIMAL *p0 = (DECIMAL*) p;
                        DECIMAL *p1 = (DECIMAL*) pSrc;
#ifdef WIN32_64
                        DECIMAL &dec0 = p0[n];
                        DECIMAL &dec1 = p1[n];
                        equal = (dec0.Lo64 == dec1.Lo64 && dec0.Hi32 == dec1.Hi32 && dec0.signscale == dec1.signscale);
#else
                        equal = (p0[n] == p1[n]);
#endif
                    }
                        break;
                    case VT_VARIANT:
                        equal = IsEqual(((const tagVARIANT*) p)[n], ((const tagVARIANT*) pSrc)[n]);
                        break;
                    default:
                        fixed = true;
                        break;
                }
                if (!equal || fixed)
                    break;
            }
            if (equal && fixed) {
                equal = (::memcmp(p, pSrc, len * vt0.parray->cbElements) == 0);
            }
            SafeArrayUnaccessData(vt0.parray);
            SafeArrayUnaccessData(vt1.parray);
            return equal;
        } else {
            switch (vtMe) {
                case VT_BSTR:
                {
                    if (vt0.bstrVal == vt1.bstrVal)
                        return true;
                    if (!vt0.bstrVal || !vt1.bstrVal)
                        return false;
                    unsigned int len = SysStringLen(vt0.bstrVal);
                    unsigned int lenSrc = SysStringLen(vt1.bstrVal);
                    if (len != lenSrc)
                        return false;
                    return (::memcmp(vt0.bstrVal, vt1.bstrVal, len * sizeof (UTF16)) == 0);
                }
                case VT_I1:
                case VT_UI1:
                    return (vt0.bVal == vt1.bVal);
                case VT_I2:
                case VT_UI2:
                    return (vt0.uiVal == vt1.uiVal);
                case VT_I4:
                case VT_UI4:
                case VT_INT:
                case VT_UINT:
                    return (vt0.ulVal == vt1.ulVal);
                case VT_I8:
                case VT_UI8:
#ifndef _WIN32_WCE
                    return (vt0.ullVal == vt1.ullVal);
#else
                    return (vt0.cyVal.int64 == vt1.cyVal.int64);
#endif
                case VT_R4:
                    return (vt0.fltVal == vt1.fltVal);
                case VT_R8:
                    return (vt0.dblVal == vt1.dblVal);
                case VT_BOOL:
                {
                    bool b0 = vt0.boolVal ? true : false;
                    bool b1 = vt1.boolVal ? true : false;
                    return (b0 == b1);
                }
                case VT_DATE:
#ifdef WIN32_64
                    return (vt0.date == vt1.date);
#else
                    return (vt0.ullVal == vt1.ullVal);
#endif
                case VT_CY:
                    return (vt0.cyVal.int64 == vt1.cyVal.int64);
                case VT_DECIMAL:
#ifdef WIN32_64
                    return (vt0.decVal.Lo64 == vt1.decVal.Lo64 && vt0.decVal.Hi32 == vt1.decVal.Hi32 && vt0.decVal.signscale == vt1.decVal.signscale);
#else
                    return (vt0.decVal == vt1.decVal);
#endif
                default:
                    assert(false); //not implemented and shouldn't come here
                    break;
            }
        }
        return false;
    }

    /**
     * Map a variant onto MS VARIANT data type
     * @param v A reference to a variant instance
     * @return MS VARIANT data type
     */
    inline static unsigned short Map2VarintType(const UVariant &v) noexcept {
        return v.vt;
    }
};


#endif
