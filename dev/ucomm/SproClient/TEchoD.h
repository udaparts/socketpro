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
        return send<MyStruct>(idEchoMyStructCEchoSys, my).get();
    }

    CUQueue EchoUQueue(const CUQueue &q) {
        return send<CUQueue>(idEchoUQueueCEchoSys, q).get();
    }

    UVariant EchoComplex0(double d, const wchar_t* s, const UVariant &simpleObj, bool b, /*out*/std::wstring &sOut) {
        auto sb = sendRequest(idEchoComplex0CEchoSys, d, s, simpleObj, b).get();
        sb >> sOut;
        return sb->Load<UVariant>();
    }
};

class CRouteHandler0 : public CAsyncServiceHandler {
public:

    CRouteHandler0(CClientSocket *pClientSocket)
    : CAsyncServiceHandler(sidRouteSvs0, pClientSocket) {
    }

public:

    std::string DoMyEcho0(const char *str) {
        return send<std::string>(idREcho0, str).get();
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
                    SendRouteeResult(idREcho1, str + " from routing peer 0 ++++++");
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
        return send<std::string>(idREcho1, str).get();
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
                    SendRouteeResult(idREcho0, str + " from routing peer 1 -----");
                }
                break;
            default:
                break;
        }
    }
};

#endif
