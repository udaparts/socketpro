// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../../../include/aserverw.h"
using namespace SPA;
using namespace SPA::ServerSide;

#ifndef WIN32_64
#include <chrono>
#include <thread> 
#endif

class CHttpPeer : public CHttpPeerBase {
protected:
    bool DoAuthentication(const wchar_t *userId, const wchar_t *password) {
        std::wcout << L"Web DoAuthentication, user id = " << userId << L", password = " << password << std::endl;
        unsigned int groups[] = {1, 2, 7};
        bool entered = GetPush().Subscribe(groups, 3);
        return true; //true -- permitted; and false -- denied
    }

    void OnFastRequestArrive(unsigned short requestId, unsigned int len) {
        switch (requestId) {
            case (unsigned short)tagHttpRequestID::idDelete:
            case (unsigned short)tagHttpRequestID::idPut:
            case (unsigned short)tagHttpRequestID::idTrace:
            case (unsigned short)tagHttpRequestID::idOptions:
            case (unsigned short)tagHttpRequestID::idHead:
            case (unsigned short)tagHttpRequestID::idMultiPart:
            case (unsigned short)tagHttpRequestID::idConnect:
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
            case (unsigned short)tagHttpRequestID::idGet:
                if (::strstr(path, "."))
                    DownloadFile(path + 1);
                else
                    SendResult("test result --- GET ---");
                break;
            case (unsigned short)tagHttpRequestID::idPost:
                SendResult("+++ POST +++ test result");
                break;
            case (unsigned short)tagHttpRequestID::idUserRequest:
                if (RequestName == "sayHello")
#ifdef WIN32_64
                    SendResult(SayHello(args[0].bstrVal, args[1].bstrVal).c_str());
#else
					SendResult(SayHello(SPA::Utilities::ToWide(args[0].bstrVal), SPA::Utilities::ToWide(args[1].bstrVal)).c_str());
#endif
                else if (RequestName == "sleep") {
                    Sleep((unsigned int)args[0].intVal);
                    SendResult("");
                }
				else if (RequestName == "doException")
					throw CUExCode("This is a test exception on purpose", 12345);
                else
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