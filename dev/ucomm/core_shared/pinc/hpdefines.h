
#pragma once

#include <algorithm>
#include <functional>
#include <boost/unordered_map.hpp> //MS unordered_map implementation has performance problem
#include "../../include/membuffer.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "../../include/userver.h"

namespace UHTTP {

    extern std::string CONTENT_LENGTH;
    extern std::string TRANSFER_ENCODING;
    extern std::string CONTENT_TYPE;
    extern std::string CHUNKED;

	static bool iequals(const char *s0, const char *s1) {
#ifdef WIN32_64
		return (0 == ::_stricmp(s0, s1));
#else
		return (0 == ::strcasecmp(s0, s1));
#endif
	}
	
	static bool iequals(const char *s0, const char *s1, size_t n) {
#ifdef WIN32_64
		return (0 == ::_strnicmp(s0, s1, n));
#else
		return (0 == ::strncasecmp(s0, s1, n));
#endif
	}

	static bool iequals(const wchar_t *s0, const wchar_t *s1) {
#ifdef WIN32_64
		return (0 == ::_wcsicmp(s0, s1));
#else
		return (0 == ::wcscasecmp(s0, s1));
#endif
	}

	static bool iequals(const wchar_t *s0, const wchar_t *s1, size_t n) {
#ifdef WIN32_64
		return (0 == ::_wcsnicmp(s0, s1, n));
#else
		return (0 == ::wcsncasecmp(s0, s1, n));
#endif
	}

    unsigned int UriDecode(const unsigned char *strIn, unsigned int nLenIn, unsigned char *strOut);

    typedef SPA::CScopeUQueueEx<SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE / 2, SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE / 4 > CHttpHeaderScopeUQueue;

    template<class TChar>
    struct ci_equal : public std::binary_function<std::basic_string<TChar>, std::basic_string<TChar>, bool> {
        typedef std::basic_string<TChar> string;

        inline bool operator() (const string &s1, const string & s2) const {
            size_t len1 = s1.size();
            size_t len2 = s2.size();
            if (len2 != len1)
                return false;
            for (size_t n = 0; n < len1; ++n) {
                if (::tolower(s1[n]) != ::tolower(s2[n]))
                    return false;
            }
            return true;
        }
    };

    /*
    template<class TString>
    struct ci_hash : public std::hash<TString> {

            size_t operator()(const TString & _Keyval) const {
                    size_t _Val = 2166136261U;
                    size_t _First = 0;
                    size_t _Last = _Keyval.size();
                    size_t _Stride = 1 + _Last / 10;
                    for (; _First < _Last; _First += _Stride)
                            _Val = 16777619U * _Val ^ (size_t)::tolower(_Keyval[_First]);
                    return _Val;
            }
    };*/

    template<class TString>
    struct ci_hash : public std::unary_function<TString, size_t> {

        inline size_t operator()(const TString & x) const {
            size_t seed = 0;
            for (auto it = x.cbegin(), end = x.cend(); it != end; ++it) {
                boost::hash_combine(seed, ::tolower(*it));
            }
            return seed;
        }
    };

    typedef boost::unordered_map<std::string, std::string, ci_hash<std::string>, ci_equal<char> > StringMap;
    typedef StringMap::iterator StringMapIter;
    typedef StringMap::const_iterator StringMapConstIter;
    typedef StringMap StringMapA;
    typedef StringMapA::iterator StringMapIterA;
    typedef StringMapA::const_iterator StringMapConstIterA;

    typedef boost::unordered_map<std::wstring, std::wstring, ci_hash<std::wstring>, ci_equal<wchar_t> > StringMapW;
    typedef StringMapW::iterator StringMapIterW;
    typedef StringMapW::const_iterator StringMapConstIterW;

    enum tagParseStatus {
        psFailed = -1,
        psInitial = 0,
        psMethod = 1,
        psVersion = 2,
        psUrl = 3,
        psTopLine = 4,
        psHeader = 5,
        psHeaders = 6,
        psPartialContent = 7,
        psPartialBlock = 8,
        psBlock = 9,
        psComplete = 10,
    };

