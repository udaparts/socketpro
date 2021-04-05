
#include <iostream>
#include "../urawsocket/rawclient.h"

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
	case tagSessionEvent::seConnecting:
		break;
	case tagSessionEvent::seConnected:
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
	case tagSessionEvent::seThreadKilled:
		std::cout << "tagSessionEvent::seThreadKilled\n";
		break;
	case tagSessionEvent::seClosingSession:
		std::cout << "tagSessionEvent::seClosingSession\n";
		break;
	case tagSessionEvent::seSessionClosed:
		std::cout << "tagSessionEvent::seSessionClosed\n";
		break;
	case tagSessionEvent::seSessionKilled:
		std::cout << "tagSessionEvent::seSessionKilled\n";
		break;
	case tagSessionEvent::seTimer:
		break;
	default:
		break;
	}
}

int main()
{
	std::shared_ptr<IRawThread> pIRawThread(CreateSessions(events, 10, SPA::tagThreadApartment::taNone));
	bool ok = pIRawThread->IsStarted();
	ok = pIRawThread->Start();
	ok = pIRawThread->AddSession();
	ok = pIRawThread->IsStarted();
	unsigned int sessions = pIRawThread->GetSessions();
	ok = pIRawThread->IsBusy();
	ok = pIRawThread->Kill();
	ok = pIRawThread->IsStarted();
	sessions = pIRawThread->GetSessions();
	pIRawThread.reset();
    std::cout << "Hello World!\n";
}
