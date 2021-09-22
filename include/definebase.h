
#ifndef __UCOMM_BASE_DEFINES_H_____
#define __UCOMM_BASE_DEFINES_H_____

#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include <ctime>
#include <math.h>
#if defined(WINCE) || defined(UNDER_CE) || defined(_WIN32_WCE)
#include <time.h>
#if (_MSC_VER < 1600)
#define nullptr NULL
#endif
#else
#include <sys/timeb.h>
#include <stdint.h>
#include <typeinfo>
#endif
#include <string.h>

#define MB_ERROR_UNKNOWN				(-1)

#define MB_BAD_DESERIALIZATION			0xAAAA0000
#define MB_SERIALIZATION_NOT_SUPPORTED	0xAAAA0001
#define	MB_BAD_OPERATION				0xAAAA0002
#define MB_BAD_INPUT					0xAAAA0003
#define MB_NOT_SUPPORTED				0xAAAA0004
#define MB_STL_EXCEPTION				0xAAAA0005
#define MB_UNKNOWN_EXCEPTION			0xAAAA0006
#define MB_QUEUE_FILE_NOT_AVAILABLE		0xAAAA0007
#define MB_ALREADY_DEQUEUED				0xAAAA0008
#define MB_ROUTEE_DISCONNECTED			0xAAAA0009
#define MB_REQUEST_ABORTED				0xAAAA000A

#define MAX_USERID_CHARS		255
#define	MAX_PASSWORD_CHARS		255

#ifndef	NDEBUG
#include <iostream>
#endif

namespace SPA {

    /**
     * Defines for supported compression options
     */
    typedef enum class tagZipLevel {
        zlDefault = 0,
        zlBestSpeed = 1,
        zlBestCompression = 2
    } ZipLevel;

    /**
     * 
     */
    enum class tagSocketOption {
        soTcpNoDelay = 1,
        soReuseAddr = 4,
        soKeepAlive = 8,
        soSndBuf = 0x1001, /* send buffer size */
        soRcvBuf = 0x1002, /* receive buffer size */
    };

    /**
     * 
     */
    enum class tagSocketLevel {
        slTcp = 6,
        slSocket = 0xFFFF,
    };

    /**
     * Defines for supported operation systems
     */
    enum class tagOperationSystem {
        osWin = 0,
        osApple = 1,
        osMac = osApple,
        osUnix = 2,
        osLinux = osUnix,
        osBSD = osUnix,
        osAndroid = 3,
        osWinCE = 4, /**< Old window pocket pc, ce or smart phone devices*/
        osWinPhone = osWinCE,
    };

    /**
     * Defines for MS COM object thread apartments for window platforms only
     */
    enum class tagThreadApartment {
        /// no COM apartment involved
        taNone = 0,

        /// STA apartment
        taApartment = 1,

        /// MTA (free) or neutral apartments
        taFree = 2,
    };
};


#if defined(WIN32) || defined(_WIN64) || defined(WINDOWS) || defined(_WIN32_WCE) || defined(WIN32_WCE) || defined(WIN64)
#ifndef WIN32_64
#define WIN32_64
#endif
#define ftime   _ftime

#if _MSC_VER < 1900
#define noexcept
#endif

#include <winsock2.h>
#include <windows.h>

namespace SPA {
    typedef __int64 INT64;
    typedef unsigned __int64 UINT64;
};

#define U_MODULE_HIDDEN
#define U_MODULE_OPENED

#else
#include <stdlib.h>
#include <dlfcn.h>
#include <wchar.h>
#include <sys/time.h>
#include <unistd.h>
#include <memory>

#ifdef __clang__
#define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else

#endif


#define GetProcAddress dlsym
#define FreeLibrary dlclose

#define WINAPI
#define CALLBACK

typedef void* HINSTANCE;

#ifndef BOOST_SYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED 1
#endif

namespace SPA {
    typedef int64_t INT64;
    typedef uint64_t UINT64;
};

#pragma pack(push,1)

typedef union tagCY {

    struct {
        unsigned int Lo;
        int Hi;
    };
    int64_t int64;
} CY;

#pragma pack(pop)

#pragma pack(push,1)

