
#ifndef __UMB_CLIENT_WRAPPER_H_
#define __UMB_CLIENT_WRAPPER_H_

#include "membuffer.h"
#include "ccloader.h"
#include <unordered_map>
#include <deque>
#ifdef WIN32_64
#include <functional>
#else
#include <tr1/functional>
#endif


namespace SPA {
    namespace ClientSide {

        class CAsyncServiceHandler;

        class CAsyncResult;

        typedef std::tr1::function<void(CAsyncResult&) > ResultHandler;

        class CAsyncResult {
        private:

            CAsyncResult(CAsyncServiceHandler *pAsyncServiceHandler, unsigned short ReqId, CUQueue &q, ResultHandler & rh)
            : AsyncServiceHandler(pAsyncServiceHandler), RequestId(ReqId), UQueue(q), CurrentAsyncResultHandler(rh) {
            }

        public:

            template<class ctype >
            CUQueue& operator >>(ctype & receiver) {
                UQueue >> receiver;
                return UQueue;
            }

        public:
            CAsyncServiceHandler *AsyncServiceHandler;
            unsigned short RequestId;
            CUQueue &UQueue;
            ResultHandler &CurrentAsyncResultHandler;

        private:
            CAsyncResult(const CAsyncResult & ar);
            CAsyncResult& operator=(const CAsyncResult & ar);

            friend class CAsyncServiceHandler;
        };

        struct CConnectionContext {

            CConnectionContext()
            : Port(0),
            EncrytionMethod(NoEncryption),
            V6(false),
            Zip(false) {

            }

			bool operator==(const CConnectionContext &cc) const {
				return (Host == cc.Host &&
						Port == cc.Port &&
						UserId == cc.UserId &&
						Password == cc.Password &&
						EncrytionMethod == cc.EncrytionMethod &&
						V6 == cc.V6 &&
						Zip == cc.Zip);
			}

            std::string Host;
            unsigned int Port;
            std::wstring UserId;
            std::wstring Password;
            tagEncryptionMethod EncrytionMethod;
            bool V6;
            bool Zip;
        };

        struct IClientQueue : public IMessageQueueBasic {
            virtual bool StartQueue(const char *qName, unsigned int ttl, bool secure = true, bool dequeueShared = false) const = 0;
            virtual bool IsDequeueEnabled() const = 0;
            virtual bool AppendTo(const IClientQueue & clientQueue) const = 0;
            virtual bool AppendTo(const USocket_Client_Handle *handles, unsigned int count) const = 0;
            virtual bool EnsureAppending(const IClientQueue & clientQueue) const = 0;
            virtual bool EnsureAppending(const USocket_Client_Handle *handles, unsigned int count) const = 0;
            virtual USocket_Client_Handle GetHandle() const = 0;
        };

        /**
         * 
         */
        class CClientSocket {
        public:
            virtual ~CClientSocket();

        protected:
            CClientSocket();

        private:
            bool Attach(CAsyncServiceHandler *p);
            void Detach(CAsyncServiceHandler *p);
            CAsyncServiceHandler *Seek(unsigned int nServiceId);
            void Set(USocket_Client_Handle h);

        private:
            CClientSocket(const CClientSocket &cs);
            CClientSocket& operator=(const CClientSocket &cs);

            class CPushImpl : public IPushEx {
            public:

                CPushImpl()
                : m_hSocket((UClientSocketBase*) NULL) {
                }

            public:
                virtual bool Subscribe(const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const;
                virtual bool PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool SendUserMessage(const UVariant& vtMessage, const wchar_t *strUserId) const;
                virtual bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const;
                virtual void Unsubscribe() const;

            public:
                USocket_Client_Handle m_hSocket;
            };

            class CQueueImpl : public IClientQueue {
            public:
                bool StartQueue(const char *qName, unsigned int ttl, bool secure = true, bool dequeueShared = false) const;
                void StopQueue(bool permanent = false);
                unsigned int GetMessagesInDequeuing() const;
                SPA::UINT64 GetMessageCount() const;
                SPA::UINT64 GetQueueSize() const;
                bool IsAvailable() const;
                bool IsSecure() const;
                const char* GetQueueFileName() const;
                const char* GetQueueName() const;
                UINT64 CancelQueuedRequests(UINT64 startIndex, UINT64 endIndex) const;
                bool IsDequeueEnabled() const;
                bool AbortJob() const;
                bool StartJob() const;
                bool EndJob() const;
                UINT64 GetJobSize() const;
                USocket_Client_Handle GetHandle() const;
                bool AppendTo(const IClientQueue &clientQueue) const;
                bool AppendTo(const USocket_Client_Handle *handles, unsigned int count) const;
                bool EnsureAppending(const IClientQueue & clientQueue) const;
                bool EnsureAppending(const USocket_Client_Handle *handles, unsigned int count) const;
                UINT64 GetLastIndex() const;
                bool IsDequeueShared() const;
                unsigned int GetTTL() const;
                UINT64 RemoveByTTL() const;
                tagQueueStatus GetQueueOpenStatus() const;
                void Reset() const;
                std::time_t GetLastMessageTime() const;

