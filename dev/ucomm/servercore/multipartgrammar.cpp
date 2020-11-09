
#include "multipartgrammar.h"

namespace UHTTP
{

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART

    bool CMultipartGrammar::Parse(const char *subdata, unsigned int len, const char *boundary, CMultiplaxContext &sub, parse_info<> &pi) {
        unsigned int n;
        CMultipartGrammar mg;
        const char *end = subdata + len;
        mg.m_pSub = &sub;
        mg.m_pPi = &pi;
        pi.stop = subdata;
        mg.m_strBoundary = boundary;
        CScanner scan(subdata, subdata + len);
        mg.m_pScanner = &scan;
        mg.parse(scan);
        char *start = (char*) sub.MHeaders->GetBuffer();
        CHeaderValue *hv = (CHeaderValue *) sub.GetHeaderValue(len);
        for (n = 0; n < len; ++n, ++hv) {
            if (!hv->Header.Length) {
                hv->Header.Start = start;
                hv->Value.Start = start;
                start += hv->Value.Length;
                continue;
            }
            hv->Header.Start = start;
            start += hv->Header.Length;
            *start = (char) 0;
            ++start;
            hv->Value.Start = start;
            start += hv->Value.Length;
            *start = (char) 0;
            ++start;
        }

        return true;
    }

    const char* ParseMultipartHeader(const char *subdata, unsigned int len, const char *boundary, CMultiplaxContext & sub) {
        if (!subdata || !boundary || !len)
            return subdata;
        parse_info<> pi;
        bool b = CMultipartGrammar::Parse(subdata, len, boundary, sub, pi);
        pi.length = (unsigned int) (pi.stop - subdata);
        return pi.stop;
    }
#endif
}