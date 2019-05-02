
#ifndef __MB_UZIP_H__
#define __MB_UZIP_H__

#include "../include/membuffer.h"

#define FAST_COMPRESS_MIN_SIZE 700
#define ZLIB_COMPRESS_MIN_SIZE 350

namespace SPA {
    int MapSockOption(tagSocketOption so);
    int MapSockLevel(tagSocketLevel sl);
    bool Compress(tagZipLevel zl, const void *pSource, unsigned int srcSize, void *pDestination, unsigned int &desSize);
    bool Decompress(tagZipLevel zl, const void *pSource, unsigned int srcSize, void *pDestination, unsigned int &desSize);
    extern bool g_bAdapterUTF16;
    std::wstring ToNativeString(const UTF16 *chars, unsigned int len);
};

#endif 
