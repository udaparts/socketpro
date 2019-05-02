// HttpClientTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <boost/format.hpp>
#include "badhttpclient.h"
#include <assert.h>

using namespace std;
using namespace UHTTP;
using namespace UHTTP::Client;

void TestAjax();

int main(int argc, char* argv[]) {
    const unsigned int MAX_COUNT = 500;
    unsigned int count = 0;
    CIoService io;
    UHTTP::Client::BadHttpClient bad_http(io, false);

    std::string scope_channel = "TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D";
    std::string sub_channel = "VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D";
    std::string quote_doc = "{\"Event\":\"Test\","
            "\"RequestId\": \"19bb384f-7936-48cd-b11e-cc0eb3bde85f\","
            "\"Data\": {"
            "\"Whatever\": 123}}"
            ;
    std::string headers = "Host: localhost:20901\r\n"
            "User-Agent: HttpClient/1.0 uuid/93d9d955-4b2e-4d06-8d52-abb021c56047\r\n"
            "Accept: application/json\r\n"
            "Accept-Charset: utf-8\r\n"
            "Connection: keep-alive\r\n"
            "X-RSE-Version: 2011-05-01\r\n"
            "X-RSE-Mode: test\r\n";

    std::string get_end = "Host: localhost:20901\r\n"
            "User-Agent: HttpClient/1.0 uuid/93d9d955-4b2e-4d06-8d52-abb021c56047\r\n"
            "Accept: application/json\r\n"
            "Accept-Charset: utf-8\r\n"
            "\r\n";

    std::string all;

    all = boost::str(boost::format("POST /%1%/%2% HTTP/1.1\r\n%3%Content-Length: %4%\r\n\r\n%5%") % scope_channel % sub_channel % headers % quote_doc.size() % quote_doc);
    /*bool ok = bad_http.DoHttpRequest(request.data(), request.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());*/

    quote_doc = "{\"Event\":\"ActivateAccount\",\"Data\": {\"Mode\": \"RealTime\"}}";
    std::string request = boost::str(boost::format("POST /%1%/%2% HTTP/1.1\r\n%3%Content-Length: %4%\r\n\r\n%5%") % scope_channel % sub_channel % headers % quote_doc.size() % quote_doc);

    all += request;

    /*ok = bad_http.DoHttpRequest(request.data(), request.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());*/

    quote_doc = "{\"Event\":\"Heartbeat\",\"Data\": { }}";
    request = boost::str(boost::format("POST /%1%/%2% HTTP/1.1\r\n%3%Content-Length: %4%\r\n\r\n%5%") % scope_channel % sub_channel % headers % quote_doc.size() % quote_doc);
    all += request;

    /*ok = bad_http.DoHttpRequest(request.data(), request.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());
     */
    quote_doc = "{\"Event\":\"StopRestore\",\"Data\": {\"RestoreId\" : 1,\"BackupConfigurationId\": 234}}";
    request = boost::str(boost::format("POST /%1%/%2% HTTP/1.1\r\n%3%Content-Length: %4%\r\n\r\n%5%") % scope_channel % sub_channel % headers % quote_doc.size() % quote_doc);
    all += request;

    bool ok;
    do {
        ok = bad_http.DoHttpRequest(all.data(), all.size(), false, 4);
        ++count;
    } while (ok && count < MAX_COUNT);
    // assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    cout << "Type a data" << endl;
    cin >> ok;

    std::string req0 = "GET /health?verbose=false HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    req0 = "GET /all HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=7307381 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=7307381&sort=-1 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=7307381&sort=0 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    //make sort=-5
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=7307381&sort=-5 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    //make sort=+5
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=7307381&sort=5 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    //make max-events = -1
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=-1&last-known-id=7307381 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    //HEAD
    req0 = "HEAD /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.GetStatusCode() == 200 && bad_http.IsOpen());

    //DELETE
    req0 = "DELETE /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1& HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.IsOpen() && bad_http.GetStatusCode() == 200);

    //TRACE
    req0 = "TRACE /k5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/VlNZeHhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5 HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.IsOpen() && bad_http.GetStatusCode() >= 200 && bad_http.GetStatusCode() < 300);

    //PUT
	req0 = boost::str(boost::format("PUT /%1%/%2% HTTP/1.1\r\n%3%Content-Length: %4%\r\n\r\n%5%") % scope_channel % sub_channel % headers % quote_doc.size() % quote_doc);
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.IsOpen() && bad_http.GetStatusCode() >= 200 && bad_http.GetStatusCode() < 300);

    //OPTIONS
	req0 = boost::str(boost::format("OPTIONS /%1%/%2% HTTP/1.1\r\n%3%Content-Length: %4%\r\n\r\n%5%") % scope_channel % sub_channel % headers % quote_doc.size() % quote_doc);
    ok = bad_http.DoHttpRequest(req0.data(), req0.size(), false);
    assert(ok && bad_http.IsOpen() && bad_http.GetStatusCode() >= 200 && bad_http.GetStatusCode() < 300); 

    // ---- HTTP Negative Tests ----

    //bad method
    UHTTP::Client::BadHttpClient bad_http0(io, false);
    req0 = "GeT /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/HhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1&mybad HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http0.DoHttpRequest(req0.data(), req0.size(), false);
    assert(!ok && !bad_http0.IsOpen() && bad_http0.GetStatusCode() >= 500);

    //bad version
    UHTTP::Client::BadHttpClient bad_http1(io, false);
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/HhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1&mybad HTTP/1.5\r\n";
    req0 += get_end;
    ok = bad_http1.DoHttpRequest(req0.data(), req0.size(), false);
    assert(!ok && !bad_http1.IsOpen() && bad_http1.GetStatusCode() >= 500);

    //bad top line with extra \r\n
    UHTTP::Client::BadHttpClient bad_http2(io, false);
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/HhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1&mybad HTTP/1.1\r\n\r\n";
    req0 += get_end;
    ok = bad_http2.DoHttpRequest(req0.data(), req0.size(), false);
    assert(!ok && !bad_http2.IsOpen() && bad_http2.GetStatusCode() >= 500);

    //bad method
    UHTTP::Client::BadHttpClient bad_http3(io, false);
    req0 = "GEET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/HhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1&mybad HTTP/1.1\r\n";
    req0 += get_end;
    ok = bad_http3.DoHttpRequest(req0.data(), req0.size(), false);
    assert(!ok && !bad_http3.IsOpen() && bad_http3.GetStatusCode() >= 500);

    //bad version 2
    UHTTP::Client::BadHttpClient bad_http4(io, false);
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/HhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1&mybad HTTP/15\r\n";
    req0 += get_end;
    ok = bad_http4.DoHttpRequest(req0.data(), req0.size(), false);
    assert(!ok && !bad_http4.IsOpen() && bad_http4.GetStatusCode() >= 500);

    //bad HTTP
    UHTTP::Client::BadHttpClient bad_http5(io, false);
    req0 = "GET /TzExek5tcy9HcFlKZDJGWUltUEFrcUg5dFVCSUVKd0FGTFJMc3NyYzJ4cWJUdnR6ak5SK3dTaDFEbFJlUzZuL0VHN2l0S0dUOFVnT0ppb3NDSlVQV2tGcGNrWnZjbU5sVDI1bA%3D%3D/HhPY1V0ZURVV25DUHo5RUFtR01NVGU5SEMzQWM3VFNyU0prRmZRN3VhVCsrZDhicTFXalAvUU9iSitYZ0tQMk53UVJWZ0N4dHZsVHhVTWVBQ2tGcGNrWnZjbU5sVDI1bA%3D%3D?echo=true&max-events=5&last-known-id=-1&mybad HtTP/15\r\n";
    req0 += get_end;
    ok = bad_http5.DoHttpRequest(req0.data(), req0.size(), false);
    assert(!ok && !bad_http5.IsOpen() && bad_http5.GetStatusCode() >= 500);

    TestAjax();

    return 0;
}

