#ifndef __UMB_COMM_SERVER_H__
#define __UMB_COMM_SERVER_H__

#include "ucomm.h"

//errors for the client method SwitchTo
#define ERROR_WRONG_SWITCH			(0x7FFFF100)
#define ERROR_AUTHENTICATION_FAILED		(0x7FFFF101)
#define ERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE	(0x7FFFF102)
#define ERROR_NOT_SWITCHED_YET			(0x7FFFF103)
#define ERROR_BAD_REQUEST			(0x7FFFF104)

namespace SPA {
    namespace ServerSide {

        enum class tagAuthenticationMethod {
            amOwn = 0,
            amMixed = (amOwn + 1),
            amIntegrated = (amMixed + 1),
            amTrusted = (amIntegrated + 1)
        };

        /**
         * 
         */
        enum class tagHttpMethod {
            hmUnknown = 0,
            hmGet = 1,
            hmPost = 2,
            hmHead = 3,
            hmPut = 4,
            hmDelete = 5,
            hmOptions = 6,
            hmTrace = 7,
            hmConnect = 8,
        };

        /**
         * 
         */
        enum class tagTransport {
            tUnknown = -1,
            tWebSocket = 0,
            tFlash = 1,
            tAjax = 2,
            tScript = 3,
        };

        /**
         * 
         */
        enum class tagTransferEncoding {
            teUnknown = 0,
            teChunked = 1,
            teCompress = 2,
            teDeflate = 3,
            teGZip = 4,
            teIdentity = 5,
        };

        /**
         * 
         */
        enum class tagContentMultiplax {
            cmUnknown = 0,
            cmMixed = 1,
            cmAlternative = 2,
            cmDigest = 3,
            cmParallel = 4,
            cmFormData = 5,
            cmReport = 6,
            cmSigned = 7,
            cmEncrypted = 8,
            cmRelated = 9,
            cmByteRanges = 10,
        };

        /**
         * 
         */
        struct CHttpHeaderValue {
            const char *Header;
            const char *Value;
        };

        enum class tagHttpRequestID : unsigned short {
            idGet = 129,
            idPost = 130,
            idHead = 131,
            idPut = 132,
            idDelete = 133,
            idOptions = 134,
            idTrace = 135,
            idConnect = 136,
            idMultiPart = 137, //HTTP POST MUTIPLE PART
            idUserRequest = 138, //SocketPro HTTP User Request
        };

        enum class tagRoutingAlgorithm {
            raDefault = 0,
            raRandom,
            raAverage,
        };

        enum class tagThreadEvent {
            teStarted = 0,
            teKilling = 1
        };

    }; //ServerSide
}; //namespace SPA

#define	SOCKET_NOT_FOUND		(0xFFFFFFFF)
#define REQUEST_CANCELED		(0xFFFFFFFE)
#define RESULT_SENDING_FAILED	(0xFFFFFFFD)
#define BAD_OPERATION			(0xFFFFFFFC)

#ifdef __cplusplus
extern "C" {
#endif

    typedef SPA::UINT64 USocket_Server_Handle;

    typedef void (CALLBACK *POnResultsSent)(USocket_Server_Handle Handler);
    typedef void (CALLBACK *POnClose) (USocket_Server_Handle Handler, int errCode);
    typedef void (CALLBACK *POnRequestArrive)(USocket_Server_Handle Handler, unsigned short requestId, unsigned int len);
    typedef void (CALLBACK *POnFastRequestArrive)(USocket_Server_Handle Handler, unsigned short requestId, unsigned int len);
    typedef int (CALLBACK *PSLOW_PROCESS)(unsigned short requestId, unsigned int len, USocket_Server_Handle Handler);
    typedef void (CALLBACK *POnSlowRequestProcessed)(USocket_Server_Handle Handler, unsigned short requestId);
    typedef void (CALLBACK *POnBaseRequestCame)(USocket_Server_Handle Handler, unsigned short requestId);
    typedef void (CALLBACK *POnSwitchTo)(USocket_Server_Handle Handler, unsigned int oldServiceId, unsigned int newServiceId);
    typedef void (CALLBACK *POnChatRequestComing) (USocket_Server_Handle handler, SPA::tagChatRequestID chatRequestID, unsigned int len);
    typedef void (CALLBACK *POnChatRequestCame) (USocket_Server_Handle handler, SPA::tagChatRequestID chatRequestId);
    typedef bool (CALLBACK *POnHttpAuthentication) (USocket_Server_Handle handler, const wchar_t *userId, const wchar_t *password);

    struct CSvsContext {
        SPA::tagThreadApartment m_ta; //required with a worker thread

        //called within main thread
        POnSwitchTo m_OnSwitchTo; //called when a service is switched
        POnRequestArrive m_OnRequestArrive;
        POnFastRequestArrive m_OnFastRequestArrive; //request processed within main thread
        POnBaseRequestCame m_OnBaseRequestCame; //SocketPro defines a set of base requests
        POnSlowRequestProcessed m_OnRequestProcessed; //called when a slow request processed

        POnClose m_OnClose; //native socket event

        //called within worker thread
        PSLOW_PROCESS m_SlowProcess; //required with a worker thread	

        POnChatRequestComing m_OnChatRequestComing;
        POnChatRequestCame m_OnChatRequestCame;
        POnResultsSent m_OnResultsSent;
        POnHttpAuthentication m_OnHttpAuthentication; //HttpAuthentication
    };

#ifdef __cplusplus
}
#endif

#endif
