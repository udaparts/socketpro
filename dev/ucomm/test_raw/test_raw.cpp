#include <iostream>
#include "../include/rawclient.h"
#include "../include/membuffer.h"

using namespace SPA;

CUQueue g_buffer;

void CALLBACK events(SessionPoolHandle thread, tagSessionPoolEvent se, SessionHandle sh) {
    switch (se) {
        case tagSessionPoolEvent::seUnknown:
            break;
        case tagSessionPoolEvent::seStarted:
            std::cout << "seStarted\n";
            break;
        case tagSessionPoolEvent::seCreatingThread:
            std::cout << "seCreatingThread\n";
            break;
        case tagSessionPoolEvent::seThreadCreated:
            std::cout << "seThreadCreated\n";
            break;
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
        case tagSessionPoolEvent::seKillingThread:
            std::cout << "seKillingThread\n";
            break;
        case tagSessionPoolEvent::seShutdown:
            std::cout << "seShutdown\n";
            break;
        case tagSessionPoolEvent::seSessionCreated:
            std::cout << "seSessionCreated\n";
            break;
        case tagSessionPoolEvent::seSslShaking:
            std::cout << "seSslShaking\n";
            break;
        case tagSessionPoolEvent::seLocked:
            std::cout << "seLocked\n";
            break;
        case tagSessionPoolEvent::seUnlocked:
            std::cout << "seUnlocked\n";
            break;
        case tagSessionPoolEvent::seThreadDestroyed:
            std::cout << "seThreadDestroyed\n";
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
        case tagSessionPoolEvent::seSessionDestroyed:
            std::cout << "seSessionDestroyed\n";
            break;
        case tagSessionPoolEvent::seTimer:
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
    return true;
}

int main() {
    bool ok;
    int ec;
    {
#ifndef WIN32_64
        ok = SetVerify("/etc/ssl/certs");
#endif
        SetCertVerifyCallback(CVCallback);
        std::shared_ptr<ISessionPool> pIRawThread(CreateASessionPool(OnAvailable, events, 3));
        auto channel = pIRawThread->FindAClosedSession();
        ok = channel->Connect("news.yahoo.com", 443, tagEncryptionMethod::TLSv1, false, true);
        auto cert = channel->GetUCert();
        std::string em = cert->Verify(&ec);
        std::cout << "em: " << em << ", ec: " << ec << "\n";
        unsigned int count = pIRawThread->ConnectAll("news.yahoo.com", 443, tagEncryptionMethod::TLSv1);
        channel = pIRawThread->Lock(100);
        const char *http_req = "GET /coronavirus HTTP/1.1\r\nHost: news.yahoo.com\r\n\r\n";
        int res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /us HTTP/1.1\r\nHost: news.yahoo.com\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /politics HTTP/1.1\r\nHost: news.yahoo.com\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        http_req = "GET /world HTTP/1.1\r\nHost: news.yahoo.com\r\n\r\n";
        res = channel->Send((const unsigned char*) http_req, (unsigned int) ::strlen(http_req));
        std::cout << "Press a key to shut down the application ......\n";
        ok = pIRawThread->Unlock(channel);
        ::getchar();
    }
    g_buffer.SetNull();
    std::cout << (const char*) g_buffer.GetBuffer() << "\n";
    return 0;
}
