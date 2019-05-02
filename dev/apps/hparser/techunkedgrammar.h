#pragma once

#include "stdafx.h"
#include <boost/spirit/include/classic_lists.hpp>

namespace UHTTP {

    struct CTEChunkedGrammar : public grammar<CTEChunkedGrammar>, private boost::noncopyable {
    public:

        template <typename ScannerT>
        class definition {
            typedef rule<ScannerT> CRule;
        public:

            definition(CTEChunkedGrammar const& self)
            : m_self((CTEChunkedGrammar&) self) {
                CStrAction newHead(boost::bind(&definition::NewHead, this, _1, _2));
                CUInt32Action newLength(boost::bind(&definition::NewLength, this, _1));
                CStrAction newFooter(boost::bind(&definition::NewFooter, this, _1, _2));
                CStrAction newChunked(boost::bind(&definition::NewChunked, this, _1, _2));
                m_Sep = R_CRLF[newHead] >> (hex_p[newLength] - '0') >> R_CRLF;
                m_Rule = *(m_Sep[newChunked]) >> (R_CRLF >> '0' >> R_CRLF >> R_CRLF)[newFooter];
            }

        private:

            void NewChunked(const char* start, const char* end) {
                const char *myEnd = end + m_hv.Header.Length;
                if ((myEnd + 7) <= m_self.m_pScanner->last) {
                    m_self.m_pScanner->first = myEnd;
                    m_hv.Value.Start = end;
                    m_hv.Value.Length = m_hv.Header.Length;
                    m_self.m_pSub->MemoryBuffer << m_hv;
                } else {
                    //stop parsing here because no enough data for parsing
                    m_self.m_pScanner->first = m_self.m_pScanner->last;
                }
            }

            void NewFooter(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                m_self.m_pSub->ParseStatus = psComplete;
            }

            void NewLength(unsigned int len) {
                m_hv.Header.Length = len;
            }

            void NewHead(const char* start, const char* end) {
                m_hv.Header.Start = start;
            }

        public:

            CRule const& start() {
                ::memset(&m_hv, 0, sizeof (m_hv));
                return m_Rule;
            }

        private:
            CRule m_Sep;
            CRule m_Rule;
            CTEChunkedGrammar &m_self;
            CHeaderValue m_hv;
        };

    public:
        static bool Parse(const char *subdata, unsigned int len, CSubHeaderValue &sub, parse_info<> &pi);

    public:
        CSubHeaderValue *m_pSub;
        parse_info<> *m_pPi;
        CScanner *m_pScanner;

    private:
        static std::vector<CTEChunkedGrammar *> m_vTEChunkedGrammar;
        static SPA::CUCriticalSection m_cs;
    };
}



