#ifndef	___UCOMM__UTIL_HEADER_FILE___H___
#define ___UCOMM__UTIL_HEADER_FILE___H___

#include <assert.h>
#include "definebase.h"
#include <sstream>

#ifdef WIN32_64
#if _MSC_VER < 1700
#else
#include <atomic>
#define ATOMIC_AVAILABLE
#endif
#include "wincommutil.h"
#else
#include <atomic>
#define ATOMIC_AVAILABLE
#include "nixcommutil.h"
#ifdef USE_BOOST_LARGE_INTEGER_FOR_DECIMAL
#include <boost/multiprecision/cpp_int.hpp>
#endif
#endif

#define CUExCode(errMsg, errCode)  SPA::CUException(errMsg, __FILE__, __LINE__, __FUNCTION__, errCode)
#define CUEx(errMsg)  CUExCode(errMsg, MB_ERROR_UNKNOWN)
#define CUSEx(errMsg) CUExCode(errMsg, MB_BAD_DESERIALIZATION)

namespace SPA {

    class CSpinLock {
    private:
#ifndef ATOMIC_AVAILABLE
        volatile long m_locked;
#else
        std::atomic<int> m_locked;
#endif

        //no copy constructor
        CSpinLock(const CSpinLock &sl);
        //no assignment operator
        CSpinLock& operator=(const CSpinLock &sl);

#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)
    public:
        std::atomic<UINT64> Contention;
#endif

    public:

        static const UINT64 MAX_CYCLE = (UINT64) (~0);

        CSpinLock()
        : m_locked(0)
#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)
        ,
        Contention(0)
#endif
        {
        }

        /**
         * Lock a critical section
         * @param max_cycle The max spin number
         * @return The actual spin number
         * If the returned value is zero or less than the given max spin number, the locking is successful.
         * Otherwise, the locking is failed
         */
        inline UINT64 lock(UINT64 max_cycle = MAX_CYCLE) volatile {
            UINT64 cycle = 0;
#ifndef ATOMIC_AVAILABLE
            while (::_InterlockedCompareExchange(&m_locked, 1, 0)) {
#else
            int no_lock = 0;
            while (!m_locked.compare_exchange_strong(no_lock, 1, std::memory_order_acquire, std::memory_order_relaxed)) {
                assert(no_lock);
                no_lock = 0;
#endif
                ++cycle;
                if (cycle >= max_cycle) {
                    break;
                }
            }
#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)
            if (cycle) {
                Contention.fetch_add(cycle, std::memory_order_relaxed);
            }
#endif
            return cycle;
        }

        /**
         * Try to lock a critical section
         * @return True if successful, and false if failed
         */
        inline bool try_lock() volatile {
            return (0 == lock(0));
        }

        /**
         * Unlock a critical section
         * @remark Must call the method lock first before calling this method
         */
        inline void unlock() volatile {
            assert(m_locked); //must call the method lock first
#ifdef ATOMIC_AVAILABLE
            m_locked.store(0, std::memory_order_release);
#else
            m_locked = 0;
#endif
        }
    };

    class CSpinAutoLock {
    public:

        /**
         * Create an instance of CSpinAutoLock, and automatically lock a critical section
         */
        CSpinAutoLock(CSpinLock &cs, UINT64 max_cycle = CSpinLock::MAX_CYCLE)
        : m_cs(cs), m_locked(m_cs.lock(max_cycle) < max_cycle) {
            assert(max_cycle); //If max_cycle == 0, m_locked will be wrong!
        }

        /**
         * Destroy an instance of CSpinAutoLock, and automatically unlock a critical section
         */
        ~CSpinAutoLock() {
            if (m_locked) {
                m_cs.unlock();
            }
        }

        inline explicit operator bool() const {
            return m_locked;
        }

    private:
        /// Copy constructor disabled
        CSpinAutoLock(const CSpinAutoLock &al);

        /// Assignment operator disabled
        CSpinAutoLock& operator=(const CSpinAutoLock &al);

        CSpinLock &m_cs;
        bool m_locked;
    };

#ifdef BOOST_MP_CPP_INT_HPP
    using namespace boost::multiprecision;
    typedef number<cpp_int_backend<96, 96, unsigned_magnitude, unchecked, void> > uint96_t;
#endif

    static void ParseDec(const char *data, DECIMAL &dec) {
        assert(data);
        dec.Hi32 = 0;
        dec.wReserved = 0;
        const char* posNegative = ::strchr(data, '-');
        if (posNegative) {
            dec.sign = 0x80;
            data = posNegative;
        } else {
            dec.sign = 0;
        }
        const char *end;
        UINT64 temp = atoull(data, end);
        if (*end == '.') {
            const char *start = ++end;
            UINT64 dot = atoull(end, end);
            unsigned char scale = (unsigned char) (end - start);
            dec.scale = scale;
            while (scale) {
                temp = (temp << 3) + (temp << 1);
                --scale;
            }
            dec.Lo64 = temp + dot;
        } else {
            dec.Lo64 = temp;
            dec.scale = 0;
        }
    }

