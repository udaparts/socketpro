
#ifndef _SOCKETPRO_LINUX_VARIANT_DEFINITION_H_
#define _SOCKETPRO_LINUX_VARIANT_DEFINITION_H_

#define E_UNEXPECTED ((int)0x8000FFFF)
#define DISP_E_ARRAYISLOCKED ((int)0x8002000D)
#define E_OUTOFMEMORY ((int)0x8007000E)
#define E_INVALIDARG ((int)0x80070057)

#define S_OK 0

typedef int HRESULT;
typedef unsigned short VARTYPE;
typedef unsigned short WORD;
typedef short VARIANT_BOOL;

typedef wchar_t *BSTR; //It must be a null-terminated string without null char in middle on non-windows platforms.

typedef void *PVOID;

#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

namespace SPA {
    namespace Utilities {
        BSTR ToBSTR(const char *str, size_t chars = (~0));
    }; //namespace Utilities
}; //namespace SPA

inline static unsigned int SysStringLen(const wchar_t* bstr) {
    if (!bstr) {
        return 0;
    }
    return (unsigned int) ::wcslen(bstr);
}

static BSTR SysAllocString(const wchar_t *sz, unsigned int wchars = (~0)) {
    if (!sz && wchars == (unsigned int) (~0)) {
        return nullptr;
    } else if (sz && wchars == (unsigned int) (~0)) {
        wchars = SysStringLen(sz);
    }
    unsigned int bytes = wchars + 1;
    bytes *= sizeof (wchar_t);
    BSTR bstr = (BSTR)::malloc(bytes);
    if (bstr) {
        if (sz) {
            ::memcpy(bstr, sz, bytes - sizeof (wchar_t));
        } else {
            ::memset(bstr, 0, bytes - sizeof (wchar_t));
        }
        bstr[wchars] = 0;
    }
    return bstr;
}

static BSTR SysAllocStringLen(const wchar_t *sz, unsigned int wchars) {
    return SysAllocString(sz, wchars);
}

inline static void SysFreeString(BSTR &bstr) {
    if (bstr) {
        ::free(bstr);
        bstr = nullptr;
    }
}

typedef struct tagSAFEARRAYBOUND {
    unsigned int cElements;
    int lLbound; //It must be 0 on non-windows platforms.
} SAFEARRAYBOUND;

//BITs used by tagSAFEARRAY.fFeatures
#define FADF_HAVEVARTYPE 0x0080 //An array that has a VT type
#define FADF_BSTR 0x0100 //An array of BSTRs
#define FADF_VARIANT 0x0800 //An array of VARIANTs

typedef struct tagSAFEARRAY {

    tagSAFEARRAY() {
        ::memset(this, 0, sizeof (tagSAFEARRAY));
    }
    unsigned short cDims; // Count of dimensions in this array. It must be 1 on non-windows platforms.
    unsigned short fFeatures; // Flags used by the SafeArray.
    unsigned int cbElements; // Size of an element of the array.
    unsigned int cLocks; // Number of times the array has been locked without corresponding unlock.
    PVOID pvData; // Pointer to the data.
    SAFEARRAYBOUND rgsabound[1]; // One bound for each dimension.
} SAFEARRAY;

typedef struct tagVARIANT {

    tagVARIANT() {
        ::memset(this, 0, sizeof (tagVARIANT));
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
    PVOID pvData = buffer + sizeof (SAFEARRAY);
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
    } else if (*ppsaOut) {
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
        ::memset(pvarg, 0, sizeof (tagVARIANT));
    }
}