            public:
                USocket_Client_Handle m_hSocket;

            private:
                size_t m_nQIndex;
            };

        public:
            static bool IsClientQueueIndexPossiblyCrashed();
            static void SetClientWorkDirectory(const char *dir);
            static const char* GetClientWorkDirectory();
            static const char* GetVersion();
			void Close() const;
			void Shutdown(tagShutdownType st = stBoth) const;
            bool operator==(const CClientSocket &cs) const;
            void SetZip(bool zip) const;
            bool GetZip() const;
            void SetZipLevel(SPA::tagZipLevel zl) const;
            SPA::tagZipLevel GetZipLevel() const;
            unsigned int GetConnTimeout() const;
            unsigned int GetRecvTimeout() const;
            void SetRecvTimeout(unsigned int timeout) const;
            void SetConnTimeout(unsigned int timeout) const;
            void SetAutoConn(bool autoConnecting) const;
            bool GetAutoConn() const;
            bool Sendable() const;
            IUcert* GetUCert() const;
            void* GetSSL() const;
            bool IgnoreLastRequest(unsigned short reqId) const;
            unsigned int GetRouteeCount() const;
            bool IsRouting() const;

            /**
             * 
             * @return 
             */
            unsigned int GetCountOfRequestsInQueue() const;

            /**
             * 
             * @return 
             */
            unsigned short GetCurrentRequestID() const;

            unsigned int GetCurrentServiceID() const;

            unsigned short GetServerPingTime() const;

            /**
             * 
             * @return 
             */
            unsigned int GetCurrentResultSize() const;

            /**
             * 
             * @return 
             */
            tagEncryptionMethod GetEncryptionMethod() const;

            /**
             * 
             * @return 
             */
            int GetErrorCode() const;

            /**
             * 
             * @return 
             */
            std::string GetErrorMsg() const;


            /**
             * 
             * @return 
             */
            bool IsConnected() const;

            /**
             * 
             * @param em
             */
            void SetEncryptionMethod(tagEncryptionMethod em) const;

            /**
             * 
             * @return 
             */
            USocket_Client_Handle GetHandle() const;

            /**
             * 
             * @param h
             * @return 
             */
            static CClientSocket* Seek(USocket_Client_Handle h);

            //If socket is closed, batching requests or timed out, it will return false

            /**
             * 
             * @param nTimeout
             * @return 
             */
            bool WaitAll(unsigned int nTimeout = (~0)) const;

            /**
             * 
             * @param requestsQueued
             * @return 
             */
            bool Cancel(unsigned int requestsQueued = (~0)) const;

            /**
             * 
             * @return 
             */
            bool IsRandom() const;

            /**
             * 
             * @return 
             */
            unsigned int GetBytesInSendingBuffer() const;

            /**
             * 
             * @return 
             */
            unsigned int GetBytesInReceivingBuffer() const;

            /**
             * 
             * @return 
             */
            unsigned int GetBytesBatched() const;

            /**
             * 
             * @return 
             */
            UINT64 GetBytesReceived() const;

            /**
             * 
             * @return 
             */
            UINT64 GetBytesSent() const;

            /**
             * 
             * @param userId
             */
            void SetUID(const wchar_t *userId) const;

            /**
             * 
             * @return 
             */
            std::wstring GetUID() const;

            /**
             * 
             * @param password
             */
            void SetPassword(const wchar_t *password) const;

            /**
             * 
             * @return 
             */
            UINT64 GetSocketNativeHandle() const;

            /**
             * 
             * @return 
             */
            IPushEx& GetPush();

            IClientQueue& GetClientQueue();

            /**
             * 
             * @param os
             * @return 
             */
            tagOperationSystem GetPeerOs(bool *endian = NULL) const;

