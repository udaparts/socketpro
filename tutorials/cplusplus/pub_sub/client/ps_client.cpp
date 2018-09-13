// hw_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HW.h"

std::wstring ToString(const CMessageSender& ms) {
	SPA::CScopeUQueue su;
	std::wstring msg(L"sender attributes = (ip = ");
	SPA::Utilities::ToWide(ms.IpAddress, strlen(ms.IpAddress), *su);
	msg += (const wchar_t*)su->GetBuffer();
	msg += L", port = ";
	msg += std::to_wstring((SPA::UINT64)ms.Port);
	msg += L", self = ";
	msg += ms.SelfMessage ? L"true" : L"false";
	msg += L", service id = ";
	msg += std::to_wstring((SPA::UINT64)ms.ServiceId);
	msg += L", userid = ";
	msg += ms.UserId;
	msg += L")";
	return msg;
}

int main(int argc, char* argv[]) {
	CConnectionContext cc;
	cc.Host = "127.0.0.1";
	cc.Port = 20901;

	std::cout << "Input this user id ......" << std::endl;
	std::getline(std::wcin, cc.UserId);

	cc.Password = L"MyPassword";
	cc.EncrytionMethod = TLSv1;

	//for windows platforms, you can also use windows system store instead
#ifdef WIN32_64
	CClientSocket::SSL::SetVerifyLocation("root"); //or "my", "my@currentuser", "root@localmachine"
#else
	//CA file is located at the directory ..\SocketProRoot\bin
	CClientSocket::SSL::SetVerifyLocation("ca.cert.pem"); //linux
#endif
	typedef CSocketPool<HelloWorld, CClientSocket> CMyPool;
	CMyPool spHw;

	spHw.DoSslServerAuthentication = [](CMyPool *sender, CClientSocket * cs)->bool {
		int errCode;
		IUcert *cert = cs->GetUCert();
		std::cout << cert->SessionInfo << std::endl;

		const char* res = cert->Verify(&errCode);

		//do ssl server certificate authentication here

		return (errCode == 0); //true -- user id and password will be sent to server
	};

	bool ok = spHw.StartSocketPool(cc, 1, 1);
	auto hw = spHw.Seek(); //or auto hw = spHw.Lock();

	CClientSocket::CPushImpl &push = hw->GetAttachedClientSocket()->GetPush();
	push.OnPublish = [](CClientSocket *cs, const CMessageSender &sender, const unsigned int *groups, unsigned int count, const SPA::UVariant & message) {
		std::wcout << std::endl << L"A message (message) from " << ToString(sender) << L" to groups ";
		std::cout << ToString(groups, count) << std::endl;
	};

	push.OnSubscribe = [](CClientSocket *cs, const CMessageSender &sender, const unsigned int *groups, unsigned int count) {
		std::wcout << std::endl << ToString(sender);
		std::cout << " has just joined groups " << ToString(groups, count) << std::endl;
	};

	push.OnUnsubscribe = [](CClientSocket *cs, const CMessageSender &sender, const unsigned int *groups, unsigned int count) {
		std::wcout << std::endl << ToString(sender);
		std::cout << " has just left from groups " << ToString(groups, count) << std::endl;
	};

	push.OnSendUserMessage = [](CClientSocket *cs, const CMessageSender &sender, const SPA::UVariant & message) {
		std::wcout << std::endl << L"A message (message) from " << ToString(sender) << std::endl;
	};

	unsigned int chat_ids[] = {1, 2};

	//asynchronously process multiple requests with inline batching for best network efficiency
	ok = hw->SendRequest(idSayHelloHelloWorld, L"Jack", L"Smith", [](CAsyncResult & ar) {
		std::wstring ret;
		ar >> ret;
		std::wcout << ret << std::endl;
	});

#ifdef WIN32_64
	SPA::UVariant message(L"We are going to call the method Sleep");
#else
	SPA::UVariant message(std::wstring(L"We are going to call the method Sleep"));
#endif
	ok = push.Publish(message, chat_ids, 2);

	ok = hw->SendRequest(idSleepHelloWorld, (int) 5000, NULL_RH);

	std::wstring receiver;
	std::cout << "Input a receiver for receiving my message ......" << std::endl;
	std::getline(std::wcin, receiver);
#ifdef WIN32_64
	message = (L"A message from " + cc.UserId).c_str();
#else
	message = L"A message from " + cc.UserId;
#endif
	ok = push.SendUserMessage(message, receiver.c_str());

	ok = hw->WaitAll();
	std::cout << "Press a key to shutdown the demo application ......" << std::endl;
	::getchar();
	return 0;
}

