#pragma once

#include "../include/aserverw.h"
extern SPA::CUCriticalSection g_mutex;

class CHttpPeer : public SPA::ServerSide::CHttpPeerBase {
    static const unsigned short MY_MAKE_REQUEST_ID = (unsigned short)SPA::tagBaseRequestID::idReservedTwo + 1;

protected:

    void OnReleaseSource(bool bClosing, unsigned int info) {
        /*
            SPA::CAutoLock sl(g_mutex);
            if (!bClosing)
                std::wcout << "OnReleaseSource, sender = " << GetUID() << ", new service id = " << info << std::endl;
            else if (info) {
                std::wcout << "OnReleaseSource, sender = " << GetUID() << ", error message = " << GetErrorMessage().c_str() << std::endl;
            }
         */
    }

    void OnSwitchFrom(unsigned int nOldServiceId) {
        bool fake = IsFakeRequest();
        assert(true);
        /*
        unsigned int groups[] = {1, 2};
        GetPush().Enter(groups, 2);
        SPA::CAutoLock sl(g_mutex);
        std::wcout << L"OnSwitchFrom, sender = " << GetUID() << L", old service id = " << nOldServiceId << std::endl; 
         */
    }

    void OnSubscribe(const unsigned int *pGroup, unsigned int count) {
        //SPA::CAutoLock sl(g_mutex);
        //std::wcout << "OnSubscribe, sender = " << GetUID() << ", group count = " << count << std::endl;
    }

    void OnUnsubscribe(const unsigned int *pGroup, unsigned int count) {
        //SPA::CAutoLock sl(g_mutex);
        //std::wcout << "OnUnsubscribe, sender = " << GetUID() << ", group count = " << count << std::endl;
    }

    void OnPublish(const SPA::UVariant& vtMessage, const unsigned int *pGroup, unsigned int count) {
        //SPA::CAutoLock sl(g_mutex);
        //std::wcout << L"OnBroadcast, sender = " << GetUID() << L", group count = " << count << L", msg = " << std::endl;
    }

    void OnSendUserMessage(const wchar_t* receiver, const SPA::UVariant& vtMessage) {
        //SPA::CAutoLock sl(g_mutex);
        //std::wcout << L"OnPostUserMessage, sender = " << GetUID() << L", receiver = " << receiver << L", msg = " << std::endl;
    }

    void OnChatRequestCame(SPA::tagChatRequestID chatRequestId) {
        bool fake = IsFakeRequest();
        //SPA::CAutoLock sl(g_mutex);
        //std::wcout << L"OnChatRequestCame, sender = " << GetUID() << L", chat request id = " << chatRequestId << ", faked = " << fake << std::endl;
    }

    bool DoAuthentication(const wchar_t *userId, const wchar_t *password) {
        SPA::CAutoLock sl(g_mutex);
        std::wcout << L"DoAuthentication, user id = " << userId << L", password = " << password << std::endl;
        unsigned int groups[] = {1, 2, 7};
        bool entered = GetPush().Subscribe(groups, 3);
        return true;
    }

    void MyMakeRequest() {
        SendResult("MyTest");
    }

    void OnFastRequestArrive(unsigned short requestId, unsigned int len) {
        bool fake = IsFakeRequest();
        switch (requestId) {
            case MY_MAKE_REQUEST_ID:
                assert(true);
                MyMakeRequest();
                break;
			case (unsigned short)SPA::ServerSide::tagHttpRequestID::idDelete:
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idPut:
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idTrace:
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idOptions:
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idHead:
            {
                unsigned int res = SendResult("");
                res = 0;
                assert(!fake);
            }
                break;
            default:
                assert(false);
                break;
        }
    }