void TestAjax() {
    bool ok;
    CIoService io;
    UHTTP::Client::BadHttpClient bad_http(io, false);

    const std::string headers = "Accept: */*\r\n" \
    "Accept-Language:	en-us\r\n" \
    "Referer:	http://localhost:20901/ws0.htm?test=me&cye=yye&kerui=yang\r\n" \
    "uhttprequest: 1\r\n" \
    "Content-Type: application/json; charset=utf-8\r\n"
            "UA-CPU: AMD64\r\n" \
    "Accept-Encoding: gzip, deflate\r\n" \
    "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Win64; x64; Trident/5.0; .NET CLR 2.0.50727; SLCC2; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E)\r\n" \
    "Host: localhost:20901\r\n" \
    "Connection: Keep-Alive\r\n" \
    "Cache-Control: no-cache\r\n"
            ;

    //bad id
    std::string doc = "{\"n\":\"uping\",\"i\":3,\"v\":1.2,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[]}";
    std::string ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //bad id
    doc = "{\"n\":\"doMyTest\",\"i\":5,\"v\":1.2,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[]}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //bad id -Id
    doc = "{\"n\":\"doMyTest\",\"i\":5,\"v\":1.2,\"Id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[]}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //extra args -- my
    doc = "{\"n\":\"doMyTest\",\"i\":5,\"v\":1.2,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[], \"my\":2}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //no request name
    doc = "{\"i\":5,\"v\":1.2,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[]}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //bad args -v1
    doc = "{\"n\":\"doMyTest\",\"i\":5,\"v1\":1.2,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[]}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //bad type
    doc = "{\"n\":\"doMyTest\",\"i\":5,\"v\":true,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a\":[]}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

    //bad json
    doc = "{\"n\":\"doMyTest\",\"i\":5,\"v\":true,\"id\":\"5+DK7bMaRZii35CJbAMXvzs=\",\"a:[]}";
    ajaxRequest = boost::str(boost::format("POST /ws0.htm?test=me&cye=yye&kerui=yang HTTP/1.1\r\n%1%Content-Length: %2%\r\n\r\n%3%") % headers % doc.size() % doc);
    ok = bad_http.DoHttpRequest(ajaxRequest.data(), ajaxRequest.size(), false, 1);
    assert(ok && !bad_http.IsOpen());

}
