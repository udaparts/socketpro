
#ifndef	___NIX_UCOMM__UTIL_HEADER_FILE___H___
#define ___NIX_UCOMM__UTIL_HEADER_FILE___H___

#include <cstdlib>
#include <boost/thread/recursive_mutex.hpp>
//#include <mutex>
#include <boost/uuid/uuid.hpp>
#include <exception>

#ifdef __ANDROID__
namespace std {

    template <typename T>
    std::string to_string(T value) {
        std::ostringstream os;
        os << value;
        return os.str();
    }
}
#endif

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

#ifdef BOOST_UUID_HPP
typedef boost::uuids::uuid GUID;
static_assert(sizeof (GUID) == sizeof (DECIMAL), "GUID and DECIMAL should have the same size");
typedef GUID UUID;
typedef GUID CLSID;
#endif

namespace SPA {
    typedef boost::recursive_mutex CUCriticalSection;
    typedef boost::recursive_mutex::scoped_lock CAutoLock;

    //typedef std::recursive_mutex CUCriticalSection;
    //typedef std::lock_guard<std::recursive_mutex> CAutoLock;

    /** 
     * A class for managing MB exception on non-window platforms
     * The class tracks error stack for error message, function name, line number and file name
     */
    class CUException : public std::exception {
    public:

        /**
         * Create an instance of CUException
         * @param errMsg Error message
         * @param file A C/C++ file name
         * @param line Line number where the exception is created
         * @param funcName A function name string
         * @param errCode A given error code with default to -1 (unavailable)
         */
        CUException(const char * const errMsg, const char *file, unsigned int line, const char *funcName, int errCode = MB_ERROR_UNKNOWN)
        : m_errCode(errCode), m_errMsg(errMsg) {
            std::ostringstream ss;
            ss << "function: " << funcName << "@line: " << line << " of file: " << file;
            m_stack = ss.str();
#ifndef	NDEBUG
            std::cout << "function: " << funcName << "@line: " << line << " of file: " << file << std::endl;
#endif
        }

        /**
         * Create an instance of CUException from an instance of std::exception with no error code available
         */
        explicit CUException(const std::exception &ex)
        : m_errCode(MB_ERROR_UNKNOWN), m_errMsg(ex.what()) {
        }

        /**
         * Create an instance of CUException
         * @param errMsg Error message
         * @param stack A string for error location of source file
         * @param errCode A given error code with default to -1 (unavailable)
         */
        CUException(const char * const errMsg, const char *stack = "", int errCode = MB_ERROR_UNKNOWN)
        : m_errCode(errCode), m_stack(stack), m_errMsg(errMsg) {
#ifndef	NDEBUG
            std::cout << "Error message: " << errMsg << ", stack: " << stack << ", errCode: " << errCode << std::endl;
#endif
        }

        /**
         * Create an instance of CUException from an instance of CMBExption
         */
        CUException(const CUException &ex)
        : m_errCode(ex.m_errCode), m_stack(ex.m_stack), m_errMsg(ex.m_errMsg) {
        }

        ~CUException() throw () {

        }

    public:

        /**
         * Copy an instance of CUException from an instance of CMBExption
         */
        CUException& operator=(const CUException &ex) {
            m_errCode = ex.m_errCode;
            m_stack = ex.m_stack;
            m_errMsg = ex.m_errMsg;
            return *this;
        }

        /**
         * Create an instance of CUException from an instance of std::exception
         */
        CUException& operator=(const std::exception &ex) {
            m_errCode = MB_ERROR_UNKNOWN;
            m_stack.clear();
            m_errMsg = ex.what();
            return *this;
        }

        /**
         * Get error code, and -1 means there is no error code available
         * @return An error code
         */
        int GetErrCode() const {
            return m_errCode;
        }

        /**
         * Get error stack
         * @return An error stack describing error location
         */
        const std::string& GetStack() const {
            return m_stack;
        }

        /**
         * Get an error message
         * @return An error message
         */
        virtual const char * what() const throw () {
            return m_errMsg.c_str();
        }

    private:
        int m_errCode;
        std::string m_stack;
        std::string m_errMsg;
    };
};

#endif
