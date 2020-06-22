// UZip.cpp: implementation of the CUZip class.
//
//////////////////////////////////////////////////////////////////////

#include "uzip_internal.h"
#include <assert.h>
//#include "LzmaLib.h"
#include "zutil.h"
#include "../include/membuffer.h"

#ifdef WIN32_64
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#endif

namespace SPA {

    bool g_bAdapterUTF16 = (GetWCharSize(GetOS()) != sizeof (wchar_t));

    std::wstring ToNativeString(const UTF16 *chars, unsigned int len) {
        assert(g_bAdapterUTF16);
        assert(sizeof (wchar_t) != sizeof (UTF16));
        if (chars == nullptr || len == 0)
            return std::wstring();
        unsigned int utfchars = GetLen(chars);
#if defined(__ANDROID__) || defined(ANDROID)
        return Utilities::ToWide(chars, utfchars);
#else
        CScopeUQueue su;
#ifdef WCHAR32
        Utilities::ToWide(chars, utfchars, *su);
#endif
        const wchar_t *str = (const wchar_t *)su->GetBuffer();
        len = su->GetSize() / sizeof (wchar_t);
        return std::wstring(str, str + len);
#endif
    }

    CUCriticalSection CUZip::m_mutexZip;
    std::vector<SPA::CUZip*> CUZip::m_vUZip;

    int MapSockOption(tagSocketOption so) {
        int data = (int) so;
        switch (so) {
            case soTcpNoDelay:
                data = TCP_NODELAY;
                break;
            case soReuseAddr:
                data = SO_REUSEADDR;
                break;
            case soKeepAlive:
                data = SO_KEEPALIVE;
                break;
            case soSndBuf:
                data = SO_SNDBUF;
                break;
            case soRcvBuf:
                data = SO_RCVBUF;
                break;
            default:
                break;
        }
        return data;
    }

    int MapSockLevel(tagSocketLevel sl) {
        int data = (int) sl;
        switch (sl) {
            case slTcp:
                data = IPPROTO_TCP;
                break;
            case slSocket:
                data = SOL_SOCKET;
                break;
            default:
                break;
        }
        return data;
    }

    SPA::CUZip* CUZip::LockZip() {
        SPA::CUZip *p = nullptr;
        {
            CAutoLock sl(m_mutexZip);
            if (m_vUZip.size()) {
                p = m_vUZip.back();
                m_vUZip.pop_back();
            }
        }
        if (!p)
            p = new SPA::CUZip;
        return p;
    }

    void CUZip::UnlockZip(SPA::CUZip *UZip) {
        if (UZip) {
            CAutoLock sl(m_mutexZip);
            m_vUZip.push_back(UZip);
        }
    }

    bool Compress(tagZipLevel zl, const void *pSource, unsigned int srcSize, void *pDestination, unsigned int &desSize) {
        CUZip *p = CUZip::LockZip();
        bool ok = p->Compress(zl, pSource, srcSize, pDestination, desSize);
        CUZip::UnlockZip(p);
        return ok;
    }

    bool Decompress(tagZipLevel zl, const void *pSource, unsigned int srcSize, void *pDestination, unsigned int &desSize) {
        CUZip *p = CUZip::LockZip();
        bool ok = p->Decompress(zl, pSource, srcSize, pDestination, desSize);
        CUZip::UnlockZip(p);
        return ok;
    }

    CUZip::CUZip() {
        m_bDeflateInit = false;
        m_Deflate.zalloc = (alloc_func) 0;
        m_Deflate.zfree = (free_func) 0;
        m_Deflate.opaque = (voidpf) 0;

        m_bInflateInit = false;
        m_Inflate.zalloc = (alloc_func) 0;
        m_Inflate.zfree = (free_func) 0;
    }

    CUZip::~CUZip() {
        int err;
        if (m_bInflateInit) {
            err = inflateEnd(&m_Inflate);
        }

        if (m_bDeflateInit) {
            err = deflateEnd(&m_Deflate);
        }
    }

    bool CUZip::Decompress(tagZipLevel zl, const void *pSource, unsigned int ulSrcSize, void *pDestination, unsigned int &ulDesSize) {
        bool bSuc;
        if (pSource == nullptr || ulSrcSize == 0) {
            ulDesSize = 0;
            return true;
        }
        if (pDestination == nullptr || ulDesSize == 0)
            return false;
        switch (zl) {
            case zlBestSpeed:
            {
                ulDesSize = (unsigned int) LZ4_uncompress_unknownOutputSize((const char*) pSource, (char *) pDestination, (int) ulSrcSize, (int) ulDesSize);
                bSuc = ((int) ulDesSize >= 0) ? true : false;
            }
                break;
            case zlDefault:
            {
                int err;
                if (!m_bInflateInit) {
                    err = inflateInit(&m_Inflate);
                    if (err != Z_OK)
                        return false;
                    m_bInflateInit = true;
                }

                m_Inflate.next_in = (Bytef*) pSource;
                m_Inflate.avail_in = (uInt) ulSrcSize;

                m_Inflate.next_out = (unsigned char *) pDestination;
                m_Inflate.avail_out = (uInt) ulDesSize;

                err = inflate(&m_Inflate, Z_FINISH);
                bSuc = (err == Z_STREAM_END);

                ulDesSize = m_Inflate.total_out;

                err = inflateReset(&m_Inflate);
            }
                break;
                /*case zlBestCompression:
                {
                if(ulSrcSize < LZMA_PROPS_SIZE)
                {
                ulDesSize = 0;
                return false;
                }
                size_t des = ulDesSize;
                size_t srcLen = ulSrcSize - LZMA_PROPS_SIZE;
                unsigned char *Props = (unsigned char *)pSource;
                int res =  LzmaUncompress((unsigned char *)pDestination, &des, Props + LZMA_PROPS_SIZE, &srcLen, Props, LZMA_PROPS_SIZE);
                ulDesSize = (unsigned int)des;
                bSuc = (res == SZ_OK || res == SZ_ERROR_INPUT_EOF);
                }
                break;*/
            default:
                assert(false);
                return false;
                break;
        }
        return bSuc;
    }

