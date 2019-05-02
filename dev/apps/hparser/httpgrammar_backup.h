#pragma once


#include "stdafx.h"

namespace UHTTP {

    struct CHttpGrammar : public grammar<CHttpGrammar>,
    private boost::noncopyable {
    public:

        template <typename ScannerT>
        class definition {
            typedef rule<ScannerT> CRule;
        public:

            definition(CHttpGrammar const& self)
            : m_self((CHttpGrammar&) self) {
                CStrAction newUrl(boost::bind(&definition::NewUrl, this, _1, _2));
                CDoubleAction newDouble(boost::bind(&definition::NewDouble, this, _1));
                CStrAction newHead(boost::bind(&definition::NewHead, this, _1, _2));
                CStrAction newValue(boost::bind(&definition::NewValue, this, _1, _2));
                CStrAction newMethod(boost::bind(&definition::NewMethod, this, _1, _2));
                CStrAction newConfirm(boost::bind(&definition::NewConfirm, this, _1, _2));
                CStrAction newFull(boost::bind(&definition::NewFull, this, _1, _2));
                CStrAction newTopLine(boost::bind(&definition::NewTopLine, this, _1, _2));

                HTTP_VERSION = str_p(" HTTP/") >> strict_real_p[newDouble] >> R_CRLF;
                URL = *(anychar_p - HTTP_VERSION);

                m_Rule = ((str_p("GET") | "POST" | "PUT" | "HEAD" | "DELETE" | "TRACE" | "OPTIONS") [newMethod] >>
                        str_p(" /")[newConfirm] >> !URL[newUrl] >> HTTP_VERSION)[newTopLine] >>
                        ((R_HEADER[newHead] >> *space_p >> ':' >> *space_p >> R_VALUE[newValue]) % R_CRLF) >>
                        R_HEADER_END[newFull] >> *anychar_p;

                /*m_Rule = ((str_p("GET") | "POST" | "PUT" | "HEAD" | "DELETE" | "TRACE" | "OPTIONS" | "CONNECT" |
                        "COPY" | "LOCK" | "MKCOL" | "MOVE" | "PROPFIND" | "PROPPATCH" | "UNLOCK" | "REPORT" |
                        "MKACTIVITY" | "CHECKOUT" | "MERGE" | "M-SEARCH" | "NOTIFY" | "SUBSCRIBE" |
                        "UNSUBSCRIBE" | "PATCH") [newMethod] >>
                        str_p(" /")[newConfirm] >> !URL[newUrl] >> HTTP_VERSION)[newTopLine] >>
                        ((R_HEADER[newHead] >> *space_p >> ':' >> *space_p >> R_VALUE[newValue]) % R_CRLF) >>
                        R_HEADER_END[newFull] >> *anychar_p;*/
            }

        private:

            void NewTopLine(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                m_self.m_pRC->ParseStatus = psTopLine;
            }

            void NewFull(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                m_self.m_pRC->ParseStatus = psHeaders;
                m_self.m_pScanner->first = m_self.m_pScanner->last;
                m_self.m_pRC->Content = end;
            }

            void NewConfirm(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                m_self.m_pRC->ParseStatus = psMethod1;
                m_self.m_pRC->Url.Start = end;
                m_self.m_pRC->Url.Length = 0;
            }

            void NewValue(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                m_hv.Value.Start = start;
                m_hv.Value.Length = (unsigned int) (end - start);
                const char *c = m_hv.Header.Start;
                if ((*c == 'C' || *c == 'c') && m_hv.Header.Length == 14) //
                {
                    std::string s(c, c + m_hv.Header.Length);
                    if (boost::iequals(CONTENT_LENGTH, s)) {
                        s.assign(start, end);
                        try {
                            m_self.m_pRC->ContentLen = boost::lexical_cast<MB::U_UINT64 > (s);
                        } catch (...) {
                        }
                    }
                } else if ((*c == 'C' || *c == 'c') && m_hv.Header.Length == 12) //Content-Type
                {
                    bool bQuote = false;
                    const char *be = NULL;
                    const char *b = NULL;
                    std::string v(start, end);
                    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
                    const char *m = ::strstr(v.data(), "multipart/");
                    if (m)
                        b = ::strstr(m, "boundary=");
                    if (b && b[9] == '"')
                        bQuote = true;
                    if (b) {
                        if (bQuote)
                            be = strpbrk(b + 10, "\"");
                        else {
                            be = strpbrk(b + 9, ";");
                            if (!be)
                                be = b + ::strlen(b);
                        }
                    }
                    std::string s(c, c + m_hv.Header.Length);
                    if (boost::iequals(CONTENT_TYPE, s) && be) {
                        unsigned int offset = (unsigned int) (b - m);
                        m_hv.Value.Start = start + offset + 9 + bQuote;
                        m_hv.Value.Length = (unsigned int) (be - b) - 9 - bQuote;
                        if (memcmp(v.data(), "multipart/mixed", 15) == 0)
                            m_self.m_pRC->CM = cmMixed;
                        else if (memcmp(v.data(), "multipart/digest", 16) == 0)
                            m_self.m_pRC->CM = cmDigest;
                        else if (memcmp(v.data(), "multipart/form-data", 19) == 0)
                            m_self.m_pRC->CM = cmFormData;
                        else if (memcmp(v.data(), "multipart/alternative", 20) == 0)
                            m_self.m_pRC->CM = cmAlternative;
                        else if (memcmp(v.data(), "multipart/parallel", 18) == 0)
                            m_self.m_pRC->CM = cmParallel;
                        else if (memcmp(v.data(), "multipart/byteranges", 20) == 0)
                            m_self.m_pRC->CM = cmByteRanges;
                        else if (memcmp(v.data(), "multipart/report", 16) == 0)
                            m_self.m_pRC->CM = cmReport;
                        else if (memcmp(v.data(), "multipart/signed", 16) == 0)
                            m_self.m_pRC->CM = cmSigned;
                        else if (memcmp(v.data(), "multipart/related", 17) == 0)
                            m_self.m_pRC->CM = cmRelated;
                        else if (memcmp(v.data(), "multipart/encrypted", 19) == 0)
                            m_self.m_pRC->CM = cmEncrypted;
                        else {
                            m_hv.Value.Start = start;
                        }
                    }
                } else if ((*c == 'T' || *c == 't') && m_hv.Header.Length == 17) //Transfer-Encoding
                {
                    std::string s(c, c + m_hv.Header.Length);
                    if (boost::iequals(TRANSFER_ENCODING, s)) {
                        s.assign(start, end);
                        if (boost::iequals(s, "chunked"))
                            m_self.m_pRC->TE = teChunked;
                        else if (boost::iequals(s, "gzip"))
                            m_self.m_pRC->TE = teGZip;
                        else if (boost::iequals(s, "compress"))
                            m_self.m_pRC->TE = teCompress;
                        else if (boost::iequals(s, "deflate"))
                            m_self.m_pRC->TE = teDeflate;
                        else if (boost::iequals(s, "identity"))
                            m_self.m_pRC->TE = teIdentity;
                    }

                }
                m_self.m_pRC->MemoryBuffer << m_hv;
            }

            void NewMethod(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                m_self.m_pRC->ParseStatus = psMethod0;
                switch (*start) {
                    case 'G':
                        m_self.m_pRC->Method = hmGet;
                        break;
                    case 'P':
                        if (start[1] == 'O')
                            m_self.m_pRC->Method = hmPost;
                        else if (start[1] == 'U')
                            m_self.m_pRC->Method = hmPut;
                        else if (start[1] == 'A')
                            m_self.m_pRC->Method = hmPatch;
                        else if (start[5] == 'I')
                            m_self.m_pRC->Method = hmPropfind;
                        else
                            m_self.m_pRC->Method = hmProppatch;
                        break;
                    case 'C':
                        if (start[1] == 'H')
                            m_self.m_pRC->Method = hmCheckout;
                        else if (start[2] == 'N')
                            m_self.m_pRC->Method = hmConnect;
                        else
                            m_self.m_pRC->Method = hmCopy;
                        break;
                    case 'D':
                        m_self.m_pRC->Method = hmDelete;
                        break;
                    case 'O':
                        m_self.m_pRC->Method = hmOptions;
                        break;
                    case 'T':
                        m_self.m_pRC->Method = hmTrace;
                        break;
                    case 'H':
                        m_self.m_pRC->Method = hmHead;
                        break;
                    case 'L':
                        m_self.m_pRC->Method = hmLock;
                        break;
                    case 'M':
                        if (start[1] == '-')
                            m_self.m_pRC->Method = hmM_search;
                        else if (start[1] == 'E')
                            m_self.m_pRC->Method = hmMerge;
                        else if (start[1] == 'O')
                            m_self.m_pRC->Method = hmMove;
                        else if (start[2] == 'A')
                            m_self.m_pRC->Method = hmMkactivity;
                        else
                            m_self.m_pRC->Method = hmMkcol;
                        break;
                    case 'N':
                        m_self.m_pRC->Method = hmNotify;
                        break;
                    case 'S':
                        m_self.m_pRC->Method = hmSubscribe;
                        break;
                    case 'U':
                        if (start[2] == 'L')
                            m_self.m_pRC->Method = hmUnlock;
                        else
                            m_self.m_pRC->Method = hmUnsubscribe;
                        break;
                    default:
                        m_self.m_pRC->Method = hmUnknown;
                        break;
                }
            }

            void NewUrl(const char* start, const char* end) {
                m_self.m_pPi->stop = end;
                unsigned int len = (unsigned int) (end - start);
                CHttpRequestScopeUQueue qUrl(MB::IsBigEndian(), MY_OPERATION_SYSTEM, len + 1);
                unsigned int res = UriDecode((const unsigned char *) start, len, (unsigned char *) qUrl->GetBuffer());
                while (res != len) {
                    ::memcpy((char*) start, qUrl->GetBuffer(), res);
                    len = res;
                    res = UriDecode((const unsigned char *) start, len, (unsigned char *) qUrl->GetBuffer());
                }
                m_self.m_pRC->Url.Start = start - 1;
                m_self.m_pRC->Url.Length = len + 1;
                m_self.m_pRC->ParseStatus = psUrl;
                const char *str = strchr(start, '?');
                if (str != NULL && str <= end) {
                    char *q = (char*) str;
                    *q = 0;
                    m_self.m_pRC->Url.Length = (unsigned int) (str - m_self.m_pRC->Url.Start);
                    m_self.m_pRC->Params.Start = str + 1;
                    m_self.m_pRC->Params.Length = (unsigned int) (end - m_self.m_pRC->Params.Start);
                }
            }

            void NewHead(const char* start, const char* end) {
                m_hv.Header.Start = start;
                m_hv.Header.Length = (unsigned int) (end - start);
            }

            void NewDouble(double dVersion) {
                m_self.m_pRC->Version = dVersion;
                m_self.m_pRC->ParseStatus = psVersion;
            }

        public:

            CRule const& start() {
                ::memset(&m_hv, 0, sizeof (m_hv));
                return m_Rule;
            }

        private:
            CRule URL;
            CRule HTTP_VERSION;
            CRule m_Rule;
            CRule R_HV;
            CHttpGrammar &m_self;
            CHeaderValue m_hv;
        };

        static bool Parse(const char *request, unsigned int len, CRequestContext &rc, parse_info<> &pi);

    public:
        CRequestContext *m_pRC;
        parse_info<> *m_pPi;
        CScanner *m_pScanner;

    private:
        static std::vector<CHttpGrammar *> m_vHttpGrammar;
        static MB::CUCriticalSection m_cs;
    };


};

