
#ifndef	___WIN_UCOMM__UTIL_HEADER_FILE___H___
#define ___WIN_UCOMM__UTIL_HEADER_FILE___H___

#include <exception>
#ifndef NDEBUG
#include <iostream>
#endif

namespace SPA {

    /** 
     * A class for managing SPA exception on window platforms
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
#ifdef WINCE
        : std::exception(), m_errCode(errCode), m_msg(errMsg) {
#else
        : std::exception(errMsg), m_errCode(errCode), m_msg(errMsg) {
#endif
            std::ostringstream ss;
            ss << "function: " << funcName << "@line: " << line << " of file: " << file;
            m_stack = ss.str();
#ifndef	NDEBUG
            std::cout << "function: " << funcName << "@line: " << line << " of file: " << file << std::endl;
#endif
        }

        /**
         * Create an instance of CUException
         * @param errMsg Error message
         * @param stack A string for error location of source file
         * @param errCode A given error code with default to -1 (unavailable)
         */
        CUException(const char * const errMsg, const char *stack = "", int errCode = MB_ERROR_UNKNOWN)
#ifdef WINCE
        : std::exception(), m_errCode(errCode), m_stack(stack), m_msg(errMsg) {
#else
        : std::exception(errMsg), m_stack(stack), m_errCode(errCode), m_msg(errMsg) {
#endif
#ifndef	NDEBUG
            std::cout << "Error message: " << errMsg << ", stack: " << stack << ", errCode: " << errCode << std::endl;
#endif
        }

        /**
         * Create an instance of CUException from an instance of std::exception with no error code available
         */
        explicit CUException(const std::exception &ex)
        : std::exception(ex),
        m_errCode(MB_ERROR_UNKNOWN), m_msg(ex.what()) {
        }

        /**
         * Create an instance of CUException from an instance of CMBExption
         */
        CUException(const CUException &ex)
        : std::exception(ex),
        m_errCode(ex.m_errCode),
        m_stack(ex.m_stack),
        m_msg(ex.what()) {
        }

    public:

        virtual const char * what() const {
            return m_msg.c_str();
        }

        /**
         * Copy an instance of CUException from an instance of CMBExption
         */
        CUException& operator=(const CUException &ex) {
            if (&ex == this)
                return *this;
            std::exception &e = *this;
            e = ex;
            m_errCode = ex.m_errCode;
            m_stack = ex.m_stack;
            m_msg = ex.what();
            return *this;
        }

        /**
         * Create an instance of CUException from an instance of std::exception
         */
        CUException& operator=(const std::exception &ex) {
            std::exception &e = *this;
            e = ex;
            m_errCode = MB_ERROR_UNKNOWN;
            m_stack.clear();
            m_msg = ex.what();
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

    private:
        int m_errCode;
        std::string m_stack;
        std::string m_msg;
    };

    /** 
     * A class for managing a critical section on window platforms
     */
    class CUCriticalSection {
    public:

        /**
         * Create an instance of CUCriticalSection, and automatically initialize a critical section
         */
        CUCriticalSection() {
            ::InitializeCriticalSection(&m_sec);
        }

        /**
         * Destroy an instance of CUCriticalSection, and automatically destroy a critical section
         */
        ~CUCriticalSection() {
            ::DeleteCriticalSection(&m_sec);
        }

        /**
         * Lock a critical section
         */
        inline void lock() {
            ::EnterCriticalSection(&m_sec);
        }

        /**
         * Unlock a critical section
         */
        inline void unlock() {
            ::LeaveCriticalSection(&m_sec);
        }

    private:
        /// Copy constructor disabled
        CUCriticalSection(const CUCriticalSection &cs);

        /// Assignment operator disabled
        CUCriticalSection& operator=(const CUCriticalSection &cs);

    public:
        CRITICAL_SECTION m_sec;
    };

    /** 
     * A class for automatically locking a critical section
     * This class is usually used with stack
     */
    class CAutoLock {
    public:

        /**
         * Create an instance of CAutoLock, and automatically lock a critical section
         */
        CAutoLock(CUCriticalSection &cs)
        : m_cs(cs) {
            m_cs.lock();
        }

        /**
         * Destroy an instance of CAutoLock, and automatically unlock a critical section
         */
        ~CAutoLock() {
            m_cs.unlock();
        }

    private:
        /// Copy constructor disabled
        CAutoLock(const CAutoLock &al);

        /// Assignment operator disabled
        CAutoLock& operator=(const CAutoLock &al);

        CUCriticalSection &m_cs;
    };
};

#endif