static HRESULT VariantClear(tagVARIANT *pvarg) {
    HRESULT hr = E_INVALIDARG;
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

class CComVariant : public tagVARIANT {
public:

    CComVariant() {
    }

    ~CComVariant() {
        Clear();
    }

    CComVariant(const tagVARIANT& varSrc) {
        Copy(&varSrc);
    }

    CComVariant(const CComVariant& varSrc) {
        Copy(&varSrc);
    }

    CComVariant(const char* lpszSrc) {
        vt = VT_BSTR;
        bstrVal = SPA::Utilities::ToBSTR(lpszSrc);
    }

    CComVariant(const wchar_t* lpszSrc) {
        vt = VT_BSTR;
        bstrVal = SysAllocString(lpszSrc);
    }

    CComVariant(bool bSrc) {
        vt = VT_BOOL;
        boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
    }

    CComVariant(int nSrc) {
        vt = VT_I4;
        intVal = nSrc;
    }

    CComVariant(unsigned char nSrc) {
        vt = VT_UI1;
        bVal = nSrc;
    }

    CComVariant(short nSrc) {
        vt = VT_I2;
        iVal = nSrc;
    }

    CComVariant(float fltSrc) {
        vt = VT_R4;
        fltVal = fltSrc;
    }

    CComVariant(double dblSrc) {
        vt = VT_R8;
        dblVal = dblSrc;
    }

    CComVariant(SPA::INT64 nSrc) {
        vt = VT_I8;
        llVal = nSrc;
    }

    CComVariant(SPA::UINT64 nSrc) {
        vt = VT_UI8;
        ullVal = nSrc;
    }

    CComVariant(CY cySrc) {
        vt = VT_CY;
        cyVal = cySrc;
    }

    CComVariant(const tagDEC& dec) {
        vt = VT_DECIMAL;
        decVal = dec;
    }

    CComVariant(char cSrc) {
        vt = VT_I1;
        cVal = cSrc;
    }

    CComVariant(unsigned short nSrc) {
        vt = VT_UI2;
        uiVal = nSrc;
    }

    CComVariant(unsigned int nSrc) {
        vt = VT_UI4;
        uintVal = nSrc;
    }

    CComVariant(SPA::UDateTime dt) {
        vt = VT_DATE;
        ullVal = dt.time;
    }

    // Assignment Operators

    CComVariant& operator=(const CComVariant& varSrc) {
        if (this != &varSrc) {
            Copy(&varSrc);
        }
        return *this;
    }

    CComVariant& operator=(const tagVARIANT& varSrc) {
        if (this != &varSrc) {
            Copy(&varSrc);
        }
        return *this;
    }

    CComVariant& operator=(const char* lpszSrc) {
        Clear();
        vt = VT_BSTR;
        bstrVal = SPA::Utilities::ToBSTR(lpszSrc);
        return *this;
    }

    CComVariant& operator=(const wchar_t* lpszSrc) {
        Clear();
        vt = VT_BSTR;
        bstrVal = SysAllocString(lpszSrc);
        return *this;
    }

    CComVariant& operator=(bool bSrc) {
        if (vt != VT_BOOL) {
            Clear();
            vt = VT_BOOL;
        }
        boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
        return *this;
    }

    CComVariant& operator=(int nSrc) {
        if (vt != VT_I4) {
            Clear();
            vt = VT_I4;
        }
        intVal = nSrc;
        return *this;
    }

    CComVariant& operator=(unsigned char nSrc) {
        if (vt != VT_UI1) {
            Clear();
            vt = VT_UI1;
        }
        bVal = nSrc;
        return *this;
    }

    CComVariant& operator=(short nSrc) {
        if (vt != VT_I2) {
            Clear();
            vt = VT_I2;
        }
        iVal = nSrc;
        return *this;
    }

    CComVariant& operator=(float fltSrc) {
        if (vt != VT_R4) {
            Clear();
            vt = VT_R4;
        }
        fltVal = fltSrc;
        return *this;
    }

    CComVariant& operator=(double dblSrc) {
        if (vt != VT_R8) {
            Clear();
            vt = VT_R8;
        }
        dblVal = dblSrc;
        return *this;
    }

    CComVariant& operator=(CY cySrc) {
        if (vt != VT_CY) {
            Clear();
            vt = VT_CY;
        }
        cyVal = cySrc;
        return *this;
    }

    CComVariant& operator=(const tagDEC& dec) {
        if (vt != VT_DECIMAL) {
            Clear();
            vt = VT_DECIMAL;
        }
        decVal = dec;
        return *this;
    }

    CComVariant& operator=(SPA::UDateTime dt) {
        if (vt != VT_DATE) {
            Clear();
            vt = VT_DATE;
        }
        ullVal = dt.time;
        return *this;
    }

    CComVariant& operator=(char cSrc) {
        if (vt != VT_I1) {
            Clear();
            vt = VT_I1;
        }
        cVal = cSrc;
        return *this;
    }

    CComVariant& operator=(unsigned short nSrc) {
        if (vt != VT_UI2) {
            Clear();
            vt = VT_UI2;
        }
        uiVal = nSrc;
        return *this;
    }

    CComVariant& operator=(unsigned int nSrc) {
        if (vt != VT_UI4) {
            Clear();
            vt = VT_UI4;
        }
        uintVal = nSrc;
        return *this;
    }

    CComVariant& operator=(SPA::INT64 nSrc) {
        if (vt != VT_I8) {
            Clear();
            vt = VT_I8;
        }
        llVal = nSrc;
        return *this;
    }

    CComVariant& operator=(SPA::UINT64 nSrc) {
        if (vt != VT_UI8) {
            Clear();
            vt = VT_UI8;
        }
        ullVal = nSrc;
        return *this;
    }

    // Comparison Operators

    bool operator==(const tagVARIANT& varSrc) const {
        return SPA::IsEqual(*this, varSrc);
    }

    bool operator!=(const tagVARIANT& varSrc) const {
        return !operator==(varSrc);
    }

    // Operations

    HRESULT Clear() {
        return VariantClear(this);
    }

    HRESULT Copy(const VARIANT* pSrc) {
        return VariantCopy(this, const_cast<VARIANT*> (pSrc));
    }
};

static HRESULT VariantChangeType(VARIANT *pvargDest, const VARIANT *pvarSrc, unsigned short wFlags, VARTYPE vt) {
	if (!pvargDest || !pvarSrc)
		return E_INVALIDARG;
	::VariantClear(pvargDest);
	pvargDest->vt = vt;
	switch (vt) {
	case VT_I1:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->cVal = pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->cVal = (char)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->cVal = (char)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->cVal = (char)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->cVal = (char)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->cVal = (char)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->cVal = (char)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->cVal = (char)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->cVal = (char)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->cVal = (char)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->cVal = (char)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->cVal = (char)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			INT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
			if (res)
				pvargDest->cVal = (char)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			INT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%lld", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->cVal = (char)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
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
			pvargDest->iVal = (short)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->iVal = (short)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->iVal = (short)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->iVal = (short)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->iVal = (short)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->iVal = (short)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->iVal = (short)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->iVal = (short)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->iVal = (short)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->iVal = (short)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			INT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
			if (res)
				pvargDest->iVal = (short)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			INT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%lld", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->iVal = (short)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			return E_UNEXPECTED;
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
			pvargDest->intVal = (int)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->intVal = (int)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->intVal = (int)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->intVal = (int)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->intVal = (int)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->intVal = (int)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->intVal = (int)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->intVal = (int)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->intVal = (int)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			INT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
			if (res)
				pvargDest->intVal = (int)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			INT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%lld", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->intVal = (int)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
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
			pvargDest->llVal = (INT64)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->llVal = (INT64)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->llVal = (INT64)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->llVal = (INT64)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->llVal = (INT64)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->llVal = (pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->llVal = (INT64)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			INT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
			if (res)
				pvargDest->llVal = (char)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			INT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%lld", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->llVal = data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_R4:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->fltVal = (float)pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->fltVal = (float)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->fltVal = (float)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->fltVal = (float)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->fltVal = (float)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->fltVal = (float)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->fltVal = (float)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->fltVal = (float)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->fltVal = pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->fltVal = (float)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->fltVal = (float)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->fltVal = (float)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			float data;
			int res = swscanf(pvarSrc->bstrVal, L"%f", &data);
			if (res)
				pvargDest->fltVal = data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			float data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%f", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->fltVal = data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_R8:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->dblVal = (double)pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->dblVal = (double)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->dblVal = (double)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->dblVal = (double)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->dblVal = (double)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->dblVal = (double)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->dblVal = (double)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->dblVal = (double)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->dblVal = pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->dblVal = pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->dblVal = (double)(pvarSrc->boolVal ? 1 : 0);
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
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			double data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%lf", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->dblVal = data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_UI1:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->bVal = (unsigned char)pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->bVal = (unsigned char)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->bVal = (unsigned char)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->bVal = (unsigned char)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->bVal = (unsigned char)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->bVal = (unsigned char)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->bVal = (unsigned char)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->bVal = (unsigned char)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->bVal = (unsigned char)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->bVal = (unsigned char)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->bVal = (unsigned char)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->bVal = (unsigned char)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			UINT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
			if (res)
				pvargDest->bVal = (char)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			UINT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%llu", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->bVal = (unsigned char)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_UI2:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->uiVal = (unsigned short)pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->uiVal = (unsigned short)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->uiVal = (unsigned short)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->uiVal = (unsigned short)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->uiVal = (unsigned short)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->uiVal = (unsigned short)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->uiVal = (unsigned short)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->uiVal = (unsigned short)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->uiVal = (unsigned short)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->uiVal = (unsigned short)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->uiVal = (unsigned short)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->uiVal = (unsigned short)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			UINT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
			if (res)
				pvargDest->uiVal = (unsigned short)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			UINT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%llu", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->uiVal = (unsigned short)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_UINT:
	case VT_UI4:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->uintVal = (unsigned int)pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->uintVal = (unsigned int)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->uintVal = (unsigned int)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->uintVal = (unsigned int)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->uintVal = (unsigned int)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->uintVal = (unsigned int)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->uintVal = (unsigned int)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->uintVal = (unsigned int)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->uintVal = (unsigned int)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->uintVal = (unsigned int)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->uintVal = (unsigned int)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->uintVal = (unsigned int)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			UINT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%lld", &data);
			if (res)
				pvargDest->uintVal = (unsigned int)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			UINT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%llu", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->uintVal = (unsigned int)data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_UI8:
		switch (pvarSrc->vt) {
		case VT_I1:
			pvargDest->ullVal = (UINT64)pvarSrc->cVal;
			break;
		case VT_I2:
			pvargDest->ullVal = (UINT64)pvarSrc->iVal;
			break;
		case VT_INT:
		case VT_I4:
			pvargDest->ullVal = (UINT64)pvarSrc->intVal;
			break;
		case VT_I8:
			pvargDest->ullVal = (UINT64)pvarSrc->llVal;
			break;
		case VT_UI1:
			pvargDest->ullVal = (UINT64)pvarSrc->bVal;
			break;
		case VT_UI2:
			pvargDest->ullVal = (UINT64)pvarSrc->uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			pvargDest->ullVal = (UINT64)pvarSrc->ulVal;
			break;
		case VT_UI8:
			pvargDest->ullVal = (UINT64)pvarSrc->ullVal;
			break;
		case VT_R4:
			pvargDest->ullVal = (UINT64)pvarSrc->fltVal;
			break;
		case VT_R8:
			pvargDest->ullVal = (UINT64)pvarSrc->dblVal;
			break;
		case VT_BOOL:
			pvargDest->ullVal = (UINT64)(pvarSrc->boolVal ? 1 : 0);
			break;
		case VT_DECIMAL:
			pvargDest->ullVal = (UINT64)SPA::ToDouble(pvarSrc->decVal);
			break;
		case VT_BSTR:
		{
			UINT64 data;
			int res = swscanf(pvarSrc->bstrVal, L"%llu", &data);
			if (res)
				pvargDest->ullVal = data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
		break;
		case (VT_I1 | VT_ARRAY) :
		{
			UINT64 data;
			const char *s;
			::SafeArrayAccessData(pvarSrc->parray, (void**)&s);
			int res = sscanf(s, "%llu", &data);
			::SafeArrayUnaccessData(pvarSrc->parray);
			if (res)
				pvargDest->ullVal = data;
			else
			{
				pvargDest->vt = VT_EMPTY;
				return E_UNEXPECTED;
			}
		}
								break;
		default:
			pvargDest->vt = VT_EMPTY;
			return E_UNEXPECTED;
		}
		break;
	case VT_BSTR:
		break;
	case (VT_I1 | VT_ARRAY) :
		break;
	case VT_DATE:
		break;
	case VT_DECIMAL:
		break;
	case VT_BOOL:
		break;
	default:
		return E_UNEXPECTED;
	}
	return S_OK;
}

#endif