#ifndef ___SOCKETPRO_CLIENT_HANDLER_TONE_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TONE_H__

#include "../include/aclientw.h"

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TOne_i.h"

//client handler for service CTOne

class CTOne : public CAsyncServiceHandler {
public:

    CTOne(CClientSocket *pClientSocket)
    : CAsyncServiceHandler(sidCTOne, pClientSocket) {
    }

public:

    int QueryCount() {
        return send<int>(idQueryCountCTOne).get();
    }

    int QueryGlobalCount() {
        return send<int>(idQueryGlobalCountCTOne).get();
    }

    int QueryGlobalFastCount() {
        return send<int>(idQueryGlobalFastCountCTOne).get();
    }

    void Sleep(int nTime) {
        sendRequest(idSleepCTOne, nTime).get();
    }

    UVariant Echo(const UVariant &objInput) {
        return send<UVariant>(idEchoCTOne, objInput).get();
    }

    bool EchoEx(const char* str, const wchar_t* wstr, const MyStruct/*Please override >> && << for the class!!!*/&ms, /*out*/std::string &strOut, /*out*/std::wstring &wstrOut) {
        auto sb = sendRequest(idEchoExCTOne, str, wstr, ms).get();
        sb >> strOut >> wstrOut;
        return sb->Load<bool>();
    }
};
#endif
