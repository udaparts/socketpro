#include "stdafx.h"
#include "httpgrammar.h"


namespace UHTTP {

    const char HEX2DEC[256] = {
        /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
        /* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 3 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

        /* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

        /* 8 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 9 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* A */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* B */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

        /* C */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* D */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* E */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* F */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };

    unsigned int UriDecode(const unsigned char *strIn, unsigned int nLenIn, unsigned char *strOut) {
        const unsigned char *pSrc = strIn;
        unsigned int SRC_LEN = nLenIn;
        const unsigned char *SRC_END = pSrc + SRC_LEN;
        const unsigned char *SRC_LAST_DEC = SRC_END - 2; // last decodable '%' 

        unsigned char *pStart = strOut;
        unsigned char *pEnd = pStart;
        unsigned int ulGet = 0;

        while (pSrc < SRC_LAST_DEC) {
            ++ulGet;
            if (*pSrc == '%') {
                char dec1, dec2;
                if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                        && -1 != (dec2 = HEX2DEC[*(pSrc + 2)])) {
                    *pEnd++ = (dec1 << 4) + dec2;
                    pSrc += 3;
                    continue;
                }
            }
            *pEnd++ = *pSrc++;
        }

        // the last 2- chars
        while (pSrc < SRC_END) {
            ++ulGet;
            *pEnd++ = *pSrc++;
        }
        return ulGet;
    }

    std::string CONTENT_LENGTH("Content-Length");
    std::string TRANSFER_ENCODING("Transfer-Encoding");
    std::string CONTENT_TYPE("Content-Type");
    std::string CHUNKED("chunked");

    const char* ParseHttp(const char *request, unsigned int len, CRequestContext &rc) {
        rc.Initialize();
        if (!request)
            return nullptr;
        return rc.ParseHttpHeader(request, len);
    }
}
