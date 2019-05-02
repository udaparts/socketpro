// hparser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "httpcontext.h"

int main(int argc, char* argv[]) {
    unsigned int size;
    UHTTP::CRequestContext rc;
    UHTTP::CMultiplaxContext sbv;
    UHTTP::CHttpGrammar grammar;
    MB::CScopeUQueue su;

    const char *strPost = "POST /path/script.cgi HTTP/1.1\r\n"
            "From : frog@jmarshall.com\r\n"
            "User-Agent: HTTPTool/1.0\r\n"
            "Content-type: multipart/mixed; boundary=\"simple boundary\"\r\n"
            "\r\n"
            "--simple boundary\r\n"
            "\r\n"
            "This is implicitly typed plain ASCII text.\r\n"
            "It does NOT end with a linebreak.\r\n"
            "--simple boundary\r\n"
            "Content-type : text/plain; charset=us-ascii\r\n"
            "\r\n"
            "This is explicitly typed plain ASCII text.\r\n"
            "It DOES end with a linebreak.\r\n"
            "\r\n"
            "--simple boundary--";

    /*strPost = "POST /path/script.cgi HTTP/1.1\r\n"
        "From : frog@jmarshall.com\r\n"
        "User-Agent: HTTPTool/1.0\r\n"
                    "Content-Type: text/plain\r\n"
                    "Transfer-Encoding: chunked\r\n"
                    "\r\n"
                    "020\r\n"
                    "This is the data in the first \r\n"
                    "\r\n"
                    "1C\r\n"
                    "and this is the second one\r\n"
                    "\r\n"
                    "3\r\n"
                    "con"
                    "\r\n"
                    "8\r\n"
                    "sequence"
                    "\r\n"
                    "0\r\n\r\n"
                    ;*/

    /*strPost = "POST /path/script.cgi HTTP/1.1\r\n"
        "From : frog@jmarshall.com\r\n"
        "User-Agent: HTTPTool/1.0\r\n"
        "Content-type: multipart/mixed; boundary=\"simple boundary\"\r\n"
        "\r\n"
        "--simple boundary\r\n"
        "\r\n"
        "This is implicitly typed plain ASCII text.\r\n"
        "It does NOT end with a linebreak.\r\n"
        "--simple boundary\r\n"
        "Content-type : text/plain; charset=us-ascii\r\n"
        "\r\n"
        "This is explicitly typed plain ASCII text.\r\n"
        "It DOES end with a linebreak.\r\n"
                    "\r\n"
        "--simple boundary--";*/

    /*strPost = "GET /path/script.cgi?mytest=234&your=5&p1=testpararm HTTP/1.1\r\n"
        "From : frog@jmarshall.com\r\n"
        "User-Agent: HTTPTool/1.0\r\n"
        "Content-type: text\r\n"
        "\r\n";*/

    /*strPost =	"POST /path/script.cgi HTTP/1.0\r\n"
                            "From: frog@jmarshall.com\r\n"
                            "User-Agent: HTTPTool/1.0\r\n"
                            "Content-Type: application/x-www-form-urlencoded\r\n"
                            "Content-Length: 32\r\n"
                            "\r\n"
                            "home=Cosby&favorite+flavor=flies";*/

    strPost = "GET /_ylt=ArQy3ImI.T4DRgxNmT_7R8.evZx4/SIG=13uh5atnr/EXP=1327950210/**https%3A//login.yahoo.com/config/login%3F.src=fpctx%26.intl=us%26.done=http%253A%252F%252Fwww.yahoo.com%252F HTTP/1.0\r\n"
            "Accept:*/*\r\n"
            "Accept-Charset:ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n"
            "Accept-Encoding:gzip,deflate,sdch\r\n"
            "Accept-Language:en-US,en;q=0.8\r\n"
            "Connection:keep-alive\r\n"
            "Cookie:idtag=76%03%06XU%c2%a0%c3%b2o%12%c3%89%c3%aaCwo%10%c3%8c; loginkey=-559895936; cat0=2; __utma=40492976.787905343.1315448872.1327202399.1327288593.85; __utmz=40492976.1325379079.81.27.utmcsr=yahoo|utmccn=(organic)|utmcmd=organic|utmctr=www.quicklz.org%20; cntid=225; mguid=d10ae881-77cd-4e01-871d-5a20c610b424\r\n"
            "Host:s.codeproject.com\r\n"
            "Referer:http://www.codeproject.com/\r\n"
            "User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.77 Safari/535.7\r\n\r\n";

    const UHTTP::CHeaderValue *pHeaders;
    unsigned int len;
    UHTTP::CHttpContext *p = UHTTP::CHttpContext::Lock();
    p->CreateInitialResponeHeaders(*su);
    const char *start = (const char*) su->GetBuffer();
    const char *stop = p->ParseHeaders(strPost);
    pHeaders = p->GetRequestHeaders(size);
    bool ok = p->IsGZipAccepted();
    ok = p->IsKeepAlive();

    do {
        if (p->GetPS() != UHTTP::psComplete)
            break;
        size = (unsigned int) ::strlen(stop);
        start = stop;
        if (p->GetCM() != UHTTP::cmUnknown) {
            unsigned int n;
            const UHTTP::CHeaderValue *bundary = p->SeekMultipart();
            stop = UHTTP::ParseMultipart(start, size, bundary->Value.Start, sbv);
            len = (unsigned int) (stop - start);
            const UHTTP::CHeaderValue* pHV = sbv.GetHeaderValue(size);
            while (size) {
                const char *head;
                const char *end;
                for (n = 0; n < size; ++n, ++pHV) {
                    if (pHV->Header.Length != 0)
                        continue;
                    head = pHV->Value.Start;
                    end = head + pHV->Value.Length;
                    ++n;
                    break;
                }
                sbv.MemoryBuffer.Pop(n * sizeof (UHTTP::CHeaderValue));
                pHV = sbv.GetHeaderValue(size);
            }
            break;
        }

        if (p->GetTE() == UHTTP::teChunked) {
            break;
        }

        if (p->GetTE() == UHTTP::teGZip) {
            break;
        }

        switch (p->GetMethod()) {
            case UHTTP::hmGet:
                break;
            case UHTTP::hmPost:
                len = (unsigned int) p->GetContentLength();
                break;
            default:
                break;
        }
    } while (false);
    UHTTP::CHttpContext::Unlock(p);
    return 0;
}

