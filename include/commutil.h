#ifndef	___UCOMM__UTIL_HEADER_FILE___H___
#define ___UCOMM__UTIL_HEADER_FILE___H___

#include <assert.h>
#include "definebase.h"
#include <sstream>

#ifdef WIN32_64
typedef DWORD UTHREAD_ID;
#pragma warning (disable: 4244)  //'argument': conversion from 'OLECHAR' to 'const _Elem', possible loss of data
#if _MSC_VER < 1700
#else
#include <atomic>
#define ATOMIC_AVAILABLE
#endif
#include "wincommutil.h"
#else
typedef pthread_t UTHREAD_ID;
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
        CSpinLock(const CSpinLock &sl) = delete;
        //no assignment operator
        CSpinLock& operator=(const CSpinLock &sl) = delete;

#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)
    public:
        std::atomic<UINT64> Contention;
#endif

    public:

        static const UINT64 MAX_CYCLE = (UINT64) (~0);

        CSpinLock() noexcept
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
         * @return true if the locking is successful. Otherwise, the locking is failed
         */
        inline bool lock(UINT64 max_cycle = MAX_CYCLE) volatile noexcept {
#ifndef ATOMIC_AVAILABLE
            while (::_InterlockedCompareExchange(&m_locked, 1, 0)) {
#else
            int no_lock = 0;
            while (!m_locked.compare_exchange_strong(no_lock, 1, std::memory_order_acquire, std::memory_order_relaxed)) {
                no_lock = 0;
#endif
#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)
                Contention.fetch_add(1, std::memory_order_relaxed);
#endif
                if (max_cycle > 1) {
                    --max_cycle;
                } else {
                    return false;
                }
            }
            return true;
        }

        /**
         * Try to lock a critical section
         * @return True if successful, and false if failed
         */
        inline bool try_lock() volatile noexcept {
            return lock(1);
        }

        /**
         * Unlock a critical section
         * @remark Must call the method lock first before calling this method
         */
        inline void unlock() volatile noexcept {
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
        CSpinAutoLock(CSpinLock &cs, UINT64 max_cycle = CSpinLock::MAX_CYCLE) noexcept
        : m_cs(cs), m_locked(cs.lock(max_cycle)) {
        }

        /**
         * Destroy an instance of CSpinAutoLock, and automatically unlock a critical section
         */
        ~CSpinAutoLock() noexcept {
            if (m_locked) {
                m_cs.unlock();
            }
        }

        inline operator bool() const noexcept {
            return m_locked;
        }

    private:
        /// Copy constructor disabled
        CSpinAutoLock(const CSpinAutoLock &al) = delete;

        /// Assignment operator disabled
        CSpinAutoLock& operator=(const CSpinAutoLock &al) = delete;

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
        if (!data) {
            return false;
        }
#ifdef WIN32_64
        wchar_t buffer[64] = {0};
        size_t len = ::strlen(data);
        assert(len < sizeof (buffer) / sizeof (wchar_t));
        if (len >= sizeof (buffer) / sizeof (wchar_t)) {
            return false;
        }
        for (size_t n = 0; n < len; ++n) {
            buffer[n] = data[n];
        }
        HRESULT hr = ::VarDecFromStr(buffer, ::GetThreadLocale(), 0, &dec);
        if (FAILED(hr)) {
            return false;
        }
        dec.wReserved = 0;
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
        BSTR bstr;
        ::VarBstrFromDec(&decVal, ::GetThreadLocale(), 0, &bstr);
        std::string s(bstr, bstr + ::SysStringLen(bstr));
        ::SysFreeString(bstr);
        return std::move(s);
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

    static inline void ToDecimal(INT64 n, DECIMAL &dec) noexcept {
        memset(&dec, 0, sizeof (dec));
        if (n < 0) {
            dec.sign = 0x80;
            dec.Lo64 = (UINT64) (-n);
        } else {
            dec.Lo64 = (UINT64) n;
        }
    }

};

#endif