typedef struct tagDEC {
    unsigned short wReserved;

    //make sure this is the same with windows
#if 0

    tagDEC() : wReserved(0), signscale(0), Hi32(0), Lo64(0) noexcept {
    }
#endif

    union {

        struct {
            unsigned char scale;
            unsigned char sign;
        };
        unsigned short signscale;
    };
    unsigned int Hi32;

    union {

        struct {
            unsigned int Lo32;
            unsigned int Mid32;
        };
        uint64_t Lo64;
    };

    inline bool operator==(const tagDEC & dec) const noexcept {
        return (Lo64 == dec.Lo64 && Hi32 == dec.Hi32 && signscale == dec.signscale);
    }
} DECIMAL;
#pragma pack(pop)

static_assert(sizeof (tagDEC) == 16, "Bad tagDEC structure size!");

#if __GNUC__ >= 4
#define U_MODULE_HIDDEN __attribute__ ((visibility ("hidden")))
#define U_MODULE_OPENED __attribute__ ((visibility ("default")))
#else
#define U_MODULE_HIDDEN
#define U_MODULE_OPENED
#endif

#endif

#define INVALID_NUMBER		((SPA::UINT64)(~0))

#ifdef __cplusplus
extern "C" {
#endif

    void WINAPI SetLastCallInfo(const char *str);
    typedef void (WINAPI *PSetLastCallInfo)(const char *str);

#ifdef __cplusplus
}
#endif

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)

#define WCHAR32
typedef timeval SYSTEMTIME;
#else
#define WCHAR16
#endif

namespace SPA {
    static const wchar_t *EMPTY_INTERNAL = L"";

    /** 
     * Query operation system
     * @return The operation system
     */
    static constexpr tagOperationSystem GetOS() {
#if defined(__ANDROID__) || defined(ANDROID)
        return tagOperationSystem::osAndroid;
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
        return tagOperationSystem::osLinux;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
        return tagOperationSystem::osBSD;
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        return tagOperationSystem::osWin;
#elif defined(UNDER_CE) || defined(_WIN32_WCE) || defined(WIN32_WCE) || defined(WINCE)
        return tagOperationSystem::osWinCE;
#elif defined(_WIN64) || defined(WIN64)
        return tagOperationSystem::osWin;
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
        return tagOperationSystem::osApple;
#else
        return tagOperationSystem::osUnknown;
#endif
    }

    /** 
     * Check if current operation system is big endian
     * @return True for big endian and false for little endian
     */
    static constexpr bool IsBigEndian() {
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        return false;
#elif defined(UNDER_CE) || defined(_WIN32_WCE) || defined(WIN32_WCE)
        return false;
#elif defined(_WIN64) || defined(WIN64)
        return false;
#elif defined(__ANDROID__) || defined(ANDROID)
        return false;
#elif defined(__sparc) || defined(__sparc__) \
	|| defined(_POWER) || defined(__powerpc__) \
	|| defined(__ppc__) || defined(__hpux) || defined(__hppa) \
	|| defined(_MIPSEB) \
	|| defined(__s390__)
        return true;
#else
        return false;
#endif
    }

    /** 
     * Query the size of wide char in byte from a given operation system
     * @param os Operation system
     * @return The size of wide char in byte
     */
    static inline unsigned char GetWCharSize(tagOperationSystem os) noexcept {
        switch (os) {
            case tagOperationSystem::osWin:
            case tagOperationSystem::osWinCE:
                return 2;
                break;
            default:
                break;
        }
        return 4;
    }

    /** 
     * Query if the current system is one of MS window platforms
     * @param os Operation system
     * @return True or false
     */
    static inline bool IsWinOs(tagOperationSystem os) noexcept {
        switch (os) {
            case tagOperationSystem::osWin:
            case tagOperationSystem::osWinCE:
                return true;
                break;
            default:
                break;
        }
        return false;
    }
};

#define MY_OPERATION_SYSTEM  (SPA::GetOS()) 

namespace SPA {

    inline static size_t GetLen(const char *str) {
        if (!str) {
            return 0;
        }
        return ::strlen(str);
    }

