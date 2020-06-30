

#ifndef __SOCKET_COMM_H___
#define __SOCKET_COMM_H___

#include "commutil.h"

namespace SPA {

    enum tagBaseRequestID {
        idUnknown = 0,
        idSwitchTo = 1,
        idRouteeChanged = (idSwitchTo + 1),
        idEncrypted = (idRouteeChanged + 1),
        idBatchZipped = (idEncrypted + 1),
        idCancel = (idBatchZipped + 1),
        idGetSockOptAtSvr = (idCancel + 1),
        idSetSockOptAtSvr = (idGetSockOptAtSvr + 1),
        idDoEcho = (idSetSockOptAtSvr + 1),
        idTurnOnZipAtSvr = (idDoEcho + 1),
        idStartBatching = (idTurnOnZipAtSvr + 1),
        idCommitBatching = (idStartBatching + 1),
        idStartMerge = (idCommitBatching + 1),
        idEndMerge = (idStartMerge + 1),
        idPing = (idEndMerge + 1),
        idEnableClientDequeue = (idPing + 1),
        idServerException = (idEnableClientDequeue + 1),
        idAllMessagesDequeued = (idServerException + 1),
        idHttpClose = (idAllMessagesDequeued + 1), //SocketPro HTTP Close
        idSetZipLevelAtSvr = (idHttpClose + 1),
        idStartJob = (idSetZipLevelAtSvr + 1),
        idEndJob = (idStartJob + 1),
        idRoutingData = (idEndJob + 1),
        idDequeueConfirmed = (idRoutingData + 1),
        idMessageQueued = (idDequeueConfirmed + 1),
        idStartQueue = (idMessageQueued + 1),
        idStopQueue = (idStartQueue + 1),
        idRoutePeerUnavailable = idStopQueue + 1,
        idDequeueBatchConfirmed = idRoutePeerUnavailable + 1,
        idInterrupt = idDequeueBatchConfirmed + 1,
        idReservedOne = 0x100,
        idReservedTwo = 0x2001
    };

    enum tagChatRequestID {
        idEnter = 65,
        idSpeak = 66,
        idSpeakEx = 67,
        idExit = 68,
        idSendUserMessage = 69,
        idSendUserMessageEx = 70,
    };

    enum tagServiceID {
        sidReserved1 = 1,
        sidStartup = 0x100,
        sidChat = (sidStartup + 1),
        sidHTTP = (sidChat + 1),
        sidFile = (sidHTTP + 1),
        sidODBC = (sidFile + 1),
        sidReserved = 0x10000000
    };

    enum tagEncryptionMethod {
        NoEncryption = 0,
        TLSv1 = 1,
    };

    enum tagShutdownType {
        stReceive = 0,
        stSend = 1,
        stBoth = 2
    };

    enum tagQueueStatus {
        //everything is fine
        qsNormal = 0,

        qsMergeComplete = 1,

        //merge push not completed yet
        qsMergePushing = 2,

        //merge incomplete (job incomplete or crash)
        qsMergeIncomplete = 3,

        //job incomplete (crash or endjob not found)
        qsJobIncomplete = 4,

        //an incomplete message detected
        qsCrash = 5,

        //file open error
        qsFileError = 6,

        //queue file opened but can't decrypt existing queued messages because of bad password found
        qsBadPassword = 7,

        //duplicate name error
        qsDuplicateName = 8,
    };

    enum tagOptimistic {
        oMemoryCached = 0,
        oSystemMemoryCached = 1,
        oDiskCommitted = 2
    };

    struct IMessageQueueBasic {
        virtual UINT64 CancelQueuedRequests(UINT64 startIndex, UINT64 endIndex) const = 0;
        virtual bool AbortJob() const = 0;
        virtual bool StartJob() const = 0;
        virtual bool EndJob() const = 0;
        virtual UINT64 RemoveByTTL() const = 0;
        virtual void Reset() const = 0;
        virtual void StopQueue(bool permanent = false) = 0;
        virtual unsigned int GetMessagesInDequeuing() const = 0;
        virtual UINT64 GetMessageCount() const = 0;
        virtual UINT64 GetQueueSize() const = 0;
        virtual bool IsAvailable() const = 0;
        virtual bool IsSecure() const = 0;
        virtual const char* GetQueueFileName() const = 0;
        virtual const char* GetQueueName() const = 0;
        virtual UINT64 GetJobSize() const = 0;
        virtual UINT64 GetLastIndex() const = 0;
        virtual bool IsDequeueShared() const = 0;
        virtual unsigned int GetTTL() const = 0;
        virtual tagQueueStatus GetQueueOpenStatus() const = 0;
        virtual std::time_t GetLastMessageTime() const = 0;
        virtual tagOptimistic GetOptimistic() const = 0;
        virtual void SetOptimistic(tagOptimistic optimistic) const = 0;
    };

    struct IPush {
        virtual bool Subscribe(const unsigned int *pChatGroupId, unsigned int count) const = 0;
        virtual bool Publish(const VARIANT& vtMessage, const unsigned int *pChatGroupId, unsigned int count) const = 0;
        virtual bool SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId) const = 0;
        virtual void Unsubscribe() const = 0;
    };

    struct IPushEx : public IPush {
        virtual bool PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const = 0;
        virtual bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const = 0;
    };

    struct CertInfo {
        const char* Issuer;
        const char* Subject;
        const char* NotBefore;
        const char* NotAfter;
        bool Validity;
        const char* SigAlg;
        const char* CertPem;
        const char* SessionInfo;
        unsigned int PKSize;
        const unsigned char* PublicKey;
        unsigned int AlgSize;
        const unsigned char* Algorithm;
        unsigned int SNSize;
        const unsigned char* SerialNumber;
    };

    struct IUcert : public CertInfo {
        virtual const char* Verify(int *errCode) const = 0;
    };
};

typedef bool(CALLBACK *PCertificateVerifyCallback) (bool, int, int, const char *errMsg, SPA::CertInfo*);

#endif //__SOCKET_COMM_H___