    enum tagWSOpCode {
        ocMsgContinuation = 0,
        ocTextMsg = 1,
        ocBinaryMsg = 2,
        ocConnectionClose = 8,
        ocPing = 9,
        ocPong = 10
    };

    struct CHttpUnit {
        const char *Start;
        unsigned int Length;
    };

    struct CHttpParameter {
        const char *Param;
        const char *Value;
    };

    struct CHeaderValue {
        CHttpUnit Header;
        CHttpUnit Value;
    };

    //http://tools.ietf.org/html/rfc6455#section-5.2
    //http://www.altdevblogaday.com/2012/01/23/writing-your-own-websocket-server/
    //https://github.com/zaphoyd/websocketpp/blob/master/src/websocket_frame.hpp
    //Op-code	Meaning
    //0x0	Message continuation [continuation]
    //0x1	Text message [non-control]
    //0x2	Binary message [non-control]
    //0x8	Connection Close [control]
    //0x9	Ping [control]
    //0xA	Pong [control]

    class CWebSocketMsg {
    public:
        static const unsigned int MASK_SIZE = sizeof (int);
        static const SPA::UINT64 MAX_INT64_LEN = 0x7FFFFFFFFFFFFFFF;
        static const unsigned short MAX_INT16_LEN = 0xFFFF;
        static const unsigned char MAX_INT8_LEN = 125;
        static const unsigned char UINT16_INDICATOR = 126;
        static const unsigned char UINT64_INDICATOR = 127;
        static const unsigned char BITS_OPCODE = 0x0F;
        static const unsigned char BIT_RSV3 = 0x10;
        static const unsigned char BIT_RSV2 = 0x20;
        static const unsigned char BIT_RSV1 = 0x40;
        static const unsigned char BIT_FIN = 0x80;
        static const unsigned char BITS_PAYLOAD = 0x7F;
        static const unsigned char BIT_MASK = 0x80;

        CWebSocketMsg()
        : Top0(0), Top1(0), ParseStatus(psInitial), Content(*suContent) {
            ::memset(Mask, 0, sizeof (Mask));
        }

    public:

        void Initialize() {
            Top0 = 0;
            Top1 = 0;
            ::memset(Mask, 0, sizeof (Mask));
            ParseStatus = psInitial;
            Content.SetSize(0);
        }

        inline bool IsMasked() {
            return ((Top1 & BIT_MASK) == BIT_MASK);
        }

        inline bool IsFin() {
            return ((Top0 & BIT_FIN) == BIT_FIN);
        }

        inline tagWSOpCode GetOpCode() {
            return (tagWSOpCode) (Top0 & BITS_OPCODE);
        }

        inline unsigned char GetLen() {
            return (Top1 & BITS_PAYLOAD);
        }

    private:
        CWebSocketMsg(const CWebSocketMsg &wsm);
        CWebSocketMsg& operator=(const CWebSocketMsg &wsm);
        SPA::CScopeUQueue suContent;

    public:
        unsigned char Top0;
        unsigned char Top1;
        unsigned char Mask[MASK_SIZE];
        tagParseStatus ParseStatus;
        SPA::CUQueue &Content;
    };

    //Content length unknown
    const SPA::UINT64 CONTENT_LEN_UNKNOWN = ((SPA::UINT64)(-1));

    class CSubHeaderValue {
    private:
        CHttpHeaderScopeUQueue su;

        /// Copy constructor disabled
        CSubHeaderValue(const CSubHeaderValue &hv);

        /// Assignment operator disabled
        CSubHeaderValue& operator=(const CSubHeaderValue &hv);

    public:

        CSubHeaderValue()
        : ParseStatus(psInitial), MemoryBuffer(*su) {
            if (MemoryBuffer.GetMaxSize() > SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE)
                MemoryBuffer.ReallocBuffer(SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE / 2);
        }

        /**
         * Get a pointer to an array of header/value structures
         * @param size A reference to integer for receiving the size of the array of header/value structures
         * @return Return a pointer to an array of header/value structures. Note it will never be a nullptr but size could be 0
         */
        inline const CHeaderValue* GetHeaderValue(unsigned int &size) const {
            size = MemoryBuffer.GetSize() / sizeof (CHeaderValue);
            return (const CHeaderValue*) MemoryBuffer.GetBuffer();
        }