    inline static size_t GetLen(const wchar_t *str) {
        if (!str) {
            return 0;
        }
        return ::wcslen(str);
    }

    template<typename TChar>
    inline size_t GetLen(const TChar *str) {
        if (str) {
            const TChar* start = str;
            for (; *str; ++str) {
            }
            return str - start;
        }
        return 0;
    }

#if _MSC_VER < 1900 && defined(WCHAR16)
    typedef wchar_t UTF16;
#else
    typedef char16_t UTF16;
#define NATIVE_UTF16_SUPPORTED
#endif

    template<typename TChar>
    int UCompare(const TChar* s1, const TChar* s2) {
        if (s1 == s2) {
            return 0;
        }
        if (!s1) {
            return -1;
        }
        if (!s2) {
            return 1;
        }
        int res = *s1 - *s2;
        while (!res && *s1++ && *s2++) {
            res = *s1 - *s2;
        }
        if (res < 0) {
            return -1;
        } else if (res > 0) {
            return 1;
        }
        return 0;
    }

    template<typename TChar>
    int UCompareNoCase(const TChar *s1, const TChar *s2) {
        if (s1 == s2) {
            return 0;
        }
        if (!s1) {
            return -1;
        }
        if (!s2) {
            return 1;
        }
        int res = tolower(*s1) - tolower(*s2);
        while (!res && *s1++ && *s2++) {
            res = tolower(*s1) - tolower(*s2);
        }
        return res;
    }

    //The following functions atoxxx work correctly for standard and normal number strings without any preventions.
    //The function atof does NOT support number strings with e or E.
    //These functions are reused for faster parsing string into numbers.

    template<typename TChar>
    inline int atoi(const TChar *p, const TChar *&end) {
        assert(p);
        assert(*p != ' ');
        bool neg;
        if (*p == '-') {
            neg = true;
            ++p;
        } else {
            neg = false;
        }
        int x = 0;
        while (*p >= '0' && *p <= '9') {
            x = (x * 10) + (*p - '0');
            ++p;
        }
        if (neg) {
            x = -x;
        }
        end = p;
        return x;
    }

    template<typename TChar>
    inline unsigned int atoui(const TChar *p, const TChar *&end) {
        assert(p);
        assert(*p != ' ');
        unsigned int x = 0;
        while (*p >= '0' && *p <= '9') {
            x = (x << 3) + (x << 1) + (*p - '0');
            ++p;
        }
        end = p;
        return x;
    }

    template<typename TChar>
    inline INT64 atoll(const TChar *p, const TChar *&end) {
        assert(p);
        assert(*p != ' ');
        bool neg;
        if (*p == '-') {
            neg = true;
            ++p;
        } else {
            neg = false;
        }
        INT64 x = 0;
        while (*p >= '0' && *p <= '9') {
            x = (x * 10) + (*p - '0');
            ++p;
        }
        if (neg) {
            x = -x;
        }
        end = p;
        return x;
    }

    template<typename TChar>
    inline UINT64 atoull(const TChar *p, const TChar *&end) {
        assert(p);
        assert(*p != ' ');
        UINT64 x = 0;
        while (*p >= '0' && *p <= '9') {
            x = (x << 3) + (x << 1) + (*p - '0');
            ++p;
        }
        end = p;
        return x;
    }

    template<typename TChar>
    inline double atof(const TChar *p, const TChar *&end) {
        assert(p);
        assert(*p != ' ');
        bool neg;
        if (*p == '-') {
            neg = true;
            ++p;
        } else {
            neg = false;
        }
        double r = (double) atoull(p, end);
        if (*end == '.') {
            const char *start = ++end;
            double dot = (double) atoull(start, end);
            unsigned int n = (unsigned int) (end - start);
            UINT64 temp = 1;
            while (n) {
                temp = (temp << 3) + (temp << 1);
                --n;
            }
            dot /= temp;
            r += dot;
        }
        if (neg) {
            r = -r;
        }
        return r;
    }

    //slower than itoa butter faster than std::to_string

