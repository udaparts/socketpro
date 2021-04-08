#include <iostream>
#include "../include/rawclient.h"
#include "../include/membuffer.h"

using namespace SPA;

CUQueue g_buffer;

void CALLBACK events(SessionPoolHandle thread, tagSessionPoolEvent se, SessionHandle sh) {
    switch (se) {
        case tagSessionPoolEvent::seConnected:
            std::cout << "seConnected\n";
        {
            char em[1024];
            int errCode = sh->GetErrorCode(em, sizeof (em));
            if (errCode) {
                std::cout << "ec: " << errCode << ", em: " << em << "\n";
            }
        }
            break;
        case tagSessionPoolEvent::seSslShaking:
            std::cout << "seSslShaking\n";
        {
            char em[1024];
            int errCode = sh->GetErrorCode(em, sizeof (em));
            if (errCode) {
                std::cout << "ec: " << errCode << ", em: " << em << "\n";
            }
        }
            break;
        case tagSessionPoolEvent::seLocked:
            std::cout << "seLocked\n";
            break;
        case tagSessionPoolEvent::seUnlocked:
            std::cout << "seUnlocked\n";
            break;
        case tagSessionPoolEvent::seSessionClosed:
            std::cout << "seSessionClosed\n";
        {
            char em[1024];
            int errCode = sh->GetErrorCode(em, sizeof (em));
            if (errCode) {
                std::cout << "ec: " << errCode << ", em: " << em << "\n";
            }
        }
            break;
        default:
            break;
    }
}

void CALLBACK OnAvailable(SessionHandle sh, const unsigned char *data, unsigned int bytes) {
    g_buffer.Push(data, bytes);
#if 0
    g_buffer.SetNull();
    const char *start = (const char*) g_buffer.GetBuffer();
    const char *headers = strstr(start, "\r\n\r\n");
    if (headers) {
        unsigned int len = headers - start + 4;
        std::string headers(start, len);
        g_buffer.Pop(len);
    }
#endif
}

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, SPA::CertInfo * ci) {
    std::cout << "depth: " << depth << ", errCode: " << errMessage << "\n";
    return true;
}

int main() {
    bool ok;
    int ec;
    {
#ifndef WIN32_64
        ok = SetVerify("ca.cert.pem");
#endif
        SetCertVerifyCallback(CVCallback);
        std::shared_ptr<ISessionPool> pIRawThread(CreateASessionPool(OnAvailable, events, 1));
        auto channel = pIRawThread->FindAClosedSession();
        ok = channel->Connect("windesk", 20901, tagEncryptionMethod::TLSv1, false, true);
        auto cert = channel->GetUCert();
        std::string em = cert->Verify(&ec);
        std::cout << "em: " << em << ", ec: " << ec << "\n";
        unsigned int count = pIRawThread->ConnectAll("windesk", 20901, tagEncryptionMethod::TLSv1);
        std::cout << "count: " << count << "\n";
        channel = pIRawThread->Lock(100);
        const char *http_req = "GET /index.html HTTP/1.1\r\nHost: windesk\r\n\r\n";
        int res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /ws0.htm HTTP/1.1\r\nHost: windesk\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /events.htm HTTP/1.1\r\nHost: windesk\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /socketprofaqs.htm HTTP/1.1\r\nHost: windesk\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        std::cout << "Press a key to shut down the application ......\n";
        ok = pIRawThread->Unlock(channel);
        ::getchar();
    }
    g_buffer.SetNull();
    std::cout << (const char*) g_buffer.GetBuffer() << "\n";
    return 0;
}
