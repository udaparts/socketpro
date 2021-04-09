#include "../include/membuffer.h"
#include "../include/rawclient.h"
#include "tdsdef.h"
#include <iostream>

using namespace SPA;

SPA::CUQueue g_buffer;

void CALLBACK events(SessionPoolHandle thread, tagSessionPoolEvent se, SessionHandle sh) {
	switch (se) {
	case tagSessionPoolEvent::seConnected:
		std::cout << "seConnected\n";
		{
			char em[1024];
			int errCode = sh->GetErrorCode(em, sizeof(em));
			if (errCode) {
				std::cout << "ec: " << errCode << ", em: " << em << "\n";
			}
		}
		break;
	case tagSessionPoolEvent::seSslShaking:
		std::cout << "seSslShaking\n";
		{
			char em[1024];
			int errCode = sh->GetErrorCode(em, sizeof(em));
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
			int errCode = sh->GetErrorCode(em, sizeof(em));
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
}

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, SPA::CertInfo * ci) {
	std::cout << "depth: " << depth << ", errCode: " << errMessage << "\n";
	return true;
}

int main()
{
	unsigned short s = 0x1551;
	s = tds::ChangeEndian(s);

	tds::PacketHeader ph;
	std::shared_ptr<ISessionPool> pIRawThread(CreateASessionPool(OnAvailable, events, 1));
	auto channel = pIRawThread->FindAClosedSession();
	bool ok = channel->Connect("windesk", 1433, tagEncryptionMethod::NoEncryption, false, true);
	channel = pIRawThread->Lock(100);
	std::cout << "Press a key to shut down the application ......\n";
	ok = pIRawThread->Unlock(channel);
	::getchar();
	return 0;
}