    template<typename TChar>
    const TChar* ToString(int x, TChar* buff, unsigned char& chars) {
        assert(buff && chars > 11);
        if (!x) {
            buff[0] = '0';
            buff[1] = 0;
            chars = 1;
            return buff;
        }
        buff[11] = 0;
        unsigned char start = 10;
        bool neg = (x < 0);
        if (neg) x *= -1;
        while (x) {
            buff[start] = (x % 10 + '0');
            x /= 10;
            --start;
        }
        chars = 10 - start;
        if (neg) {
            buff[start] = '-';
            ++chars;
        } else {
            ++start;
        }
        return buff + start;
    }

    //slower than itoa butter faster than std::to_string

    template<typename TChar>
    const TChar* ToString(unsigned int x, TChar* buff, unsigned char& chars) {
        assert(buff && chars > 10);
        if (!x) {
            buff[0] = '0';
            buff[1] = 0;
            chars = 1;
            return buff;
        }
        buff[10] = 0;
        unsigned char start = 9;
        while (x) {
            buff[start] = (x % 10 + '0');
            x /= 10;
            --start;
        }
        chars = 9 - start;
        ++start;
        return buff + start;
    }

    template<typename TChar>
    const TChar* ToString(INT64 x, TChar* buff, unsigned char& chars) {
        assert(buff && chars > 20);
        if (!x) {
            buff[0] = '0';
            buff[1] = 0;
            chars = 1;
            return buff;
        }
        buff[20] = 0;
        unsigned char start = 19;
        bool neg = (x < 0);
        if (neg) x *= -1;
        while (x) {
            buff[start] = (x % 10 + '0');
            x /= 10;
            --start;
        }
        chars = 19 - start;
        if (neg) {
            buff[start] = '-';
            ++chars;
        } else {
            ++start;
        }
        return buff + start;
    }

    template<typename TChar>
    const TChar* ToString(UINT64 x, TChar* buff, unsigned char& chars) {
        assert(buff && chars > 20);
        if (!x) {
            buff[0] = '0';
            buff[1] = 0;
            chars = 1;
            return buff;
        }
        buff[20] = 0;
        unsigned char start = 19;
        while (x) {
            buff[start] = (x % 10 + '0');
            x /= 10;
            --start;
        }
        chars = 19 - start;
        ++start;
        return buff + start;
    }

    template<typename TChar>
    const TChar* ToString(double d, TChar* str, unsigned char& chars, unsigned char precision) {
        if (!str || !chars) {
            chars = 0;
            return nullptr;
        }
        --chars;
        unsigned char total = chars;
        chars = 0;
        char format[6];
        char data[64];
        if (precision > 57) precision = 57; //57 = 64 - 6(-.e308) - 1(null)
        format[0] = '%';
        format[1] = '.';
        if (precision >= 10) {
            format[2] = precision / 10 + '0';
            format[3] = (precision % 10) + '0';
            format[4] = 'g';
            format[5] = 0;
        } else {
            format[2] = (precision % 10) + '0';
            format[3] = 'g';
            format[4] = 0;
        }
#ifdef WIN32_64
        int len = sprintf_s(data, sizeof (data), format, d);
#else
        int len = sprintf(data, format, d);
#endif
        if (len > total) len = total; //truncated because of recv memory
        for (int n = 0; n < len; ++n) {
            str[n] = data[n];
            ++chars;
        }
        str[chars] = 0;
        return str;
    }

#pragma pack(push,1)

    struct UDateTime {
    private:
        static constexpr unsigned int MAX_NANO_SECONDS = 999999999;

    public:
        static void ParseTime(const char *str, int &hour, int &min, int &second, unsigned int &ns) {
            const char *end;
            hour = atoi(str, end);
            min = atoi(++end, end);
            second = atoi(++end, end);
            if (*end == '.') {
                ns = (unsigned int) (atof(end, end) * 1000000000 + 0.5); //rounded to 1 nanosecond
            } else {
                ns = 0;
            }
        }

        static void ParseDateTime(const char *str, int &year, int &month, int &day, int &hour, int &min, int &second, unsigned int &ns) {
            const char *end;
            year = atoi(str, end);
            month = atoi(++end, end);
            day = atoi(++end, end);
            if (*end == ' ') {
                ParseTime(++end, hour, min, second, ns);
            } else {
                hour = 0;
                min = 0;
                second = 0;
                ns = 0;
            }
        }

