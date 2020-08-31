#ifndef __UCOMM_SHARED_MEMORY_QUEUE_H_
#define __UCOMM_SHARED_MEMORY_QUEUE_H_

#include "safequeue.h"
#include <algorithm>
#include "spvariant.h"

#ifdef WIN32_64
#ifdef SUPPORT_PACKING_ATL_STR
#include <atlstr.h>
#endif
#ifdef SUPPORT_PACKING_COMUTIL
#include <comutil.h>
#endif

#endif

namespace SPA {

    extern const UINT64 SAFE_DOUBLE;

    template<typename TChar>
    bool IsEqual(const TChar *s0, const TChar *s1, bool case_sensitive) {
        return (0 == (case_sensitive ? UCompare(s0, s1) : UCompareNoCase(s0, s1)));
    }

#ifndef WINCE

    template<typename TChar>
    void Trim(std::basic_string<TChar> &s) {
        while (s.size() && ::isspace(s.back())) {
            s.pop_back();
        }
        while (s.size() && ::isspace(s.front())) {
            s.erase(s.begin());
        }
    }
#endif

    class CUQueue;

    namespace Utilities {
        using SPA::GetLen;
        using SPA::Trim;
        using SPA::IsEqual;
        void ToWide(const char *utf8, size_t chars, CUQueue &q, bool append = false);
        void ToUTF8(const wchar_t *str, size_t wchars, CUQueue &q, bool append = false);
        std::string ToUTF8(const wchar_t *str, size_t wchars = (size_t) (~0));
        std::string ToUTF8(const std::wstring &s);
        std::wstring ToWide(const char *utf8, size_t chars = (size_t) (~0));
        std::wstring ToWide(const std::string &s);
        void ToUTF16(const char *str, size_t chars, CUQueue &q, bool append = false);
        void ToUTF16(const wchar_t *str, size_t wchars, CUQueue &q, bool append = false);
        std::basic_string<UTF16> ToUTF16(const char *utf8, size_t chars = (size_t) (~0));
        std::basic_string<UTF16> ToUTF16(const std::string &s);
        std::basic_string<UTF16> ToUTF16(const wchar_t *str, size_t wchars = (size_t) (~0));
        std::basic_string<UTF16> ToUTF16(const std::wstring &s);

#ifdef NATIVE_UTF16_SUPPORTED
        void ToWide(const char16_t *str, size_t chars, CUQueue &q, bool append = false);
        void ToUTF8(const char16_t *str, size_t chars, CUQueue &q, bool append = false);
        std::string ToUTF8(const char16_t *str, size_t chars = (size_t) (~0));
        std::string ToUTF8(const std::basic_string<char16_t> &s);
        std::wstring ToWide(const char16_t *str, size_t chars = (size_t) (~0));
        std::wstring ToWide(const std::basic_string<char16_t> &s);
#endif

#ifdef WIN32_64
        BSTR ToBSTR(const char *utf8, size_t chars = (size_t) (~0));
        std::wstring GetErrorMessage(DWORD dwError);
#elif defined(WCHAR32)
        BSTR ToBSTR(const char *utf8, size_t chars);
        BSTR SysAllocStringLen(const wchar_t *sz, unsigned int wchars);
#else
#endif
    };

    static const unsigned int DEFAULT_MEMORY_BUFFER_BLOCK_SIZE = ((unsigned int) 4 * 1024);
    static const unsigned int DEFAULT_INITIAL_MEMORY_BUFFER_SIZE = ((unsigned int) 4 * 1024);
    static const unsigned int UQUEUE_END_POSTION = ((unsigned int) (~0));
    static const unsigned int UQUEUE_NULL_LENGTH = ((unsigned int) (~0));

    /** 
     * A memory buffer class for packing and unpacking various types of data intelligently and efficiently with auto-memory reallocation as needed. 
     * Differences in integer endian and wide char size are automatically corrected across different operation systems.
     * If endian equality is false or wide char size is different, wide char string will be converted into or from UCS-2 low endian string.
     * Operators << and >> are overloaded for easy serialization and de-serialization. All strings are serialized or de-serialized with length ahead.
     * Its copy constructor and assignment operators are disabled.
     */
    class CUQueue {
    private:
        static void ChangeArrayInt(void *p, unsigned char sizePerInt, unsigned int count);

    public:

        /** 
         * Construct an instance of CUQueue
         * @param maxLen The initial empty memory size in byte with default to 4096
         * @param blockSize The size of memory block in byte with default to 4096
         * @param os Operation system
         * @param bigEndian True for big endian and false for little endian
         */
        CUQueue(unsigned int maxLen = DEFAULT_INITIAL_MEMORY_BUFFER_SIZE,
                unsigned int blockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE,
                tagOperationSystem os = MY_OPERATION_SYSTEM,
                bool bigEndian = IsBigEndian()
                )
        : m_nMaxBuffer(maxLen),
        m_nSize(0),
        m_nHeadPos(0),
        m_os(os),
        m_bSameEndian(bigEndian == IsBigEndian()), m_8ToW(false), m_ToUtf8(false)
#ifdef WIN32_64
        , m_TimeEx(false)
#endif
        {
            if (blockSize == 0) {
                m_nBlockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
            } else {
                m_nBlockSize = blockSize;
            }
            if (m_nMaxBuffer) {
                m_pBuffer = (unsigned char*) ::malloc(m_nMaxBuffer);
            } else {
                m_pBuffer = nullptr;
            }
        }

        /** 
         * Ensure releasing its internal memory and destruct the object
         */
        virtual ~CUQueue() {
            Empty();
        }

        /** 
         * Change an integer (int16, int32, int64, uint16, uint32, uint64, float and double only) endian
         * @param p A pointer to an integer
         * @param size The size per integer in byte
         * @return Successfulness of change in integer endian 
         */
        static bool ChangeEndian(unsigned char *p, unsigned int size) {
            unsigned char byte;
            assert(p);
            switch (size) {
                case 2:
                    byte = p[0];
                    p[0] = p[1];
                    p[1] = byte;
                    break;
                case 4:
                    byte = p[0];
                    p[0] = p[3];
                    p[3] = byte;
                    byte = p[1];
                    p[1] = p[2];
                    p[2] = byte;
                    break;
                case 8:
                    byte = p[0];
                    p[0] = p[7];
                    p[7] = byte;
                    byte = p[1];
                    p[1] = p[6];
                    p[6] = byte;
                    byte = p[2];
                    p[2] = p[5];
                    p[5] = byte;
                    byte = p[3];
                    p[3] = p[4];
                    p[4] = byte;
                    break;
                default:
                    return false;
                    break;
            }
            return true;
        }

    private:
        // no copy constructor!
        CUQueue(const CUQueue &mc) = delete;
        // no assignment operator!
        CUQueue& operator=(const CUQueue &mc) = delete;