            bool DoEcho() const;
            bool SetSockOpt(tagSocketOption optName, int optValue, tagSocketLevel level = slSocket) const;
            bool SetSockOptAtSvr(tagSocketOption optName, int optValue, tagSocketLevel level = slSocket) const;
            bool TurnOnZipAtSvr(bool enableZip) const;
            bool SetZipLevelAtSvr(SPA::tagZipLevel zipLevel) const;
            std::string GetPeerName(unsigned int *port) const;

        protected:
            /**
             * 
             * @param nError
             */
            virtual void OnSocketClosed(int nError);


            /**
             * 
             * @param nError
             */
            virtual void OnHandShakeCompleted(int nError);

            /**
             * 
             * @param nError
             */
            virtual void OnSocketConnected(int nError);

            /**
             * 
             * @param requestId
             * @param len
             */
            virtual void OnRequestProcessed(unsigned short requestId, unsigned int len);

            /**
             * 
             * @param sender
             * @param pGroup
             * @param count
             */
            virtual void OnSubscribe(CMessageSender sender, const unsigned int *pGroup, unsigned int count);

            /**
             * 
             * @param sender
             * @param pGroup
             * @param count
             */
            virtual void OnUnsubscribe(CMessageSender sender, const unsigned int *pGroup, unsigned int count);

            /**
             * 
             * @param sender
             * @param pGroup
             * @param count
             * @param vtMessage
             */
            virtual void OnPublish(CMessageSender sender, const unsigned int *pGroup, unsigned int count, const SPA::UVariant &vtMessage);

            /**
             * 
             * @param sender
             * @param pGroup
             * @param count
             * @param pMessage
             * @param size
             */
            virtual void OnPublishEx(CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);

            /**
             * 
             * @param sender
             * @param message
             */
            virtual void OnSendUserMessage(CMessageSender sender, const SPA::UVariant &message);

            /**
             * 
             * @param sender
             * @param pMessage
             * @param size
             */
            virtual void OnSendUserMessageEx(CMessageSender sender, const unsigned char *pMessage, unsigned int size);

            /**
             * 
             * @param requestId
             * @param errMessage
             * @param errWhere
             * @param errCode
             */
            virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);

            /**
             * 
             * @param requestId
             */
            virtual void OnBaseRequestProcessed(unsigned short requestId);


            virtual void OnAllRequestsProcessed(unsigned short lastRequestId);

        private:
            static void WINAPI OnSocketClosed(USocket_Client_Handle handler, int nError);
            static void WINAPI OnHandShakeCompleted(USocket_Client_Handle handler, int nError);
            static void WINAPI OnSocketConnected(USocket_Client_Handle handler, int nError);
            static void WINAPI OnRequestProcessed(USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
            static void WINAPI OnSubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count);
            static void WINAPI OnUnsubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count);
            static void WINAPI OnBroadcastEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnBroadcast(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnPostUserMessage(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnPostUserMessageEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnServerException(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            static void WINAPI OnBaseRequestProcessed(USocket_Client_Handle handler, unsigned short requestId);
            static void WINAPI OnAllRequestsProcessed(USocket_Client_Handle handler, unsigned short lastRequestId);

            CAsyncServiceHandler *GetCurrentHandler();

        private:
            USocket_Client_Handle m_hSocket;
            CPushImpl m_PushImpl;
            CUCriticalSection m_mx;
            std::vector<CAsyncServiceHandler*> m_vHandler;
            CQueueImpl m_QueueImpl;
            CConnectionContext m_cc;

            static CUCriticalSection m_mutex;
            static std::vector<CClientSocket*> m_vClientSocket;

            template<typename THandler, typename TCS>
            friend class CSocketPool;
            friend class CAsyncServiceHandler;
        };

        class CAsyncServiceHandler {
        public:
            virtual ~CAsyncServiceHandler();

        protected:
            CAsyncServiceHandler(unsigned int nServiceId, CClientSocket *cs = NULL);

        private:
            CAsyncServiceHandler(const CAsyncServiceHandler&);
            CAsyncServiceHandler& operator=(const CAsyncServiceHandler&);

        protected:
            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc);
            virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            virtual void OnBaseRequestProcessed(unsigned short requestId);

        public:
            virtual bool SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int size, const ResultHandler &rh);

            unsigned int GetSvsID() const;
            bool SendRequest(unsigned short reqId, const ResultHandler &rh);
            CClientSocket *GetAttachedClientSocket();
            bool WaitAll(unsigned int timeOut = (~0));
            bool StartBatching();
            bool CommitBatching(bool bBatchingAtServerSide = false);
            bool AbortBatching();
            bool IsBatching();
            unsigned int RemoveAsyncHandlers();
            bool IsDequeuedResult();
            void AbortDequeuedMessage();
            bool IsDequeuedMessageAborted();
            bool IsRouteeRequest();

            bool ProcessR0(unsigned short reqId) {
                CScopeUQueue su;
                return P(reqId, su);
            }

            template<typename T0>
            bool ProcessR0(unsigned short reqId, const T0 &t0) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, su);
            }

