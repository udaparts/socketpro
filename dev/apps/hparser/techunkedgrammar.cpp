#include "stdafx.h"
#include "techunkedgrammar.h"

namespace UHTTP {

    std::vector<CTEChunkedGrammar *> CTEChunkedGrammar::m_vTEChunkedGrammar;
    SPA::CUCriticalSection CTEChunkedGrammar::m_cs;

    bool CTEChunkedGrammar::Parse(const char *subdata, unsigned int len, CSubHeaderValue &sub, parse_info<> &pi) {
        CTEChunkedGrammar *p = NULL;
        subdata -= 2; //backward 2 chars for \r\n
        len += 2;
        m_cs.lock();
        if (m_vTEChunkedGrammar.size()) {
            p = m_vTEChunkedGrammar.back();
            m_vTEChunkedGrammar.pop_back();
        }
        m_cs.unlock();
        if (!p)
            p = new CTEChunkedGrammar;
        p->m_pSub = &sub;
        p->m_pPi = &pi;
        pi.stop = subdata;
        CScanner scan(subdata, subdata + len);
        p->m_pScanner = &scan;
        p->parse(scan);
        m_cs.lock();
        m_vTEChunkedGrammar.push_back(p);
        m_cs.unlock();

        if (sub.ParseStatus != psComplete) {
            unsigned int len;
            const CHeaderValue *pHV = sub.GetHeaderValue(len);
            if (len) {
                --len;
                pi.stop = pHV[len].Value.Start + pHV[len].Value.Length + 2;
            } else
                pi.stop = subdata + 2;
        }
        return true;
    }

    const char* ParseChunked(const char *subdata, unsigned int len, CSubHeaderValue &sub) {
        sub.Initialize();
        if (!subdata || !len)
            return NULL;
        parse_info<> pi;
        bool b = CTEChunkedGrammar::Parse(subdata, len, sub, pi);
        return pi.stop;
    }

}