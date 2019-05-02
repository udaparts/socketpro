#include "stdafx.h"
#include "httpcontext.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include "base64.h"
#include "SHA1.h"
#include <assert.h>


namespace UHTTP {

    std::string CHttpContext::AppName;
    std::map<int, std::string> CHttpContext::ErrorCodeMap;
    StringMapA CHttpContext::BuiltinFileContentTypeMap;
    CHttpContext::SetMap CHttpContext::sm;
    MB::CUCriticalSection CHttpContext::m_cs;
    std::queue<CHttpContext*> CHttpContext::m_qHC;

    CHttpContext::SetMap::SetMap() {
        CHttpContext::ErrorCodeMap[100] = "Continue";
        CHttpContext::ErrorCodeMap[101] = "Switching Protocols";
        CHttpContext::ErrorCodeMap[102] = "Processing";
        CHttpContext::ErrorCodeMap[103] = "Checkpoint";
        CHttpContext::ErrorCodeMap[122] = "Request-URI too long";

        CHttpContext::ErrorCodeMap[200] = "OK";
        CHttpContext::ErrorCodeMap[201] = "Created";
        CHttpContext::ErrorCodeMap[202] = "Accepted";
        CHttpContext::ErrorCodeMap[203] = "Non-Authoritative Information";
        CHttpContext::ErrorCodeMap[204] = "No Content";
        CHttpContext::ErrorCodeMap[205] = "Reset Content";
        CHttpContext::ErrorCodeMap[206] = "Partial Content";
        CHttpContext::ErrorCodeMap[207] = "Multi-Status";
        CHttpContext::ErrorCodeMap[208] = "Already Reporte";
        CHttpContext::ErrorCodeMap[226] = "IM Used";

        CHttpContext::ErrorCodeMap[300] = "Multiple Choices";
        CHttpContext::ErrorCodeMap[301] = "Moved Permanently";
        CHttpContext::ErrorCodeMap[302] = "Found";
        CHttpContext::ErrorCodeMap[303] = "See Other";
        CHttpContext::ErrorCodeMap[304] = "Not Modified";
        CHttpContext::ErrorCodeMap[305] = "Use Proxy";
        CHttpContext::ErrorCodeMap[306] = "Switch Proxy";
        CHttpContext::ErrorCodeMap[307] = "Temporary Redirect";
        CHttpContext::ErrorCodeMap[308] = "Resume Incomplete";

        CHttpContext::ErrorCodeMap[400] = "Bad Request";
        CHttpContext::ErrorCodeMap[401] = "Unauthorized";
        CHttpContext::ErrorCodeMap[402] = "Payment Required";
        CHttpContext::ErrorCodeMap[403] = "Forbidden";
        CHttpContext::ErrorCodeMap[404] = "Not Found";
        CHttpContext::ErrorCodeMap[405] = "Method Not Allowed";
        CHttpContext::ErrorCodeMap[406] = "Not Acceptable";
        CHttpContext::ErrorCodeMap[407] = "Proxy Authentication Required";
        CHttpContext::ErrorCodeMap[408] = "Request Timeout";
        CHttpContext::ErrorCodeMap[409] = "Conflict";
        CHttpContext::ErrorCodeMap[410] = "Gone";
        CHttpContext::ErrorCodeMap[411] = "Length Required";
        CHttpContext::ErrorCodeMap[412] = "Precondition Failed";
        CHttpContext::ErrorCodeMap[413] = "Request Entity Too Large";
        CHttpContext::ErrorCodeMap[414] = "Request-URI Too Long";
        CHttpContext::ErrorCodeMap[415] = "Unsupported Media Type";
        CHttpContext::ErrorCodeMap[416] = "Requested Range Not Satisfiable";
        CHttpContext::ErrorCodeMap[417] = "Expectation Failed";
        CHttpContext::ErrorCodeMap[418] = "I'm a teapot";
        CHttpContext::ErrorCodeMap[422] = "Unprocessable Entity";
        CHttpContext::ErrorCodeMap[423] = "Locked";
        CHttpContext::ErrorCodeMap[424] = "Failed Dependency";
        CHttpContext::ErrorCodeMap[425] = "Unordered Collection";
        CHttpContext::ErrorCodeMap[426] = "Upgrade Required";
        CHttpContext::ErrorCodeMap[428] = "Precondition Required";
        CHttpContext::ErrorCodeMap[429] = "Too Many Requests";
        CHttpContext::ErrorCodeMap[431] = "Request Header Fields Too Large";
        CHttpContext::ErrorCodeMap[444] = "No Response";
        CHttpContext::ErrorCodeMap[449] = "Retry With";
        CHttpContext::ErrorCodeMap[450] = "Blocked by Windows Parental Controls";
        CHttpContext::ErrorCodeMap[499] = "Client Closed Request";

        CHttpContext::ErrorCodeMap[500] = "Internal Server Error";
        CHttpContext::ErrorCodeMap[501] = "Not Implemented";
        CHttpContext::ErrorCodeMap[502] = "Bad Gateway";
        CHttpContext::ErrorCodeMap[503] = "Service Unavailable";
        CHttpContext::ErrorCodeMap[504] = "Gateway Timeout";
        CHttpContext::ErrorCodeMap[505] = "HTTP Version Not Supported";
        CHttpContext::ErrorCodeMap[506] = "Variant Also Negotiates";
        CHttpContext::ErrorCodeMap[507] = "Insufficient Storage";
        CHttpContext::ErrorCodeMap[508] = "Loop Detected";
        CHttpContext::ErrorCodeMap[509] = "Bandwidth Limit Exceeded";
        CHttpContext::ErrorCodeMap[510] = "Not Extended";
        CHttpContext::ErrorCodeMap[511] = "Network Authentication Required";
        CHttpContext::ErrorCodeMap[598] = "Network read timeout error";
        CHttpContext::ErrorCodeMap[599] = "Network connect timeout error";

        CHttpContext::BuiltinFileContentTypeMap["htm"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["html"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["js"] = "application/x-javascript";
        CHttpContext::BuiltinFileContentTypeMap["xml"] = "application/xml";
        CHttpContext::BuiltinFileContentTypeMap["xsl"] = "application/xml";
        CHttpContext::BuiltinFileContentTypeMap["xslt"] = "application/xslt+xml";
        CHttpContext::BuiltinFileContentTypeMap["swf"] = "application/x-shockwave-flash";
        CHttpContext::BuiltinFileContentTypeMap["php"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["json"] = "application/json; charset=utf-8";
        CHttpContext::BuiltinFileContentTypeMap["rtf"] = "text/rtf";
        CHttpContext::BuiltinFileContentTypeMap["txt"] = "text/plain; charset=utf-8";
        CHttpContext::BuiltinFileContentTypeMap["mhtml"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["css"] = "text/css";
        CHttpContext::BuiltinFileContentTypeMap["jpeg"] = "image/jpeg";
        CHttpContext::BuiltinFileContentTypeMap["jpe"] = "image/jpeg";
        CHttpContext::BuiltinFileContentTypeMap["bmp"] = "image/bmp";
        CHttpContext::BuiltinFileContentTypeMap["cgm"] = "image/cgm";
        CHttpContext::BuiltinFileContentTypeMap["svg"] = "image/svg+xml";
        CHttpContext::BuiltinFileContentTypeMap["tif"] = "image/tiff";
        CHttpContext::BuiltinFileContentTypeMap["tiff"] = "image/tiff";
        CHttpContext::BuiltinFileContentTypeMap["wbmp"] = "image/vnd.wap.wbmp";
        CHttpContext::BuiltinFileContentTypeMap["djv"] = "image/vnd.djvu";
        CHttpContext::BuiltinFileContentTypeMap["djvu"] = "image/vnd.djvu";
        CHttpContext::BuiltinFileContentTypeMap["ras"] = "image/x-cmu-raster";
        CHttpContext::BuiltinFileContentTypeMap["ico"] = "image/x-icon";
        CHttpContext::BuiltinFileContentTypeMap["pnm"] = "image/x-portable-anymap";
        CHttpContext::BuiltinFileContentTypeMap["pbm"] = "image/x-portable-bitmap";
        CHttpContext::BuiltinFileContentTypeMap["pgm"] = "image/x-portable-graymap";
        CHttpContext::BuiltinFileContentTypeMap["ppm"] = "image/x-portable-pixmap";
        CHttpContext::BuiltinFileContentTypeMap["rgb"] = "image/x-rgb";
        CHttpContext::BuiltinFileContentTypeMap["rbm"] = "image/x-xbitmap";
        CHttpContext::BuiltinFileContentTypeMap["rpm"] = "image/x-xpixmap";
        CHttpContext::BuiltinFileContentTypeMap["rwd"] = "image/x-xwindowdump";
        CHttpContext::BuiltinFileContentTypeMap["vrml"] = "model/vrml";
        CHttpContext::BuiltinFileContentTypeMap["vrl"] = "model/vrml";
        CHttpContext::BuiltinFileContentTypeMap["xap"] = "application/x-silverlight-app"; //application/x-silverlight-app for cross-domain
        CHttpContext::BuiltinFileContentTypeMap["mathml"] = "application/mathml+xml";
        CHttpContext::BuiltinFileContentTypeMap["doc"] = "application/msword";
        CHttpContext::BuiltinFileContentTypeMap["bin"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["dms"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["lha"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["lzh"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["exe"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["class"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["so"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["dll"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["pdf"] = "application/pdf";
        CHttpContext::BuiltinFileContentTypeMap["ai"] = "application/postscript";
        CHttpContext::BuiltinFileContentTypeMap["eps"] = "application/postscript";
        CHttpContext::BuiltinFileContentTypeMap["ps"] = "application/postscript";
        CHttpContext::BuiltinFileContentTypeMap["rdf"] = "application/rdf+xml";
        CHttpContext::BuiltinFileContentTypeMap["xul"] = "application/vnd.mozilla.xul+xml";
        CHttpContext::BuiltinFileContentTypeMap["xls"] = "application/vnd.ms-excel";
        CHttpContext::BuiltinFileContentTypeMap["ppt"] = "application/vnd.ms-powerpoint";
        CHttpContext::BuiltinFileContentTypeMap["wbxml"] = "application/vnd.wap.wbxml";
        CHttpContext::BuiltinFileContentTypeMap["zip"] = "application/zip";
        CHttpContext::BuiltinFileContentTypeMap["au"] = "audio/basic";
        CHttpContext::BuiltinFileContentTypeMap["snd"] = "audio/basic";
        CHttpContext::BuiltinFileContentTypeMap["mid"] = "audio/midi";
        CHttpContext::BuiltinFileContentTypeMap["midi"] = "audio/midi";
        CHttpContext::BuiltinFileContentTypeMap["kar"] = "audio/midi";
        CHttpContext::BuiltinFileContentTypeMap["rtx"] = "text/richtext";
        CHttpContext::BuiltinFileContentTypeMap["wmls"] = "text/vnd.wap.wmlscript";
        CHttpContext::BuiltinFileContentTypeMap["wml"] = "text/vnd.wap.wml";
        CHttpContext::BuiltinFileContentTypeMap["tsv"] = "text/tab-separated-values";
        CHttpContext::BuiltinFileContentTypeMap["mpe"] = "video/mpeg";
        CHttpContext::BuiltinFileContentTypeMap["mpeg"] = "video/mpeg";
        CHttpContext::BuiltinFileContentTypeMap["mpg"] = "video/mpeg";
        CHttpContext::BuiltinFileContentTypeMap["qt"] = "video/quicktime";
        CHttpContext::BuiltinFileContentTypeMap["mov"] = "video/quicktime";
        CHttpContext::BuiltinFileContentTypeMap["mxu"] = "video/vnd.mpegurl";
        CHttpContext::BuiltinFileContentTypeMap["avi"] = "video/x-msvideo";
        CHttpContext::BuiltinFileContentTypeMap["movie"] = "video/x-sgi-movie";
        CHttpContext::BuiltinFileContentTypeMap["ice"] = "x-conference/x-cooltalk";
        CHttpContext::BuiltinFileContentTypeMap["asc"] = "text/plain";
        CHttpContext::BuiltinFileContentTypeMap["shtml"] = "text/html";
        CHttpContext::BuiltinFileContentTypeMap["ra"] = "audio/x-realaudio";
        CHttpContext::BuiltinFileContentTypeMap["wav"] = "audio/x-wav";
        CHttpContext::BuiltinFileContentTypeMap["xaml"] = "application/xaml+xml";
        CHttpContext::BuiltinFileContentTypeMap["manifest"] = "application/manifest";
        CHttpContext::BuiltinFileContentTypeMap["application"] = "application/x-ms-application";
        CHttpContext::BuiltinFileContentTypeMap["xbap"] = "application/x-ms-xbap";
        CHttpContext::BuiltinFileContentTypeMap["deploy"] = "application/octet-stream";
        CHttpContext::BuiltinFileContentTypeMap["xps"] = "application/vnd.ms-xpsdocument";

        CHttpContext::SetAppName();
    }

    CHttpContext::CHttpContext()
    : RequestHeaders(*suRequestHeaders),
    m_ResponseCode(200),
    m_pMultiplaxContext(NULL),
    m_bWebSocket(false),
    m_file(NULL) {

    }

    void CHttpContext::CloseFile()
    {
      if (m_file != NULL)
      {
        if(m_HttpResponse.GZip)
          ::gzclose(m_file);
        else
          ::fclose(m_file);
        m_file = NULL;
      }
    }

    CHttpContext::~CHttpContext() {
        CloseFile();
        delete m_pMultiplaxContext;
    }

    unsigned int CHttpContext::GetResponseCode() const {
        return m_ResponseCode;
    }

    void CHttpContext::SetResponseCode(unsigned int errCode) {
        m_ResponseCode = errCode;
    }

    enumHttpResponseStatus CHttpContext::PrepareHttpResponse(const unsigned char *buffer, unsigned int len, MB::CUQueue &qResponse, enumHttpResponseFeedPlan fp) {
        if (buffer == NULL)
            len = 0;
        if (m_HttpResponse.Status == hrsInitial) {
            switch (fp) {
                case hrfpAll:
                {
                    char str[128];
                    ::sprintf(str, "%d", len);
                    m_mapResponse["Content-Length"] = str;
                }
                    m_HttpResponse.Status = hrsCompleted;
                    break;
                default:
                    if (len == 0)
                        m_HttpResponse.Status = hrsHeadersOnly;
                    else
                        m_HttpResponse.Status = hrsContent;
                    break;
            }
            GetResponeHeaders(qResponse);
            qResponse.Push("\r\n", 2);
            qResponse.SetNull();
            assert(qResponse.GetSize() == ::strlen((const char*) qResponse.GetBuffer()));
        }
        qResponse.Push(buffer, len);
        if (len == 0)
            m_HttpResponse.Status = hrsCompleted;
        return m_HttpResponse.Status;
    }

    void CHttpContext::SetDefaultResponseHeaders() {
        if (IsKeepAlive())
            m_mapResponse["Connection"] = "keep-alive";
        const char *dot = ::strrchr(m_RequestContext.Url.Start, '.');
        if (dot != NULL) {
            std::string extension(dot + 1, m_RequestContext.Url.Start + m_RequestContext.Url.Length);
            StringMapA::const_iterator it = BuiltinFileContentTypeMap.find(extension);
            if (it != BuiltinFileContentTypeMap.cend()) {
                m_mapResponse["Content-Type"] = it->second;
            }
        }
    }

    void CHttpContext::SetDate() {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
        using namespace boost::filesystem;
        char strBuffer[128 + 1];
        //set date
        ptime Now = second_clock::universal_time();
        date d = Now.date();
        time_duration td = Now.time_of_day();
        unsigned int year = d.year();
        unsigned int month = d.month();
        unsigned int day = d.day();
        unsigned int dayOfWeek = d.day_of_week();
        unsigned int hour = td.hours();
        unsigned int minute = td.minutes();
        unsigned int second = td.seconds();
        const char *strWeekday;
        switch (dayOfWeek) {
            case 0:
                strWeekday = "Sun";
                break;
            case 1:
                strWeekday = "Mon";
                break;
            case 2:
                strWeekday = "Tus";
                break;
            case 3:
                strWeekday = "Wed";
                break;
            case 4:
                strWeekday = "Thu";
                break;
            case 5:
                strWeekday = "Fri";
                break;
            default:
                strWeekday = "Sat";
                break;
        }
        const char *strMonth;
        switch (month) {
            case 1:
                strMonth = "Jan";
                break;
            case 2:
                strMonth = "Feb";
                break;
            case 3:
                strMonth = "Mar";
                break;
            case 4:
                strMonth = "Apr";
                break;
            case 5:
                strMonth = "May";
                break;
            case 6:
                strMonth = "Jun";
                break;
            case 7:
                strMonth = "Jul";
                break;
            case 8:
                strMonth = "Aug";
                break;
            case 9:
                strMonth = "Sep";
                break;
            case 10:
                strMonth = "Oct";
                break;
            case 11:
                strMonth = "Nov";
                break;
            default:
                strMonth = "Dec";
                break;
        }
        ::sprintf(strBuffer, "%s, %.2d %s %d %.2d:%.2d:%.2d GMT", strWeekday, day, strMonth, year, hour, minute, second);
        m_mapResponse["Date"] = strBuffer;
    }

    void CHttpContext::SetAppName() {
        int n;
        char strBuffer[1025];
#ifdef WIN32_64
        GetModuleFileNameA(NULL, strBuffer, sizeof (strBuffer));
#else
        //ps -p2493 -O tname > name.txt
#endif
        int len = (int) ::strlen(strBuffer);
        for (n = len - 1; n >= 0; --n) {
            char c = strBuffer[n];
#ifdef WIN32_64
            if (c == '\\' || c == ':')
#else
            if (c == '/')
#endif
            {
                break;
            } else {
                AppName.insert(AppName.begin(), c);
            }
        }
#ifdef WIN32_64
        std::transform(AppName.begin(), AppName.end(), AppName.begin(), ::tolower);
        n = (int) AppName.rfind('.');
        if (n > 0)
            AppName = AppName.substr(0, n);
#else

#endif
    }

    const char* CHttpContext::SeekResponseHeaderValue(const char *header) const {
        const char *str = NULL;
        std::string h(header);
        boost::trim(h);
        StringMapA::const_iterator it = m_mapResponse.find(h);
        if (it != m_mapResponse.cend())
            str = it->second.c_str();
        return str;
    }

    void CHttpContext::GetResponeHeaders(MB::CUQueue &mq) {
        const char *str;
        CEcPointer p = ErrorCodeMap.find(m_ResponseCode);
        if (p != ErrorCodeMap.end())
            str = p->second.c_str();
        else {
            assert(false);
            str = "Unknown Error Code";
        }

        {
            char strBuffer[128 + 1];
            ::sprintf(strBuffer, "HTTP/1.1 %d %s\r\n", m_ResponseCode, str);
            mq.Push(strBuffer);
        }

        SetDate();
        m_mapResponse["Server"] = AppName;
        StringMapA::iterator it;
        StringMapA::iterator end = m_mapResponse.end();
        for (it = m_mapResponse.begin(); it != end; ++it) {
            mq.Push(it->first.c_str(), (unsigned int) it->first.size());
            mq.Push(": ", 2);
            mq.Push(it->second.c_str(), (unsigned int) it->second.size());
            mq.Push("\r\n", 2);
        }
        mq.SetNull();
    }

    const std::map<int, std::string>& CHttpContext::GetErrorCodeMap() {
        return ErrorCodeMap;
    }

    tagParseStatus CHttpContext::GetPS() const {
        return m_RequestContext.ParseStatus;
    }

    tagTransferEncoding CHttpContext::GetTE() const {
        return m_RequestContext.TE;
    }

    tagContentMultiplax CHttpContext::GetCM() const {
        return m_RequestContext.CM;
    }

    const HttpResponse& CHttpContext::GetHttpResponseSituation() const {
        return m_HttpResponse;
    }

    MB::U_UINT64 CHttpContext::GetContentLength() const {
        return m_RequestContext.ContentLen;
    }

    const StringMapA& CHttpContext::GetResponseHeaderMap() const {
        return m_mapResponse;
    }

    const char* CHttpContext::ParseMultipart(const char *subdata, unsigned int len) {
        if (subdata == NULL || len == 0)
            throw CMBExCode("No data available for parsing multipart content", MB_BAD_OPERATION);
        if (m_RequestContext.CM == cmUnknown || subdata == NULL || len == 0)
            throw CMBExCode("No http multipart found in request", MB_BAD_OPERATION);
        if (m_pMultiplaxContext == NULL)
            m_pMultiplaxContext = new CMultiplaxContext;
        const UHTTP::CHeaderValue *boundary = SeekMultipart();
        const char *stop = UHTTP::ParseMultipart(subdata, len, boundary->Value.Start, *m_pMultiplaxContext);
        return stop;
    }

    bool CHttpContext::DownloadFile(MB::CUQueue &RecvBuffer) {
        if (m_HttpResponse.Status != hrsDownloadingFile || m_file == NULL)
            return false;
        size_t read;
        CHttpDownFileBuffer FileBuffer;
        char *str = (char*) FileBuffer->GetBuffer();
        if(m_HttpResponse.GZip)
          read = (size_t)gzread(m_file, str, RESPONSE_CHUNK_SIZE);
        else
          read = fread(str, RESPONSE_CHUNK_SIZE, RESPONSE_CHUNK_SIZE, m_file);

        char buf[128];
        ::sprintf(buf, "%X\r\n", (unsigned int) read);
        RecvBuffer.Push(buf);
        if (read)
            RecvBuffer.Push(FileBuffer->GetBuffer(), (unsigned int)read);
        RecvBuffer.Push("\r\n");
        if (read && read < RESPONSE_CHUNK_SIZE) {
            CloseFile();
            RecvBuffer.Push("0\r\n\r\n");
            m_HttpResponse.Status = hrsCompleted;
        }
        RecvBuffer.CleanTrack();
        return true;
    }

    bool CHttpContext::StartDownloadFile(const char *file, MB::CUQueue &RecvBuffer) {
        if (m_HttpResponse.Status != hrsInitial)
            return false;
        CloseFile();
		StringMapA::const_iterator it = m_mapResponse.find("Content-Encoding");
		if(it != m_mapResponse.cend() && boost::iequals(it->second, "gzip"))
            m_HttpResponse.GZip = true;
        else
            m_HttpResponse.GZip = false;
		if(m_HttpResponse.GZip)
			m_file = (FILE*)::gzopen(file, "r");
		else
			m_file = ::fopen(file, "r");
        if (m_file == NULL) {
            SetResponseCode(404);
            PrepareHttpResponse(NULL, 0, RecvBuffer, hrfpAll);
        } else {
            enumHttpResponseFeedPlan fp;
            CHttpDownFileBuffer FileBuffer;
			MB::U_UINT64 read;
            char *str = (char*) FileBuffer->GetBuffer();
			if(m_HttpResponse.GZip)
				read = (MB::U_UINT64)::gzread(m_file, str, RESPONSE_CHUNK_SIZE);
			else
				read = fread(str, RESPONSE_CHUNK_SIZE, RESPONSE_CHUNK_SIZE, m_file);
            if (read >= RESPONSE_CHUNK_SIZE) {
                m_HttpResponse.Chunked = true;
                AddResponseHeader("Transfer-Encoding", "chunked");
                fp = hrfpChuncked;
            } else {
                fp = hrfpAll;
                m_HttpResponse.Chunked = false;
                if(!m_HttpResponse.GZip)
                {
                  char str[128];
                  ::sprintf(str, "%llu", read);
                  AddResponseHeader("Content-Length", str);
                }
                else
                {
                  m_mapResponse.erase("Content-Length");
                  m_mapResponse["Connection"] = "close";
                }
            }
            PrepareHttpResponse(NULL, 0, RecvBuffer, hrfpHeaderOnly);
            if (fp == hrfpChuncked) {
                char str[128];
                ::sprintf(str, "%X\r\n", (unsigned int)read);
                RecvBuffer.Push(str);
            }
            RecvBuffer.Push(FileBuffer->GetBuffer(), (unsigned int)read);
            if (fp == hrfpChuncked)
                RecvBuffer.Push("\r\n");
            else
                CloseFile();
            RecvBuffer.CleanTrack();
            m_HttpResponse.FeedPlan = fp;
            if (fp == hrfpChuncked)
                m_HttpResponse.Status = hrsDownloadingFile;
            else
                m_HttpResponse.Status = hrsCompleted;
        }
        return true;
    }

    const char* CHttpContext::ParseHeaders(const char *str) {
        const char *space = ::strchr(str, ' ');
        if (space == NULL)
            return str;
        const char *ss = ::strstr(str, "\r\n\r\n");
        if (space >= ss)
            return str;
        memset(&m_HttpResponse, 0, sizeof (m_HttpResponse));
        RequestHeaders.SetSize(0);
        m_mapResponse.clear();
        m_ResponseCode = 200;
        unsigned int len = (unsigned int) (ss - str) + 4;
        RequestHeaders.Push(str, len);
        const char *start = (const char*) RequestHeaders.GetBuffer();
        const char *end = ParseHttp(start, len, m_RequestContext);
        if (m_RequestContext.ParseStatus < psHeaders)
            return str;
        SetDefaultResponseHeaders();
        len = (unsigned int) (end - start);
        str += len;
        m_RequestContext.Content = str;
        switch (m_RequestContext.Method) {
            case hmHead:
            case hmGet:
            case hmPut:
            case hmDelete:
            case hmTrace:
                m_RequestContext.ParseStatus = psComplete;
                break;
            default:
                if (m_RequestContext.ContentLen != CONTENT_LEN_UNKNOWN) {
                    size_t len = ::strlen(str);
                    if (m_RequestContext.ContentLen <= len)
                        m_RequestContext.ParseStatus = psComplete;
                    else if (len)
                        m_RequestContext.ParseStatus = psPartialContent;
                }
                break;
        }

        if (m_RequestContext.Method = hmGet) {
            const CHeaderValue *pWebKeyheader = SeekRequestHeader("Sec-WebSocket-Key");
            const CHeaderValue *pUpgrade = SeekRequestHeader("Upgrade");
            const CHeaderValue *pProtocol = SeekRequestHeader("Sec-WebSocket-Protocol");
            if (pWebKeyheader && pUpgrade) {
                m_bWebSocket = true;
                SetResponseCode(101);
                AddResponseHeader("Upgrade", "websocket");
                if (IsKeepAlive())
                    AddResponseHeader("Connection", "Upgrade, keep-alive");
                else
                    AddResponseHeader("Connection", "Upgrade");
                if (pProtocol)
                    AddResponseHeader("Sec-WebSocket-Protocol", pProtocol->Value.Start);
                AddSecWebSocketAcceptHeader(pWebKeyheader->Value.Start);
            }
        }
        return str;
    }

    bool CHttpContext::AddResponseHeader(const char *str, const char *value) {
        if (!value)
            value = "";
        std::string header(str);
        boost::trim(header);
        if (header.length() == 0)
            return false;
        m_mapResponse[header] = value;
        return true;
    }

    bool CHttpContext::AddSecWebSocketAcceptHeader(const char *key) {
        if (key == NULL)
            return false;
        CHttpScopeUQueue su;
        su->Push(key);
        if (su->GetSize() == 0)
            return false;
        //http://tools.ietf.org/html/rfc6455
        //http://en.wikipedia.org/wiki/WebSocket
        su->Push("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        su->SetNull();

        unsigned char bytes[32] = {0};
        char str[32] = {0};
        Utilities::CSHA1 sha1;
        sha1.Update(su->GetBuffer(), su->GetSize());
        sha1.Final();
        bool ok = sha1.GetHash(bytes);
        unsigned int res = Utilities::CBase64::encode(bytes, 20, str);
        m_mapResponse["Sec-WebSocket-Accept"] = str;
        return true;
    }

    const CHeaderValue* CHttpContext::SeekHeader(const std::string &header) const {
        unsigned int n;
        unsigned int size;
        const CHeaderValue *p = m_RequestContext.GetHeaderValue(size);
        for (n = 0; n < size; ++n, ++p) {
            if (boost::iequals(header, p->Header.Start))
                return p;
        }
        return NULL;
    }

    bool CHttpContext::IsGZipAccepted() const {
        static std::string ae = "Accept-Encoding";
        const CHeaderValue *p = SeekHeader(ae);
        if (!p)
            return false;
        std::string v(p->Value.Start);
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        return (v.find("gzip") != v.npos);
    }

    bool CHttpContext::IsChunkOk() const {
        return true;
    }

    bool CHttpContext::IsKeepAlive() const {
        static std::string connection = "Connection";
        const CHeaderValue *p = SeekHeader(connection);
        if (!p)
            return false;
        std::string v(p->Value.Start);
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        return (v.find("keep-alive") != v.npos);
    }

    const CHeaderValue* CHttpContext::GetRequestHeaders(unsigned int &count) const {
        return m_RequestContext.GetHeaderValue(count);
    }

    const CHeaderValue* CHttpContext::SeekMultipart() const {
        return m_RequestContext.SeekMultipart();
    }

    const CHeaderValue* CHttpContext::SeekRequestHeader(const char *header) const {
        return m_RequestContext.SeekHeaderValue(header);
    }

    tagHttpMethod CHttpContext::GetMethod() const {
        return m_RequestContext.Method;
    }

    const CHttpUnit& CHttpContext::GetUrl() const {
        return m_RequestContext.Url;
    }

    bool CHttpContext::IsWebSocket() const {
        return m_bWebSocket;
    }

    const CHttpUnit& CHttpContext::GetParams() const {
        return m_RequestContext.Params;
    }

    double CHttpContext::GetVersion() const {
        return m_RequestContext.Version;
    }

    CHttpContext* CHttpContext::Lock() {
        CHttpContext *p;
        m_cs.lock();
        if (m_qHC.size()) {
            p = m_qHC.front();
            m_qHC.pop();
        } else
            p = NULL;
        m_cs.unlock();
        if (p) {
            ::memset(&p->m_HttpResponse, 0, sizeof (HttpResponse));
            p->m_bWebSocket = false;
            return p;
        }
        p = new CHttpContext;
        return p;
    }

    void CHttpContext::Unlock(CHttpContext *p) {
        if (!p)
            return;
        p->m_RequestContext.MemoryBuffer.SetSize(0);
        p->m_mapResponse.clear();
        p->m_ResponseCode = 200;
		p->CloseFile();
        m_cs.lock();
        m_qHC.push(p);
        m_cs.unlock();
    }


}