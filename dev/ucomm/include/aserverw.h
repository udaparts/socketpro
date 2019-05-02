#ifndef __UMB_SERVER_WRAPPER_H_
#define __UMB_SERVER_WRAPPER_H_

#include "userver.h"
#include "membuffer.h"
#include <fstream>
#include <deque>

//this may be used for debug
#define SET_SERVER_CALL_INFO(str) SPA::ServerSide::SetLastCallInfo(str, __LINE__, __FUNCTION__)

namespace SPA {
    namespace ServerSide {
        //this may be used for debug
        void SetLastCallInfo(const char *str, int data, const char *func);

        class CBaseService;

        struct IServerQueue : public IMessageQueueBasic {
            virtual unsigned int GetHandle() const = 0;
            virtual bool AppendTo(const IServerQueue & serverQueue) const = 0;
            virtual bool AppendTo(const unsigned int *handles, unsigned int count) const = 0;
            virtual bool EnsureAppending(const IServerQueue & serverQueue) const = 0;
            virtual bool EnsureAppending(const unsigned int *handles, unsigned int count) const = 0;
            virtual UINT64 BatchEnqueue(unsigned int count, const unsigned char *msgStruct) const = 0;
        };

        class CServerQueue : public IServerQueue {
        public:
            CServerQueue(unsigned int handle);

        public:
            unsigned int GetHandle() const;
            bool AppendTo(const IServerQueue &serverQueue) const;
            bool AppendTo(const unsigned int *handles, unsigned int count) const;
            bool EnsureAppending(const IServerQueue & serverQueue) const;
            bool EnsureAppending(const unsigned int *handles, unsigned int count) const;
            bool AbortJob() const;
            bool StartJob() const;
            bool EndJob() const;
            UINT64 GetJobSize() const;
            UINT64 GetLastIndex() const;
            unsigned int GetMessagesInDequeuing() const;
            bool IsDequeueShared() const;
            unsigned int GetTTL() const;
            UINT64 RemoveByTTL() const;
            tagQueueStatus GetQueueOpenStatus() const;
            void Reset() const;
            std::time_t GetLastMessageTime() const;
            tagOptimistic GetOptimistic() const;
            void SetOptimistic(tagOptimistic optimistic) const;

            UINT64 Enqueue(unsigned short reqId, const unsigned char *buffer, unsigned int size) const;
            UINT64 Enqueue(unsigned short reqId, const CUQueue & qBuffer) const;

            UINT64 Enqueue(unsigned short reqId) const {
                return Enqueue(reqId, (const unsigned char*) nullptr, (unsigned int) 0);
            }

