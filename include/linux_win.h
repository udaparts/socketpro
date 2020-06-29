#ifndef	___SOCKETPRO_LINUX_WIN___H___
#define ___SOCKETPRO_LINUX_WIN___H___

#define E_UNEXPECTED ((int)0x8000FFFF)
#define DISP_E_ARRAYISLOCKED ((int)0x8002000D)
#define E_OUTOFMEMORY ((int)0x8007000E)
#define E_INVALIDARG ((int)0x80070057)
#define DISP_E_TYPEMISMATCH ((int)0x80020005)
#define DISP_E_BADVARTYPE ((int)0x80020008)
#define DISP_E_OVERFLOW ((int)0x8002000A)
#define VARIANT_ALPHABOOL ((unsigned short)0x02)

#define S_OK ((int)0)

typedef int HRESULT;
typedef unsigned short VARTYPE;
typedef unsigned short WORD;
typedef short VARIANT_BOOL;

typedef char16_t *BSTR; //It must be a null-terminated string without null char in middle on non-windows platforms.

typedef void *PVOID;

#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

//
// and the inverse
//
#define FAILED(hr) (((HRESULT)(hr)) < 0)

enum VARENUM {
    VT_EMPTY = 0,
    VT_NULL = 1,
    VT_I2 = 2,
    VT_I4 = 3,
    VT_R4 = 4,
    VT_R8 = 5,
    VT_CY = 6,
    VT_DATE = 7,
    VT_BSTR = 8,
    VT_DISPATCH = 9,
    VT_ERROR = 10,
    VT_BOOL = 11,
    VT_VARIANT = 12,
    VT_UNKNOWN = 13,
    VT_DECIMAL = 14,
    VT_I1 = 16,
    VT_UI1 = 17,
    VT_UI2 = 18,
    VT_UI4 = 19,
    VT_I8 = 20,
    VT_UI8 = 21,
    VT_INT = 22,
    VT_UINT = 23,
    VT_VOID = 24,
    VT_HRESULT = 25,
    VT_PTR = 26,
    VT_SAFEARRAY = 27,
    VT_CARRAY = 28,
    VT_USERDEFINED = 29,
    VT_LPSTR = 30,
    VT_LPWSTR = 31,
    VT_RECORD = 36,
    VT_INT_PTR = 37,
    VT_UINT_PTR = 38,
    VT_FILETIME = 64,
    VT_BLOB = 65,
    VT_STREAM = 66,
    VT_STORAGE = 67,
    VT_STREAMED_OBJECT = 68,
    VT_STORED_OBJECT = 69,
    VT_BLOB_OBJECT = 70,
    VT_CF = 71,
    VT_CLSID = 72,
    VT_VERSIONED_STREAM = 73,
    VT_BSTR_BLOB = 0xfff,
    VT_VECTOR = 0x1000,
    VT_ARRAY = 0x2000,
    VT_BYREF = 0x4000,
    VT_RESERVED = 0x8000,
    VT_ILLEGAL = 0xffff,
    VT_ILLEGALMASKED = 0xfff,
    VT_TYPEMASK = 0xfff
};

typedef struct tagSAFEARRAYBOUND {
    unsigned int cElements;
    int lLbound; //It must be 0 on non-windows platforms.
} SAFEARRAYBOUND;

//BITs used by tagSAFEARRAY.fFeatures
#define FADF_HAVEVARTYPE 0x0080 //An array that has a VT type
#define FADF_BSTR 0x0100 //An array of BSTRs
#define FADF_VARIANT 0x0800 //An array of VARIANTs

typedef struct tagSAFEARRAY {
    //make sure this is the same with windows
#if 0

    tagSAFEARRAY() : cDims(0), fFeatures(0), cbElements(0), cLocks(0), pvData(nullptr) {
    }
#endif
    unsigned short cDims; // Count of dimensions in this array. It must be 1 on non-windows platforms.
    unsigned short fFeatures; // Flags used by the SafeArray.
    unsigned int cbElements; // Size of an element of the array.
    unsigned int cLocks; // Number of times the array has been locked without corresponding unlock.
    unsigned char* pvData; // Pointer to the data.
    SAFEARRAYBOUND rgsabound[1]; // One bound for each dimension.
} SAFEARRAY;

typedef struct tagVARIANT {

    tagVARIANT() : vt(VT_EMPTY) {
    }
    VARTYPE vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;

    union {
        SPA::INT64 llVal;
        int lVal;
        unsigned char bVal;
        short iVal;
        float fltVal;
        double dblVal;
        VARIANT_BOOL boolVal;
        HRESULT scode;
        CY cyVal;
        BSTR bstrVal;
        SAFEARRAY *parray;
        char cVal;
        unsigned short uiVal;
        unsigned int ulVal;
        SPA::UINT64 ullVal;
        int intVal;
        unsigned int uintVal;
        DECIMAL decVal;
    };
} VARIANT;

static_assert(sizeof (tagVARIANT) == 24, "Bad tagVARIANT structure size!");