            template<typename T0, typename T1>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, su);
            }

            //ProcessR1

            template<typename R0>
            bool ProcessR1(unsigned short reqId, R0 &r0) {
                CScopeUQueue su;
                return P(reqId, su, r0);
            }

            template<typename T0, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, R0 &r0) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, su, r0);
            }

            //ProcessR2

            template<typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, su, r0, r1);
            }

            //ProcessR3

            template<typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, su, r0, r1, r2);
            }

            //ProcessR4

            template<typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, su, r0, r1, r2, r3);
            }

            //ProcessR5

            template<typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, su, r0, r1, r2, r3, r4);
            }

            template<typename T0>
            bool SendRequest(unsigned short reqId, const T0 &t0, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

        private:
            //The two following functions may be public in the future
            bool Attach(CClientSocket *cs);
            void Detach();

            USocket_Client_Handle GetClientSocketHandle();

            bool P(unsigned short reqId, const CScopeUQueue &su) {

                if (!SendRequest(reqId, su->GetBuffer(), su->GetSize(), [](CAsyncResult & ar) {
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0>
            bool P(unsigned short reqId, const CScopeUQueue &su, R0 &r0) {

                if (!SendRequest(reqId, su->GetBuffer(), su->GetSize(), [&r0](CAsyncResult & ar) {
                        ar >> r0;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1>
            bool P(unsigned short reqId, const CScopeUQueue &su, R0 &r0, R1 &r1) {

                if (!SendRequest(reqId, su->GetBuffer(), su->GetSize(), [&r0, &r1](CAsyncResult & ar) {
                        ar >> r0 >> r1;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1, typename R2>
            bool P(unsigned short reqId, const CScopeUQueue &su, R0 &r0, R1 &r1, R2 &r2) {

                if (!SendRequest(reqId, su->GetBuffer(), su->GetSize(), [&r0, &r1, &r2](CAsyncResult & ar) {
                        ar >> r0 >> r1 >> r2;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1, typename R2, typename R3>
            bool P(unsigned short reqId, const CScopeUQueue &su, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {

                if (!SendRequest(reqId, su->GetBuffer(), su->GetSize(), [&r0, &r1, &r2, &r3](CAsyncResult & ar) {
                        ar >> r0 >> r1 >> r2 >> r3;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1, typename R2, typename R3, typename R4>
            bool P(unsigned short reqId, const CScopeUQueue &su, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {

                if (!SendRequest(reqId, su->GetBuffer(), su->GetSize(), [&r0, &r1, &r2, &r3, &r4](CAsyncResult & ar) {
                        ar >> r0 >> r1 >> r2 >> r3 >> r4;
                    })) {
                return false;
            }
                return WaitAll();
            }

            bool GetAsyncResultHandler(unsigned short usReqId, std::pair<unsigned short, ResultHandler> &p);
            void OnRR(unsigned short reqId, CUQueue &mc);
            void OnSE(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            void SetNULL();
            void EraseBack(size_t count);

        protected:
            virtual bool SendRouteeResult(const unsigned char *buffer, unsigned int len, unsigned short reqId = 0);
            bool SendRouteeResult(unsigned short reqId = 0);
            bool SendRouteeResult(const CUQueue &mc, unsigned short reqId = 0);
            bool SendRouteeResult(const CScopeUQueue &sb, unsigned short reqId = 0);

            template <class ctype0>
            bool SendRouteeResult(const ctype0& data0, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1, class ctype2>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, const ctype1& data2, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1 << data2;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3 << data4;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

        private:
            CClientSocket *m_pClientSocket;
            unsigned int m_nServiceId;
            CUCriticalSection m_cs;
            typedef std::deque< std::pair<unsigned short, ResultHandler> > CRHVector;
            CRHVector m_vCallback;
            CRHVector m_aBatching;
            friend class CClientSocket;
        };

        template<typename THandler, typename TCS = CClientSocket>
        class CSocketPool {
        public:
            typedef std::shared_ptr<THandler> PHandler;
            typedef std::shared_ptr<TCS> PClientSocket;
			typedef std::tr1::function<bool(CSocketPool*, TCS*)> DDoSslAuthentication;

        private:

            struct cs_hash : public std::hash<PClientSocket> {

                inline size_t operator()(const PClientSocket & key) const {
                    return (size_t) key->GetHandle();
                }
            };

            struct cs_equal : public std::binary_function<PClientSocket, PClientSocket, bool> {

                inline bool operator() (const PClientSocket &s1, const PClientSocket & s2) const {
                    return (s1 == s2);
                }
            };

        public:
            typedef std::unordered_map<PClientSocket, PHandler, cs_hash, cs_equal> CMapSocketHandler;
			
        public:

            CSocketPool(bool autoConn = true,
                    unsigned int recvTimeout = DEFAULT_RECV_TIMEOUT,
                    unsigned int connTimeout = DEFAULT_CONN_TIMEOUT)
            : m_nPoolId(0), m_autoConn(autoConn), m_recvTimeout(recvTimeout), m_connTimeout(connTimeout) {
                if (!ClientCoreLoader.IsLoaded())
                    throw CUExCode("Client core library not accessible!", MB_BAD_OPERATION);
                CAutoLock al(m_csV);
                m_vPool.push_back(this);
            }

            virtual ~CSocketPool() {
                ShutdownPool();
                CAutoLock al(m_csV);
                for (auto it = m_vPool.begin(), e = m_vPool.end(); it != e; ++it) {
                    if (this == *it) {
                        m_vPool.erase(it);
                        break;
                    }
                }
            }

        private:
            CSocketPool(const CSocketPool &sp);
            CSocketPool& operator=(const CSocketPool &sp);

        public:

            static unsigned int GetNumberOfSocketPools() {
                return ClientCoreLoader.GetNumberOfSocketPools();
            }

			DDoSslAuthentication DoSslAuthentication;

            /// <summary>
            /// Seek an async handler on the min number of requests queued in memory and its associated socket connection
            /// </summary>
            /// <returns>An async handler if found; and null or nothing if no connection is found</returns>

            PHandler Seek() {
                PHandler h;
                CAutoLock al(m_cs);
                for (auto start = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); start != end; ++end) {
                    if (!start->first->IsConnected())
                        continue;
                    if (!h)
                        h = start->second;
                    else {
                        unsigned int count0 = h->GetAttachedClientSocket()->GetCountOfRequestsInQueue();
                        unsigned int count1 = start->first->GetCountOfRequestsInQueue();
                        if (count0 > count1)
                            h = start->second;
                    }
                }
                return h;
            }

            PHandler SeekByQueue(const std::string &queueName) {
                PHandler h;
                std::string rawName;
                if (0 == queueName.size())
                    return h;
#ifdef WIN32_64
                std::transform(queueName.begin(), queueName.end(), queueName.begin(), ::tolower);
#endif
                CAutoLock al(m_cs);
                for (auto start = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); start != end; ++end) {
                    IClientQueue &cq = start->first->GetClientQueue();
                    if (!cq.IsAvailable())
                        continue;
                    if (cq.IsSecure())
                        rawName = queueName + "_0_1.mqc";
                    else
                        rawName = queueName + "_0_0.mqc";

                    std::string queueFileName = cq.GetQueueFileName();
                    size_t len = queueFileName.size();
                    size_t lenRaw = rawName.size();
                    if (lenRaw > len)
                        continue;
                    size_t pos = queueFileName.rfind(rawName);

                    //queue file name with full path
                    if (pos == 0)
                        return start->second;

                    //queue raw name only
                    if ((pos + lenRaw) == len)
                        return start->second;
                }
                return h;
            }

            inline bool IsAvg() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.IsAvg(m_nPoolId);
            }

            inline unsigned int GetSocketsCreated() {
                CAutoLock al(m_cs);
                return (unsigned int) m_mapSocketHandler.size();
            }

            inline unsigned int GetPoolId() {
                CAutoLock al(m_cs);
                return m_nPoolId;
            }

            bool IsStarted() {
                CAutoLock al(m_cs);
                unsigned int count = ClientCoreLoader.GetThreadCount(m_nPoolId);
                return (count > 0);
            }

            bool AddOneThread() {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                return ClientCoreLoader.AddOneThreadIntoPool(poolId);
            }

            bool StartSocketPool(const CConnectionContext &cc, unsigned int socketsPerThread, unsigned int threads = 0, bool avg = true, tagThreadApartment ta = taNone) {
                bool ok;
                if (!StartSocketPool(socketsPerThread, threads, avg, ta))
                    return false;
                bool first = true;

                CAutoLock al(m_cs);
                unsigned int poolId = m_nPoolId;
                size_t size = m_mapSocketHandler.size();
                for (auto it = m_mapSocketHandler.cbegin(), end = m_mapSocketHandler.cend(); it != end; ++it) {
                    USocket_Client_Handle h = it->first->GetHandle();
                    if (ClientCoreLoader.IsOpened(h)) {
                        first = false;
                        continue;
                    }

                    PClientSocket cs = it->first;
                    cs->m_cc = cc;

                    ClientCoreLoader.SetUserID(h, cs->m_cc.UserId.c_str());
                    ClientCoreLoader.SetEncryptionMethod(h, cs->m_cc.EncrytionMethod);

                    //we build connections asynchronously for all the sockets except the very first one
                    if (first) {
                        m_cs.unlock();
                        ok = (ClientCoreLoader.Connect(h, cs->m_cc.Host.c_str(), cs->m_cc.Port, true, cs->m_cc.V6) && ClientCoreLoader.WaitAll(h, (~0)));
                        m_cs.lock();
                        if (poolId != m_nPoolId || size != m_mapSocketHandler.size()) {
                            //stop here under extremely cases that other threads have just done something 
                            //on this pool of sockets during connecting the first socket
                            break;
                        }
                    } else
                        ok = ClientCoreLoader.Connect(h, cs->m_cc.Host.c_str(), cs->m_cc.Port, false, cs->m_cc.V6);
                    if (ok)
                        first = false;
                }
                unsigned int count = ClientCoreLoader.GetConnectedSockets(poolId);
                return (count > 0);
            }

            PHandler Lock(unsigned int timeout = (~0)) {
                return Lock(timeout, NULL);
            }

            PHandler Lock(unsigned int timeout, PClientSocket csSameThread) {
                return Lock(timeout, csSameThread.get());
            }

            virtual PHandler Lock(unsigned int timeout, USocket_Client_Handle hSameThread) {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                USocket_Client_Handle h = ClientCoreLoader.LockASocket(poolId, timeout, hSameThread);
                if (!h)
                    return PHandler();
                return MapToHandler(h);
            }

            void Unlock(const THandler *h) {
                if (!h)
                    return;
                const CClientSocket *cs = h->GetAttachedClientSocket();
                Unlock(cs);
            }

            void Unlock(const PHandler &handler) {
                if (!handler)
                    return;
                const CClientSocket *cs = handler->GetAttachedClientSocket();
                Unlock(cs);
            }

            void Unlock(const PClientSocket &cs) {
                Unlock(cs.get());
            }

            virtual void Unlock(const CClientSocket *cs) {
                if (cs) {
                    Unlock(cs->GetHandle());
                }
            }

            virtual void Unlock(USocket_Client_Handle h) {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                bool ok = ClientCoreLoader.UnlockASocket(poolId, h);
            }

            void ShutdownPool() {
                bool ok = ClientCoreLoader.DestroySocketPool(m_nPoolId);
                CAutoLock al(m_cs);
                m_nPoolId = 0;
            }

            inline unsigned int GetThreadsCreated() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetThreadCount(m_nPoolId);
            }

            inline unsigned int GetDisconnectedSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetDisconnectedSockets(m_nPoolId);
            }

            inline unsigned int GetSocketsPerThread() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetSocketsPerThread(m_nPoolId);
            }

            inline unsigned int GetLockedSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetLockedSockets(m_nPoolId);
            }

            inline unsigned int GetIdleSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetIdleSockets(m_nPoolId);
            }

            inline unsigned int GetConnectedSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetConnectedSockets(m_nPoolId);
            }

            inline bool DisconnectAll() {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                return ClientCoreLoader.DisconnectAll(poolId);
            }

            PHandler FindClosedOne() {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (!it->first->IsConnected() && it->second.unique())
                        return it->second;
                }
                return PHandler();
            }

        protected:

            virtual void OnSocketPoolEvent(tagSocketPoolEvent spe, const PHandler &handler) {

            }

        private:

            bool StartSocketPool(unsigned int socketsPerThread, unsigned int threads = 0, bool avg = true, tagThreadApartment ta = taNone) {
                if (IsStarted())
                    return true;
                unsigned int poolId = ClientCoreLoader.CreateSocketPool(&CSocketPool::SPE, socketsPerThread, threads, avg, ta);
                CAutoLock al(m_cs);
                m_nPoolId = poolId;
                return true;
            }

            PHandler MapToHandler(USocket_Client_Handle h) {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetHandle() == h)
                        return it->second;
                }
                return PHandler();
            }

            PClientSocket MapToSocket(USocket_Client_Handle h) {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetHandle() == h)
                        return it->first;
                }
                return PClientSocket();
            }

            static void Set(PClientSocket &cs, USocket_Client_Handle h) {
                cs->Set(h);
            }

            static PClientSocket CreateEmptySocket() {
                return PClientSocket(new TCS());
            }

            static CSocketPool *Seek(unsigned int poolId) {
                CAutoLock al(m_csV);
                for (auto it = m_vPool.begin(), e = m_vPool.end(); it != e; ++it) {
                    if ((*it)->GetPoolId() == poolId) {
                        return *it;
                    }
                }
                return NULL;
            }

            static void CALLBACK SPE(unsigned int poolId, SPA::ClientSide::tagSocketPoolEvent spe, USocket_Client_Handle h) {
                CSocketPool *sp = Seek(poolId);
                switch (spe) {
                    case speStarted:
                    {
                        CAutoLock al(m_csV);
                        for (auto it = m_vPool.rbegin(), end = m_vPool.rend(); it != end; ++it) {
                            if ((*it)->m_nPoolId == 0) {
                                sp = *it;
                                break;
                            }
                        }
                        sp->m_nPoolId = poolId;
                    }
                        break;
                    case speUSocketCreated:
                    {
                        PClientSocket clientSocket = CSocketPool<THandler, TCS>::CreateEmptySocket();
                        Set(clientSocket, h);
                        ClientCoreLoader.SetRecvTimeout(h, sp->m_recvTimeout);
                        ClientCoreLoader.SetConnTimeout(h, sp->m_connTimeout);
                        ClientCoreLoader.SetAutoConn(h, sp->m_autoConn);
                        PHandler handler(new THandler(clientSocket.get()));
                        CAutoLock al(sp->m_cs);
                        sp->m_mapSocketHandler[clientSocket] = handler;
                    }
                        break;
                    case speUSocketKilled:
                    {
                        CAutoLock al(sp->m_cs);
                        for (auto it = sp->m_mapSocketHandler.cbegin(), end = sp->m_mapSocketHandler.cend(); it != end; ++it) {
                            if (it->first->GetHandle() == h) {
                                sp->m_mapSocketHandler.erase(it);
                                break;
                            }
                        }
                    }
                        break;
                    case speConnecting:
                        ClientCoreLoader.SetPassword(h, sp->MapToSocket(h)->m_cc.Password.c_str());
                        break;
                    case speConnected:
                        if (ClientCoreLoader.IsOpened(h)) {
                            ClientCoreLoader.SetSockOpt(h, soRcvBuf, 116800, slSocket);
                            ClientCoreLoader.SetSockOpt(h, soSndBuf, 116800, slSocket);
							if (sp->DoSslAuthentication) {
								if (ClientCoreLoader.GetEncryptionMethod(h) == TLSv1 && !sp->DoSslAuthentication(sp, sp->MapToSocket(h).get()))
									return;
							}
                            bool ok = ClientCoreLoader.StartBatching(h);
                            ok = ClientCoreLoader.SwitchTo(h, sp->MapToHandler(h)->GetSvsID());
                            if (ok) {
                                ok = ClientCoreLoader.TurnOnZipAtSvr(h, sp->MapToSocket(h)->m_cc.Zip);
                                ok = ClientCoreLoader.SetSockOptAtSvr(h, soRcvBuf, 116800, slSocket);
                                ok = ClientCoreLoader.SetSockOptAtSvr(h, soSndBuf, 116800, slSocket);
                            }
                            ok = ClientCoreLoader.CommitBatching(h, false);
                        }
                        break;
                    default:
                        break;
                }
                sp->OnSocketPoolEvent(spe, sp->MapToHandler(h));
            }

        private:
            unsigned int m_nPoolId; //locked by m_cs
            bool m_autoConn;
            unsigned int m_recvTimeout;
            unsigned int m_connTimeout;
            CMapSocketHandler m_mapSocketHandler; //locked by m_cs
            CUCriticalSection m_cs;

            static CUCriticalSection m_csV;
            static std::vector<CSocketPool*> m_vPool;
        };

        template<typename THandler, typename TCS>
        CUCriticalSection CSocketPool<THandler, TCS>::m_csV;

        template<typename THandler, typename TCS>
        std::vector<CSocketPool<THandler, TCS>* > CSocketPool<THandler, TCS>::m_vPool;

		struct ReplicationSetting {
			/// <summary>
			/// An absolute path to a directory containing message queue files.
			/// </summary>
			std::string QueueDir;

			/// <summary>
			/// A password string used for encrypting source message queue. It will be ignored if either QueueDir is not valid path to a directory or none remote SocketPro server is secured.
			/// </summary>
			std::string Password;

			/// <summary>
			/// False for auto socket conencting. Otherwise, there is no auto connection.
			/// </summary>
			bool NoAutoConn;

			/// <summary>
			/// Time-to-live in seconds. It is ignored if persistent message queue feature is not used. If the value is not set or zero, the value will default to CSqlReplication<THandler>.DEFAULT_TTL (30 days).
			/// </summary>
			unsigned int TTL;

			/// <summary>
			/// A timeout for receiving result from remote SocketPro server. If the value is not set or it is zero, the value will defalut to CSocketPool<THandler>.DEFAULT_RECV_TIMEOUT (30 seconds).
			/// </summary>
			unsigned int RecvTimeout;

			/// <summary>
			/// A timeout for conneting to remote SocketPro server. If the value is not set or it is zero, the value will defalut to CSocketPool<THandler>.DEFAULT_CONN_TIMEOUT (30 seconds).
			/// </summary>
			unsigned int ConnTimeout;
		};

		template<typename THandler, typename TCS = CClientSocket>
		class CReplication
		{
		private:
#ifdef WIN32_64
			const static char dir_sep = '\\';
#else
			const static char dir_sep = '/';
#endif
		public:
			typedef std::unordered_map<CConnectionContext, std::string> CMapConnQueueFile;
			const static unsigned int DEAFULT_TTL = (unsigned int)(720 * 3600); //30 days

			CReplication(const CMapConnQueueFile &mapConnQueue, const ReplicationSetting &qms) : m_ttl(DEAFULT_TTL) {
				m_pool.DoSslAuthentication = [this](CSocketPool<THandler, TCS> *sp, TCS *cs) -> bool {
					return this->DoSslServerAuthentication(cs);
				};
				CheckInputs(mapConnQueue, qms);

			}

		protected:
			CReplication() :  m_ttl(DEAFULT_TTL) { }
			virtual bool DoSslServerAuthentication(TCS *cs) {
				return true;
			}

		private:
			CReplication(const CReplication& rep);
			CReplication& operator=(const CReplication& rep);

			void CheckInputs(const CMapConnQueueFile &mapConnQueue, const ReplicationSetting &qms) {
				if (mapConnQueue.size() == 0)
					throw CUExCode("Cannection-queue name mapping can't be empty!", MB_BAD_INPUT);
				for (auto it = mapConnQueue.begin(), end = mapConnQueue.end(); it != end; ++it) {
					if (it->second.size() == 0)
						throw CUExCode("Queue name can not be empty!", MB_BAD_INPUT);
					const CConnectionContext &cc = it->first;
					const std::string &str = it->second;
					m_mapConnQueue[cc] = str;
				}
#ifdef WINCE

#else
				if (qms.QueueDir.size() == 0)
					throw CUExCode("Queue directory can not be empty!", MB_BAD_INPUT);
	#ifdef WIN32_64
				CScopeUQueue su;
				SPA::Utilities::ToWide(qms.QueueDir.c_str(), qms.QueueDir.size(), *su);
				DWORD dwAttrib = GetFileAttributesW((const wchar_t*)su->GetBuffer());
				if ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					CreateDirectoryW((const wchar_t*)su->GetBuffer(), NULL);
	#else
				struct stat info;
				if (stat(qms.QueueDir.c_str(), &info) || (info.st_mode & S_IFDIR) != S_IFDIR)
					mkdir(qms.QueueDir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	#endif	
#endif
				if (qms.TTL == 0)
					m_ttl = DEAFULT_TTL;
				else
					m_ttl = qms.TTL;
				
				m_queueDir = qms.QueueDir;
#ifdef WIN32_64
				std::transform(m_queueDir.begin(), m_queueDir.end(), m_queueDir.begin(), ::tolower);

#endif

			}

		private:
			CSocketPool<THandler, TCS> m_pool;
			CMapConnQueueFile m_mapConnQueue;
			std::string m_queueDir;
			unsigned int m_ttl;
			CUCriticalSection m_cs;
		};
    }; //ClientSide
}; //SPA

#endif