        UINT64 Ns100 : 24; //100-nanosecond per unit
        UINT64 Second : 6; //0 - 59
        UINT64 Minute : 6; //0 - 59
        UINT64 Hour : 5; //0 - 23
        UINT64 Day : 5; //1 - 31
        UINT64 Month : 4; //0 - 11 instead of 1 - 12
        UINT64 Year : 13; //8191 == 0x1fff, From BC 6291 (8191 - 1900) to AD 9991 (1900 + 8191)
        UINT64 Neg : 1; //It will be 1 if date time is earlier than 1900-01-01

        UDateTime(UINT64 t = 0) noexcept : Ns100(0), Second(0), Minute(0), Hour(0), Day(0), Month(0), Year(0), Neg(0) {
            ::memcpy(this, &t, sizeof (UDateTime));
        }

        UDateTime(const std::tm &dt, unsigned int ns = 0) noexcept {
            Set(dt, ns);
        }

        UDateTime(const char *str) : Ns100(0), Second(0), Minute(0), Hour(0), Day(0), Month(0), Year(0), Neg(0) {
            if (str) {
                ParseFromDBString(str);
            }
        }

        UINT64 Value() const noexcept {
            return *(UINT64*)this;
        }

        void Value(UINT64 t) noexcept {
            ::memcpy(this, &t, sizeof (t));
        }

        UDateTime& operator=(const UDateTime& dt) noexcept {
            if (this != &dt) {
                ::memcpy(this, &dt, sizeof (dt));
            }
            return *this;
        }

        inline bool operator==(const UDateTime & t) const noexcept {
            return (0 == ::memcmp(this, &t, sizeof (t)));
        }

        inline bool operator!=(const UDateTime & t) const noexcept {
            return (0 != ::memcmp(this, &t, sizeof (t)));
        }

        inline unsigned int HasMicrosecond() const noexcept {
            return (unsigned int) (Ns100 / 10);
        }

        inline unsigned int HasHanosecond() const noexcept {
            return (unsigned int) (Ns100 * 100);
        }

        inline unsigned int HasDate() const noexcept {
            return (unsigned int) (Day);
        }

        inline unsigned int HasTime() const noexcept {
            return (unsigned int) (Hour * 3600 + Minute * 60 + Second);
        }

        std::time_t GetTime(bool& time_only, unsigned int* ns = nullptr) {
            std::tm dt = GetCTime(ns);
            time_only = (0 == dt.tm_mday);
#ifdef WIN32_64
            return ::_mkgmtime(&dt);
#else
            return ::timegm(&dt);
#endif
        }

        //convert UDateTime datetime back to std::tm structure and nanoseconds

        std::tm GetCTime(unsigned int *ns = nullptr) const noexcept {
            std::tm datetime;
            datetime.tm_isdst = -1;
            datetime.tm_wday = 0;
            datetime.tm_yday = 0;
            if (ns) {
                *ns = (unsigned int) (Ns100 * 100);
            }
            if (Neg) {
                datetime.tm_year = -(int) Year;
            } else {
                datetime.tm_year = (int) Year;
            }
            datetime.tm_mon = (int) Month;
            datetime.tm_mday = (int) Day;
            datetime.tm_hour = (int) Hour;
            datetime.tm_min = (int) Minute;
            datetime.tm_sec = (int) Second;
            return datetime;
        }

        void Set(const std::tm &dt, unsigned int ns = 0) noexcept {
            assert(ns <= MAX_NANO_SECONDS);
            if (ns > MAX_NANO_SECONDS) {
                ns = MAX_NANO_SECONDS;
            }
            assert(dt.tm_hour >= 0 && dt.tm_hour < 24);
            assert(dt.tm_min >= 0 && dt.tm_min < 60);
            assert(dt.tm_sec >= 0 && dt.tm_sec < 60);
            if (dt.tm_mday) {
                assert(dt.tm_year >= -8191 && dt.tm_year <= 8191);
                assert(dt.tm_mon >= 0 && dt.tm_mon <= 11);
                assert(dt.tm_mday < 32);
                if (dt.tm_year < 0) {
                    Neg = 1;
                    Year = -dt.tm_year;
                }
                else {
                    Neg = 0;
                    Year = dt.tm_year;
                }
                Month = dt.tm_mon;
                Day = dt.tm_mday;
            }
            else {
                Neg = 0;
                Year = 0;
                Month = 0;
                Day = 0;
            }
            Hour = dt.tm_hour;
            Minute = dt.tm_min;
            Second = dt.tm_sec;
            Ns100 = ns / 100;
        }