    public:

#ifdef MS_SQL_SNI_ADAPTER

        void Reset(unsigned int header_pos, unsigned int len) {
            m_nHeadPos = header_pos;
            m_nSize = len;
        }
#endif

        /**
         * Replace internal memory content with a new buffer
         * @param pos Memory start position
         * @param len The length of memory content in byte
         * @param buffer Pointer to a new buffer
         * @param size The size of a new buffer in byte 
         */
        inline void Replace(unsigned int pos, unsigned int len, const unsigned char *buffer, unsigned int size) {
            assert(pos <= m_nSize);
            assert(len <= m_nSize);
            assert((pos + len) <= m_nSize);

            unsigned char *p = (unsigned char *) GetBuffer(pos);
            if (!buffer) {
                size = 0;
            }
            if (len > size) {
                ::memcpy(p, buffer, size);
                unsigned int diff = len - size;
                Pop(diff, pos + size);
            } else if (len == size) {
                ::memcpy(p, buffer, size);
            } else //size > len
            {
                ::memcpy(p, buffer, len);
                unsigned int diff = size - len;
                Insert(buffer + len, diff, pos + len);
            }
        }

        inline void Put(char c) {
            Push((const unsigned char*) &c, sizeof (c));
        }

        inline void Put(wchar_t c) {
            Push((const unsigned char*) &c, sizeof (c));
        }

        inline void SetNull() {
            unsigned int pos = m_nHeadPos + m_nSize;
            if (m_nMaxBuffer >= pos + sizeof (wchar_t)) {
                ::memset(m_pBuffer + pos, 0, sizeof (wchar_t));
            } else {
                wchar_t c = 0;
                Push((unsigned char*) &c, sizeof (c));
                m_nSize -= sizeof (c);
            }
        }

        /**
         * Set big or little endian
         * @param bigEndian, true for big endian and false for little endian
         */
        inline void SetEndian(bool bigEndian = IsBigEndian()) {
            m_bSameEndian = (bigEndian == IsBigEndian());
        }

        /**
         * Set the operation system
         * @param os The operation system
         */
        inline void SetOS(tagOperationSystem os) {
            m_os = os;
        }

        /**
         * Check the operation system
         * @return The operation system
         */
        inline tagOperationSystem GetOS() const {
            return m_os;
        }

        /**
         * Check the endian
         * @see SetEndian()
         * @return True for big endian and false for little endian
         */
        inline bool GetEndian() const {
            return (m_bSameEndian != SPA::IsBigEndian());
        }

        /**
         * Release memory buffer, and zero all members except endian equality.
         */
        inline void Empty() {
            if (m_pBuffer) {
                ::free(m_pBuffer);
                m_pBuffer = nullptr;
                m_nMaxBuffer = 0;
                m_nSize = 0;
                m_nHeadPos = 0;
            }
        }

        /**
         * Get internal buffer pointer.
         * @param offset Offset to internal buffer in byte with default equal to 0
         * @return Internal buffer pointer
         */
        inline const unsigned char* const GetBuffer(unsigned int offset = 0) const {
            if (offset > m_nSize) {
                offset = m_nSize;
            }
            return (m_pBuffer + offset + m_nHeadPos);
        }

        /**
         * Get content size in byte.
         * @return Content length in byte
         */
        inline unsigned int GetSize() const {
            return m_nSize;
        }

        /**
         * Get unused buffer size in byte.
         * @return Unused buffer length in byte
         */
        inline unsigned int GetIdleSize() const {
            return (m_nMaxBuffer - m_nSize);
        }

        /**
         * Get unused tail buffer size in byte.
         * @return Unused tail buffer length in byte
         */
        inline unsigned int GetTailSize() const {
            return (m_nMaxBuffer - m_nSize - m_nHeadPos);
        }

        /**
         * Check if the object will automatically convert utf8 string into Unicode string when loading a ASCII string by VARIANT.
         * @return true if the object will do automatic converting, and false if the object will not
         */
        inline bool Utf8ToW() const {
            return m_8ToW;
        }

        /**
         * Enable or disable the object to automatically convert utf8 string into Unicode string when loading a ASCII string by VARIANT.
         * @param bUtf8ToW true for enabling, and false for disabling
         */
        inline void Utf8ToW(bool bUtf8ToW) {
            m_8ToW = bUtf8ToW;
        }

        /**
         * Check if the object will automatically convert UTF16-LE string into utf8 string when loading an unicode string by VARIANT.
         * @return true if the object will do automatic converting, and false if the object will not
         */
        inline bool ToUtf8() const {
            return m_ToUtf8;
        }

        /**
         * Enable or disable the object to automatically convert UTF16-LE string into utf8 string when loading an unicode string by VARIANT.
         * @param bToUtf8 true for enabling, and false for disabling
         */
        inline void ToUtf8(bool bToUtf8) {
            m_ToUtf8 = bToUtf8;
        }

#ifdef WIN32_64

        /**
         * Check if the object will use high-precision time when saving or loading a ASCII string by VARIANT.
         * @return true if the object will use high-precision time, and false if the object will not
         */
        inline bool TimeEx() const {
            return m_TimeEx;
        }

        /**
         * Enable or disable the object to use high-precision time when saving or loading a ASCII string by VARIANT.
         * @param timeEx true for enabling, and false for disabling
         */
        inline void TimeEx(bool timeEx) {
            m_TimeEx = timeEx;
        }
#endif

#ifndef WINCE

        CUQueue(CUQueue && q)
        : m_nMaxBuffer(q.m_nMaxBuffer),
        m_nSize(q.m_nSize),
        m_nHeadPos(q.m_nHeadPos),
        m_os(q.m_os),
        m_bSameEndian(q.m_bSameEndian),
        m_nBlockSize(q.m_nBlockSize),
        m_pBuffer(q.m_pBuffer),
        m_8ToW(q.m_8ToW),
        m_ToUtf8(q.m_ToUtf8)
#ifdef WIN32_64
        , m_TimeEx(q.m_TimeEx)
#endif
        {
            q.m_nMaxBuffer = 0;
            q.m_nSize = 0;
            q.m_nHeadPos = 0;
            q.m_os = MY_OPERATION_SYSTEM;
            q.m_bSameEndian = true;
            q.m_nBlockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
            q.m_pBuffer = nullptr;
            q.m_8ToW = false;
            q.m_ToUtf8 = false;
#ifdef WIN32_64
            q.m_TimeEx = false;
#endif
        }

        CUQueue& operator=(CUQueue && q) {
            if (this != &q) {
                Swap(q);
            }
            return *this;
        }
#endif

