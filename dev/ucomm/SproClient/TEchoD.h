#ifndef ___SOCKETPRO_CLIENT_HANDLER_TECHOD_H__
#define ___SOCKETPRO_CLIENT_HANDLER_TECHOD_H__

#include "../include/aclientw.h" 
#include <ctime>

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../TEchoD_i.h"

//client handler for service CEchoSys

class CEchoSys : public CAsyncServiceHandler {
public:

    CEchoSys(CClientSocket *pClientSocket)
    : CAsyncServiceHandler(sidCEchoSys, pClientSocket) {
    }

public:

    MyStruct EchoMyStruct(const MyStruct &my) {
        MyStruct EchoMyStructRtn;
        bool bProcessRy = ProcessR1(idEchoMyStructCEchoSys, my, EchoMyStructRtn);
        return EchoMyStructRtn;
    }

    CUQueue EchoUQueue(const CUQueue &q) {
        CUQueue EchoUQueueRtn;
        bool bProcessRy = ProcessR1(idEchoUQueueCEchoSys, q, EchoUQueueRtn);
        return EchoUQueueRtn;
    }

    UVariant EchoComplex0(double d, const wchar_t* s, const UVariant &simpleObj, bool b, /*out*/std::wstring &sOut) {
        UVariant EchoComplex0Rtn;
        bool bProcessRy = ProcessR2(idEchoComplex0CEchoSys, d, s, simpleObj, b, sOut, EchoComplex0Rtn);
        return EchoComplex0Rtn;
    }
};

class CRouteHandler0 : public CAsyncServiceHandler {
public:

    CRouteHandler0(CClientSocket *pClientSocket)
    : CAsyncServiceHandler(sidRouteSvs0, pClientSocket) {
    }

public:

    std::string DoMyEcho0(const char *str) {
        std::string s;
        bool bProcessRy = ProcessR1(idREcho0, str, s);
        return s;
    }

protected:

    void OnResultReturned(unsigned short reqId, SPA::CUQueue &mc) {
        std::cout << "Routee count = " << GetAttachedClientSocket()->GetRouteeCount() << std::endl;
        switch (reqId) {
            case idREcho1:
                if (IsRouteeRequest()) {
                    std::string str;
                    mc >> str;

                    std::srand((unsigned int) std::time(0));
                    int rand = std::rand();

                    GetAttachedClientSocket()->SetZip(str.size() > 700 && (rand % 2 == 0));

                    std::cout << str << std::endl;

                    assert(GetAttachedClientSocket()->GetCurrentRequestID() == reqId);

                    //send back result
                    SendRouteeResult(str + " from routing peer 0 ++++++");
                }
                break;
            default:
                break;
        }
    }
};

class CRouteHandler1 : public CAsyncServiceHandler {
public:

    CRouteHandler1(CClientSocket *pClientSocket)
    : CAsyncServiceHandler(sidRouteSvs1, pClientSocket) {
    }

    std::string DoMyEcho1(const char *str) {
        std::string s;
        bool bProcessRy = ProcessR1(idREcho1, str, s);
        return s;
    }

protected:

    void OnResultReturned(unsigned short reqId, SPA::CUQueue &mc) {
        std::cout << "Routee count = " << GetAttachedClientSocket()->GetRouteeCount() << std::endl;
        switch (reqId) {
            case idREcho0:
                if (IsRouteeRequest()) {
                    std::string str;
                    mc >> str;

                    std::srand((unsigned int) std::time(0));
                    int rand = std::rand();

                    GetAttachedClientSocket()->SetZip(str.size() > 700 && (rand % 2 == 0));

                    std::cout << str << std::endl;

                    assert(GetAttachedClientSocket()->GetCurrentRequestID() == reqId);

                    //send back result
                    SendRouteeResult(str + " from routing peer 1 -----");
                }
                break;
            default:
                break;
        }
    }
};

#endif
