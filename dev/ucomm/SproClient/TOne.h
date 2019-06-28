#ifndef ___SOCKETPRO_CLIENT_HANDLER_TONE_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TONE_H__

#include "../../include/aclientw.h"

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
        int QueryCountRtn;
        bool bProcessRy = ProcessR1(idQueryCountCTOne, QueryCountRtn);
        return QueryCountRtn;
    }

    int QueryGlobalCount() {
        int QueryGlobalCountRtn;
        bool bProcessRy = ProcessR1(idQueryGlobalCountCTOne, QueryGlobalCountRtn);
        return QueryGlobalCountRtn;
    }

    int QueryGlobalFastCount() {
        int QueryGlobalFastCountRtn;
        bool bProcessRy = ProcessR1(idQueryGlobalFastCountCTOne, QueryGlobalFastCountRtn);
        return QueryGlobalFastCountRtn;
    }

    void Sleep(int nTime) {
        bool bProcessRy = ProcessR0(idSleepCTOne, nTime);
    }

    UVariant Echo(const UVariant &objInput) {
        UVariant EchoRtn;
        bool bProcessRy = ProcessR1(idEchoCTOne, objInput, EchoRtn);
        return EchoRtn;
    }

    bool EchoEx(const char* str, const wchar_t* wstr, const MyStruct/*Please override >> && << for the class!!!*/&ms, /*out*/std::string &strOut, /*out*/std::wstring &wstrOut) {
        bool EchoExRtn;
        bool bProcessRy = ProcessR3(idEchoExCTOne, str, wstr, ms, strOut, wstrOut, EchoExRtn);
        return EchoExRtn;
    }
};
#endif