        /**
         * Swap two memory buffer 
         * @param q A reference to a memory buffer object that will be swapped with this object
         */
        void Swap(CUQueue &q) {
            bool b = m_bSameEndian;
            m_bSameEndian = q.m_bSameEndian;
            q.m_bSameEndian = b;

            unsigned int n = m_nBlockSize;
            m_nBlockSize = q.m_nBlockSize;
            q.m_nBlockSize = n;

            n = m_nHeadPos;
            m_nHeadPos = q.m_nHeadPos;
            q.m_nHeadPos = n;

            n = m_nMaxBuffer;
            m_nMaxBuffer = q.m_nMaxBuffer;
            q.m_nMaxBuffer = n;

            n = m_nSize;
            m_nSize = q.m_nSize;
            q.m_nSize = n;

            tagOperationSystem os = m_os;
            m_os = q.m_os;
            q.m_os = os;

            unsigned char *p = m_pBuffer;
            m_pBuffer = q.m_pBuffer;
            q.m_pBuffer = p;

            b = m_8ToW;
            m_8ToW = q.m_8ToW;
            q.m_8ToW = b;

            b = m_ToUtf8;
            m_ToUtf8 = q.m_ToUtf8;
            q.m_ToUtf8 = b;

#ifdef WIN32_64
            b = m_TimeEx;
            m_TimeEx = q.m_TimeEx;
            q.m_TimeEx = b;
#endif
        }

        /**
         * Set content size in byte.
         * @param size New content length in byte
         * @param adjustHeadPos Indicator for adjusting content head position with default to true
         */
        inline void SetSize(unsigned int size, bool adjustHeadPos = true) {
            assert(m_nMaxBuffer >= (size + m_nHeadPos));
            if (adjustHeadPos && m_nHeadPos) {
                m_nSize = size;
                if (m_nSize) {
                    ::memmove(m_pBuffer, m_pBuffer + m_nHeadPos, m_nSize);
                }
                m_nHeadPos = 0;
            } else {
                m_nSize = size;
                if (size == 0) {
                    m_nHeadPos = 0;
                }
            }
        }

        /**
         * Check the max buffer size in byte.
         * @see ReallocBuffer()
         * @return The totally allocated buffer size in byte
         */
        inline unsigned int GetMaxSize() const {
            return m_nMaxBuffer;
        }

        /**
         * Check the buffer block size in byte
         * @return The buffer block size in byte
         */
        inline unsigned int GetBlockSize() const {
            return m_nBlockSize;
        }

        /**
         * Reset the buffer block size in byte
         * @param blockSize A new buffer block size in byte with default to 1024
         */
        inline void SetBlockSize(unsigned int blockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE) {
            if (blockSize == 0) {
                blockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
            }
            m_nBlockSize = blockSize;
        }

        /**
         * Append a new buffer
         * @param buffer Pointer to a new buffer
         * @param len Size of the buffer in byte
         */
        inline void Push(const unsigned char* buffer, unsigned int len) {
            Insert(buffer, len, UQUEUE_END_POSTION);
        }

        /**
         * Check the header position of content. If there is no content available, the method returns 0
         * @return The header position of content in byte
         */
        inline unsigned int GetHeadPosition() const {
            return m_nHeadPos;
        }

        /**
         * Move the header of content to very beginning.
         */
        inline void SetHeadPosition() {
            if (m_nHeadPos) {
                if (m_nSize) {
                    ::memmove(m_pBuffer, m_pBuffer + m_nHeadPos, m_nSize);
                }
                m_nHeadPos = 0;
            }
        }

        /**
         * Zero all of buffer except content
         */
        inline void CleanTrack() {
            assert(m_nMaxBuffer >= (m_nHeadPos + m_nSize));
            if (m_pBuffer && m_nMaxBuffer > 0) {
                if (m_nHeadPos > 0) {
                    ::memset(m_pBuffer, 0, m_nHeadPos);
                }
                unsigned int position = m_nHeadPos + m_nSize;
                unsigned int ulLen = m_nMaxBuffer - position;
                if (ulLen > 0) {
                    ::memset(m_pBuffer + position, 0, ulLen);
                }
            }
        }

        /**
         * Re-allocate buffer and keep existing content as much as possible
         * @param size The new size of buffer in byte
         */
        inline void ReallocBuffer(unsigned int size) {
            if (size == 0) {
                size = m_nBlockSize;
            }
            if (m_nSize > size) {
                m_nSize = size;
            }
            m_nMaxBuffer = size;
            unsigned char *pBuffer = (unsigned char*) ::malloc(m_nMaxBuffer);
            if (m_nSize) {
                ::memcpy(pBuffer, m_pBuffer + m_nHeadPos, m_nSize);
            }
            ::free(m_pBuffer);
            m_pBuffer = pBuffer;
            m_nHeadPos = 0;
        }

        /**
         * Insert a buffer of data into a proper location. The internal buffer will be automatically reallocated if required.
         * @param buffer Pointer to a buffer
         * @param len The size of the buffer in byte
         * @param position The position in byte with default to 0
         */
        inline void Insert(const unsigned char* buffer, unsigned int len, unsigned int position = 0) {
            if (!buffer || !len) {
                return;
            }

            if (position > m_nSize) {
                position = m_nSize;
            } else if (position == 0 && m_nHeadPos >= len) {
                m_nHeadPos -= len;
                ::memcpy(m_pBuffer + m_nHeadPos, buffer, len);
                m_nSize += len;
                return;
            }

            if ((m_nMaxBuffer - m_nHeadPos - m_nSize) >= len) {
                if (position == m_nSize) {
                    ::memcpy(m_pBuffer + m_nHeadPos + m_nSize, buffer, len);
                    m_nSize += len;
                    return;
                }
            }
            SetHeadPosition();
            if ((m_nMaxBuffer - m_nSize) >= len) {
                ::memmove(m_pBuffer + position + len, m_pBuffer + position, m_nSize - position);
                ::memcpy(m_pBuffer + position, buffer, len);
                m_nSize += len;
                return;
            }
            if (len + (SPA::UINT64)m_nSize >= 0xf0000000) {
                m_nMaxBuffer = UQUEUE_END_POSTION;
            } else {
                m_nMaxBuffer += m_nBlockSize * (1 + (len + m_nSize) / m_nBlockSize);
            }
            unsigned char *temp = (unsigned char*) ::malloc(m_nMaxBuffer);
            if (position) {
                ::memcpy(temp, m_pBuffer, position);
            }
            ::memcpy(temp + position, buffer, len);
            if (m_nSize > position) {
                ::memcpy(temp + position + len, m_pBuffer + position, m_nSize - position);
            } else if (m_nSize != position) {
                assert(false);
            }
            ::free(m_pBuffer);
            m_pBuffer = temp;
            m_nSize += len;
        }

