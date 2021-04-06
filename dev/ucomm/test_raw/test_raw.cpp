#include <iostream>
#include "../urawsocket/rawclient.h"
#include "../include/membuffer.h"

SPA::CUQueue g_buffer;

void CALLBACK events(PIRawThread thread, tagSessionEvent se, USessionHandle sh) {
	switch (se)
	{
	case tagSessionEvent::seUnknown:
		break;
	case tagSessionEvent::seStarted:
		std::cout << "tagSessionEvent::seStarted\n";
		break;
	case tagSessionEvent::seCreatingThread:
		std::cout << "tagSessionEvent::seCreatingThread\n";
		break;
	case tagSessionEvent::seThreadCreated:
		std::cout << "tagSessionEvent::seThreadCreated\n";
		break;
	case tagSessionEvent::seConnected:
		std::cout << "tagSessionEvent::seConnected\n";
		{
			char em[1024];
			int errCode = sh->GetErrorCode(em, sizeof(em));
			if (errCode) {
				std::cout << "ec: " << errCode << ", em: " << em << "\n";
			}
		}
		break;
	case tagSessionEvent::seKillingThread:
		std::cout << "tagSessionEvent::seKillingThread\n";
		break;
	case tagSessionEvent::seShutdown:
		std::cout << "tagSessionEvent::seShutdown\n";
		break;
	case tagSessionEvent::seSessionCreated:
		std::cout << "tagSessionEvent::seSessionCreated\n";
		break;
	case tagSessionEvent::seHandShakeCompleted:
		break;
	case tagSessionEvent::seLocked:
		break;
	case tagSessionEvent::seUnlocked:
		break;
	case tagSessionEvent::seThreadDestroyed:
		std::cout << "tagSessionEvent::seThreadDestroyed\n";
		break;
	case tagSessionEvent::seSessionClosed:
		std::cout << "tagSessionEvent::seSessionClosed\n";
		{
			char em[1024];
			int errCode = sh->GetErrorCode(em, sizeof(em));
			if (errCode) {
				std::cout << "ec: " << errCode << ", em: " << em << "\n";
			}
		}
		break;
	case tagSessionEvent::seSessionDestroyed:
		std::cout << "tagSessionEvent::seSessionDestroyed\n";
		break;
	case tagSessionEvent::seTimer:
		break;
	default:
		break;
	}
}

void CALLBACK OnAvailable(USessionHandle sh, const unsigned char *data, unsigned int bytes) {
	g_buffer.Push(data, bytes);
#if 0
	g_buffer.SetNull();
	const char *start = (const char*)g_buffer.GetBuffer();
	const char *headers = strstr(start, "\r\n\r\n");
	if (headers) {
		unsigned int len = headers - start + 4;
		std::string headers(start, len);
		g_buffer.Pop(len);
	}
#endif
}

int main()
{
	bool ok;
	{
		std::shared_ptr<IRawThread> pIRawThread(CreateSessions(OnAvailable, events, 3, SPA::tagThreadApartment::taNone));
		ok = pIRawThread->Start();
		auto channel = pIRawThread->FindAClosedSession();
		ok = channel->Connect("microsoft.com", 443, SPA::tagEncryptionMethod::TLSv1, false, true, 5000);
#if 0
		const char *http_req = "GET / HTTP/1.1\r\nHost: google.com\r\n\r\n";
		int res = channel->Send((const unsigned char*)http_req, (unsigned int)::strlen(http_req));
		http_req = "GET /events.htm HTTP/1.1\r\nHost: udaparts.com\r\n\r\n";
		res = channel->Send((const unsigned char*)http_req, (unsigned int)::strlen(http_req));
		http_req = "GET /registration.htm HTTP/1.1\r\nHost: udaparts.com\r\n\r\n";
		res = channel->Send((const unsigned char*)http_req, (unsigned int)::strlen(http_req));
		http_req = "GET /socketprofaqs.htm HTTP/1.1\r\nHost: udaparts.com\r\n\r\n";
		res = channel->Send((const unsigned char*)http_req, (unsigned int)::strlen(http_req));
#endif
		std::cout << "Press a key to shut down the application ......\n";
		::getchar();
	}
	g_buffer.SetNull();
	std::cout << (const char*)g_buffer.GetBuffer() << "\n";
	return 0;
}