        inline void Initialize() {
            ParseStatus = psInitial;
            MemoryBuffer.SetSize(0);
        }

        const char* ParseHeadValue(const char *line, unsigned int len = -1) {
            if (line == nullptr || len == 0)
                return line;
            if (len == -1)
                len = (unsigned int) ::strlen(line);
            const char *raw = line;
            const char *end = line + len;
            CHeaderValue hv;
            ::memset(&hv, 0, sizeof (hv));
            do {
                const char *hh = SkipBlank(line, len);
                const char *colon = ::strchr(hh, ':');
                if (colon == hh) {
                    ParseStatus = psFailed; //no header
                    return line;
                }
                if (nullptr == colon)
                    return line; //header not completed yet
                const char *he = BackBlank(colon - 1, (unsigned int) (colon - hh + 1));
                hv.Header.Start = hh;
                hv.Header.Length = (unsigned int) (he - hh + 1);
                const char *crlf = ::strstr(++colon, "\r\n");
                if (crlf == nullptr)
                    return line; //value not completed yet
                len -= (unsigned int) (colon - line + 2);
                hh = SkipBlank(colon, len);

                /*
                if (hh == crlf) {
                        ParseStatus = psFailed; //no value
                        return line;
                }*/

                he = BackBlank(crlf - 1, (unsigned int) (crlf - hh));
                hv.Value.Start = hh;
                hv.Value.Length = (unsigned int) (he - hh + 1);
                ParseStatus = psHeader;
                MemoryBuffer << hv;
                line = crlf + 2;
                len = (unsigned int) (end - line);
            } while (len > 2); //CRLF
            return end - len;
        }

    private:

        const char* SkipBlank(const char *line, unsigned int len) {
            while (len && ::isspace(*line)) {
                ++line;
                --len;
            }
            return line;
        }

        const char* BackBlank(const char *line, unsigned int len) {
            while (len && ::isspace(*line)) {
                --line;
                --len;
            }
            return line;
        }

    public:
        tagParseStatus ParseStatus;
        SPA::CUQueue &MemoryBuffer;
    };

    typedef SPA::CScopeUQueueEx<SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE, SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CHttpSubRequestScopeUQueue;

    /** 
     * A structure for describing HTTP request parsing result
     */
    class CRequestContext : public CSubHeaderValue {
    private:
        /// Copy constructor disabled
        CRequestContext(const CRequestContext &rc);

        /// Assignment operator disabled
        CRequestContext& operator=(const CRequestContext &rc);

        static const char* MatchMethod(const char *input, unsigned int size, const char *method, unsigned len, tagParseStatus &ps) {
            ps = psInitial;
            if (input == nullptr)
                size = 0;
            unsigned int n, count = (size > len) ? len : size;
            for (n = 0; n < count; ++n) {
                if (method[n] != input[n]) {
                    ps = psFailed;
                    return input + n;
                }
            }
            if (count == len)
                ps = psMethod;
            return input + n;
        }

        static const char* ParseHttpMethod(const char *method, unsigned len, tagParseStatus &ps, SPA::ServerSide::tagHttpMethod &hm) {
            const char *end;
            hm = SPA::ServerSide::tagHttpMethod::hmUnknown;
            do {
                end = MatchMethod(method, len, "GET ", 4, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmGet;
                    break;
                } else if (ps == psInitial)
                    break;

                end = MatchMethod(method, len, "POST ", 5, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmPost;
                    break;
                } else if (ps == psInitial)
                    break;

                end = MatchMethod(method, len, "HEAD ", 5, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmHead;
                    break;
                } else if (ps == psInitial)
                    break;

                end = MatchMethod(method, len, "TRACE ", 6, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmTrace;
                    break;
                } else if (ps == psInitial)
                    break;

                end = MatchMethod(method, len, "OPTIONS ", 8, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmOptions;
                    break;
                } else if (ps == psInitial)
                    break;

                end = MatchMethod(method, len, "PUT ", 4, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmPut;
                    break;
                } else if (ps == psInitial)
                    break;

                end = MatchMethod(method, len, "DELETE ", 7, ps);
                if (ps == psMethod) {
                    hm = SPA::ServerSide::tagHttpMethod::hmDelete;
                    break;
                } else if (ps == psInitial)
                    break;
            } while (false);
            return end;
        }

