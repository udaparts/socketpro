
#ifndef _SOCKETPRO_LINUX_VARIANT_DEFINITION_H_
#define _SOCKETPRO_LINUX_VARIANT_DEFINITION_H_

#include "linux_win.h"

class CComVariant : public tagVARIANT {
public:

    CComVariant() noexcept {
    }

    ~CComVariant() noexcept {
        ::VariantClear(this);
    }

    CComVariant(const tagVARIANT& varSrc) {
        Copy(&varSrc);
    }

    CComVariant(const CComVariant& varSrc) {
        Copy(&varSrc);
    }

    CComVariant(const char* lpszSrc) {
        if (lpszSrc) {
            vt = VT_BSTR;
            bstrVal = SPA::Utilities::ToBSTR(lpszSrc);
        }
    }

    CComVariant(const char16_t* lpszSrc) {
        if (lpszSrc) {
            vt = VT_BSTR;
            bstrVal = SysAllocString(lpszSrc);
        }
    }

    CComVariant(const wchar_t* lpszSrc) {
        if (lpszSrc) {
            vt = VT_BSTR;
            bstrVal = SysAllocString(lpszSrc);
        }
    }

    CComVariant(bool bSrc) noexcept {
        vt = VT_BOOL;
        boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
    }

    CComVariant(int nSrc) noexcept {
        vt = VT_I4;
        intVal = nSrc;
    }

    CComVariant(unsigned char nSrc) noexcept {
        vt = VT_UI1;
        bVal = nSrc;
    }

    CComVariant(short nSrc) noexcept {
        vt = VT_I2;
        iVal = nSrc;
    }

    CComVariant(float fltSrc) noexcept {
        vt = VT_R4;
        fltVal = fltSrc;
    }

    CComVariant(double dblSrc) noexcept {
        vt = VT_R8;
        dblVal = dblSrc;
    }

    CComVariant(SPA::INT64 nSrc) noexcept {
        vt = VT_I8;
        llVal = nSrc;
    }

    CComVariant(SPA::UINT64 nSrc) noexcept {
        vt = VT_UI8;
        ullVal = nSrc;
    }

    CComVariant(CY cySrc) noexcept {
        vt = VT_CY;
        cyVal = cySrc;
    }

    CComVariant(const tagDEC& dec) noexcept {
        decVal = dec;
        vt = VT_DECIMAL;
    }

    CComVariant(char cSrc) noexcept {
        vt = VT_I1;
        cVal = cSrc;
    }

    CComVariant(unsigned short nSrc) noexcept {
        vt = VT_UI2;
        uiVal = nSrc;
    }

    CComVariant(unsigned int nSrc) noexcept {
        vt = VT_UI4;
        uintVal = nSrc;
    }

    CComVariant(SPA::UDateTime dt) noexcept {
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
        if (lpszSrc) {
            vt = VT_BSTR;
            bstrVal = SPA::Utilities::ToBSTR(lpszSrc);
        }
        return *this;
    }

    CComVariant& operator=(const char16_t* lpszSrc) {
        Clear();
        if (lpszSrc) {
            vt = VT_BSTR;
            bstrVal = SysAllocString(lpszSrc);
        }
        return *this;
    }

    CComVariant& operator=(const wchar_t* lpszSrc) {
        Clear();
        if (lpszSrc) {
            vt = VT_BSTR;
            bstrVal = SysAllocString(lpszSrc);
        }
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

    bool operator==(const tagVARIANT& varSrc) const noexcept {
        return SPA::IsEqual(*this, varSrc);
    }

    bool operator!=(const tagVARIANT& varSrc) const noexcept {
        return !SPA::IsEqual(*this, varSrc);
    }

    // Operations

    inline HRESULT Clear() {
        return ::VariantClear(this);
    }

    inline HRESULT Copy(const VARIANT* pSrc) {
        return ::VariantCopy(this, const_cast<VARIANT*> (pSrc));
    }
};

HRESULT VariantChangeType(VARIANT *pvargDest, const VARIANT *pvarSrc, unsigned short wFlags, VARTYPE vt);

#endif