    bool CUZip::Compress(tagZipLevel zl, const void *pSource, unsigned int ulSrcSize, void *pDestination, unsigned int &ulDesSize) {
        bool bSuc;
        if (pSource == nullptr || ulSrcSize == 0) {
            ulDesSize = 0;
            return true;
        }

        if (pDestination == nullptr || ulDesSize == 0)
            return false;

        switch (zl) {
            case zlBestSpeed:
            {
                if ((int) ulDesSize < LZ4_compressBound((int) ulSrcSize))
                    return false;
                ulDesSize = (unsigned int) LZ4_compress((const char*) pSource, (char *) pDestination, ulSrcSize);
                bSuc = (ulDesSize > 0) ? true : false;
            }
                break;
            case zlDefault:
            {
                int err;
                unsigned long ul = (unsigned long) (1.1 * ulSrcSize) + 15;
                if (ul > ulDesSize)
                    return false;

                if (!m_bDeflateInit) {
                    err = deflateInit(&m_Deflate, Z_DEFAULT_COMPRESSION);
                    if (err != Z_OK)
                        return false;
                    m_bDeflateInit = true;
                }

                m_Deflate.next_in = (Bytef*) pSource;
                m_Deflate.avail_in = (uInt) ulSrcSize;

                m_Deflate.next_out = (unsigned char *) pDestination;
                m_Deflate.avail_out = (uInt) ulDesSize;

                err = deflate(&m_Deflate, Z_FINISH);
                bSuc = (err == Z_STREAM_END);
                ulDesSize = m_Deflate.total_out;

                err = deflateReset(&m_Deflate);
            }
                break;
                //case zlBestCompression:
                //{
                //	if(ulDesSize < LZMA_PROPS_SIZE)
                //	{
                //		ulDesSize = 0;
                //		return false;
                //	}
                //	unsigned char *Props = (unsigned char *)pDestination;
                //	size_t sizeProps = LZMA_PROPS_SIZE;
                //	size_t sizeDes = ulDesSize - LZMA_PROPS_SIZE;
                //	int res = LzmaCompress(	Props + LZMA_PROPS_SIZE, &sizeDes, 
                //							(const unsigned char *)pSource, ulSrcSize,
                //							Props, 
                //							&sizeProps, /* *outPropsSize must be = 5 */
                //							1,      /* 0 <= level <= 9, default = 5 */
                //							64*1024,  /* default = (1 << 24) */
                //							3,        /* 0 <= lc <= 8, default = 3  */
                //							0,        /* 0 <= lp <= 4, default = 0  */
                //							2,        /* 0 <= pb <= 4, default = 2  */
                //							32,        /* 5 <= fb <= 273, default = 32 */
                //							2 /* 1 or 2, default = 2 */
                //			);
                //	bSuc = (res == SZ_OK);
                //	if(bSuc)
                //		ulDesSize = (unsigned int)sizeDes + LZMA_PROPS_SIZE;
                //	else
                //		ulDesSize = 0;
                //}
                //	break;
            default:
                assert(false);
                return false;
                break;
        }

        return bSuc;
    }

    /*

    unsigned char CGzip::GZIP_HEADER[10] = {0x1f, 0x8b, Z_DEFLATED, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, OS_CODE};

    CGzip::CGzip()
    {
      m_Deflate.zalloc = (alloc_func)0;
      m_Deflate.zfree = (free_func)0;
      m_Deflate.opaque = (voidpf)0;
      int err = deflateInit(&m_Deflate, Z_DEFAULT_COMPRESSION);
      //int err = deflateInit2(&m_Deflate, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 8, Z_DEFAULT_STRATEGY);
    }

    CGzip::~CGzip()
    {
      int err = deflateEnd(&m_Deflate);
    }

     bool CGzip::Zip(const unsigned char *pSource, unsigned int srcSize, unsigned char *pDestination, unsigned int &desSize)
     {
       unsigned int available = desSize;
       unsigned char *receiver = pDestination;
       ::memcpy(receiver, GZIP_HEADER, sizeof(GZIP_HEADER));
       receiver += sizeof(GZIP_HEADER);
       desSize -= sizeof(GZIP_HEADER);
       unsigned int crc = crc32(0L, Z_NULL, 0);
       m_Deflate.next_in = (Bytef*)pSource;
       m_Deflate.avail_in = srcSize;
       crc = crc32(crc, m_Deflate.next_in, m_Deflate.avail_in);

       m_Deflate.next_out = receiver;
       m_Deflate.avail_out = desSize;

       int err = deflate(&m_Deflate, Z_FINISH);
       bool ok = (err == Z_STREAM_END);
       if(!ok)
             return false;
       desSize = m_Deflate.total_out + sizeof(GZIP_HEADER) + 2*sizeof(unsigned int);
       if(available < desSize)
             return false;
       receiver += m_Deflate.total_out;
       ::memcpy(receiver, &crc, sizeof(crc));
       receiver += sizeof(crc);
       ::memcpy(receiver, &srcSize, sizeof(srcSize));
       err =  deflateReset(&m_Deflate);
       return ok;
     }

     */

}