namespace SPA {
    namespace Utilities {
        BSTR ToBSTR(const char *str, size_t chars = (~0));
        BSTR SysAllocStringLen(const wchar_t *sz, unsigned int wchars = (unsigned int) (~0));
    }; //namespace Utilities
}; //namespace SPA

using SPA::Utilities::SysAllocStringLen;

inline static unsigned int SysStringLen(const char16_t* bstr) {
    if (!bstr) {
        return 0;
    }
    return (unsigned int) SPA::GetLen(bstr);
}

static BSTR SysAllocStringLen(const char16_t *sz, unsigned int chars = (unsigned int) (~0)) {
    if (!sz) {
        chars = 0;
    } else if (chars == (unsigned int) (~0)) {
        chars = SPA::GetLen(sz);
    } else {
        assert(chars <= SPA::GetLen(sz));
    }
    unsigned int bytes = chars;
    bytes <<= 1;
    BSTR bstr = (BSTR)::malloc(bytes + sizeof (char16_t));
    if (bstr) {
        if (sz && chars) {
            ::memcpy(bstr, sz, bytes);
        }
        bstr[chars] = 0;
    }
    return bstr;
}

static BSTR SysAllocString(const char16_t *sz) {
    if (!sz) {
        return nullptr;
    }
    return SysAllocStringLen(sz, (unsigned int) SPA::GetLen(sz));
}

BSTR SysAllocString(const wchar_t *sz);

inline static void SysFreeString(BSTR &bstr) {
    if (bstr) {
        ::free(bstr);
        bstr = nullptr;
    }
}

inline static HRESULT SafeArrayGetVartype(SAFEARRAY *psa, VARTYPE *pvt) {
    if (!psa) {
        assert(false);
        return E_INVALIDARG;
    }
    if (!pvt) {
        assert(false);
        return S_OK;
    }
    *pvt = (psa->fFeatures & (~(FADF_BSTR | FADF_HAVEVARTYPE)));
    return S_OK;
}

static SAFEARRAY* SafeArrayCreate(VARTYPE vt, unsigned int cDims, SAFEARRAYBOUND *rgsabound) {
    assert(cDims == 1); //we support one dimension array only!
    if (cDims != 1)
        return nullptr;
    if (!rgsabound || rgsabound->lLbound) {
        //low bound must start from 0;
        assert(false);
        return nullptr;
    }
    unsigned int bytes = sizeof (SAFEARRAY);
    unsigned int cbElements;
    switch (vt) {
        case VT_I1:
        case VT_UI1:
            cbElements = 1;
            break;
        case VT_I2:
        case VT_UI2:
        case VT_BOOL:
            cbElements = 2;
            break;
        case VT_I4:
        case VT_UI4:
        case VT_INT:
        case VT_UINT:
        case VT_R4:
            cbElements = 4;
            break;
        case VT_DATE:
        case VT_R8:
        case VT_I8:
        case VT_UI8:
            cbElements = 8;
            break;
        case VT_DECIMAL:
            cbElements = sizeof (tagDEC);
            break;
        case VT_CY:
            cbElements = sizeof (tagCY);
            break;
        case VT_BSTR:
            cbElements = sizeof (BSTR);
            break;
        case VT_VARIANT:
            cbElements = sizeof (tagVARIANT);
            break;
        default:
            //shouldn't come here
            assert(false);
            return nullptr;
            break;
    }
    bytes += (cbElements * rgsabound->cElements + sizeof (char)); //add one extra byte for null char
    unsigned char *buffer = (unsigned char*) ::malloc(bytes);
    if (!buffer)
        return nullptr;
    unsigned char* pvData = buffer + sizeof (SAFEARRAY);
    ::memset(pvData, 0, bytes - sizeof (SAFEARRAY));
    SAFEARRAY *sa = (SAFEARRAY *) buffer;
    sa->cbElements = cbElements;
    sa->cDims = cDims;
    sa->cLocks = 0;
    switch (vt) {
        case VT_BSTR:
            sa->fFeatures = (FADF_HAVEVARTYPE | FADF_BSTR | VT_BSTR);
            break;
        case VT_VARIANT:
            sa->fFeatures = (FADF_HAVEVARTYPE | FADF_VARIANT);
            break;
        default:
            sa->fFeatures = (FADF_HAVEVARTYPE | vt);
            break;
    }
    sa->rgsabound[0] = rgsabound[0];
    sa->pvData = pvData;
    return sa;
}

inline static HRESULT SafeArrayAccessData(SAFEARRAY* psa, void** ppvData) {
    if (!psa) {
        assert(false);
        return E_INVALIDARG;
    }
    if (psa->cLocks) {
        assert(false);
        return E_UNEXPECTED;
    }
    if (!ppvData) {
        assert(false);
        return S_OK;
    }
    ++psa->cLocks;
    *ppvData = psa->pvData;
    return S_OK;
}

