#include <iostream>
#include "../include/channelpool.h"
#include "../include/membuffer.h"

using namespace SPA;

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, SPA::CertInfo * ci) {
    std::cout << "depth: " << depth << ", errCode: " << errMessage << "\n";
    return true;
}

class CHttp : public CBaseHandler {
public:

    CHttp(SessionHandle sh) : CBaseHandler(sh) {
    }
    CUQueue m_buff;

protected:

    void OnAvailable(const unsigned char *data, unsigned int bytes) {
        m_buff.Push(data, bytes);
        m_buff.SetNull();
    }
};

int main() {
    bool ok;
    int ec;
    {
#ifndef WIN32_64
        ok = SetVerify("ca.cert.pem");
#endif
        SetCertVerifyCallback(CVCallback);
        CSessionPool<CHttp> pool(3);
        auto channel = pool.FindAClosedHandler();
        ok = channel->Connect("windesk", 20901, tagEncryptionMethod::TLSv1, false, true);
        auto cert = channel->GetUCert();
        std::string em = cert->Verify(&ec);
        std::cout << "em: " << em << ", ec: " << ec << "\n";
        unsigned int count = pool.ConnectAll("windesk", 20901, tagEncryptionMethod::TLSv1);
        std::cout << "count: " << count << "\n";
        channel = pool.Lock(100);
        const char *http_req = "GET /index.html HTTP/1.1\r\nHost: windesk\r\n\r\n";
        int res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /ws0.htm HTTP/1.1\r\nHost: windesk\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /events.htm HTTP/1.1\r\nHost: windesk\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /socketprofaqs.htm HTTP/1.1\r\nHost: windesk\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        ok = pool.Unlock(channel);
        std::cout << "Press a key to shut down the application ......\n";
        ::getchar();
        std::cout << (const char*) channel->m_buff.GetBuffer() << "\n";
        ::getchar();
    }
    return 0;
}