        void ToDBString(char *str, unsigned int bufferSize) const {
            assert(str);
            assert(bufferSize > 27); //2012-12-23 23:59:59.1234567
            unsigned int ns = Ns100;
            std::tm tm = GetCTime(nullptr);
#if defined (WIN32_64) && _MSC_VER >= 1600 
            if (ns) {
                if (tm.tm_mday) {
                    sprintf_s(str, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d.%07d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ns);
                } else {
                    sprintf_s(str, bufferSize, "%02d:%02d:%02d.%07d", tm.tm_hour, tm.tm_min, tm.tm_sec, ns);
                }
            } else if (tm.tm_hour || tm.tm_min || tm.tm_sec) {
                if (tm.tm_mday) {
                    sprintf_s(str, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                } else {
                    sprintf_s(str, bufferSize, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            } else {
                sprintf_s(str, bufferSize, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            }
#else
            if (ns) {
                if (tm.tm_mday) {
                    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d.%07d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ns);
                } else {
                    sprintf(str, "%02d:%02d:%02d.%07d", tm.tm_hour, tm.tm_min, tm.tm_sec, ns);
                }
            } else if (tm.tm_hour || tm.tm_min || tm.tm_sec) {
                if (tm.tm_mday) {
                    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                } else {
                    sprintf(str, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            } else {
                sprintf(str, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            }
#endif
        }

        std::string ToDBString() const {
            char str[32] = {0};
            ToDBString(str, sizeof (str));
            return str;
        }

        void ToWebString(char *str, unsigned int bufferSize) const {
            assert(str);
            assert(bufferSize > 24); //2012-12-23T23:59:59.123Z
            std::tm tm = GetCTime(nullptr);
            unsigned int ms = (unsigned int) ((Ns100 + 5000) / 10000);
#if defined (WIN32_64) && _MSC_VER >= 1600 
            if (ms) {
                if (tm.tm_mday) {
                    sprintf_s(str, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
                } else {
                    sprintf_s(str, bufferSize, "%02d:%02d:%02d.%03dZ", tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
                }
            } else if (tm.tm_hour || tm.tm_min || tm.tm_sec) {
                if (tm.tm_mday) {
                    sprintf_s(str, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                } else {
                    sprintf_s(str, bufferSize, "%02d:%02d:%02dZ", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            } else {
                sprintf_s(str, bufferSize, "%04d-%02d-%02dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            }
#else
            if (ms) {
                if (tm.tm_mday) {
                    sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
                } else {
                    sprintf(str, "%02d:%02d:%02d.%03dZ", tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
                }
            } else if (tm.tm_hour || tm.tm_min || tm.tm_sec) {
                if (tm.tm_mday) {
                    sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                } else {
                    sprintf(str, "%02d:%02d:%02dZ", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            } else {
                sprintf(str, "%04d-%02d-%02dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            }
#endif
        }

        std::string ToWebString() const {
            char str[32] = {0};
            ToWebString(str, sizeof (str));
            return str;
        }

        //string must be in the form like '2016-02-15 22:03:14.234 or 22:03:14.234

        void ParseFromDBString(const char *str) {
            assert(str);
            unsigned int ns = 0;
            std::tm tm;
            ::memset(&tm, 0, sizeof (tm));
            const char *pos = strchr(str, ':');
            const char *whitespace = strchr(str, ' ');
            if (!whitespace && pos) {
                ParseTime(str, tm.tm_hour, tm.tm_min, tm.tm_sec, ns);
            } else {
                ParseDateTime(str, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ns);
                tm.tm_year -= 1900;
                tm.tm_mon -= 1;
            }
            Set(tm, ns);
        }

#ifdef WIN32_64

        UDateTime(const SYSTEMTIME &dt, unsigned int ns = 0) noexcept {
            Set(dt, ns);
        }

        //vtDate -- windows variant datetime with accuracy to millisecond

        UDateTime(DATE vtDate, unsigned int ns = 0) {
            Set(vtDate, ns);
        }

        void Set(const SYSTEMTIME &dt, unsigned int ns = 0) noexcept {
            constexpr unsigned int MAX_VALUE = 999999;
            assert(ns <= MAX_VALUE);
            if (ns > MAX_VALUE) {
                ns = MAX_VALUE;
            }
            assert(dt.wDay < 32); //it is time, and wYear and wMonth are ignored if 0
            assert(dt.wHour < 24);
            assert(dt.wMinute < 60);
            assert(dt.wSecond < 60);
            if (dt.wDay) {
                assert(dt.wYear <= 1900 + 8191);
                assert(dt.wMonth && dt.wMonth <= 12);
                if (dt.wYear < 1900) {
                    Neg = 1;
                    Year = 1900 - dt.wYear;
                } else {
                    Neg = 0;
                    Year = dt.wYear - 1900;
                }
                Month = dt.wMonth - 1;
                Day = dt.wDay;
            } else {
                Year = 0;
                Month = 0;
                Day = 0;
                Neg = 0;
            }
            Hour = dt.wHour;
            Minute = dt.wMinute;
            Second = dt.wSecond;
            Ns100 = dt.wMilliseconds;
            Ns100 *= 10000;
            Ns100 += ns / 100;
        }

        void Set(DATE vtDate, unsigned int ns = 0) {
            assert(ns <= MAX_NANO_SECONDS);
            if (ns > MAX_NANO_SECONDS) {
                ns = MAX_NANO_SECONDS;
            }
            SYSTEMTIME st;
            BOOL ok = VariantTimeToSystemTime(vtDate, &st);
            assert(ok);
            st.wMilliseconds = ns / 1000000;
            ns = (ns % 1000000);
            Set(st, ns);
        }

        SYSTEMTIME GetSysTime(unsigned int *ns = nullptr) const noexcept {
            SYSTEMTIME datetime;
            datetime.wDayOfWeek = 0; //not set;
            if (Day) {
                if (Neg) {
                    assert(Year <= 1900 - 1601);
                    if (Year <= 299) {
                        datetime.wYear = (WORD) (1900 - Year);
                    } else {
                        datetime.wYear = 1601;
                    }
                } else {
                    datetime.wYear = (WORD) (Year + 1900);
                }
                datetime.wMonth = (WORD) (Month + 1);
                datetime.wDay = (WORD) Day;
            } else {
                datetime.wYear = 0;
                datetime.wMonth = 0;
                datetime.wDay = 0;
            }
            datetime.wHour = Hour;
            datetime.wMinute = Minute;
            datetime.wSecond = Second;
            datetime.wMilliseconds = (WORD) (Ns100 / 10000);
            if (ns) {
                *ns = (unsigned int) ((Ns100 % 10000) * 100);
            }
            return datetime;
        }

        DATE GetVariantDate(unsigned int *ns = nullptr) const {
            SYSTEMTIME st = GetSysTime(ns);
            if (ns) {
                unsigned int ms = st.wMilliseconds;
                ms *= 1000000;
                *ns += ms;
            }
            double vtDate;
            st.wMilliseconds = 0;
            BOOL ok = SystemTimeToVariantTime(&st, &vtDate);
            assert(ok);
            return vtDate;
        }
#else

        void Set(const timeval &tv) {
            std::tm *tm = std::localtime(&tv.tv_sec);
            unsigned int us = tv.tv_usec;
            Set(*tm, us * 10);
        }

        UDateTime(const timeval &tv) {
            Set(tv);
        }
#endif
    };
#pragma pack(pop) //ensure sizeof(UDateTime) == 8
#if defined(WINCE) || defined(UNDER_CE) || defined(_WIN32_WCE)

#else
    static_assert(sizeof (UDateTime) == 8, "Wrong UDateTime size");
#endif
};

#endif //__UCOMM_BASE_DEFINES_H_____