inline static HRESULT SafeArrayUnaccessData(SAFEARRAY *psa) {
    if (!psa) {
        assert(false);
        return E_INVALIDARG;
    }
    if (!psa->cLocks) {
        assert(false);
        return E_UNEXPECTED;
    }
    --psa->cLocks;
    return S_OK;
}

static HRESULT VariantClear(tagVARIANT *pvarg);

inline static HRESULT SafeArrayDestroy(SAFEARRAY *&psa) {
    if (!psa) {
        return S_OK;
    }
    if (psa->cLocks) {
        assert(false);
        return DISP_E_ARRAYISLOCKED;
    }
    if ((psa->fFeatures & FADF_BSTR) == FADF_BSTR) {
        BSTR *p = nullptr;
        unsigned int count = psa->rgsabound->cElements;
        SafeArrayAccessData(psa, (void**) &p);
        for (unsigned int n = 0; n < count; ++n) {
            SysFreeString(p[n]);
        }
    } else if ((psa->fFeatures & FADF_VARIANT) == FADF_VARIANT) {
        VARIANT *p = nullptr;
        unsigned int count = psa->rgsabound->cElements;
        SafeArrayAccessData(psa, (void**) &p);
        for (unsigned int n = 0; n < count; ++n) {
            VariantClear(p + n);
        }
    }
    ::free(psa);
    psa = nullptr;
    return S_OK;
}

static HRESULT VariantCopy(tagVARIANT *pvargDest, tagVARIANT *pvargSrc);

inline static HRESULT SafeArrayCopy(SAFEARRAY *psa, SAFEARRAY **ppsaOut) {
    if (!psa) {
        assert(false);
        return E_INVALIDARG;
    }
    if (!ppsaOut) {
        assert(false);
        return E_INVALIDARG;
    }
    unsigned int bytes = sizeof (SAFEARRAY) + psa->rgsabound->cElements * psa->cbElements + sizeof (char); //add one extra byte for null char
    unsigned char *buffer = (unsigned char*) ::malloc(bytes);
    if (!buffer) {
        return E_OUTOFMEMORY;
    }
    ::memcpy(buffer, psa, bytes);
    *ppsaOut = (SAFEARRAY*) buffer;
    (*ppsaOut)->pvData = (buffer + sizeof (SAFEARRAY));
    if ((psa->fFeatures & FADF_BSTR) == FADF_BSTR) {
        BSTR *pOut = (BSTR*) ((*ppsaOut)->pvData);
        BSTR *p = (BSTR*) (psa->pvData);
        unsigned int count = psa->rgsabound->cElements;
        for (unsigned int n = 0; n < count; ++n) {
            pOut[n] = SysAllocString(p[n]);
        }
    } else if ((psa->fFeatures & FADF_VARIANT) == FADF_VARIANT) {
        VARIANT *pOut = (VARIANT*) ((*ppsaOut)->pvData);
        VARIANT *p = (VARIANT*) (psa->pvData);
        unsigned int count = psa->rgsabound->cElements;
        ::memset(pOut, 0, count * sizeof (VARIANT));
        for (unsigned int n = 0; n < count; ++n) {
            VariantCopy(pOut + n, p + n);
        }
    }
    return S_OK;
}

inline static void VariantInit(tagVARIANT *pvarg) {
    if (pvarg) {
        pvarg->vt = VT_EMPTY;
    }
}

inline static HRESULT VariantClear(tagVARIANT *pvarg) {
    HRESULT hr;
    if (pvarg) {
        switch (pvarg->vt) {
            case VT_BSTR:
                SysFreeString(pvarg->bstrVal);
                hr = S_OK;
                break;
            default:
                if ((pvarg->vt & VT_ARRAY) == VT_ARRAY) {
                    hr = SafeArrayDestroy(pvarg->parray);
                } else {
                    hr = S_OK;
                }
                break;
        }
        pvarg->vt = VT_EMPTY;
    } else {
        hr = S_OK;
    }
    return hr;
}

static HRESULT VariantCopy(tagVARIANT *pvargDest, tagVARIANT *pvargSrc) {
    if (!pvargDest || !pvargSrc) {
        assert(false);
        return E_INVALIDARG;
    }
    HRESULT hr = VariantClear(pvargDest);
    if (hr != S_OK) {
        return hr;
    }
    if (pvargSrc->vt == VT_BSTR) {
        pvargDest->bstrVal = SysAllocString(pvargSrc->bstrVal);
        if (!pvargDest->bstrVal) {
            return E_OUTOFMEMORY;
        }
        pvargDest->vt = VT_BSTR;
    } else if ((pvargSrc->vt & VT_ARRAY) == VT_ARRAY) {
        hr = SafeArrayCopy(pvargSrc->parray, &pvargDest->parray);
        if (hr == S_OK) {
            pvargDest->vt = pvargSrc->vt;
        }
    } else {
        *pvargDest = *pvargSrc;
    }
    return hr;
}

namespace SPA {
    static bool IsEqual(const tagVARIANT &vt0, const tagVARIANT &vt1);
}

#endif