            template<typename T0>
            UINT64 Enqueue(unsigned short reqId, const T0 & t0) const {
                CScopeUQueue sb;
                sb << t0;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 & t1) const {
                CScopeUQueue sb;
                sb << t0 << t1;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 & t2) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 & t3) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 & t4) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 & t5) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 & t6) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 & t7) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 & t8) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return Enqueue(reqId, reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            UINT64 Enqueue(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 & t9) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return Enqueue(reqId, sb->GetBuffer(), sb->GetSize());
            }

            SPA::UINT64 GetMessageCount() const;
            void StopQueue(bool permanent = false);
            SPA::UINT64 GetQueueSize() const;
            bool IsAvailable() const;
            bool IsSecure() const;
            const char* GetQueueFileName() const;
            const char* GetQueueName() const;
            UINT64 CancelQueuedRequests(UINT64 startIndex, UINT64 endIndex) const;
            UINT64 BatchEnqueue(unsigned int count, const unsigned char *msgStruct) const;

        private:
            unsigned int m_handle;
            friend class CSocketProServer;
        };

        class CSocketProServer {
        public:
            CSocketProServer(int nParam = 0);
            virtual ~CSocketProServer();

        public:
            static bool IsRunning();
            static CSocketProServer* GetServer();
            static bool IsMainThread();
            static bool IsServerSSLEnabled();
            static int GetErrorCode();
            static std::string GetErrorMessage();
            static UINT64 GetRequestCount();
            static unsigned int GetCountOfClients();
            static USocket_Server_Handle GetClient(unsigned int index);
            static std::string GetLocalName();
            static const char* GetVersion();
            virtual void PostQuit();
            virtual void StopSocketProServer();
            virtual void UseSSL(const char *certFile, const char *keyFile, const char *pwdForPrivateKeyFile, const char *dhFile = nullptr);
            virtual bool Run(unsigned int listeningPort, unsigned int maxBacklog = 32, bool v6 = false);

            struct CredentialManager {
                static bool HasUserId(const wchar_t *userId);
                static std::wstring GetUserID(USocket_Server_Handle h);
                static bool SetUserID(USocket_Server_Handle h, const wchar_t *userId);
                static std::wstring GetPassword(USocket_Server_Handle h);
                static bool SetPassword(USocket_Server_Handle h, const wchar_t *password);
            };

            struct Router {
                static bool SetRouting(unsigned int serviceId0, unsigned int serviceId1, tagRoutingAlgorithm ra0 = raDefault, tagRoutingAlgorithm ra1 = raDefault);
                static unsigned int CheckRouting(unsigned int serviceId);
            };

            struct Config {
                static void SetMaxThreadIdleTimeBeforeSuicide(unsigned int maxThreadIdleTimeBeforeSuicide);
                static unsigned int GetMaxThreadIdleTimeBeforeSuicide();
                static void SetMaxConnectionsPerClient(unsigned int maxConnectionsPerClient);
                static unsigned int GetMaxConnectionsPerClient();
                static void SetTimerElapse(unsigned int timerElapse);
                static unsigned int GetTimerElapse();
                static void SetSMInterval(unsigned int SMInterval);
                static unsigned int GetSMInterval();
                static void SetPingInterval(unsigned int pingInterval);
                static unsigned int GetPingInterval();
                static void SetDefaultZip(bool bZip);
                static bool GetDefaultZip();
                static void SetDefaultEncryptionMethod(tagEncryptionMethod EncryptionMethod);
                static tagEncryptionMethod GetDefaultEncryptionMethod();
                static void SetSwitchTime(unsigned int switchTime);
                static unsigned int GetSwitchTime();
                static void SetAuthenticationMethod(tagAuthenticationMethod am);
                static tagAuthenticationMethod GetAuthenticationMethod();
                static void SetSharedAM(bool bShared);
                static bool GetSharedAM();
                static void SetPassword(const char* pwd);
                static unsigned int GetMainThreads();
            };

            struct PushManager {
                static bool AddAChatGroup(unsigned int groupId, const wchar_t *description);
                static void RemoveChatGroup(unsigned int chatGroupId);
                static std::wstring GetAChatGroupDescription(unsigned int groupId);
                static unsigned int GetCountOfChatGroups();
                static std::vector<unsigned int> GetAllCreatedChatGroups();
                static bool Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount);
                static bool SendUserMessage(const UVariant& vtMessage, const wchar_t *strUserId);
                static bool Publish(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
                static bool SendUserMessage(const wchar_t *userId, const unsigned char *message, unsigned int size);
            };

            struct DllManager {
                //return an instance to the added dll
                static HINSTANCE AddALibrary(const char *libFile, int param = 0);
                static bool RemoveALibrary(HINSTANCE hLib);
            };

            struct QueueManager {
                static CServerQueue StartQueue(const char *qName, unsigned int ttl, bool dequeueShared = true);
                static bool StopQueue(const char *qName, bool permanent = false);
                static bool IsQueueStarted(const char *qName);
                static bool IsQueueSecured(const char *qName);
                static bool IsServerQueueIndexPossiblyCrashed();
                static void SetMessageQueuePassword(const char *pwd);
                static void SetWorkDirectory(const char *dir);
                static const char* GetWorkDirectory();
            };

        protected:
            virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) = 0;
            virtual void OnAccept(USocket_Server_Handle h, int errCode);
            virtual void OnIdle(INT64 milliseconds);
            virtual void OnClose(USocket_Server_Handle h, int errCode);
            virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);
            virtual void OnSSLShakeCompleted(USocket_Server_Handle h, int errCode);

        private:
            CSocketProServer(const CSocketProServer &as);
            CSocketProServer& operator=(const CSocketProServer &as);

        private:
            static void WINAPI OnSSLShakeCompletedInternal(USocket_Server_Handle Handler, int errCode);
            static void WINAPI OnClientClosedInternal(USocket_Server_Handle Handler, int errCode);
            static void WINAPI OnAcceptedInternal(USocket_Server_Handle Handler, int errCode);
            static bool WINAPI IsPermittedInternal(USocket_Server_Handle Handler, unsigned int serviceId);
            static void WINAPI OnIdleInternal(INT64 milliseconds);

        private:
            static CSocketProServer *m_pServer;

        private:
            unsigned int m_listeningPort;
            unsigned int m_maxBacklog;
            friend class CBaseService;
            CUCriticalSection m_mutex;
        };

        class CBaseService;

        class CSocketPeer {
        private:
            CSocketPeer(const CSocketPeer &p);
            CSocketPeer& operator=(const CSocketPeer &p);

        protected:
            CSocketPeer();
            virtual ~CSocketPeer();

        public:
            const CBaseService* GetBaseService() const;
            USocket_Server_Handle GetSocketHandle() const;
            UINT64 GetSocketNativeHandle() const;
            std::wstring GetUID() const;
            unsigned short GetCurrentRequestID() const;
            unsigned int GetCurrentRequestLen() const;
            unsigned int GetRcvBytesInQueue() const;
            unsigned int GetSndBytesInQueue() const;
            void PostClose(int errCode = 0) const;
            void Close() const;
            bool IsOpened() const;
            UINT64 GetBytesReceived() const;
            UINT64 GetBytesSent() const;
            unsigned int GetSvsID() const;
            int GetErrorCode() const;
            std::string GetErrorMessage() const;
            std::string GetPeerName(unsigned int *port) const;
            bool IsBatching() const;
            void SetUserID(const wchar_t *userId) const;
            void SetUserID(const std::wstring &userId) const;
            unsigned int QueryRequestsInQueue() const;
            unsigned int GetCountOfJoinedChatGroups() const;
            std::vector<unsigned int> GetChatGroups() const;
            unsigned int SendExceptionResult(const wchar_t* errMessage, const char* errWhere, unsigned int errCode = 0, unsigned short requestId = 0, UINT64 callIndex = INVALID_NUMBER) const;
            unsigned int SendExceptionResult(const char* errMessage, const char* errWhere, unsigned int errCode = 0, unsigned short requestId = 0, UINT64 callIndex = INVALID_NUMBER) const;
            bool MakeRequest(unsigned short requestId, const unsigned char *request, unsigned int size) const;
            static bool IsMainThread();
            static UINT64 GetRequestCount();
            bool IsFakeRequest() const;
            bool IsCanceled() const;
            void* GetSSL() const;
            void DropCurrentSlowRequest() const;
            UINT64 GetCurrentRequestIndex() const;

        protected:
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnSwitchFrom(unsigned int nOldServiceId);
            virtual void OnSubscribe(const unsigned int *pGroup, unsigned int count);
            virtual void OnUnsubscribe(const unsigned int *pGroup, unsigned int count);
            virtual void OnPublish(const UVariant& vtMessage, const unsigned int *pGroup, unsigned int count);
            virtual void OnSendUserMessage(const wchar_t* receiver, const UVariant& vtMessage);
            virtual void OnChatRequestCame(tagChatRequestID chatRequestId);
            virtual void OnRequestArrive(unsigned short requestId, unsigned int len);
            virtual void OnSlowRequestProcessed(unsigned short requestId);
            virtual void OnBaseRequestArrive(unsigned short requestId);
            virtual void OnFastRequestArrive(unsigned short requestId, unsigned int len) = 0;
            virtual int OnSlowRequestArrive(unsigned short requestId, unsigned int len) = 0;
            virtual void OnResultsSent();

        private:
            CScopeUQueue m_sb;
            USocket_Server_Handle m_hHandler;
            CBaseService *m_pBase;

        protected:
            CUQueue &m_UQueue;
            bool m_bRandom;

        private:
            friend class CBaseService;
        };

        class CHttpPeerBase : public CSocketPeer {

            class CPushImpl : public IPush {
            public:

                CPushImpl()
                : m_hSocket(0) {
                }

            public:
                virtual bool Subscribe(const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const;
                virtual bool SendUserMessage(const SPA::UVariant& vtMessage, const wchar_t *strUserId) const;
                virtual void Unsubscribe() const;

            public:
                USocket_Server_Handle m_hSocket;
            };

        protected:
            CHttpPeerBase();
            virtual ~CHttpPeerBase();

        protected:
            virtual bool DoAuthentication(const wchar_t *userId, const wchar_t *password);
            virtual unsigned int SendResult(const char *utf8, unsigned int chars = (~0)) const;
            virtual unsigned int SendResult(const wchar_t *str, unsigned int chars = (~0)) const;

        public:
            IPush& GetPush();
            const std::string& GetUserRequestName() const;
            const std::vector<UVariant>& GetArgs() const;
            bool IsAuthenticated() const;
            bool SetResponseCode(unsigned int errCode) const;
            bool SetResponseHeader(const char *uft8Header, const char *utf8Value) const;
            unsigned int GetRequestHeaders(CHttpHeaderValue *HeaderValue, unsigned int count) const;
            const char* GetPath() const;
            UINT64 GetContentLength() const;
            const char* GetQuery() const;
            bool DownloadFile(const char *filePath) const;
            tagHttpMethod GetMethod() const;
            bool IsWebSocket() const;
            bool IsCrossDomain() const;
            double GetVersion() const;
            const char* GetHost() const;
            tagTransport GetTransport() const;
            tagTransferEncoding GetTransferEncoding() const;
            tagContentMultiplax GetContentMultiplax() const;
            unsigned int GetCurrentMultiplaxHeaders(CHttpHeaderValue *HeaderValue, unsigned int count) const;
            const char* GetId() const;
            unsigned int StartChunkResponse() const;
            unsigned int SendChunk(const unsigned char *buffer, unsigned int len) const;
            unsigned int EndChunkResponse(const unsigned char *buffer, unsigned int len) const;

        private:
            CPushImpl m_PushImpl;
            std::string m_WebRequestName;
            bool m_bHttpOk;
            std::vector<UVariant> m_vArg;
            CHttpPeerBase(const CHttpPeerBase &p);
            CHttpPeerBase& operator=(const CHttpPeerBase &p);
            friend class CBaseService;
        };


#define BEGIN_SWITCH(RequestId) switch(RequestId){

#define M_I0_R0(id, func) case id:{func();SendResult(id);}break;

#define M_I0_R1(id, func, R0) case id:{R0 r0;func(r0);SendResult(id,r0);}break;

#define M_I0_R2(id, func, R0, R1) case id:{R0 r0;R1 r1;func(r0,r1);SendResult(id,r0,r1);}break;

#define M_I0_R3(id, func, R0, R1, R2) case id:{R0 r0;R1 r1;R2 r2;func(r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I0_R4(id, func, R0, R1, R2, R3) case id:{R0 r0;R1 r1;R2 r2;R3 r3;func(r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I0_R5(id, func, R0, R1, R2, R3, R4) case id:{R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;func(r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I1_R0(id, func, A0) case id:{A0 a0;m_UQueue>>a0;func(a0);SendResult(id);}break;

#define M_I1_R1(id, func, A0, R0) case id:{A0 a0;R0 r0;m_UQueue>>a0;func(a0,r0);SendResult(id,r0);}break;

#define M_I1_R2(id, func, A0, R0, R1) case id:{A0 a0;R0 r0;R1 r1;m_UQueue>>a0;func(a0,r0,r1);SendResult(id,r0,r1);}break;

#define M_I1_R3(id, func, A0, R0, R1, R2) case id:{A0 a0;R0 r0;R1 r1;R2 r2;m_UQueue>>a0;func(a0,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I1_R4(id, func, A0, R0, R1, R2, R3) case id:{A0 a0;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0;func(a0,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I1_R5(id, func, A0, R0, R1, R2, R3, R4) case id:{A0 a0;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0;func(a0,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I2_R0(id, func, A0, A1) case id:{A0 a0;A1 a1;m_UQueue>>a0>>a1;func(a0,a1);SendResult(id);}break;

#define M_I2_R1(id, func, A0, A1, R0) case id:{A0 a0;A1 a1;R0 r0;m_UQueue>>a0>>a1;func(a0,a1,r0);SendResult(id,r0);}break;

#define M_I2_R2(id, func, A0, A1, R0, R1) case id:{A0 a0;A1 a1;R0 r0;R1 r1;m_UQueue>>a0>>a1;func(a0,a1,r0,r1);SendResult(id,r0,r1);}break;

#define M_I2_R3(id, func, A0, A1, R0, R1, R2) case id:{A0 a0;A1 a1;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1;func(a0,a1,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I2_R4(id, func, A0, A1, R0, R1, R2, R3) case id:{A0 a0;A1 a1;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1;func(a0,a1,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I2_R5(id, func, A0, A1, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1;func(a0,a1,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;




#define M_I3_R0(id, func, A0, A1, A2) case id:{A0 a0;A1 a1;A2 a2;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2);SendResult(id);}break;

#define M_I3_R1(id, func, A0, A1, A2, R0) case id:{A0 a0;A1 a1;A2 a2;R0 r0;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0);SendResult(id,r0);}break;

#define M_I3_R2(id, func, A0, A1, A2, R0, R1) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1);SendResult(id,r0,r1);}break;

#define M_I3_R3(id, func, A0, A1, A2, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I3_R4(id, func, A0, A1, A2, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I3_R5(id, func, A0, A1, A2, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2;func(a0,a1,a2,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;




#define M_I4_R0(id, func, A0, A1, A2, A3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3);SendResult(id);}break;

#define M_I4_R1(id, func, A0, A1, A2, A3, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0);SendResult(id,r0);}break;

#define M_I4_R2(id, func, A0, A1, A2, A3, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1);SendResult(id,r0,r1);}break;

#define M_I4_R3(id, func, A0, A1, A2, A3, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I4_R4(id, func, A0, A1, A2, A3, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I4_R5(id, func, A0, A1, A2, A3, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3;func(a0,a1,a2,a3,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I5_R0(id, func, A0, A1, A2, A3, A4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4);SendResult(id);}break;

#define M_I5_R1(id, func, A0, A1, A2, A3, A4, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0);SendResult(id,r0);}break;

#define M_I5_R2(id, func, A0, A1, A2, A3, A4, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1);SendResult(id,r0,r1);}break;

#define M_I5_R3(id, func, A0, A1, A2, A3, A4, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I5_R4(id, func, A0, A1, A2, A3, A4, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I5_R5(id, func, A0, A1, A2, A3, A4, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4;func(a0,a1,a2,a3,a4,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I6_R0(id, func, A0, A1, A2, A3, A4, A5) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5);SendResult(id);}break;

#define M_I6_R1(id, func, A0, A1, A2, A3, A4, A5, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0);SendResult(id,r0);}break;

#define M_I6_R2(id, func, A0, A1, A2, A3, A4, A5, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1);SendResult(id,r0,r1);}break;

#define M_I6_R3(id, func, A0, A1, A2, A3, A4, A5, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I6_R4(id, func, A0, A1, A2, A3, A4, A5, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I6_R5(id, func, A0, A1, A2, A3, A4, A5, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5;func(a0,a1,a2,a3,a4,a5,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I7_R0(id, func, A0, A1, A2, A3, A4, A5, A6) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6);SendResult(id);}break;

#define M_I7_R1(id, func, A0, A1, A2, A3, A4, A5, A6, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0);SendResult(id,r0);}break;

#define M_I7_R2(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1);SendResult(id,r0,r1);}break;

#define M_I7_R3(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I7_R4(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I7_R5(id, func, A0, A1, A2, A3, A4, A5, A6, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6;func(a0,a1,a2,a3,a4,a5,a6,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I8_R0(id, func, A0, A1, A2, A3, A4, A5, A6, A7) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7);SendResult(id);}break;

#define M_I8_R1(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0);SendResult(id,r0);}break;

#define M_I8_R2(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1);SendResult(id,r0,r1);}break;

#define M_I8_R3(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I8_R4(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I8_R5(id, func, A0, A1, A2, A3, A4, A5, A6, A7, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7;func(a0,a1,a2,a3,a4,a5,a6,a7,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;



#define M_I9_R0(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8);SendResult(id);}break;

#define M_I9_R1(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0);SendResult(id,r0);}break;

#define M_I9_R2(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1);SendResult(id,r0,r1);}break;

#define M_I9_R3(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I9_R4(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I9_R5(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;


#define M_I10_R0(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9);SendResult(id);}break;

#define M_I10_R1(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0);SendResult(id,r0);}break;

#define M_I10_R2(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1);SendResult(id,r0,r1);}break;

#define M_I10_R3(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;R2 r2;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1,r2);SendResult(id,r0,r1,r2);}break;

#define M_I10_R4(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;R2 r2;R3 r3;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1,r2,r3);SendResult(id,r0,r1,r2,r3);}break;

#define M_I10_R5(id, func, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, R0, R1, R2, R3, R4) case id:{A0 a0;A1 a1;A2 a2;A3 a3;A4 a4;A5 a5;A6 a6;A7 a7;A8 a8;A9 a9;R0 r0;R1 r1;R2 r2;R3 r3;R4 r4;m_UQueue>>a0>>a1>>a2>>a3>>a4>>a5>>a6>>a7>>a8>>a9;func(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,r0,r1,r2,r3,r4);SendResult(id,r0,r1,r2,r3,r4);}break;

#define END_SWITCH default: throw CUExCode("Unsupported request found", MB_NOT_SUPPORTED);break;} 

        class CClientPeer : public CSocketPeer {
        private:
            CClientPeer(const CClientPeer &p);
            CClientPeer& operator=(const CClientPeer &p);

            class CPushImpl : public IPushEx {
            public:

                CPushImpl()
                : m_hSocket(0) {
                }

            public:
                virtual bool Subscribe(const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const;
                virtual bool PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool SendUserMessage(const UVariant& vtMessage, const wchar_t *strUserId) const;
                virtual bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const;
                virtual void Unsubscribe() const;

            public:
                USocket_Server_Handle m_hSocket;
            };

        protected:
            CClientPeer();
            virtual ~CClientPeer();

        public:
            bool StartBatching() const;
            bool CommitBatching() const;
            bool AbortBatching() const;
            tagOperationSystem GetPeerOs(bool *endian = nullptr) const;
            unsigned int GetBytesBatched() const;
            bool SetZip(bool bZip) const;
            bool GetZip() const;
            void SetZipLevel(tagZipLevel zl) const;
            tagZipLevel GetZipLevel() const;
            bool IsDequeueRequest() const;
            IPushEx& GetPush();
            void AbortDequeuedMessage() const;
            bool IsDequeuedMessageAborted() const;

        protected:
            virtual void OnPublishEx(const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            virtual void OnSendUserMessageEx(const wchar_t* receiver, const unsigned char *pMessage, unsigned int size);

        public:
            unsigned int SendResult(unsigned short reqId, const unsigned char* pResult, unsigned int size) const;
            unsigned int SendResult(unsigned short reqId, const CUQueue &mc) const;
            unsigned int SendResult(unsigned short reqId, const CScopeUQueue &sb) const;
            unsigned int SendResult(unsigned short reqId) const;
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short reqId, const unsigned char* pResult, unsigned int size) const;
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short reqId, const CUQueue &mc) const;
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short reqId, const CScopeUQueue &sb) const;
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short reqId) const;

            UINT64 Dequeue(unsigned int qHandle, unsigned int messageCount, bool bNotifiedWhenAvailable, unsigned int waitTime = 0) const;
            UINT64 Dequeue(unsigned int qHandle, bool bNotifiedWhenAvailable, unsigned int maxBytes = 8 * 1024, unsigned int waitTime = 0) const;
            void EnableClientDequeue(bool enable) const;

            template<class ctype0>
            unsigned int SendResult(unsigned short usRequestID, const ctype0& data0) const {
                CScopeUQueue sb;
                sb << data0;
                return SendResult(usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0>
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short usRequestID, const ctype0& data0) const {
                CScopeUQueue sb;
                sb << data0;
                return SendResultIndex(callIndex, usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1>
            unsigned int SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1) const {
                CScopeUQueue sb;
                sb << data0 << data1;
                return SendResult(usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1>
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short usRequestID, const ctype0& data0, const ctype1& data1) const {
                CScopeUQueue sb;
                sb << data0 << data1;
                return SendResultIndex(callIndex, usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1, class ctype2>
            unsigned int SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2) const {
                CScopeUQueue sb;
                sb << data0 << data1 << data2;
                return SendResult(usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1, class ctype2>
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2) const {
                CScopeUQueue sb;
                sb << data0 << data1 << data2;
                return SendResultIndex(callIndex, usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3>
            unsigned int SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3) const {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3;
                return SendResult(usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3>
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3) const {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3;
                return SendResultIndex(callIndex, usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4>
            unsigned int SendResult(unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4) const {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3 << data4;
                return SendResult(usRequestID, sb->GetBuffer(), sb->GetSize());
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4>
            unsigned int SendResultIndex(UINT64 callIndex, unsigned short usRequestID, const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4) const {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3 << data4;
                return SendResultIndex(callIndex, usRequestID, sb->GetBuffer(), sb->GetSize());
            }

        private:
            CPushImpl m_PushImpl;
            friend class CBaseService;
        };

        class CBaseService {
            static U_MODULE_HIDDEN unsigned int m_nMainThreads;

        public:
            CBaseService(unsigned int svsId = 0, tagThreadApartment ta = taNone);
            virtual ~CBaseService();

        public:
            virtual bool AddMe(unsigned int nServiceId, tagThreadApartment ta = taNone);
            void RemoveMe();
            unsigned int GetSvsID() const;
            unsigned int GetCountOfSlowRequests() const;
            std::vector<unsigned short> GetAllSlowRequestIds() const;
            static CSvsContext GetSvsContext(unsigned int serviceId);
            const CSvsContext& GetSvsContext() const;
            bool GetReturnRandom() const;
            void SetReturnRandom(bool random) const;

            //if sRequestId <= idReservedTwo, it will return false
            bool AddSlowRequest(unsigned short sReqId) const;
            void RemoveSlowRequest(unsigned short sReqId) const;
            void RemoveAllSlowRequests() const;

            bool AddAlphaRequest(unsigned short reqId) const;
            std::vector<unsigned short> GetAlphaRequestIds() const;

            CSocketPeer* Seek(USocket_Server_Handle h) const;
            static CBaseService* SeekService(unsigned int nServiceId);
            static CBaseService* SeekService(USocket_Server_Handle h);

        protected:
            virtual CSocketPeer* GetPeerSocket() const = 0;

        private:
            CBaseService(const CBaseService &as);
            CBaseService& operator=(const CBaseService &as);

        private:
            static bool Seek(unsigned int nServiceId);
            CSocketPeer* CreatePeer(USocket_Server_Handle h, unsigned int oldServiceId);
            void ReleasePeer(USocket_Server_Handle h, bool bClosing, unsigned int info);
            void Clean();


        private:
            static void CALLBACK OnReqArrive(USocket_Server_Handle hSocket, unsigned short usRequestID, unsigned int len);
            static void CALLBACK OnFast(USocket_Server_Handle hSocket, unsigned short usRequestID, unsigned int len);
            static void CALLBACK OnSwitch(USocket_Server_Handle hSocket, unsigned int oldServiceId, unsigned int newServiceId);
            static int CALLBACK OnSlow(unsigned short usRequestID, unsigned int len, USocket_Server_Handle hSocket);
            static void CALLBACK OnClose(USocket_Server_Handle hSocket, int nError);
            static void CALLBACK OnBaseCame(USocket_Server_Handle hSocket, unsigned short usRequestID);
            static void CALLBACK OnSlowRequestProcessed(USocket_Server_Handle hSocket, unsigned short usRequestID);
            static void CALLBACK OnChatComing(USocket_Server_Handle handler, tagChatRequestID chatRequestID, unsigned int len);
            static void CALLBACK OnChatCame(USocket_Server_Handle handler, tagChatRequestID chatRequestId);
            static bool CALLBACK OnHttpAuthentication(USocket_Server_Handle handler, const wchar_t *userId, const wchar_t *password);
            static void CALLBACK OnResultsSent(USocket_Server_Handle handler);

        private:
            CSvsContext m_SvsContext;
            std::vector<CSocketPeer*> m_vPeer;
            std::deque<CSocketPeer*> m_vDeadPeer;
            unsigned int m_nServiceId;
            static U_MODULE_HIDDEN CUCriticalSection m_mutex;
            static U_MODULE_HIDDEN std::vector<CBaseService*> m_vService;
            friend class CSocketProServer;
            friend class CSocketPeer;
        };

        class CDummyPeer : public CClientPeer {
        protected:

            void OnFastRequestArrive(unsigned short requestId, unsigned int len) {
            }

            int OnSlowRequestArrive(unsigned short requestId, unsigned int len) {
                return 0;
            }
        };

        template<typename TPeer>
        class CSocketProService : public CBaseService {
        public:

            CSocketProService(unsigned int svsId = 0, tagThreadApartment ta = taNone)
            : CBaseService(svsId, ta) {
            }

        protected:

            virtual CSocketPeer* GetPeerSocket() const {
                return new TPeer;
            }

        private:
            CSocketProService(const CSocketProService &svs);
            CSocketProService& operator=(const CSocketProService &svs);
        };

        typedef CDummyPeer CNotifier;
        typedef CSocketProService<CNotifier> CNotificationService;

    }//ServerSide
}//SPA

#endif

