#pragma once

#include "stdafx.h"
#include <boost/spirit/include/classic_lists.hpp>

namespace UHTTP {

    struct CMultipartGrammar : public grammar<CMultipartGrammar>, private boost::noncopyable {
    public:

        template <typename ScannerT>
        class definition {
            typedef rule<ScannerT> CRule;
        public:

            definition(CMultipartGrammar const& self)
            : m_self((CMultipartGrammar&) self) {
                CStrAction newContent(boost::bind(&definition::NewContent, this, _1, _2));
                CStrAction newHead(boost::bind(&definition::NewHead, this, _1, _2));
                CStrAction newValue(boost::bind(&definition::NewValue, this, _1, _2));
                CStrAction newFooter(boost::bind(&definition::NewFooter, this, _1, _2));
                CStrAction newBlock(boost::bind(&definition::NewBlock, this, _1, _2));
				CStrAction newCMContent(boost::bind(&definition::NewCMContent, this, _1, _2));

                m_nBLen = (unsigned int) ::strlen(m_self.m_strBoundary) + 6; // for footer \r\n, -- and --;

                m_Boundary = str_p(m_self.m_strBoundary);
                BHeader = str_p("--") >> (m_Boundary - str_p("--"));
                SubHeaders = R_CRLF >> ((R_HEADER[newHead] >> *space_p >> ':' >> *space_p >> R_VALUE[newValue]) % R_CRLF);
                BRooter = BHeader >> str_p("--") >> R_CRLF;
				Content = (*(anychar_p - (R_CRLF >> BHeader)))[newContent];
				Block = ((BHeader >> !SubHeaders >> R_HEADER_END)[newCMContent] >> Content >> R_CRLF)[newBlock];
                m_Rule = +Block >> BRooter[newFooter];
            }

        private:
            void NewBlock(const char* start, const char* end) {
                m_self.m_pSub->ParseStatus = psBlock;
				m_self.m_pPi->hit = true;
            }

			void NewCMContent(const char* start, const char* end) {
				if(m_self.m_pSub->ParseStatus < psHeaders)
					m_self.m_pSub->ParseStatus = psHeaders;
                m_self.m_pSub->LastMCHeader = end;
            }

            void NewFooter(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
				m_self.m_pPi->full = true;
                m_self.m_pSub->ParseStatus = psComplete;
            }

            void NewValue(const char* start, const char* end) {
                MB::CUQueue &q = *m_self.m_pSub->MHeaders;
                unsigned int len = (unsigned int) (end - start);
                m_hv.Value.Length = len;
                ++len; //add extra for setting NULL
                q.Push(start, len);
                m_hv.Value.Start = (const char*) q.GetBuffer(q.GetSize() - len);
                m_self.m_pSub->MemoryBuffer << m_hv;
            }

            void NewHead(const char* start, const char* end) {
                MB::CUQueue &q = *m_self.m_pSub->MHeaders;
                unsigned int len = (unsigned int) (end - start);
                m_hv.Header.Length = len;
                ++len; //add extra for setting NULL
                q.Push(start, len);
                m_hv.Header.Start = (const char*) q.GetBuffer(q.GetSize() - len);
            }

            void NewContent(const char* start, const char* end) {
                const char *myEnd = end + m_nBLen;
                if (m_self.m_pScanner->last >= myEnd) {
					MB::CUQueue &q = *m_self.m_pSub->MHeaders;
					unsigned int len = (unsigned int) (end - start);
					m_hv.Header.Length = 0;
					m_hv.Value.Length = len;
					q.Push(start, len);
                    m_hv.Header.Start = (const char*) q.GetBuffer(q.GetSize() - len);
                    m_hv.Value.Start =  m_hv.Header.Start;
					++m_hv.Header.SC0;
                    m_self.m_pSub->MemoryBuffer << m_hv;
                } else {
                    //stop parsing here because no enough data for parsing
                    m_self.m_pScanner->first = m_self.m_pScanner->last;
                }
            }

        public:

            CRule const& start() {
                ::memset(&m_hv, 0, sizeof (m_hv));
                return m_Rule;
            }

        private:
            CRule m_Boundary, BHeader, BRooter, SubHeaders, Block, Content;
            CRule m_Rule;
            CMultipartGrammar &m_self;
            CHeaderValue m_hv;
            unsigned int m_nBLen;
        };

    public:
        static bool Parse(const char *subdata, unsigned int len, const char *boundary, CMultiplaxContext &sub, parse_info<> &pi);

    public:
        const char *m_strBoundary;
        CMultiplaxContext *m_pSub;
        parse_info<> *m_pPi;
        CScanner *m_pScanner;
    };
}