        const char* ParseHttpTopLine(const char *url, unsigned len) {
            if (url == nullptr || len == 0) {
                return url;
            }

            if (*url != '/') {
                ParseStatus = psFailed;
                return url;
            }

            const char *end = ::strstr(url, "\r\n");
            if (end == nullptr) //first line no ended yet
            {
                return url;
            }

            const char *http = ::strstr(url, " HTTP/1.1");
            do {
                if (http != nullptr) {
                    Version = 1.1;
                    break;
                }
                http = ::strstr(url, " HTTP/1.0");
                if (http != nullptr) {
                    Version = 1;
                    break;
                }
                http = ::strstr(url, " HTTP/1.2");
                if (http != nullptr)
                    Version = 1.2;
            } while (false);

            if (http == nullptr || http > end || (end - http) != 9) {
                ParseStatus = psFailed;
                return url;
            }

            unsigned int url_len = (unsigned int) (http - url);

            DealWithUrl(url, url_len);

            ParseStatus = psTopLine;

            return end + 2; //'\r\n'
        }

        void DealWithUrl(const char *url, unsigned int len) {
            const char *end = url + len;
            SPA::CScopeUQueue qUrl(MY_OPERATION_SYSTEM, SPA::IsBigEndian(), len + 1);
            unsigned int res = UriDecode((const unsigned char *) url, len, (unsigned char *) qUrl->GetBuffer());
            while (res != len) {
                ::memcpy((char*) url, qUrl->GetBuffer(), res);
                len = res;
                res = UriDecode((const unsigned char *) url, len, (unsigned char *) qUrl->GetBuffer());
            }
            Url.Start = url;
            Url.Length = len;
            ParseStatus = psUrl;
            const char *str = strchr(url, '?');
            if (str != nullptr && str <= end) {
                char *q = (char*) str;
                *q = 0;
                Url.Length = (unsigned int) (str - Url.Start);
                Params.Start = str + 1;
                Params.Length = len - Url.Length - 1; //(unsigned int) (end - Params.Start);
            }
        }

