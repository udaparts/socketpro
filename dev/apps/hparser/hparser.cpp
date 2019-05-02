// hparser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "asyncserver.h"
#include <fstream>
#include <assert.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "chat.h"


//#include <iostream>
//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/copy.hpp>
//#include <boost/iostreams/filter/gzip.hpp>

const char *strGet = "GET /_ylt=ArQy3ImI.T4DRgxNmT_7R8.evZx4/SIG=13uh5atnr/EXP=1327950210/**https%3A//login.yahoo.com/config/login%3F.src=fpctx%26.intl=us%26.done=http%253A%252F%252Fwww.yahoo.com%252F HTTP/1.1\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n"
        "Accept-Encoding:gzip,deflate,sdch\r\n"
        "Accept-Language: en-US,en;q=0.8\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Cookie: phpbb2mysql_data=a%3A2%3A%7Bs%3A11%3A%22autologinid%22%3Bs%3A0%3A%22%22%3Bs%3A6%3A%22userid%22%3Bi%3A-1%3B%7D; phpbb2mysql_sid=a2b9611e8e3da90d87597f2ab354cf52\r\n"
        "Host: www.udaparts.com\r\n"
        "Pragma: no-cache\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.162 Safari/535.19\r\n"
        "\r\n";

void TestSpeed() {
    UHTTP::CHttpContext *p;
    const char *end;
    SPA::CScopeUQueue su;

    const char *res = "Okay. I installed 3.06 on a WinXP. Attempting to download database resulted in the error below. This occurs after 2G of download. Possible this is a 2.G limit on the database. I do not know how this compares to the previous errors. Looks the same at this point. If you need more details, you should check the previous tickets and detail on this ticket";

    int n, count = 20;
    for (n = 0; n < count; ++n) {
        size_t len = ::strlen(res);
        p = UHTTP::CHttpContext::Lock();
        end = p->ParseHeaders(strGet);
        p->PrepareResponse((const unsigned char*) res, (unsigned int) len, *su);
        UHTTP::CHttpContext::Unlock(p);
        su->SetSize(0);
    }
    p = NULL;
}

int main(int argc, char* argv[]) {
    unsigned int n;
    unsigned int myint = 25590;
    unsigned char *pByte = (unsigned char*) &myint;
    CComBSTR bstrData = "This is a test";
    SPA::UVariant vtData = bstrData;
    for(n=0; n<myint; ++n)
    {
      unsigned j, mysize = 100;
      SPA::UJsonDocument jd;
      for(j=0; j<mysize; ++j)
      {       
        SPA::UJsonValue jv = SPA::MakeJsonValue(L"Test", jd.GetAllocator());
        jv = SPA::MakeJsonValue(vtData, jd.GetAllocator());
        assert(bstrData == jv.GetString());
      }
    }
    SPA::CScopeUQueue Buffer;
    SPA::UJsonDocument doc;
    doc.SetObject();
    doc.AddMember("ci", 1, doc.GetAllocator());
    doc.AddMember("rc", UHTTP::seWrongArgType, doc.GetAllocator());
    doc.AddMember("rt", "My\"test", doc.GetAllocator());

    SPA::UJsonWriter writer(*Buffer);

    doc.Accept(writer);
    Buffer->SetNull();

    SPA::CScopeUQueue su;


    const char *end;

    UHTTP::CSubHeaderValue shv;
    end = shv.ParseHeadValue(" Test : test\r\r\n\rAgain : me\r\n");
    assert(shv.ParseStatus == UHTTP::psHeader);

    shv.Initialize();
    end = shv.ParseHeadValue(" Test : test\r\r\n\r\nAgain :\r\n");
    assert(shv.ParseStatus == UHTTP::psHeader);

    shv.Initialize();
    end = shv.ParseHeadValue(" Test : test\r\r\n\rA : me\r\n");
    assert(shv.ParseStatus == UHTTP::psHeader);

    shv.Initialize();
    end = shv.ParseHeadValue(" T : t\r\r\n\rA : m\r\r\n");
    assert(shv.ParseStatus == UHTTP::psHeader);

    shv.Initialize();
    end = shv.ParseHeadValue("  : test\r\r\n\rA : me\r\n");
    assert(shv.ParseStatus == UHTTP::psFailed);

    shv.Initialize();
    end = shv.ParseHeadValue(" t : test\r\r\n\rA : \r\n");
    assert(shv.ParseStatus == UHTTP::psHeader);

    UHTTP::CRequestContext rc;
    su->Push(strGet);
    end = rc.ParseHttpHeader((const char *) su->GetBuffer(), su->GetSize());
    rc.Initialize();
    su->SetSize(0);
    //UHTTP::CHttpRequestScopeUQueue su;

    //std::ifstream file("ws0.htm", std::ios_base::binary);

    /*using namespace boost::iostreams; 
    unsigned char *p = NULL;
    std::ifstream file("ws0.htm", std::ios_base::binary);
    filtering_ostreambuf out;
    out.push(gzip_compressor());
    out.push(p, 100); */

    /*file.read((char*)su->GetBuffer(), su->GetMaxSize());
    size_t len = file.gcount();
    gzFile write = gzopen("testzip.gz", "wb");
    gzwrite(write, su->GetBuffer(), len);
    gzclose(write);*/

    const char *str = "POST /post_chunked_all_your_base HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "1e\r\nall your base are belong to us\r\n"
            "0\r\n"
            "\r\n";
    su->Push(str);
    end = rc.ParseHttpHeader((const char *) su->GetBuffer(), su->GetSize());
    rc.Initialize();
    su->SetSize(0);

    str = "POST /two_chunks_mult_zero_end HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "5\r\nhello\r\n"
            "6\r\n world\r\n"
            "000\r\n"
            "\r\n";

    str = "POST /chunked_w_bullshit_after_length HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "5; ihatew3;whatthefuck=aretheseparametersfor\r\nhello\r\n"
            "6; blahblah; blah\r\n world\r\n"
            "0\r\n"
            "\r\n";

    UHTTP::CSubHeaderValue hv;
    UHTTP::CHttpContext *p = UHTTP::CHttpContext::Lock();
    end = p->ParseHeaders(str);
    str = end;
    unsigned int len = (unsigned int) ::strlen(str);
    end = p->ParseChunked(str, len);
    const UHTTP::CHeaderValue *pHV = p->GetChunkedContext()->GetHeaderValue(len);
    for (n = 0; n < len; ++n, ++pHV) {

    }


    boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::local_time();

    TestSpeed();
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = t1 - t0;
    std::cout << "Time required = " << diff.total_milliseconds() << ", count = " << std::endl;

    Chatting::AddAChatGroup(1, L"ITDept");
    Chatting::AddAChatGroup(3, L"HRDepartment");
    Chatting::AddAChatGroup(4, L"SalesDepartment");

    CIoService io;
    UHTTP::CAsyncServer as(io, 20901, false);
    as.Run();
    return 0;
}

