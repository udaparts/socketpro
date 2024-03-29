#include "stdafx.h"
#include "HW.h"

std::wstring ToString(const CMessageSender& ms) {
    SPA::CScopeUQueue su;
    std::wstring msg(L"sender attributes = (ip = ");
    SPA::Utilities::ToWide(ms.IpAddress, strlen(ms.IpAddress), *su);
    msg += (const wchar_t*) su->GetBuffer();
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
    cc.EncrytionMethod = tagEncryptionMethod::TLSv1;

#ifdef WIN32_64
    //for windows platforms, you can also use windows system store instead
    //assuming ca.cert.pem at root, my, my@currentuser or root@localmachine
#else
    //CA file is located at the directory ../SocketProRoot/bin
    CClientSocket::SSL::SetVerifyLocation("ca.cert.pem"); //linux
#endif
    typedef CSocketPool<HelloWorld> CMyPool;
    CMyPool spHw;

    spHw.DoSslServerAuthentication = [](CMyPool *sender, CClientSocket * cs)->bool {
        int errCode;
        IUcert *cert = cs->GetUCert();
        std::cout << cert->SessionInfo << std::endl;

        const char* res = cert->Verify(&errCode);

        //do ssl server certificate authentication here

        //true -- user id and password will be sent to server
        return (errCode == 0);
    };

    bool ok = spHw.StartSocketPool(cc, 1);
    auto hw = spHw.Seek(); //or auto hw = spHw.Lock();

    CClientSocket::CPushImpl &push = hw->GetSocket()->GetPush();
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
    ok = hw->SendRequest(idSayHello, [](CAsyncResult & ar) {
        std::wcout << ar.Load<std::wstring>() << std::endl;
    }, nullptr, nullptr, L"Jack", L"Smith");

    SPA::UVariant message(L"We are going to call the method Sleep");
    ok = push.Publish(message, chat_ids, 2);

    ok = hw->SendRequest(idSleep, nullptr, nullptr, nullptr, (int) 5000);

    std::wstring receiver;
    std::cout << "Input a receiver for receiving my message ......\n";
    std::getline(std::wcin, receiver);
    message = (L"A message from " + cc.UserId).c_str();
    ok = push.SendUserMessage(message, receiver.c_str());
    std::cout << "Press a key to shutdown the demo ......\n";
    ::getchar();
    return 0;
}
