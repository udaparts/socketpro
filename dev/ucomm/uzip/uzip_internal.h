

#ifndef __MB_UZIP_H__
#define __MB_UZIP_H__

#include "../include/commutil.h"
#include "zlib.h"
#include "lz4.h"


namespace SPA {

    class CUZip {
        CUZip();

    public:
        ~CUZip();

        /*
         * QuickLz	maxRatio = 90, destination size required (srcSize + 420)
         * Zlib		maxRatio = 1013, destination size required (1.1*srcSize + 15) at least;
         * 7-zip		maxRatio = 7062 -- 128 SPA
         */
    public:
        bool Compress(tagZipLevel zl, const void *pSource, unsigned int srcSize, void *pDestination, unsigned int &desSize);
        bool Decompress(tagZipLevel zl, const void *pSource, unsigned int srcSize, void *pDestination, unsigned int &desSize);
        static SPA::CUZip* LockZip();
        static void UnlockZip(SPA::CUZip *UZip);

    private:
        bool m_bDeflateInit;
        z_stream m_Deflate;
        bool m_bInflateInit;
        z_stream m_Inflate;

        static CUCriticalSection m_mutexZip;
        static std::vector<SPA::CUZip*> m_vUZip;
    };

    /*
    class CGzip
    {
      static unsigned char GZIP_HEADER[10];

    public:
      CGzip();
      ~CGzip();

    public:
      bool Zip(const unsigned char *pSource, unsigned int srcSize, unsigned char *pDestination, unsigned int &desSize);

    private:
      z_stream m_Deflate;
    };
     */
};

#endif 