    static bool ParseDec_long(const char *data, DECIMAL &dec) {
        assert(data);

#ifdef WIN32_64
        dec.Hi32 = 0;
        dec.wReserved = 0;
        CComVariant vtSrc(data), vtDes;
        HRESULT hr = ::VariantChangeType(&vtDes, &vtSrc, 0, VT_DECIMAL);
        if (FAILED(hr)) {
            return false;
        }
        dec = vtDes.decVal;
#else
#ifdef BOOST_MP_CPP_INT_HPP
        dec.Hi32 = 0;
        dec.wReserved = 0;
        const char* posNegative = ::strchr(data, '-');
        if (posNegative) {
            dec.sign = 0x80;
            data = posNegative;
        } else {
            dec.sign = 0;
        }
        uint96_t v;
        const char *end = ::strchr(data, '.');
        if (end) {
            std::string s(data, end);
            ++end;
            dec.scale = (unsigned char) ::strlen(end);
            s += end;
            v.assign(s);
        } else {
            v.assign(data);
            dec.scale = 0;
        }
        dec.Lo64 = (v & 0xffffffffffffffff).convert_to<uint64_t>();
        v >>= 64;
        dec.Hi32 = v.convert_to<unsigned int>();
#else
        ParseDec(data, dec);
#endif
#endif
        return true;
    }

#ifndef WINCE

    static std::string ToString(const DECIMAL &decVal) {
        std::string s = std::to_string(decVal.Lo64);
        unsigned char len = (unsigned char) s.size();
        if (len <= decVal.scale) {
            s.insert(0, (decVal.scale - len) + 1, '0');
        }
        if (decVal.sign && decVal.Lo64) {
            s.insert(0, 1, '-');
        }
        if (decVal.scale) {
            size_t pos = s.length() - decVal.scale;
            s.insert(pos, 1, '.');
        }
        return s;
    }
#endif

    static std::string ToString_long(const DECIMAL &decVal) {
#ifdef WIN32_64
        VARIANT vtSrc, vtDes;
        vtSrc.decVal = decVal;
        vtSrc.vt = VT_DECIMAL;
        vtDes.vt = VT_EMPTY;
        ::VariantChangeType(&vtDes, &vtSrc, 0, VT_BSTR);
        unsigned int len = ::SysStringLen(vtDes.bstrVal);
        std::string s(vtDes.bstrVal, vtDes.bstrVal + len);
        VariantClear(&vtDes);
        return s;
#else
#ifdef BOOST_MP_CPP_INT_HPP
        uint96_t v = decVal.Hi32;
        v <<= 64;
        v += decVal.Lo64;
        std::string s = v.str();
        unsigned char len = (unsigned char) s.size();
        if (len <= decVal.scale) {
            s.insert(0, (decVal.scale - len) + 1, '0');
        }
        if (decVal.sign && (decVal.Lo64 || decVal.Hi32)) {
            s.insert(0, 1, '-');
        }
        if (decVal.scale) {
            size_t pos = s.length() - decVal.scale;
            s.insert(pos, 1, '.');
        }
        return s;
#else
        return ToString(decVal);
#endif
#endif
    }

    static inline double ToDouble(const DECIMAL &dec) {
        unsigned char scale = dec.scale;
        double d = (double) dec.Lo64;
        UINT64 temp = 1;
        while (scale) {
            temp = (temp << 3) + (temp << 1);
            --scale;
        }
        d /= temp;
        if (dec.sign) {
            d = -d;
        }
        return d;
    }

    static inline void ToDecimal(double d, DECIMAL &dec, int precision = -1) {
        if (precision > 28) {
            precision = 28;
        }
        char str[64] = {0};
        if (precision < 0) {
#if defined (WIN32_64) && _MSC_VER >= 1600 
            sprintf_s(str, sizeof (str), "%f", d);
#else
            sprintf(str, "%f", d);
#endif
        } else {
#if defined (WIN32_64) && _MSC_VER >= 1600 
            sprintf_s(str, sizeof (str), "%.*f", precision, d);
#else
            sprintf(str, "%.*f", precision, d);
#endif
        }
        ParseDec_long(str, dec);
    }

    static inline void ToDecimal(INT64 n, DECIMAL &dec) {
        memset(&dec, 0, sizeof (dec));
        if (n < 0) {
            dec.sign = 0x80;
            dec.Lo64 = (UINT64) (-n);
        } else {
            dec.Lo64 = (UINT64) n;
        }
    }

};

#ifndef WIN32_64	
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
            if (!vt0.parray && vt1.parray)
                return false;
            if (vt0.parray && !vt1.parray)
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
                        if (vt0.bstrVal && vt1.bstrVal) {
                            unsigned int chars = SysStringLen(vt0.bstrVal);
                            unsigned int charsSrc = SysStringLen(vt1.bstrVal);
                            if (chars == charsSrc) {
                                equal = (::memcmp(vt0.bstrVal, vt1.bstrVal, chars * sizeof (wchar_t)) == 0);
                            } else {
                                equal = false;
                            }
                        } else {
                            equal = (vt0.bstrVal == vt1.bstrVal);
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
                    if (!vt0.bstrVal && vt1.bstrVal)
                        return false;
                    if (vt0.bstrVal && !vt1.bstrVal)
                        return false;
                    unsigned int len = SysStringLen(vt0.bstrVal);
                    unsigned int lenSrc = SysStringLen(vt1.bstrVal);
                    if (len != lenSrc)
                        return false;
                    return (::memcmp(vt0.bstrVal, vt1.bstrVal, len * sizeof (wchar_t)) == 0);
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
                    return (::memcmp(&vt0.decVal, &vt1.decVal, sizeof (tagDEC)) == 0);
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
    inline static unsigned short Map2VarintType(const UVariant &v) {
        return v.vt;
    }
};

#endif