        void DealWithParticularHeader(CHeaderValue *hv) {
            const char *c = hv->Header.Start;
			unsigned int len = hv->Header.Length;
            if ((*c == 'C' || *c == 'c') && len == 14) //
            {
                if (iequals(CONTENT_LENGTH.c_str(), c, len)) {
                    try {
                        ContentLen = boost::lexical_cast<SPA::UINT64 > (hv->Value.Start);
                    } catch (...) {
                    }
                }
            } else if ((*c == 'C' || *c == 'c') && len == 12) //Content-Type
            {
                bool bQuote = false;
                const char *be = nullptr;
                const char *b = nullptr;
                std::string v(hv->Value.Start, hv->Value.Length);
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
                if (iequals(CONTENT_TYPE.c_str(), c, len) && be) {
                    unsigned int offset = (unsigned int) (b - m);
                    hv->Value.Start += (offset + 9 + bQuote);
                    hv->Value.Length = (unsigned int) (be - b) - 9 - bQuote;
                    if (memcmp(v.data(), "multipart/mixed", 15) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmMixed;
                    else if (memcmp(v.data(), "multipart/digest", 16) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmDigest;
                    else if (memcmp(v.data(), "multipart/form-data", 19) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmFormData;
                    else if (memcmp(v.data(), "multipart/alternative", 20) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmAlternative;
                    else if (memcmp(v.data(), "multipart/parallel", 18) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmParallel;
                    else if (memcmp(v.data(), "multipart/byteranges", 20) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmByteRanges;
                    else if (memcmp(v.data(), "multipart/report", 16) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmReport;
                    else if (memcmp(v.data(), "multipart/signed", 16) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmSigned;
                    else if (memcmp(v.data(), "multipart/related", 17) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmRelated;
                    else if (memcmp(v.data(), "multipart/encrypted", 19) == 0)
                        CM = SPA::ServerSide::tagContentMultiplax::cmEncrypted;
                    else {

                    }
                }
            } else if ((*c == 'T' || *c == 't') && len == 17) //Transfer-Encoding
            {
                if (iequals(TRANSFER_ENCODING.c_str(), c, len)) {
                    if (iequals(hv->Value.Start, "chunked"))
                        TE = SPA::ServerSide::tagTransferEncoding::teChunked;
                    else if (iequals(hv->Value.Start, "gzip"))
                        TE = SPA::ServerSide::tagTransferEncoding::teGZip;
                    else if (iequals(hv->Value.Start, "compress"))
                        TE = SPA::ServerSide::tagTransferEncoding::teCompress;
                    else if (iequals(hv->Value.Start, "deflate"))
                        TE = SPA::ServerSide::tagTransferEncoding::teDeflate;
                    else if (iequals(hv->Value.Start, "identity"))
                        TE = SPA::ServerSide::tagTransferEncoding::teIdentity;
                }
            }
        }

    public:

        CRequestContext()
        : Method(SPA::ServerSide::tagHttpMethod::hmUnknown),
        Version(0),
        Content(nullptr),
        ContentLen(CONTENT_LEN_UNKNOWN),
        TE(SPA::ServerSide::tagTransferEncoding::teUnknown),
        CM(SPA::ServerSide::tagContentMultiplax::cmUnknown) {
            ::memset(&Url, 0, sizeof (Url));
            ::memset(&Params, 0, sizeof (Params));
        }

        const char *ParseHttpHeader(const char *start, unsigned int len = -1) {
            const char *end = start;
            if (start == nullptr || len == 0)
                return start;
            if (len == -1)
                len = (unsigned int) ::strlen(start);
            do {
                end = ParseHttpMethod(start, len, ParseStatus, Method);
                if (ParseStatus != psMethod) //method not available
                    break;
                len -= (unsigned int) (end - start);
                start = end;
                end = ParseHttpTopLine(start, len);
                if (ParseStatus != psTopLine) //method not available
                    break;
                len -= (unsigned int) (end - start);
                start = end;
                end = ParseHeadValue(start, len);
                if (ParseStatus != psHeader)
                    break;
                len -= (unsigned int) (end - start);
                start = end;
                if (len >= 2) {
                    if (*start == '\r' && start[1] == '\n') {
                        ParseStatus = psHeaders;
                        end += 2;
                    } else {
                        ParseStatus = psFailed;
                    }
                }
            } while (false);

            if (ParseStatus >= psUrl) {
                char *c = (char*) Url.Start;
                c[Url.Length] = (char) 0;
            }

            if (ParseStatus >= psUrl && Params.Length) {
                char *q = (char*) (Params.Start + Params.Length);
                *q = 0;
            }

            if (ParseStatus >= psHeader) {
                unsigned n;
                char *c;
                CHeaderValue *hv = (CHeaderValue *) GetHeaderValue(len);
                for (n = 0; n < len; ++n, ++hv) {
                    c = (char*) hv->Header.Start;
                    c[hv->Header.Length] = (char) 0;
                    c = (char*) hv->Value.Start;
                    c[hv->Value.Length] = (char) 0;
                    DealWithParticularHeader(hv);
                }
            }
            return end;
        }

        /**
         * Reset internal members to initial states
         */
        inline void Initialize() {
            CSubHeaderValue::Initialize();
            Method = SPA::ServerSide::tagHttpMethod::hmUnknown;
            ::memset(&Url, 0, sizeof (Url));
            ::memset(&Params, 0, sizeof (Params));
            Version = 0;
            Content = nullptr;
            ContentLen = CONTENT_LEN_UNKNOWN;
            TE = SPA::ServerSide::tagTransferEncoding::teUnknown;
            CM = SPA::ServerSide::tagContentMultiplax::cmUnknown;
        }

        inline const CHeaderValue* SeekHeaderValue(const char *header) const {
            unsigned int n;
            unsigned int size;
            if (header == nullptr)
                return nullptr;
            const CHeaderValue *p = GetHeaderValue(size);
            for (n = 0; n < size; ++n, ++p) {
                if (iequals(header, p->Header.Start))
                    return p;
            }
            return nullptr;
        }

        inline const CHeaderValue* SeekMultipart() const {
            unsigned int n;
            unsigned int size;
            if (CM == SPA::ServerSide::tagContentMultiplax::cmUnknown)
                return nullptr;
            const CHeaderValue *p = GetHeaderValue(size);
            for (n = 0; n < size; ++n, ++p) {
                char c = p->Header.Start[0];
                if (c != 'C' && c != 'c')
                    continue;
                if (iequals(CONTENT_TYPE.c_str(), p->Header.Start))
                    return p;
            }
            return nullptr;
        }

        SPA::ServerSide::tagHttpMethod Method;
        CHttpUnit Url;
        CHttpUnit Params;
        double Version; /**< Http version like 1.0 or 1.1 */
        const char* Content; /**< A pointer to content string. It will be nullptr if HTTP parsing fails */
        SPA::UINT64 ContentLen; /**< A valid length for HTTP content, 
								* Unknown length (-1), or other negative integers as described at the above
								*/
        SPA::ServerSide::tagTransferEncoding TE; /**< teUnknown if no Transfer-Encoding is set */
        SPA::ServerSide::tagContentMultiplax CM; /**< cmUnknown if no multiplax is set */

    };

    /**
     * Parse a HTTP request and output a request context
     * @param request A pointer to a HTTP request string
     * @param len A value for HTTP request string length
     * @param rc A reference to a HTTP request context for receiving data
     * @return Return a pointer to parsing ending position. If the method returns nullptr, parsing fails
     */
    const char* ParseHttp(const char *request, unsigned int len, CRequestContext &rc);

    class CChunkedContext : public CSubHeaderValue {
        CChunkedContext(const CChunkedContext &cc);
        CChunkedContext& operator=(const CChunkedContext &cc);

    public:
        CHttpSubRequestScopeUQueue MHeaders;

        /**
         * Create an instance
         */
        CChunkedContext() {
        }

        /**
         * Reset internal members to initial states
         */
        inline void Initialize() {
            CSubHeaderValue::Initialize();
            MHeaders->SetSize(0);
        }
    };

    class CMultiplaxContext : public CChunkedContext {
        CMultiplaxContext(const CMultiplaxContext &mc);
        CMultiplaxContext& operator=(const CMultiplaxContext &mc);

    public:

        CMultiplaxContext() : BoundaryLength(0) {
        }

        unsigned int BoundaryLength; //boundary length + 4 (-- and CRLF)

        inline void Initialize() {
            CChunkedContext::Initialize();
            BoundaryLength = 0;
        }
    };

    /**
     * Parse multipart headers for HTTP Content-Type/multipart
     * @param multipart_headers A pointer to a HTTP multipart headers string
     * @param len A value for multipart_headers string length
     * @param boundary A pointer to a multipart boundary string
     * @param sub A reference to a HTTP sub header/value context for receiving data
     * @return Return a pointer to parsing ending position.
     */
    const char* ParseMultipartHeader(const char *multipart_headers, unsigned int len, const char *boundary, CMultiplaxContext &sub);

    /**
     * Parse sub content for HTTP Transfer-Encoding/chunked
     * @param subdata A pointer to a HTTP sub content string. The previous two chars must be '\r\n'
     * @param len A value for HTTP request string length
     * @param sub A reference to a HTTP sub header/value context for receiving data
     * @return Return a pointer to parsing ending position. If the method returns nullptr, parsing fails. 
     * If sub.FullParsing is true, the parsing is completed until the end. 
     * If sub.FullParsing is false, the parsing stops and is not finished yet,
     * and the return result indicates the pasrse stop position.
     */
    const char* ParseChunked(const char *subdata, unsigned int len, CSubHeaderValue &sub);



};

