#pragma once

#include "../pub_sub/server/HWImpl.h"
#include "../../../include/aserverw.h"

#ifndef WIN32_64
#include <thread>
#include <chrono>
#endif

class CHttpPeer : public SPA::ServerSide::CHttpPeerBase {
protected:

    void OnSubscribe(const unsigned int *pGroup, unsigned int count) {
        std::wcout << L"Web OnSubscribe, sender = " << GetUID() << L", groups = ";
        std::cout << ToString(pGroup, count) << std::endl;
    }

    void OnUnsubscribe(const unsigned int *pGroup, unsigned int count) {
        std::wcout << L"Web OnUnsubscribe, sender = " << GetUID() << L", groups = ";
        std::cout << ToString(pGroup, count) << std::endl;
    }

    void OnPublish(const SPA::UVariant& vtMessage, const unsigned int *pGroup, unsigned int count) {
        std::wcout << L"Web OnPublish, sender = " << GetUID() << L", groups = ";
        std::cout << ToString(pGroup, count);
        if (SPA::Map2VarintType(vtMessage) == VT_BSTR)
            std::cout << ", message = " << SPA::Utilities::ToUTF8(vtMessage.bstrVal);
        std::cout << std::endl;
    }

    void OnSendUserMessage(const wchar_t* receiver, const SPA::UVariant& vtMessage) {
        std::wcout << L"Web OnSendUserMessage, sender = " << GetUID() << L", receiver = " << receiver;
        if (SPA::Map2VarintType(vtMessage) == VT_BSTR)
            std::cout << ", message = " << SPA::Utilities::ToUTF8(vtMessage.bstrVal);
        std::cout << std::endl;
    }

    bool DoAuthentication(const wchar_t *userId, const wchar_t *password) {
        std::wcout << L"Web DoAuthentication, user id = " << userId << L", password = " << password << std::endl;
        unsigned int groups[] = {1, 2, 7};
        bool entered = GetPush().Subscribe(groups, 3);
        return true; //true -- permitted; and false -- denied
    }

    void OnFastRequestArrive(unsigned short requestId, unsigned int len) {
        switch (requestId) {
            case SPA::ServerSide::idDelete:
            case SPA::ServerSide::idPut:
            case SPA::ServerSide::idTrace:
            case SPA::ServerSide::idOptions:
            case SPA::ServerSide::idHead:
            case SPA::ServerSide::idMultiPart:
            case SPA::ServerSide::idConnect:
                SetResponseCode(501);
                SendResult("ps_server doesn't support DELETE, PUT, TRACE, OPTIONS, HEAD, CONNECT and POST with multipart");
                break;
            default:
                SetResponseCode(405);
                SendResult("ps_server only supports GET and POST without multipart");
                break;
        }
    }

    int OnSlowRequestArrive(unsigned short requestId, unsigned int len) {
        const char *path = GetPath();
        const std::string &RequestName = GetUserRequestName();
        const std::vector<SPA::UVariant> &args = GetArgs();
        switch (requestId) {
            case SPA::ServerSide::idGet:
                if (::strstr(path, "."))
                    DownloadFile(path + 1);
                else
                    SendResult("test result --- GET ---");
                break;
            case SPA::ServerSide::idPost:
                SendResult("+++ POST +++ test result");
                break;
            case SPA::ServerSide::idUserRequest:
                if (RequestName == "sayHello") {
                    SendResult(SayHello(args[0].bstrVal, args[1].bstrVal).c_str());
                }
                else if (RequestName == "sleep") {
                    Sleep((unsigned int) args[0].intVal);
                    SendResult("");
                } else
                    SendResult("Unexpected user request");
                break;
            default:
                break;
        }
        return 0;
    }

private:

    std::wstring SayHello(const std::wstring &firstName, const std::wstring &lastName) {
        //notify a message to groups [2, 3] at server side
        unsigned int groups[] = {2, 3};
        SPA::UVariant message = (L"Say hello from " + firstName + L" " + lastName).c_str();
        GetPush().Publish(message, groups, 2);
        return L"Hello " + firstName + L" " + lastName;
    }

    void Sleep(unsigned int ms) {
#ifdef WIN32_64
        ::Sleep(ms);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
        unsigned int groups[] = {2, 3};
        SPA::UVariant message = (GetUID() + L" called the method sleep").c_str();
        GetPush().Publish(message, groups, 2);
    }
};