    int OnSlowRequestArrive(unsigned short requestId, unsigned int len) {
        static std::string chunkedTest("/chunkedTest");
        bool fake = IsFakeRequest();
        assert(!fake);
        switch (requestId) {
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idGet:
            {
                const char *path = GetPath();
                if (chunkedTest == path) {
                    static std::string chunk0 = "var ar = 123456789, msg = 'this is a test message from server ';";
                    static std::string chunk1 = "alert(msg + ar);";
                    SetResponseHeader("Content-Type", "application/x-javascript");
                    unsigned int res = StartChunkResponse();
                    res = SendChunk((const unsigned char*) chunk0.c_str(), (unsigned int) chunk0.size());
                    res = EndChunkResponse((const unsigned char*) chunk1.c_str(), (unsigned int) chunk1.size());
                } else if (::strstr(path, ".")) {
                    SPA::ServerSide::CHttpHeaderValue pHeaderValule[20] = {0};
                    unsigned int res = GetRequestHeaders(pHeaderValule, 20);
                    DownloadFile(path + 1);
                } else {
                    const std::string &RequestName = GetUserRequestName();
                    const std::vector<SPA::UVariant> &args = GetArgs();
                    unsigned int res = SendResult("test result --- GET ---");
                    res = 0;
                }
            }
                break;
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idPost:
            {
                const char *path = GetPath();
                const std::string &RequestName = GetUserRequestName();
                const std::vector<SPA::UVariant> &args = GetArgs();
                unsigned int res = SendResult("+++ POST +++ test result");
                res = 0;
            }
                break;
            case (unsigned short)SPA::ServerSide::tagHttpRequestID::idUserRequest:
            {
                const std::string &RequestName = GetUserRequestName();
                const std::vector<SPA::UVariant> &args = GetArgs();
                if (RequestName == "doRequest") {
                    //std::cout << "Request = " << GetUserRequestName() << std::endl;
                    SendResult("2012-08-23T18:55:11.052Z");

                    //::Sleep(100);
                    MakeRequest(MY_MAKE_REQUEST_ID, nullptr, 0);
                } else if (RequestName == "doEnter") {
                    SendResult("Ok -- doEnter");
                } else if (RequestName == "doSpeak") {
                    SendResult("Ok -- doSpeak");
                } else if (RequestName == "doSendUserMsg") {
#ifdef WIN32_64
                    GetPush().SendUserMessage(args[1], args[0].bstrVal);
#else
                    GetPush().SendUserMessage(args[1], SPA::Utilities::ToWide(args[0].bstrVal).c_str());
#endif
                    SendResult("Ok -- doSendUserMsg");
                } else if (RequestName == "doExit") {
                    GetPush().Unsubscribe();
                    SendResult("Ok -- doExit");
                } else if (RequestName == "doException") {
                    SendExceptionResult(L"Do my exception test", "CHttpPeer::OnSlowRequestArrive", -1, requestId);
                } else if (RequestName == "sendLargeText") {
                    SendResult(args[0].bstrVal);
                } else if (RequestName == "doStudy") {
                    unsigned int pGroups[] = {1, 16, 2};
                    ::srand((unsigned int) time(nullptr));
                    int nRand = ((unsigned int) ::rand() % 10);
                    SPA::UVariant msg;
                    switch (nRand) {
                        case 0:
                            msg = 12345;
                            break;
                        case 1:
                            msg = true;
                            break;
                        case 2:
                            msg = false;
                            break;
                        case 3:
                            msg = 23.4567;
                            break;
                        case 4:
                            msg = "test messageA";
                            break;
                        case 5:
                            msg = L"---- test message W ----";
                            break;
                        case 6:
                        {
#ifdef WIN32_64
                            SYSTEMTIME st;
                            ::GetLocalTime(&st);
                            msg.vt = VT_DATE;
                            ::SystemTimeToVariantTime(&st, &msg.date);
#else
                            SPA::UDateTime udt(std::time(nullptr));
                            msg = udt;
#endif
                        }
                            break;
                        case 7:
                        {
                            SAFEARRAYBOUND sab[1] = {3, 0};
                            SAFEARRAY *pSafeArray = ::SafeArrayCreate(VT_BOOL, 1, sab);
                            VARIANT_BOOL *pBool;
                            ::SafeArrayAccessData(pSafeArray, (void**) &pBool);
                            pBool[0] = VARIANT_TRUE;
                            pBool[1] = VARIANT_FALSE;
                            pBool[2] = VARIANT_TRUE;
                            ::SafeArrayUnaccessData(pSafeArray);
                            msg.vt = (VT_ARRAY | VT_BOOL);
                            msg.parray = pSafeArray;
                        }
                            break;
                        case 8:
                        {
                            SAFEARRAYBOUND sab[1] = {2, 0};
                            SAFEARRAY *pSafeArray = ::SafeArrayCreate(VT_UI2, 1, sab);
                            unsigned short *pUShort;
                            ::SafeArrayAccessData(pSafeArray, (void**) &pUShort);
                            pUShort[0] = 25;
                            pUShort[1] = 2456;
                            ::SafeArrayUnaccessData(pSafeArray);
                            msg.vt = (VT_ARRAY | VT_UI2);
                            msg.parray = pSafeArray;
                        }
                            break;
                        case 9:
                        {
                            SAFEARRAYBOUND sab[1] = {3, 0};
                            SAFEARRAY *pSafeArray = ::SafeArrayCreate(VT_BSTR, 1, sab);
                            BSTR *pBstr;
                            ::SafeArrayAccessData(pSafeArray, (void**) &pBstr);
                            pBstr[0] = ::SysAllocString(L"Test BSTR array");
                            pBstr[1] = nullptr;
                            pBstr[2] = ::SysAllocString(L"This is a test for BSTR array");
                            ::SafeArrayUnaccessData(pSafeArray);
                            msg.vt = (VT_ARRAY | VT_BSTR);
                            msg.parray = pSafeArray;
                        }
                            break;
                        default:
                            break;
                    }
                    SendResult("Ok -- doStudy");
                    GetPush().Publish(msg, pGroups, 3);
                } else {
                    SendResult("Bad -- Unknown Request");
                }
            }
                break;
            default:
                break;
        }
        return 0;
    }
};