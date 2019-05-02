#include "stdafx.h"
#include "multipartgrammar.h"

namespace UHTTP {

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
        if (sub.ParseStatus != psComplete) {
            const CHeaderValue *pHV = sub.GetHeaderValue(len);
            if (len) {
                int n, nLen = (int) len;
                for (n = nLen - 1; n >= 0; --n) {
                    if (pHV[n].Header.Length == 0) {
                        pi.stop = pHV[n].Value.Start + pHV[n].Value.Length + 2;
                        break;
                    } else
                        sub.MemoryBuffer.Pop((unsigned int) (sizeof (CHeaderValue)), (unsigned int) (sizeof (CHeaderValue) * n));
                }
            } else
                pi.stop = subdata;
        }
		char *start = (char*)sub.MHeaders->GetBuffer();
        CHeaderValue *hv = (CHeaderValue *) sub.GetHeaderValue(len);
        for (n = 0; n < len; ++n, ++hv) {
            if (hv->Header.Length == 0)
			{
				hv->Header.Start = start;
				hv->Value.Start = start;
				start += hv->Value.Length;
			}
			else
			{
				hv->Header.Start = start;
				start += hv->Header.Length;
				*start = (char)0;
				++start;
				hv->Value.Start = start;
				start += hv->Value.Length;
				*start = (char)0;
				++start;
			}
        }

        return true;
    }

    const char* ParseMultipart(const char *subdata, unsigned int len, const char *boundary, CMultiplaxContext &sub) {
        sub.Initialize();
        if (!subdata || !boundary || !len)
            return subdata;
        parse_info<> pi;
        bool b = CMultipartGrammar::Parse(subdata, len, boundary, sub, pi);
        return pi.stop;
    }

}