        /**
         * Insert an ASCII string into a proper location
         * @param str Pointer to an ASCII string
         * @param chars The size of the string in char with default to string null-terminated
         * @param position The position in byte with default to 0
         */
        inline void Insert(const char* str, unsigned int chars = UQUEUE_NULL_LENGTH, unsigned int position = 0) {
            if (str) {
                if (chars == UQUEUE_NULL_LENGTH) {
                    chars = (unsigned int) ::strlen(str);
                }
                Insert((const unsigned char*) str, chars, position);
            }
        }

        /**
         * Insert a wide char string into a proper location
         * @param str Pointer to a wide char string
         * @param chars The size of the string in char
         * @param position The position in byte with default to 0
         */
        void Insert(const wchar_t* str, unsigned int chars, unsigned int position = 0);

        /**
         * Append an ASCII string
         * @param str Pointer to an ASCII string
         * @param chars The size of the string in char with default to string null-terminated
         */
        inline void Push(const char* str, unsigned int chars = UQUEUE_NULL_LENGTH) {
            Insert(str, chars, UQUEUE_END_POSTION);
        }

        /**
         * Append a wide char string
         * @param str Pointer to a wide char string
         * @param chars The size of the string in char with default to string null-terminated
         */
        inline void Push(const wchar_t* str, unsigned int chars = UQUEUE_NULL_LENGTH) {
            Insert(str, chars, UQUEUE_END_POSTION);
        }

        /**
         * Append an ASCII string
         * @param str An instance of standard library ASCII string
         */
        inline void Push(const std::string& str) {
            Insert(str.c_str(), (unsigned int) str.size(), UQUEUE_END_POSTION);
        }

        /**
         * Append a wide char string
         * @param str An instance of standard library wide char string
         */
        inline void Push(const std::wstring& str) {
            Insert(str.c_str(), (unsigned int) str.size(), UQUEUE_END_POSTION);
        }

        /**
         * Append an ASCII string with string length ahead (4 bytes)
         * @param str A pointer to an array of ASCII chars
         * @return Reference to this memory buffer
         */
        inline CUQueue& operator<<(const char* str) {
            unsigned int size;
            if (!str) {
                size = UQUEUE_NULL_LENGTH;
            } else {
                size = (unsigned int) ::strlen(str);
            }
            Push((const unsigned char*) &size, sizeof (size));
            if (str && size) {
                Push(str, size);
            }
            return *this;
        }

        /**
         * Append an ASCII string with string length ahead (4 bytes)
         * @param str A pointer to an array of ASCII chars
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(char* str) {
            const char *s = (const char*) str;
            return (*this << s);
        }

        /**
         * Append an ASCII string with string length ahead (4 bytes)
         * @param str An instance of standard library ASCII string
         * @return Reference to this memory buffer
         */
        inline CUQueue& operator<<(const std::string& str) {
            unsigned int size = (unsigned int) str.size();
            Push((const unsigned char*) &size, sizeof (size));
            if (size != 0) {
                Push(str.c_str(), size);
            }
            return *this;
        }

        /**
         * Append a wide char string with length ahead (4 bytes)
         * @param str An instance of standard library wide char string
         * @return Reference to this memory buffer
         */
        inline CUQueue& operator<<(const std::wstring& str) {
            return ((*this) << str.c_str());
        }

