#pragma once

#include "../core_shared/pinc/hpdefines.h"
#include "httpgrammar.h"

namespace UHTTP {

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART

    struct U_MODULE_HIDDEN CMultipartGrammar : public grammar<CMultipartGrammar> {
    public:

        template <typename ScannerT>
        class definition {
            typedef rule<ScannerT> CRule;
        public:

            definition(CMultipartGrammar const& self)
            : m_self((CMultipartGrammar&) self) {
                CStrAction newHead(std::bind(&definition::NewHead, this, std::placeholders::_1, std::placeholders::_2));
                CStrAction newValue(std::bind(&definition::NewValue, this, std::placeholders::_1, std::placeholders::_2));
                CStrAction endHeader(std::bind(&definition::EndHeader, this, std::placeholders::_1, std::placeholders::_2));

                ::memset(&m_hv, 0, sizeof (m_hv));

                MPHeader = str_p("--") >> str_p(m_self.m_strBoundary) >> R_CRLF;
                SubHeaders = R_HEADER[newHead] >> *space_p >> ':' >> *space_p >> R_VALUE[newValue] >> R_CRLF;
                m_Rule = MPHeader >> *SubHeaders >> R_CRLF[endHeader];
            }

        private:
            definition(const definition &def);
            definition& operator=(const definition &def);

            void EndHeader(const char* start, const char* end) {
                m_self.m_pSub->ParseStatus = psHeaders;
                m_self.m_pPi->stop = end;
                m_self.m_pPi->full = true;
                m_self.m_pPi->hit = true;
            }

            void NewValue(const char* start, const char* end) {
                SPA::CUQueue &q = *m_self.m_pSub->MHeaders;
                unsigned int len = (unsigned int) (end - start);
                m_hv.Value.Length = len;
                ++len; //add extra for setting nullptr
                q.Push(start, len);
                m_hv.Value.Start = (const char*) q.GetBuffer(q.GetSize() - len);
                m_self.m_pSub->MemoryBuffer << m_hv;
                m_self.m_pPi->full = true;
                m_self.m_pPi->stop = end;
            }

            void NewHead(const char* start, const char* end) {
                SPA::CUQueue &q = *m_self.m_pSub->MHeaders;
                unsigned int len = (unsigned int) (end - start);
                m_hv.Header.Length = len;
                ++len; //add extra for setting nullptr
                q.Push(start, len);
                m_hv.Header.Start = (const char*) q.GetBuffer(q.GetSize() - len);
                m_self.m_pPi->full = true;
                m_self.m_pPi->stop = end;
            }

        public:

            CRule const& start() {
                return m_Rule;
            }

        private:
            CRule MPHeader, SubHeaders;
            CRule m_Rule;
            CMultipartGrammar &m_self;
            CHeaderValue m_hv;
        };

    public:
        static bool Parse(const char *subdata, unsigned int len, const char *boundary, CMultiplaxContext &sub, parse_info<> &pi);

    public:
        const char *m_strBoundary;
        CMultiplaxContext *m_pSub;
        parse_info<> *m_pPi;
        CScanner *m_pScanner;
    };
#endif
}
