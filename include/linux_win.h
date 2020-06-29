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

#endif
