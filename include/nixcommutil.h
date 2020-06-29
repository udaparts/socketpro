
#ifndef	___NIX_UCOMM__UTIL_HEADER_FILE___H___
#define ___NIX_UCOMM__UTIL_HEADER_FILE___H___

#include <cstdlib>
//#include <boost/thread/recursive_mutex.hpp>
#include <mutex> //-pthread -std=c++11
#include <exception>

#ifdef BOOST_UUID_HPP
typedef boost::uuids::uuid GUID;
#else

//use windows GUID definition

typedef struct _GUID {
    unsigned int Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
#endif
static_assert(sizeof (GUID) == sizeof (DECIMAL), "GUID and DECIMAL should have the same size");
typedef GUID UUID;
typedef GUID CLSID;

namespace SPA {
    //typedef boost::recursive_mutex CUCriticalSection;
    //typedef boost::recursive_mutex::scoped_lock CAutoLock;

    typedef std::recursive_mutex CUCriticalSection;
    //typedef std::lock_guard<std::recursive_mutex> CAutoLock;
    typedef std::unique_lock<std::recursive_mutex> CAutoLock;

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
        }

        /**
         * Create an instance of CUException from an instance of CMBExption
         */
        CUException(const CUException &ex)
        : m_errCode(ex.m_errCode), m_stack(ex.m_stack), m_errMsg(ex.m_errMsg) {
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
        virtual const char * what() const noexcept {
            return m_errMsg.c_str();
        }

    private:
        int m_errCode;
        std::string m_stack;
        std::string m_errMsg;
    };
};

#include "linux_win.h"

#endif