        /**
         * Append a wide char string with length ahead (4 bytes)
         * @param str A pointer to an array of wide chars
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const wchar_t *str) {
            unsigned int bytes;
            unsigned int len = (str) ? (unsigned int) ::wcslen(str) : UQUEUE_NULL_LENGTH;
            if (str) {
                bytes = len * sizeof (UTF16);
            } else {
                bytes = UQUEUE_NULL_LENGTH;
            }
            Push((const unsigned char*) &bytes, sizeof (bytes));
            if (!str || bytes == 0) {
                return *this;
            }
            unsigned int start = GetSize();
            Insert(str, len, UQUEUE_END_POSTION);
            len <<= 1;
            unsigned int diff = GetSize() - start;
            if (diff != len) {
                Replace(start - sizeof (unsigned int), sizeof (unsigned int), (const unsigned char*) &diff, sizeof (diff));
            }
            return *this;
        }

        /**
         * Append a wide char string with length ahead (4 bytes)
         * @param str A pointer to an array of wide chars
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(wchar_t *str) {
            const wchar_t *s = (const wchar_t *)str;
            return (*this << s);
        }

        /**
         * Pop content into standard library ASCII string
         * @param str An instance of standard library ASCII string for receiving
         * @return Reference to this memory buffer
         */
        inline CUQueue& operator>>(std::string& str) {
            unsigned int size;
            Pop((unsigned char*) &size, sizeof (unsigned int));
            switch (size) {
                case UQUEUE_NULL_LENGTH:
                case 0:
                    str.clear();
                    break;
                default:
                    if (size > GetSize()) {
                        throw CUException("Bad data for loading ASCII string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                    }
                    str.assign((const char*) GetBuffer(), size);
                    Pop(size);
                    break;
            }
            return *this;
        }

        /**
         * Pop content into standard library wstring
         * @param str An instance of standard library wstring for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(std::wstring& str);

        /**
         * Discard internal buffer at a proper position
         * @param len The size of buffer in byte
         * @param position The offset location of content in byte with default to 0
         * @return The actual size of content discarded in byte
         */
        inline unsigned int Pop(unsigned int len, unsigned int position = 0) {
            if (len == 0) {
                return 0;
            } else if (m_nSize <= position) {
                return 0;
            } else if (len > m_nSize - position) {
                len = m_nSize - position;
            }
            m_nSize -= len;
            if (m_nSize) {
                if (position == 0) {
                    m_nHeadPos += len;
                } else {
                    ::memmove(m_pBuffer + position + m_nHeadPos, m_pBuffer + len + position + m_nHeadPos, m_nSize - position);
                }
            } else {
                m_nHeadPos = 0;
            }
            return len;
        }

        /**
         * Pop internal content at a proper position into a buffer
         * @param buffer Pointer to a buffer for receiving data
         * @param len The size of the buffer in byte
         * @param position The offset location of internal content in byte with default to 0
         * @return The actual size of content popped in byte
         */
        inline unsigned int Pop(unsigned char* buffer, unsigned int len, unsigned int position = 0) {
            if (m_nSize < position || 0 == len || nullptr == buffer) {
                return 0;
            }
            if (m_nSize < (len + position)) {
                throw CUException("Remaining data in size smaller than expected size", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
            }
            unsigned char *src = m_pBuffer + position + m_nHeadPos;
            ::memcpy(buffer, src, len);
            m_nSize -= len;
            if (m_nSize) {
                if (position) {
                    ::memmove(src, src + len, m_nSize - position);
                } else {
                    m_nHeadPos += len;
                }
            } else {
                m_nHeadPos = 0;
            }
            return len;
        }

        /**
         * Append an unsigned short into the memory buffer
         * @param data an unsigned short
         * @return The reference to this memory buffer
         */
        CUQueue& operator<<(unsigned short data) {
            Push((unsigned char*) &data, sizeof (data));
            return *this;
        }

        /**
         * Append a memory buffer with length ahead (4 bytes)
         * @param mc An instance of memory buffer
         * @return The reference to this memory buffer
         */
        inline CUQueue& operator<<(const CUQueue &mc) {
            unsigned int size = mc.GetSize();
            Push(&size);
            Push(mc.GetBuffer(), size);
            return *this;
        }

        /**
         * Pop a memory buffer with length ahead (4 bytes) into a memory buffer
         * @param mc An instance of memory buffer for receiving data
         * @return The reference to this memory buffer
         */
        inline CUQueue& operator>>(CUQueue &mc) {
            unsigned int size;
            Pop((unsigned char*) &size, sizeof (unsigned int));
            if (size != UQUEUE_NULL_LENGTH && size != 0) {
                if (size > GetSize()) {
                    throw CUException("Bad data for loading a memory chunk", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                }
                mc.Push(GetBuffer(), size);
                Pop(size);
            }
            return *this;
        }

        /**
         * Insert a VARIANT structure into a proper location
         * @param vtData A VARIANT structure
         * @param position The position in byte with default to 0
         */
        void Insert(const VARIANT &vtData, unsigned int position = 0);

        /**
         * Append a VARIANT structure
         * @param vtData A VARIANT structure
         */
        void Push(const VARIANT &vtData) {
            Insert(vtData, UQUEUE_END_POSTION);
        }

        /**
         * Pop content into a VARIANT structure from a proper location
         * @param vtData A VARIANT structure for receiving data
         * @param position The position in byte with default to 0
         */
        unsigned int Pop(VARIANT& vtData, unsigned int position = 0);

        /**
         * Append a VARIANT structure
         * @param vtData A VARIANT structure
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const VARIANT &vtData) {
            Insert(vtData, UQUEUE_END_POSTION);
            return *this;
        }

        /**
         * Pop content into a VARIANT structure
         * @param vtData A VARIANT structure for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(VARIANT &vtData) {
            Pop(vtData);
            return *this;
        }

#ifdef __ATLCOMCLI_H__

        /**
         * Append a BSTR with string length ahead (4 bytes)
         * @param bstr An ATL BSTR
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const CComBSTR &bstr) {
            (*this) << bstr.m_str;
            return *this;
        }

        /**
         * Pop content into ATL BSTR string
         * @param str An instance of ATL BSTR string for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(CComBSTR &bstr) {
            unsigned int ulSize;
            ATLASSERT(GetSize() >= sizeof (ulSize));
            Pop((unsigned char*) &ulSize, sizeof (unsigned int));
            if (ulSize == UQUEUE_NULL_LENGTH) {
                bstr.Empty();
            } else if (ulSize == 0) {
                bstr = L"";
            } else {
                if (ulSize > GetSize()) {
                    throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                }
                BSTR bstrTemp = ::SysAllocStringLen((LPCWSTR) GetBuffer(), ulSize / sizeof (wchar_t));
                bstr.Attach(bstrTemp);
                Pop(ulSize); //discard
            }
            return *this;
        }
#endif

#if defined(__ATLCOMCLI_H__) || defined(_SOCKETPRO_LINUX_VARIANT_DEFINITION_H_)

        /**
         * Append a CComVariant structure
         * @param vtData A CComVariant structure
         */
        void Push(const CComVariant &vtData) {
            Insert((const VARIANT &) vtData, UQUEUE_END_POSTION);
        }

        /**
         * Pop content into a CComVariant structure from a proper location
         * @param vtData A CComVariant structure for receiving data
         * @param position The position in byte with default to 0
         */
        unsigned int Pop(CComVariant& vtData, unsigned int position = 0) {
            return Pop((VARIANT&) vtData);
        }

        /**
         * Append a CComVariant structure
         * @param vtData A CComVariant structure
         * @return Reference to this memory buffer 
         */
        CUQueue& operator<<(const CComVariant &vtData) {
            Insert((const VARIANT &) vtData, UQUEUE_END_POSTION);
            return *this;
        }

        /**
         * Pop content into a CComVariant structure
         * @param vtData A CComVariant structure for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(CComVariant &vtData) {
            Pop((VARIANT &) vtData);
            return *this;
        }
#endif

#ifdef _INC_COMUTIL

        /**
         * Append a BSTR with string length ahead (4 bytes)
         * @param bstr An ATL BSTR
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const _bstr_t &bstr) {
            (*this) << (const wchar_t *) bstr;
            return *this;
        }

        /**
         * Pop content into ATL BSTR string
         * @param str An instance of ATL BSTR string for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(_bstr_t &bstr) {
            unsigned int ulSize;
            assert(GetSize() >= sizeof (ulSize));
            Pop((unsigned char*) &ulSize, sizeof (unsigned int));
            if (ulSize == UQUEUE_NULL_LENGTH) {
                bstr.Assign(nullptr);
            } else if (ulSize == 0) {
                bstr = L"";
            } else {
                if (ulSize > GetSize()) {
                    throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                }
                BSTR bstrTemp = ::SysAllocStringLen((LPCWSTR) GetBuffer(), ulSize / sizeof (wchar_t));
                bstr.Attach(bstrTemp);
                Pop(ulSize); //discard
            }
            return *this;
        }

        /**
         * Pop content into a CComVariant structure from a proper location
         * @param vtData A CComVariant structure for receiving data
         * @param position The position in byte with default to 0
         */
        unsigned int Pop(_variant_t& vtData, unsigned int position = 0) {
            return Pop((VARIANT&) vtData);
        }

        /**
         * Append a CComVariant structure
         * @param vtData A CComVariant structure
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const _variant_t &vtData) {
            Insert((const VARIANT &) vtData, UQUEUE_END_POSTION);
            return *this;
        }

        /**
         * Pop content into a CComVariant structure
         * @param vtData A CComVariant structure for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(_variant_t &vtData) {
            Pop((VARIANT &) vtData);
            return *this;
        }
#endif

#ifdef __ATLSTR_H__

        /**
         * Append an ATL wide string with string length ahead (4 bytes)
         * @param str An ATL wide string
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const ATL::CAtlStringW &str) {
            *this << LPCWSTR(str);
            return *this;
        }

        /**
         * Append an ATL ASCII string with string length ahead (4 bytes)
         * @param str An ATL ASCII string
         * @return Reference to this memory buffer
         */
        CUQueue& operator<<(const ATL::CAtlStringA &str) {
            unsigned int size = (str.GetString() ? (unsigned int) str.GetLength() : UQUEUE_NULL_LENGTH);
            Push(&size);
            if (size != UQUEUE_NULL_LENGTH && size > 0) {
                Push((const char*) str.GetString(), size);
            }
            return *this;
        }

        /**
         * Pop content into ATL wide char string
         * @param str An instance of ATL wide char string for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(ATL::CAtlStringW &str) {
            unsigned int size;
            Pop((unsigned char*) &size, sizeof (unsigned int));
            switch (size) {
                case UQUEUE_NULL_LENGTH:
                    str.Empty();
                    break;
                case 0:
                    str = L"";
                    break;
                default:
                    if (size > GetSize()) {
                        throw CUException("Bad data for loading UNICODE string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                    }
                    str.Empty();
                    str.Append((const wchar_t*)GetBuffer(), size / sizeof (wchar_t));
                    Pop(size);
            }
            return *this;
        }

        /**
         * Pop content into ATL ASCII string
         * @param str An instance of ATL ASCII string for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(ATL::CAtlStringA &str) {
            unsigned int size;
            Pop((unsigned char*) &size, sizeof (unsigned int));
            switch (size) {
                case UQUEUE_NULL_LENGTH:
                    str.Empty();
                    break;
                case 0:
                    str = "";
                    break;
                default:
                    str.Empty();
                    if (size > GetSize()) {
                        throw CUException("Bad data for loading ASCII string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                    }
                    str.Append((const char*) GetBuffer(), size);
                    Pop(size);
                    break;
            }
            return *this;
        }
#endif


#ifdef NATIVE_UTF16_SUPPORTED

        /**
         * Insert an UTF16 string into a proper location
         * @param str Pointer to an UTF16 string
         * @param chars The size of the string in char with default to string null-terminated
         * @param position The position in byte with default to 0
         */
        inline void Insert(const char16_t* str, unsigned int chars = UQUEUE_NULL_LENGTH, unsigned int position = 0) {
            if (str) {
                if (chars == UQUEUE_NULL_LENGTH) {
                    chars = (unsigned int) GetLen(str);
                }
                Insert((const unsigned char*) str, chars << 1, position);
            }
        }

        /**
         * Append an UTF16 string
         * @param str Pointer to an UTF16 string
         * @param chars The size of the string in char with default to string null-terminated
         */
        inline void Push(const char16_t* str, unsigned int chars = UQUEUE_NULL_LENGTH) {
            Insert(str, chars, UQUEUE_END_POSTION);
        }

        CUQueue& operator<<(const char16_t *str) {
            unsigned int size;
            if (!str) {
                size = UQUEUE_NULL_LENGTH;
                Push((const unsigned char*) &size, sizeof (unsigned int));
                return *this;
            }
            size = (unsigned int) GetLen(str);
            size <<= 1;
            Push((const unsigned char*) &size, sizeof (unsigned int));
            if (size) {
                Push((const unsigned char*) str, size);
            }
            return *this;
        }

        CUQueue& operator<<(char16_t *str) {
            *this << (const char16_t*) str;
            return *this;
        }

        CUQueue& operator<<(const std::u16string& str) {
            unsigned int size = (unsigned int) str.size();
            size <<= 1;
            Push((const unsigned char*) &size, sizeof (unsigned int));
            if (size) {
                Push((const unsigned char*) str.c_str(), size);
            }
            return *this;
        }

        CUQueue& operator<<(const char16_t& c) {
            Push((const unsigned char*) &c, sizeof (c));
            return *this;
        }

        CUQueue& operator>>(char16_t& c) {
            Pop((unsigned char*) &c, sizeof (c));
            return *this;
        }

        /**
         * Pop content into std::u16string string
         * @param str An instance of std::u16string string for receiving data
         * @return Reference to this memory buffer
         */
        CUQueue& operator>>(std::u16string &str) {
            unsigned int size;
            Pop((unsigned char*) &size, sizeof (unsigned int));
            switch (size) {
                case UQUEUE_NULL_LENGTH:
                case 0:
                    str.clear();
                    break;
                default:
                    if (size > GetSize()) {
                        throw CUException("Bad data for loading UTF16 string", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                    }
                    const char16_t *s = (const char16_t*) GetBuffer();
                    str.assign(s, size >> 1);
                    Pop(size); //discard
                    break;
            }
            return *this;
        }
#endif

        inline void Insert(const UDateTime &dt, unsigned int pos = 0) {
            Insert((const unsigned char*) &dt.time, sizeof (dt.time), pos);
        }

        inline void Push(const UDateTime &dt) {
            Insert(dt, UQUEUE_END_POSTION);
        }

        inline unsigned int Pop(UDateTime &dt, unsigned int position = 0) {
            unsigned int res = Pop((unsigned char*) &dt.time, sizeof (dt.time), position);
            return res;
        }

        inline CUQueue& operator<<(const UDateTime &dt) {
            Insert(dt, UQUEUE_END_POSTION);
            return *this;
        }

        inline CUQueue& operator>>(UDateTime &dt) {
            Pop(dt);
            return *this;
        }

        /**
         * Insert a data into this memory object at a position
         * @param buffer Pointer to a primitive type of data or a structure
         * @param position A position indicator with default to 0
         */
        template<class PrimitiveType>
        inline void Insert(const PrimitiveType* buffer, unsigned int position = 0) {
            Insert((const unsigned char*) buffer, sizeof (PrimitiveType), position);
        }

        /**
         * Pop a data from this memory object at a position
         * @param buffer Pointer to a primitive type of data or a structure for receiving data
         * @param position A position indicator with default to 0
         * @return Actual data length in byte
         */
        template <class PrimitiveType>
        inline unsigned int Pop(PrimitiveType *buffer, unsigned int position = 0) {
            return Pop((unsigned char*) buffer, sizeof (PrimitiveType), position);
        }

        /**
         * Append a data into this memory object
         * @param buffer Pointer to a primitive type of data or a structure
         */
        template <class PrimitiveType>
        inline void Push(const PrimitiveType *buffer) {
            Push((const unsigned char*) buffer, sizeof (PrimitiveType));
        }

        CUQueue& operator<<(const char &c) {
            Push((const unsigned char*) &c, 1);
            return *this;
        }

        CUQueue& operator<<(const wchar_t &c) {
#ifdef WIN32_64
            Push((const unsigned char*) &c, sizeof (c));
#else
            UTF16 utf16 = (UTF16) c;
            Push((const unsigned char*) &utf16, sizeof (UTF16));
#endif
            return *this;
        }

        template <typename PrimitiveType>
        CUQueue& operator<<(const std::vector<PrimitiveType> &data) {
            unsigned int count = (unsigned int) data.size();
            *this << count;
            if (count) {
                Push((const unsigned char*) data.data(), count * sizeof (PrimitiveType));
            }
            return *this;
        }

        template <typename PrimitiveType>
        CUQueue& operator>>(std::vector<PrimitiveType> &data) {
            unsigned int count;
            *this >> count;
            if (count) {
                unsigned int bytes = count * sizeof (PrimitiveType);
                if (bytes > GetSize()) {
                    throw CUException("Bad data for loading array of data", __FILE__, __LINE__, __FUNCTION__, MB_BAD_DESERIALIZATION);
                }
                data.resize(count);
                unsigned char *buffer = (unsigned char *) data.data();
                Pop(buffer, bytes);
            } else {
                data.clear();
            }
            return *this;
        }

        /**
         * Append a data into this memory object
         * @param data A reference to a primitive type of data or a structure
         * @return The reference to this memory buffer
         */
        template<class PrimitiveType>
        inline CUQueue& operator<<(const PrimitiveType &data) {
            Push(&data);
            return *this;
        }

        CUQueue& operator>>(char &data) {
            Pop((unsigned char*) &data, sizeof (data));
            return *this;
        }

        CUQueue& operator>>(wchar_t &data) {
#ifdef WIN32_64
            Pop((unsigned char*) &data, sizeof (data));
#else
            UTF16 c;
            Pop((unsigned char*) &c, sizeof (c));
            data = c;
#endif
            return *this;
        }

        /**
         * Pop a data from this memory object
         * @param data A reference to a primitive type of data or a structure for receiving data
         * @return The reference to this memory buffer
         */
        template<class PrimitiveType>
        inline CUQueue& operator>>(PrimitiveType &data) {
            Pop((unsigned char*) &data, sizeof (data));
            return *this;
        }

        template<typename R>
        inline R Load() {
            R r;
            *this >> r;
            return r;
        }

        template<typename T>
        inline CUQueue& Save(const T& data) {
            *this << data;
            return *this;
        }

        template<typename T, typename ...Ts>
        inline CUQueue& Save(const T& data, const Ts& ... others) {
            Save(data);
            Save(others ...);
            return *this;
        }

    private:
        unsigned int m_nMaxBuffer;
        unsigned int m_nSize;
        unsigned int m_nHeadPos;
        tagOperationSystem m_os;
        bool m_bSameEndian;
        unsigned int m_nBlockSize;
        unsigned char *m_pBuffer;
        bool m_8ToW;
        bool m_ToUtf8;
#ifdef WIN32_64
        bool m_TimeEx;
#endif
    };
    typedef CUQueue* PUQueue;

    /**
     * Shared and reusable buffer size in kilobytes
     */
    extern unsigned int SHARED_BUFFER_CLEAN_SIZE;

    /** 
     * The template class CScopeUQueueEx (a smart pointer for CUQueue) stores a pointer to a dynamically allocated memory buffer object. 
     * The object pointed to, if available, is guaranteed to be recycled into a memory buffer pool for reuse on destruction of the CScopeUQueueEx.
     * Internal CUQueue objects are guaranteed to be reused or recycled from or into buffer pool thread-safely.
     * Operators << and >> are overloaded for easy serialization and de-serialization. 
     * Note that you can detach or re-attach a pointer to CUQueue object at run time. Also, various methods will throw exceptions if no CUQueue object is attached.
     * Its copy constructor and assignment operators are disabled.
     */
    template<unsigned int InitSize, unsigned int BlockSize, typename mb = CUQueue>
    class CScopeUQueueEx {
        // Copy constructor disabled
        CScopeUQueueEx(const CScopeUQueueEx& sb) = delete;

        // Assignment operator disabled
        CScopeUQueueEx& operator=(const CScopeUQueueEx& sb) = delete;

    private:

        class CQPool : public CSafeDeque<mb*> {
        public:
            typedef mb *PMB;
            typedef CSafeDeque<mb*> base;

            void DestroyUQueuePool() {
                CSpinAutoLock al(base::m_sl);
                for (size_t n = 0; n < base::m_count; ++n) {
                    PMB p = base::m_p[base::m_header + n];
                    delete p;
                }
                base::m_header = 0;
                base::m_count = 0;
            }

            void CleanUQueuePool() {
                CSpinAutoLock al(base::m_sl);
                for (size_t n = 0; n < base::m_count; ++n) {
                    PMB p = base::m_p[base::m_header + n];
                    p->CleanTrack();
                }
            }

            UINT64 GetMemoryConsumed() {
                UINT64 size = 0;
                CSpinAutoLock al(base::m_sl);
                for (size_t n = 0; n < base::m_count; ++n) {
                    PMB p = base::m_p[base::m_header + n];
                    size += p->GetMaxSize();
                }
                return size;
            }

            void ResetSize(unsigned int newSize = InitSize) {
                CSpinAutoLock al(base::m_sl);
                for (size_t n = 0; n < base::m_count; ++n) {
                    PMB p = base::m_p[base::m_header + n];
                    if (p->GetMaxSize() > newSize) {
                        p->ReallocBuffer(newSize);
                    }
                }
            }

            inline PMB Lock(tagOperationSystem os, bool bigEndian, unsigned int initSize, unsigned int blockSize) {
                PMB p;
                if (base::pop_front(p)) {
                    p->SetEndian(bigEndian);
                    p->SetOS(os);
                    p->SetBlockSize(blockSize);
                    if (p->GetMaxSize() < initSize) {
                        p->ReallocBuffer(initSize);
                    }
                    p->Utf8ToW(false);
                    p->ToUtf8(false);
#ifdef WIN32_64
                    p->TimeEx(false);
#endif
                } else {
                    p = new mb(initSize, blockSize, os, bigEndian);
                }
                return p;
            }

            inline void Unlock(PMB &memoryChunk) {
                if (!memoryChunk) {
                    return;
                }
                memoryChunk->SetSize(0);
                base::push_front(memoryChunk);
                memoryChunk = nullptr;
            }
        };

    public:
        typedef mb *PMB;

        /** 
         * Construct an instance of CScopeUQueueEx, and automatically lock a memory buffer object from its pool
         * @param os The operation system with default to the current platform
         * @param bigEndian True for big endian and false for little endian
         * @param initSize The size of memory allocated in byte for internal memory buffer object
         * @param blockSize The block size in byte for internal memory buffer object
         */
        CScopeUQueueEx(tagOperationSystem os = MY_OPERATION_SYSTEM, bool bigEndian = IsBigEndian(), unsigned int initSize = InitSize, unsigned int blockSize = BlockSize)
        : m_pUQueue(Lock(os, bigEndian, initSize, blockSize)) {
        }

        /** 
         * Ensure recycling its internal memory buffer object if available into memory buffer pool for reuse, and destruct this object
         */
        ~CScopeUQueueEx() {
            Unlock(m_pUQueue);
        }
#ifndef WINCE

        /** 
         * Movable copy constructor
         */
        CScopeUQueueEx(CScopeUQueueEx && su)
        : m_pUQueue(su.m_pUQueue) {
            su.m_pUQueue = nullptr;
        }
#endif

    public:

#ifndef WINCE

        /** 
         * Movable assignment operator
         */
        CScopeUQueueEx& operator=(CScopeUQueueEx && su) {
            if (this != &su) {
                Swap(su);
            }
            return *this;
        }
#endif

#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)

        static UINT64 GetContention() {
            return m_memPool.contention();
        }
#endif

        /** 
         * Return a pointer to an internal memory buffer object. The method will return nullptr if no internal memory buffer object is available
         * @return A pointer to an internal memory buffer object
         */
        inline mb* operator->() const {
            return m_pUQueue;
        }

        /** 
         * Return a pointer to an internal memory buffer object. The method will return nullptr if no internal memory buffer object is available
         * @return A pointer to an internal memory buffer object
         */
        inline mb* Get() const {
            return m_pUQueue;
        }

        /** 
         * Return a pointer to an internal memory buffer object. The method will return nullptr if no internal memory buffer object is available
         * @return A pointer to an internal memory buffer object
         */
        inline operator PMB() const {
            return m_pUQueue;
        }

        /** 
         * Return a reference to an internal memory buffer object. The method may throw exception if no internal memory buffer object is available
         * @return A reference to an internal memory buffer object
         */
        inline mb& operator*() const throw () {
            return *m_pUQueue;
        }

        /** 
         * Check if there is an internal memory buffer object available
         * @return True or false
         */
        inline bool Available() const {
            return (m_pUQueue != nullptr);
        }

        /** 
         * Swap internal memory buffer objects between two instances of CScopeUQueueEx
         * @param sb A reference to a CScopeUQueueEx object
         */
        inline void Swap(CScopeUQueueEx& sb) {
            mb *p = sb.m_pUQueue;
            sb.m_pUQueue = m_pUQueue;
            m_pUQueue = p;
        }

        /** 
         * Detach internal memory buffer object from this CScopeUQueueEx object
         * @return A pointer to a memory buffer object
         */
        inline mb* Detach() {
            mb *p = m_pUQueue;
            m_pUQueue = nullptr;
            return p;
        }

        /** 
         * Recycle an existing memory buffer object into its pool if available, and attach a new memory buffer object
         * @param p A pointer to a new memory buffer object
         */
        inline void Attach(mb *p) {
            Unlock(m_pUQueue);
            m_pUQueue = p;
        }

        /**
         * 
         * @param q
         * @return 
         */
        inline mb& operator<<(const mb &q) {
            if (!m_pUQueue) {
                m_pUQueue = Lock(q.GetOS(), q.GetEndian(), InitSize, BlockSize);
            }
            *m_pUQueue << q;
            return *m_pUQueue;
        }

        /**
         * 
         * @param q
         * @return 
         */
        inline mb& operator>>(mb &q) {
            if (!m_pUQueue) {
                m_pUQueue = Lock(q.GetOS(), q.GetEndian(), InitSize, BlockSize);
            }
            *m_pUQueue >> q;
            return *m_pUQueue;
        }

        /**
         * Push a data into internal memory buffer object, and throw an exception if no memory buffer object is available
         * @param data A reference to a type of data or a structure
         * @return The reference to this memory buffer
         */
        template<class ctype>
        inline mb& operator<<(const ctype &data) throw () {
            *m_pUQueue << data; //make sure this operator is correct as expected
            return *m_pUQueue;
        }

        /**
         * Pop a data from internal memory buffer object, and throw an exception if no memory buffer object is available
         * @param data A reference to a type of data or a structure for receiving data
         * @return The reference to internal memory buffer object
         */
        template<class ctype>
        inline mb& operator>>(ctype &data) throw () {
            *m_pUQueue >> data; //make sure this operator is correct as expected
            return *m_pUQueue;
        }

        /**
         * Destroy all existing memory objects in pool
         */
        static void DestroyUQueuePool() {
            m_memPool.DestroyUQueuePool();
        }

        /**
         * Reset all existing memory objects in pool to initial size (InitSize)
         */
        static void ResetSize(unsigned int newSize = InitSize) {
            m_memPool.ResetSize(newSize);
        }

        /**
         * Zero all existing memory objects in pool
         */
        static void CleanUQueuePool() {
            m_memPool.CleanUQueuePool();
        }

        /**
         * Lock a memory buffer object from pool
         * @param os The operation system with default to the current platform
         * @param bigEndian True for big endian and false for little endian
         * @param initSize The size of memory allocated in byte for internal memory buffer object
         * @param blockSize The block size in byte for internal memory buffer object
         * @return A pointer to a memory buffer object
         */
        static inline PMB Lock(tagOperationSystem os = MY_OPERATION_SYSTEM, bool bigEndian = IsBigEndian(), unsigned int initSize = InitSize, unsigned int blockSize = BlockSize) {
            return m_memPool.Lock(os, bigEndian, initSize, blockSize);
        }

        /**
         * Recycle a memory buffer object into pool for reuse
         * @param memoryChunk A pointer to a memory buffer object
         */
        static inline void Unlock(PMB &memoryChunk) {
            m_memPool.Unlock(memoryChunk);
        }

        /**
         * Get the total size of pooled memory buffer objects
         * @return The total size of pooled memory buffer objects in byte
         */
        static UINT64 GetMemoryConsumed() {
            return m_memPool.GetMemoryConsumed();
        }

    private:
        mb *m_pUQueue;
        static U_MODULE_HIDDEN CQPool m_memPool;
    };

    template<unsigned int InitSize, unsigned int BlockSize, typename mb>
    typename CScopeUQueueEx<InitSize, BlockSize, mb>::CQPool CScopeUQueueEx<InitSize, BlockSize, mb>::m_memPool;

    typedef CScopeUQueueEx<DEFAULT_INITIAL_MEMORY_BUFFER_SIZE, DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CScopeUQueue;

    template<typename T>
    int swscanf(BSTR bstrVal, const wchar_t* format, T *t) {
#ifdef WIN32_64
        return ::swscanf((const wchar_t*)bstrVal, format, t);
#else
        CScopeUQueue sb;
        CUQueue &q = *sb;
        Utilities::ToWide(bstrVal, GetLen(bstrVal), q, true);
        return ::swscanf((const wchar_t*)q.GetBuffer(), format, t);
#endif
    }
}; //namespace SPA

#endif //__UCOMM_SHARED_MEMORY_QUEUE_H_
