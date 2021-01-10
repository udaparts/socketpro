#ifndef __UMB_CLIENT_WRAPPER_H_
#define __UMB_CLIENT_WRAPPER_H_

#include "ccloader.h"
#include "ucomm.h"
#include "membuffer.h"
#include <unordered_map>
#include <memory>
#include <functional>

#ifdef HAVE_FUTURE
#include <future>
#ifdef WIN32_64

#if defined(_COROUTINE_)
#define HAVE_COROUTINE 2
namespace SPA {
    namespace ClientSide {
        typedef std::coroutine_handle<> CRHandle;
    }
}
#elif defined(_EXPERIMENTAL_RESUMABLE_)
#define HAVE_COROUTINE 1
namespace SPA {
    namespace ClientSide {
        typedef std::experimental::coroutine_handle<> CRHandle;
    }
}
#endif
#else
#ifdef _GLIBCXX_COROUTINE
#define HAVE_COROUTINE 2
namespace SPA {
    namespace ClientSide {
        typedef std::coroutine_handle<> CRHandle;
    }
}
#endif
#endif
#endif

#ifdef PHP_ADAPTER_PROJECT
#define NO_MIDDLE_TIER
#include <cctype>
#elif defined NODE_JS_ADAPTER_PROJECT
#include "udatabase.h"
#define NO_MIDDLE_TIER
#include <cctype>
#ifdef WIN32_64
//warning C4251: 'node::CallbackScope::try_catch_': class 'v8::TryCatch' needs to have dll-interface to be used by clients
#pragma warning(disable: 4251)
//warning C4275: non dll-interface class 'v8::Platform' used as base for dll-interface class 'node::MultiIsolatePlatform'
#pragma warning(disable: 4275)
#else

#endif

#include <deque>

#include <uv.h>
#include <v8.h>

namespace v8 {

    template<class T>
    class NJANonCopyablePersistentTraits {
    public:
        typedef Persistent<T, NJANonCopyablePersistentTraits<T> > NJANonCopyablePersistent;
        static const bool kResetInDestructor = true;

        template<class S, class M>
        V8_INLINE static void Copy(const Persistent<S, M>& source, NJANonCopyablePersistent* dest) {
            Uncompilable<Object>();
        }

        template<class O> V8_INLINE static void Uncompilable() {
#ifdef WIN32_64
            TYPE_CHECK(O, Primitive);
#endif
        }
    };
};

using v8::Local;
using v8::Object;
using v8::Isolate;
using v8::Value;
using v8::FunctionCallbackInfo;
using v8::Persistent;
using v8::Function;
using v8::Exception;
using v8::String;
using v8::Number;
using v8::Boolean;
using v8::Uint32;
using v8::HandleScope;
using v8::Int32;
using v8::Array;

namespace NJA {
    class NJSocketPool;
    void ThrowException(Isolate* isolate, const char *str);
    Local<Value> From(Isolate* isolate, const VARIANT &vt);
    Local<Value> DbFrom(Isolate* isolate, SPA::CUQueue &buff);
    Local<String> ToStr(Isolate* isolate, const char *str, size_t len = (size_t) INVALID_NUMBER);
    Local<String> ToStr(Isolate* isolate, const SPA::UTF16 *str, size_t len = (size_t) INVALID_NUMBER);
    bool IsNullOrUndefined(const Local<Value> &v);
}
#endif

//this may be used for debug
#define SET_CLIENT_CALL_INFO(str) SPA::ClientSide::SetLastCallInfo(str, __LINE__, __FUNCTION__)

namespace SPA {
    namespace ClientSide {

        //this may be used for debug
        void SetLastCallInfo(const char *str, int data, const char *func);

        extern CUCriticalSection g_csSpPool;

        class CAsyncServiceHandler;
        class CAsyncResult;

        typedef std::function<void(CAsyncResult&) > DResultHandler;
        typedef DResultHandler ResultHandler; //ResultHandler will be removed in the near future

        const static DResultHandler NULL_RH;

        struct ErrInfo {

            ErrInfo(int errCode = 0, const wchar_t *errMsg = L"")
            : ec(errCode), em(errMsg ? errMsg : L"") {
            }

            int ec; //error code, 0 -- success
            std::wstring em; //error message

            virtual std::wstring ToString() {
                std::wstring s = L"ec: " + std::to_wstring(ec);
                s += (L", em: " + em);
                return s;
            }
        };

        struct CServerError : public ErrInfo {

            CServerError(int errCode, const wchar_t *errMsg, const char *stack, unsigned req_id)
            : ErrInfo(errCode, errMsg), location(stack ? stack : ""), reqId(req_id) {
            }

            std::string location;
            unsigned short reqId;

            std::wstring ToString() {
                std::wstring s = ErrInfo::ToString();
                s += (L", where: " + Utilities::ToWide(location));
                s += (L", reqId: " + std::to_wstring(reqId));
                return s;
            }
        };

        struct CSocketError : public ErrInfo {

            CSocketError(int errCode, const wchar_t* errMsg, short req_id, bool before_call)
            : ErrInfo(errCode, errMsg), reqId(req_id), before(before_call) {
            }

            unsigned short reqId;
            bool before;

            virtual std::wstring ToString() {
                std::wstring s = ErrInfo::ToString();
                s += (L", reqId: " + std::to_wstring(reqId));
                s += (std::wstring(L", before: ") + (before ? L"true" : L"false"));
                return s;
            }
        };

        class CAsyncResult {
        private:

            CAsyncResult(CAsyncServiceHandler *pAsyncServiceHandler, unsigned short ReqId, CUQueue &q, DResultHandler & rh)
            : AsyncServiceHandler(pAsyncServiceHandler), RequestId(ReqId), UQueue(q), CurrentAsyncResultHandler(rh) {
            }

        public:

            template<class ctype >
            inline CUQueue& operator>>(ctype & receiver) {
                UQueue >> receiver;
                return UQueue;
            }

            template<typename R>
            inline R Load() {
                R r;
                UQueue >> r;
                return std::move(r);
            }

        public:
            CAsyncServiceHandler *AsyncServiceHandler;
            unsigned short RequestId;
            CUQueue &UQueue;
            DResultHandler &CurrentAsyncResultHandler;

        private:
            CAsyncResult(const CAsyncResult & ar) = delete;
            CAsyncResult& operator=(const CAsyncResult & ar) = delete;

            friend class CAsyncServiceHandler;
        };

        template<typename Del>
        struct IUDel {
            virtual void operator+=(const Del& d) = 0;
            virtual void operator-=(const Del& d) = 0;
            virtual void operator=(const Del& d) = 0;
            virtual size_t Count() = 0;
            virtual operator bool() = 0;
        };

        template<typename Del>
        struct IUDelImpl : public IUDel<Del> {

            IUDelImpl(CSpinLock *cs = nullptr) : m_cs(cs) {
            }

            virtual void operator+=(const Del& d) {
                if (d) {
                    m_cs->lock();
                    m_vD.push_back(d);
                    m_cs->unlock();
                }
            }

            virtual void operator-=(const Del& d) {
                if (d) {
                    m_cs->lock();
                    m_vD.push_back(d);
                    m_cs->unlock();
                }
            }

            virtual void operator=(const Del& d) {
                m_cs->lock();
                m_vD.clear();
                if (d) {
                    m_vD.push_back(d);
                }
                m_cs->unlock();
            }

            virtual size_t Count() {
                m_cs->lock();
                auto size = m_vD.size();
                m_cs->unlock();
                return size;
            }

            virtual operator bool() {
                m_cs->lock();
                auto size = m_vD.size();
                m_cs->unlock();
                return (size > 0);
            }

            template<typename P0>
            void Invoke(const P0& p0) {
                CSpinAutoLock al(*m_cs);
                for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                    auto &d = *it;
                    d(p0);
                }
            }

            template<typename P0, typename P1>
            void Invoke(const P0& p0, const P1& p1) {
                CSpinAutoLock al(*m_cs);
                for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                    auto &d = *it;
                    d(p0, p1);
                }
            }

            template<typename P0, typename P1, typename P2>
            void Invoke(const P0& p0, const P1& p1, const P2& p2) {
                CSpinAutoLock al(*m_cs);
                for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                    auto &d = *it;
                    d(p0, p1, p2);
                }
            }

            template<typename P0, typename P1, typename P2, typename P3>
            void Invoke(const P0& p0, const P1& p1, const P2& p2, const P3& p3) {
                CSpinAutoLock al(*m_cs);
                for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                    auto &d = *it;
                    d(p0, p1, p2, p3);
                }
            }

            template<typename P0, typename P1, typename P2, typename P3, typename P4>
            void Invoke(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4) {
                CSpinAutoLock al(*m_cs);
                for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                    auto &d = *it;
                    d(p0, p1, p2, p3, p4);
                }
            }

            template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
            void Invoke(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5) {
                CSpinAutoLock al(*m_cs);
                for (auto it = m_vD.cbegin(), end = m_vD.cend(); it != end; ++it) {
                    auto &d = *it;
                    d(p0, p1, p2, p3, p4, p5);
                }
            }

        public:

            void SetCS(CSpinLock *cs) {
                m_cs = cs;
            }

        protected:
            CSpinLock *m_cs;
            std::vector<Del> m_vD;

        private:
            IUDelImpl(const IUDelImpl &impl);
            IUDelImpl& operator=(const IUDelImpl &impl);
        };

#ifdef NODE_JS_ADAPTER_PROJECT

        static CUQueue& operator<<(CUQueue &q, const CMessageSender &ms) {
            q << ms.UserId << ms.IpAddress << ms.Port << ms.ServiceId << ms.SelfMessage;
            return q;
        }

        static Local<Object> ToMessageSender(Isolate *isolate, CUQueue &q) {
            SPA::CDBString user;
            std::string ipAddr;
            unsigned short Port;
            unsigned int ServiceId;
            bool SelfMessage;
            q >> user >> ipAddr >> Port >> ServiceId >> SelfMessage;
            auto ctx = isolate->GetCurrentContext();
            Local<Object> obj = Object::New(isolate);
            obj->Set(ctx, NJA::ToStr(isolate, u"UserId", 6), NJA::ToStr(isolate, user.c_str(), user.size()));
            obj->Set(ctx, NJA::ToStr(isolate, u"IpAddr", 6), NJA::ToStr(isolate, ipAddr.c_str(), ipAddr.size()));
            obj->Set(ctx, NJA::ToStr(isolate, u"Port", 4), Uint32::New(isolate, Port));
            obj->Set(ctx, NJA::ToStr(isolate, u"SvsId", 5), Number::New(isolate, ServiceId));
            obj->Set(ctx, NJA::ToStr(isolate, u"Self", 4), Boolean::New(isolate, SelfMessage));
            return obj;
        }
#endif

        struct CConnectionContext {

            CConnectionContext() noexcept
            : Port(0),
            EncrytionMethod(tagEncryptionMethod::NoEncryption),
            V6(false),
            Zip(false) {

            }

            CConnectionContext(const char *addr, unsigned short port, const wchar_t *userId, const wchar_t *pwd, tagEncryptionMethod em = tagEncryptionMethod::NoEncryption, bool v6 = false, bool zip = false)
            : Host(addr ? addr : ""),
            Port(port),
            UserId(userId ? userId : L""),
            Password(pwd ? pwd : L""),
            EncrytionMethod(em),
            V6(v6),
            Zip(zip) {
            }

#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
            //not accurate but better than nothing here
            //Client core internal checking works much better, starting from version 6.2.0.4

            bool operator==(const CConnectionContext &cc) const {
                if (this == &cc)
                    return true;
                return (Port == cc.Port && Host.size() == cc.Host.size() &&
                        std::equal(Host.begin(), Host.end(), cc.Host.begin(), [](char a, char b) {
                            return std::tolower(a) == std::tolower(b); }));
            }
#else

            bool operator==(const CConnectionContext &cc) const noexcept {
                if (this == &cc)
                    return true;
                return (Host == cc.Host &&
                        Port == cc.Port &&
                        UserId == cc.UserId &&
                        Password == cc.Password &&
                        EncrytionMethod == cc.EncrytionMethod &&
                        V6 == cc.V6 &&
                        Zip == cc.Zip &&
                        IsEqual(AnyData, cc.AnyData));
            }
#endif
            std::string Host;
            unsigned int Port;
            std::wstring UserId;
            std::wstring Password;
            tagEncryptionMethod EncrytionMethod;
            bool V6;
            bool Zip;
            UVariant AnyData;
        };

        struct IClientQueue : public IMessageQueueBasic {
            virtual bool StartQueue(const char *qName, unsigned int ttl, bool secure = true, bool dequeueShared = false) const = 0;
            virtual bool IsDequeueEnabled() const = 0;
            virtual bool AppendTo(const IClientQueue & clientQueue) const = 0;
            virtual bool AppendTo(const USocket_Client_Handle *handles, unsigned int count) const = 0;
            virtual bool EnsureAppending(const IClientQueue & clientQueue) const = 0;
            virtual bool EnsureAppending(const USocket_Client_Handle *handles, unsigned int count) const = 0;
            virtual void EnableRoutingQueueIndex(bool enable) const = 0;
            virtual bool IsRoutingQueueIndexEnabled() const = 0;
            virtual USocket_Client_Handle GetHandle() const = 0;
        };

        class CClientSocket {
        public:
            virtual ~CClientSocket();

        protected:
            CClientSocket() noexcept;

        private:
            bool Attach(CAsyncServiceHandler *p);
            void Detach(CAsyncServiceHandler *p);
            void Set(USocket_Client_Handle h);

        private:
            CClientSocket(const CClientSocket &cs) = delete;
            CClientSocket& operator=(const CClientSocket &cs) = delete;

            class CQueueImpl : public IClientQueue {
            public:

                CQueueImpl() : m_hSocket(0), m_nQIndex(0) {
                }
                CQueueImpl(const CQueueImpl& qi) = delete;
                CQueueImpl& operator=(const CQueueImpl& qi) = delete;

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
                void EnableRoutingQueueIndex(bool enable) const;
                bool IsRoutingQueueIndexEnabled() const;
                tagOptimistic GetOptimistic() const;
                void SetOptimistic(tagOptimistic optimistic) const;

            public:
                USocket_Client_Handle m_hSocket;

            private:
                unsigned int m_nQIndex;
            };
        public:

            class CPushImpl : public IPushEx {
            private:

                CPushImpl() noexcept
                : m_cs(nullptr), OnSendUserMessage(m_lstUser), OnSendUserMessageEx(m_lstUserEx),
                OnPublish(m_lstPublish), OnPublishEx(m_lstPublishEx), OnSubscribe(m_lstSub),
                OnUnsubscribe(m_lstUnsub) {
                }

            public:
                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int) > DOnSubscribe;
                typedef std::function<void(CClientSocket*, const CMessageSender&, const SPA::UVariant&) > DOnSendUserMessage;
                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned char*, unsigned int) > DOnSendUserMessageEx;
                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int, const SPA::UVariant&) > DOnPublish;
                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int, const unsigned char*, unsigned int) > DOnPublishEx;
                typedef DOnSubscribe DOnUnsubscribe;

            public:
                virtual bool Subscribe(const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool Publish(const VARIANT& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const;
                virtual bool PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool SendUserMessage(const VARIANT& vtMessage, const wchar_t *strUserId) const;
                virtual bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const;
                virtual void Unsubscribe() const;

            private:
                CPushImpl(const CPushImpl &p) = delete;
                CPushImpl& operator=(const CPushImpl &p) = delete;

            private:
                CClientSocket *m_cs;
                friend class CClientSocket;
                IUDelImpl<DOnSendUserMessage> m_lstUser;
                IUDelImpl<DOnSendUserMessageEx> m_lstUserEx;
                IUDelImpl<DOnPublish> m_lstPublish;
                IUDelImpl<DOnPublishEx> m_lstPublishEx;
                IUDelImpl<DOnSubscribe> m_lstSub;
                IUDelImpl<DOnSubscribe> m_lstUnsub;

            public:
                IUDel<DOnSendUserMessage>& OnSendUserMessage;
                IUDel<DOnSendUserMessageEx>& OnSendUserMessageEx;
                IUDel<DOnPublish>& OnPublish;
                IUDel<DOnPublishEx>& OnPublishEx;
                IUDel<DOnSubscribe>& OnSubscribe;
                IUDel<DOnUnsubscribe>& OnUnsubscribe;
            };

        public:
            //the version of client core library
            static const char* GetVersion();

            //persistant message queue

            struct QueueConfigure {
                static bool IsClientQueueIndexPossiblyCrashed();
                static void SetWorkDirectory(const char *dir);
                static const char* GetWorkDirectory();
                static void SetMessageQueuePassword(const char *pwd);
            };

            //openssl server certificate verification

            struct SSL {
                static bool SetVerifyLocation(const char *caFile);
                static void SetCertificateVerifyCallback(PCertificateVerifyCallback cvc);
            };

            void Close() const;
            void Shutdown(tagShutdownType st = tagShutdownType::stBoth) const;
            bool operator==(const CClientSocket &cs) const noexcept;
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
            tagConnectionState GetConnectionState() const;
            void* GetSslHandle() const;
            bool IgnoreLastRequest(unsigned short reqId) const;
            unsigned int GetRouteeCount() const;

            inline bool IsRouting() const noexcept {
                return m_routing;
            }
            unsigned int GetCountOfRequestsInQueue() const;
            unsigned short GetCurrentRequestID() const;

            inline unsigned int GetCurrentServiceID() const noexcept {
                return m_nCurrSvsId;
            }
            unsigned short GetServerPingTime() const;
            unsigned int GetCurrentResultSize() const;
            tagEncryptionMethod GetEncryptionMethod() const;
            int GetErrorCode() const;
            std::string GetErrorMsg() const;
            bool IsConnected() const;
            void SetEncryptionMethod(tagEncryptionMethod em) const;

            inline USocket_Client_Handle GetHandle() const {
                return m_hSocket;
            }

            unsigned int GetPoolId() const;
            const CConnectionContext& GetConnectionContext() const noexcept;
            static CClientSocket* Seek(USocket_Client_Handle h) noexcept;

            //If socket is closed, batching requests or timed out, it will return false
            bool WaitAll(unsigned int nTimeout = (~0)) const;
            bool Cancel(unsigned int requestsQueued = (~0)) const;

            inline bool IsRandom() const noexcept {
                return m_bRandom;
            }
            unsigned int GetBytesInSendingBuffer() const;
            unsigned int GetBytesInReceivingBuffer() const;
            unsigned int GetBytesBatched() const;
            UINT64 GetBytesReceived() const;
            UINT64 GetBytesSent() const;
            void SetUID(const wchar_t *userId) const;
            std::wstring GetUID() const;
            void SetPassword(const wchar_t *password) const;
            UINT64 GetSocketNativeHandle() const;
            CPushImpl& GetPush() noexcept;
            IClientQueue& GetClientQueue() noexcept;
            tagOperationSystem GetPeerOs(bool *endian = nullptr) const;

            bool DoEcho() const;
            bool SetSockOpt(tagSocketOption optName, int optValue, tagSocketLevel level = tagSocketLevel::slSocket) const;
            bool SetSockOptAtSvr(tagSocketOption optName, int optValue, tagSocketLevel level = tagSocketLevel::slSocket) const;
            bool TurnOnZipAtSvr(bool enableZip) const;
            bool SetZipLevelAtSvr(SPA::tagZipLevel zipLevel) const;
            std::string GetPeerName(unsigned int *port) const;

            typedef std::function<void(CClientSocket*, int) > DSocketEvent;
            typedef std::function<void(CClientSocket*, unsigned short, const wchar_t*, const char*, unsigned int) > DExceptionFromServer;

        protected:
            virtual void OnSocketClosed(int nError);
            virtual void OnHandShakeCompleted(int nError);
            virtual void OnSocketConnected(int nError);
            virtual void OnRequestProcessed(unsigned short requestId, CUQueue &q);
            virtual void OnSubscribe(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count);
            virtual void OnUnsubscribe(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count);
            virtual void OnPublish(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count, const SPA::UVariant &vtMessage);
            virtual void OnPublishEx(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            virtual void OnSendUserMessage(const CMessageSender& sender, const SPA::UVariant &message);
            virtual void OnSendUserMessageEx(const CMessageSender& sender, const unsigned char *pMessage, unsigned int size);
            virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
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
            static void WINAPI OnPostProcessing(USocket_Client_Handle handler, unsigned int hint, SPA::UINT64 data);

            CAsyncServiceHandler *GetCurrentHandler() noexcept;

        private:
            CSpinLock m_cs;
            USocket_Client_Handle m_hSocket;
            CPushImpl m_PushImpl;
            CAsyncServiceHandler *m_pHandler;
            CQueueImpl m_QueueImpl;
            CConnectionContext m_cc;
            bool m_bRandom;
            bool m_endian;
            tagOperationSystem m_os;
            unsigned int m_nCurrSvsId;
            bool m_routing;
            CUQueue m_qRecv;
            static CSpinLock m_mutex;
            static std::vector<CClientSocket*> m_vClientSocket;

            template<typename THandler, typename TCS>
            friend class CSocketPool;
            friend class CAsyncServiceHandler;
            friend class CPushImpl;
#ifdef NODE_JS_ADAPTER_PROJECT
            uv_async_t *m_asyncType;
            friend class NJA::NJSocketPool;
#endif
            IUDelImpl<DSocketEvent> m_implClosed;
            IUDelImpl<DSocketEvent> m_implHSC;
            IUDelImpl<DSocketEvent> m_implConnected;
            IUDelImpl<DExceptionFromServer> m_implEFS;

        public:
            IUDel<DSocketEvent>& SocketClosed;
            IUDel<DSocketEvent>& HandShakeCompleted;
            IUDel<DSocketEvent>& SocketConnected;
            IUDel<DExceptionFromServer>& ExceptionFromServer;

#ifdef ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
        public:
            typedef std::function<void(CClientSocket*, unsigned short) > DRequestEvent;
            typedef std::function<void(CClientSocket*, unsigned short, CUQueue &) > DRequestProcessed;

        private:
            IUDelImpl<DRequestEvent> m_implBRP;
            IUDelImpl<DRequestEvent> m_implARP;

            struct CIRequestProcessed : public IUDelImpl<DRequestProcessed> {
                void Invoke(CClientSocket *cs, unsigned short reqId, CUQueue &mc);
            };
            CIRequestProcessed m_implRP;

        public:
            IUDel<DRequestEvent>& BaseRequestProcessed;
            IUDel<DRequestEvent>& AllRequestsProcessed;
            IUDel<DRequestProcessed>& RequestProcessed;
#endif
        };

        typedef CClientSocket* PClientSocket;

        class CAsyncServiceHandler {
            SPA::CScopeUQueue m_suCallback;
            SPA::CScopeUQueue m_suBatching;
            static void CleanQueue(CUQueue &q);

        public:

            const static int SESSION_CLOSED_AFTER = -1000;
            const static int SESSION_CLOSED_BEFORE = -1001;
            const static int REQUEST_CANCELED = -1002;
            const static UINT64 DEFAULT_INTERRUPT_OPTION = 1;
            static const wchar_t* const SESSION_CLOSED_BEFORE_ERR_MSG;
            static const wchar_t* const SESSION_CLOSED_AFTER_ERR_MSG;
            static const wchar_t* const REQUEST_CANCELED_ERR_MSG;

            virtual ~CAsyncServiceHandler();

            typedef std::function<void(CAsyncServiceHandler *ash, unsigned short reqId) > DBaseRequestProcessed;
            typedef std::function<bool(CAsyncServiceHandler *ash, unsigned short reqId, CUQueue& mb) > DResultReturned;
            typedef std::function<void(CAsyncServiceHandler *ash, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) > DServerException;

            typedef std::function<void(CAsyncServiceHandler *ash, bool canceled) > DDiscarded;
            static DServerException NULL_SE;
            static DDiscarded NULL_ABORTED;

        protected:
            CAsyncServiceHandler(unsigned int nServiceId, CClientSocket *cs);

        private:
            CAsyncServiceHandler(const CAsyncServiceHandler&) = delete;
            CAsyncServiceHandler& operator=(const CAsyncServiceHandler&) = delete;

            struct CResultCb {

                CResultCb(const DResultHandler &rh, const DDiscarded& discarded, const DServerException &exceptionFromServer)
                : AsyncResultHandler(rh), Discarded(discarded), ExceptionFromServer(exceptionFromServer) {
                }

                //no copy constructor or assignment operator
                CResultCb(const CResultCb &rcb) = delete;
                CResultCb& operator=(const CResultCb &rcb) = delete;

                DResultHandler AsyncResultHandler;
                DDiscarded Discarded;
                DServerException ExceptionFromServer;
            };
            typedef std::pair<unsigned short, CResultCb*> *PRR_PAIR;

            class CRR : public CSafeDeque<PRR_PAIR> {
            public:

                inline PRR_PAIR Reuse() noexcept {
                    PRR_PAIR pp;
                    if (!pop_front(pp)) {
                        pp = nullptr;
                    }
                    return pp;
                }

                inline void Recycle(PRR_PAIR p) {
                    if (p) {
                        push_front(p);
                    }
                }

                void ClearResultCallbackPool(size_t remaining) {
                    CSpinAutoLock al(m_sl);
                    if (remaining > m_count) {
                        remaining = m_count;
                    }
                    PRR_PAIR *start = m_p + m_header;
                    for (size_t it = remaining; it < m_count; ++it) {
                        PRR_PAIR p = start[it];
                        delete p->second;
                        delete p;
                    }
                    m_count = remaining;
                    if (!m_count) {
                        m_header = 0;
                    }
                }
            };
            static CRR m_rrStack;

        protected:
            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc);
            virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            virtual void OnBaseRequestprocessed(unsigned short reqId);
            virtual void OnAllProcessed();
            virtual void OnInterrupted(UINT64 options);

        public:

#if defined(MONITORING_SPIN_CONTENTION) && defined(ATOMIC_AVAILABLE)

            UINT64 GetContention() noexcept {
                return m_cs.Contention; //m_vBatching & m_vCallback
            }

            UINT64 GetCbContention() noexcept {
                return m_csCb.Contention; //callbacks
            }

            static UINT64 GetCacheContention() noexcept {
                return m_rrStack.contention(); //m_rrStack
            }
#endif
            unsigned int GetRequestsQueued() noexcept;
            void ShrinkDeque();

            inline unsigned int GetSvsID() const noexcept {
                return m_nServiceId;
            }
            void SetSvsID(unsigned int serviceId) noexcept;
            virtual bool SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int size, const DResultHandler& rh, const DDiscarded& discarded = nullptr, const DServerException& serverException = nullptr);
            bool SendRequest(unsigned short reqId, const DResultHandler& rh = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr);

            inline CClientSocket *GetAttachedClientSocket() noexcept {
                return m_pClientSocket;
            }

            inline CClientSocket *GetSocket() noexcept {
                return m_pClientSocket;
            }
            virtual bool WaitAll(unsigned int timeOut = (~0));
            virtual bool Interrupt(UINT64 options);
            bool StartBatching();
            bool CommitBatching(bool bBatchingAtServerSide = false);
            bool AbortBatching();
            bool IsBatching();
            virtual unsigned int CleanCallbacks();
            bool IsDequeuedResult();
            void AbortDequeuedMessage();
            bool IsDequeuedMessageAborted();
            bool IsRouteeRequest();
            static void ClearResultCallbackPool(unsigned int remaining);
            static unsigned int CountResultCallbacksInPool() noexcept;
            static UINT64 GetCallIndex() noexcept;

            template<typename ...Ts>
            bool SendRequest(unsigned short reqId, const DResultHandler& rh, const DDiscarded& discarded, const DServerException& se, const Ts& ...t) {
                CScopeUQueue sb;
                sb->Save(t ...);
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh, discarded, se);
            }

#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else

            void raise(unsigned short req_id) {
                CClientSocket *cs = GetSocket();
                int ec = cs->GetErrorCode();
                if (ec) {
                    std::string em = cs->GetErrorMsg();
                    throw CSocketError(ec, Utilities::ToWide(em).c_str(), req_id, true);
                } else {
                    throw CSocketError(SESSION_CLOSED_BEFORE, SESSION_CLOSED_BEFORE_ERR_MSG, req_id, true);
                }
            }

#ifdef HAVE_FUTURE

            std::future<CScopeUQueue> sendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int size) {
                std::shared_ptr<std::promise<CScopeUQueue> > prom(new std::promise<CScopeUQueue>);
                if (!SendRequest(reqId, pBuffer, size, [prom](CAsyncResult & ar) {
                        CScopeUQueue sb;
                        sb->Swap(ar.UQueue);
                        try {
                            prom->set_value(std::move(sb));
                        } catch (std::future_error&) {
                            //ignore it
                        }
                    }, get_aborted(prom, reqId), get_se(prom))) {
                    raise(reqId);
                }
                return prom->get_future();
            }

            std::future<CScopeUQueue> sendRequest(unsigned short reqId) {
                return sendRequest(reqId, (const unsigned char *) nullptr, (unsigned int) 0);
            }

            template<typename ...Ts>
            std::future<CScopeUQueue> sendRequest(unsigned short reqId, const Ts& ... args) {
                CScopeUQueue sb;
                sb->Save(args ...);
                return sendRequest(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R>
            static DDiscarded get_aborted(const std::shared_ptr<std::promise<R> >& prom, unsigned short req_id) {
                return [prom, req_id](CAsyncServiceHandler *h, bool canceled) {
                    try {
                        if (canceled) {
                            prom->set_exception(std::make_exception_ptr(CSocketError(REQUEST_CANCELED, REQUEST_CANCELED_ERR_MSG, req_id, false)));
                        } else {
                            CClientSocket* cs = h->GetSocket();
                            int ec = cs->GetErrorCode();
                            if (ec) {
                                std::string em = cs->GetErrorMsg();
                                prom->set_exception(std::make_exception_ptr(CSocketError(ec, Utilities::ToWide(em).c_str(), req_id, false)));
                            } else {
                                prom->set_exception(std::make_exception_ptr(CSocketError(SESSION_CLOSED_AFTER, SESSION_CLOSED_AFTER_ERR_MSG, req_id, false)));
                            }
                        }
                    } catch (std::future_error&) {
                        //ignore
                    }
                };
            }

            template<typename R>
            static DServerException get_se(const std::shared_ptr<std::promise<R> >& prom) {
                return [prom](CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char* errWhere, unsigned int errCode) {
                    try {
                        prom->set_exception(std::make_exception_ptr(CServerError(errCode, errMsg, errWhere, reqId)));
                    } catch (std::future_error&) {
                        //ignore
                    }
                };
            }

            template<typename R>
            std::future<R> send(unsigned short reqId, const unsigned char *pBuffer, unsigned int size) {
                std::shared_ptr<std::promise<R> > prom(new std::promise<R>);
                if (!SendRequest(reqId, pBuffer, size, [prom](CAsyncResult & ar) {
                        try {
                            prom->set_value(ar.Load<R>());
                        } catch (std::future_error&) {
                            //ignore it
                        } catch (...) {
                            prom->set_exception(std::current_exception());
                        }
                    }, get_aborted(prom, reqId), get_se(prom))) {
                    raise(reqId);
                }
                return prom->get_future();
            }

            template<typename R>
            std::future<R> send(unsigned short reqId) {
                return send<R>(reqId, (const unsigned char *) nullptr, (unsigned int) 0);
            }

            template<typename R, typename ... Ts>
            std::future<R> send(unsigned short reqId, const Ts& ... args) {
                CScopeUQueue sb;
                sb->Save(args ...);
                return send<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }
#endif

#ifdef HAVE_COROUTINE

            template<typename R>
            struct CWaiterBase {

                struct CWaiterContext {

                    CWaiterContext(unsigned short reqId) noexcept
                    : m_done(false), m_reqId(reqId) {
                    }

                    CWaiterContext(const CWaiterContext& wc) = delete;
                    CWaiterContext(CWaiterContext&& wc) = delete;
                    CWaiterContext& operator=(const CWaiterContext& wc) = delete;
                    CWaiterContext& operator=(CWaiterContext&& wc) = delete;

                    bool await_ready() noexcept {
                        CSpinAutoLock al(m_cs);
                        return m_done;
                    }
                    //call this method from pool worker thread only

                    void resume() noexcept {
                        CSpinAutoLock al(m_cs);
                        if (!m_done) {
                            m_done = true;
                            if (m_rh) {
                                m_rh.resume();
                            }
                        }
                    }

                    unsigned short get_id() noexcept {
                        return m_reqId;
                    }

                    R m_r;
                    std::exception_ptr m_ex;

                private:

                    bool await_suspend(CRHandle rh) noexcept {
                        CSpinAutoLock al(m_cs);
                        if (!m_done) {
                            m_rh = rh;
                            return true;
                        }
                        return false;
                    }

                    CSpinLock m_cs;
                    bool m_done; //protected by m_cs
                    CRHandle m_rh; //protected by m_cs
                    unsigned short m_reqId;
                    friend struct CWaiterBase;
                };

                CWaiterBase(unsigned short reqId)
                : m_wc(new CWaiterContext(reqId)) {
                }

                bool await_suspend(CRHandle rh) noexcept {
                    return m_wc->await_suspend(rh);
                }

                R&& await_resume() {
                    if (m_wc->m_ex) {
                        std::rethrow_exception(m_wc->m_ex);
                    }
                    return std::move(m_wc->m_r);
                }

                bool await_ready() noexcept {
                    return m_wc->await_ready();
                }

            protected:

                DServerException get_se() noexcept {
                    auto& wc = m_wc;
                    return [wc](CAsyncServiceHandler* ash, unsigned short reqId, const wchar_t* errMsg, const char* errWhere, unsigned int errCode) {
                        wc->m_ex = std::make_exception_ptr(CServerError(errCode, errMsg, errWhere, reqId));
                        wc->resume();
                    };
                }

                DDiscarded get_aborted() noexcept {
                    auto& wc = m_wc;
                    return [wc](CAsyncServiceHandler* h, bool canceled) {
                        if (canceled) {
                            wc->m_ex = std::make_exception_ptr(CSocketError(REQUEST_CANCELED, REQUEST_CANCELED_ERR_MSG, wc->get_id(), false));
                        } else {
                            CClientSocket* cs = h->GetSocket();
                            int ec = cs->GetErrorCode();
                            if (ec) {
                                std::string em = cs->GetErrorMsg();
                                wc->m_ex = std::make_exception_ptr(CSocketError(ec, Utilities::ToWide(em).c_str(), wc->get_id(), false));
                            } else {
                                wc->m_ex = std::make_exception_ptr(CSocketError(SESSION_CLOSED_AFTER, SESSION_CLOSED_AFTER_ERR_MSG, wc->get_id(), false));
                            }
                        }
                        wc->resume();
                    };
                }

                std::shared_ptr<CWaiterContext> m_wc;
            };

            template<typename R>
            struct RWaiter : public CWaiterBase<R> {

                RWaiter(CAsyncServiceHandler* ash, unsigned short reqId, const unsigned char* pBuffer, unsigned int size)
                : CWaiterBase<R>(reqId) {
                    auto& wc = this->m_wc;
                    if (!ash->SendRequest(reqId, pBuffer, size, [wc](CAsyncResult & ar) {
                            try {
                                ar >> wc->m_r;
                            } catch (...) {
                                wc->m_ex = std::current_exception();
                            }
                            wc->resume();
                        }, this->get_aborted(), this->get_se())) {
                        ash->raise(reqId);
                    }
                }
            };

            template<typename R>
            RWaiter<R> wait_send(unsigned short reqId, const unsigned char* pBuffer, unsigned int size) {
                return RWaiter<R>(this, reqId, pBuffer, size);
            }

            template<typename R>
            RWaiter<R> wait_send(unsigned short reqId) {
                return RWaiter<R>(this, reqId, (const unsigned char*) nullptr, (unsigned int) 0);
            }

            template<typename R, typename ... Ts>
            RWaiter<R> wait_send(unsigned short reqId, const Ts& ... args) {
                CScopeUQueue sb;
                sb->Save(args ...);
                return RWaiter<R>(this, reqId, sb->GetBuffer(), sb->GetSize());
            }

            struct BWaiter : public CWaiterBase<CScopeUQueue> {

                BWaiter(CAsyncServiceHandler* ash, unsigned short reqId, const unsigned char* pBuffer, unsigned int size)
                : CWaiterBase<CScopeUQueue>(reqId) {
                    auto& wc = m_wc;
                    if (!ash->SendRequest(reqId, pBuffer, size, [wc](CAsyncResult & ar) {
                            wc->m_r->Swap(ar.UQueue);
                            wc->resume();
                        }, get_aborted(), get_se())) {
                        ash->raise(reqId);
                    }
                }
            };

            BWaiter wait_sendRequest(unsigned short reqId, const unsigned char* pBuffer, unsigned int size) {
                return BWaiter(this, reqId, pBuffer, size);
            }

            BWaiter wait_sendRequest(unsigned short reqId) {
                return BWaiter(this, reqId, (const unsigned char*) nullptr, (unsigned int) 0);
            }

            template<typename ... Ts>
            BWaiter wait_sendRequest(unsigned short reqId, const Ts& ... args) {
                CScopeUQueue sb;
                sb->Save(args ...);
                return BWaiter(this, reqId, sb->GetBuffer(), sb->GetSize());
            }
#endif
#endif
        private:
            //The two following functions may be public in the future
            bool Attach(CClientSocket *cs);
            void Detach();

            inline USocket_Client_Handle GetClientSocketHandle() const noexcept;

            bool GetAsyncResultHandler(unsigned short usReqId, PRR_PAIR &p) noexcept;
            void OnRR(unsigned short reqId, CUQueue &mc);
            void OnSE(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            void SetNULL() noexcept;
            void EraseBack(unsigned int count);
            void AppendTo(CAsyncServiceHandler &from);
            static bool Remove(CUQueue &q, PRR_PAIR p) noexcept;

        protected:
            virtual void OnPostProcessing(unsigned int hint, UINT64 data);
            virtual void OnMergeTo(CAsyncServiceHandler & to);
            virtual bool SendRouteeResult(const unsigned char *buffer, unsigned int len, unsigned short reqId = 0);
            bool SendRouteeResult(unsigned short reqId = 0);

            template<typename ...Ts>
            bool SendRouteeResult(unsigned short reqId, const Ts& ... data) {
                CScopeUQueue sb;
                sb->Save(data ...);
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), reqId);
            }

#if defined(NODE_JS_ADAPTER_PROJECT)
        public:
            typedef Persistent<Function, v8::NJANonCopyablePersistentTraits<Function> > CNJFunc;

            virtual SPA::UINT64 SendRequest(Isolate* isolate, int args, Local<Value> *argv, unsigned short reqId, const unsigned char *pBuffer, unsigned int size);

            void Backup(std::shared_ptr<CNJFunc>& f) {
                m_fBackup.push_back(std::move(f));
            }

            void CleanFuncBackups() {
                m_fBackup.clear();
            }

        private:

            enum class tagEvent {
                eResult = 0,
                eDiscarded,
                eException
            };
            static void req_cb(uv_async_t* handle);

            struct ReqCb {
                unsigned short ReqId;
                tagEvent Type;
                PUQueue Buffer;
                std::shared_ptr<CNJFunc> Func;

                ReqCb(CAsyncResult& ar, tagEvent type) : ReqId(ar.RequestId), Type(type), Buffer(CScopeUQueue::Lock(ar.UQueue.GetOS(), ar.UQueue.GetEndian())) {
                }

                ReqCb(unsigned short reqId, tagEvent type) : ReqId(reqId), Type(type), Buffer(CScopeUQueue::Lock()) {
                }

                ReqCb(ReqCb&& rcb) noexcept : ReqId(rcb.ReqId), Type(rcb.Type), Buffer(rcb.Buffer), Func(std::move(rcb.Func)) {
                    rcb.Buffer = nullptr;
                }

                ReqCb(const ReqCb& rcb) = delete;
                ReqCb& operator=(const ReqCb& rcb) = delete;
                ReqCb& operator=(ReqCb&& rcb) = delete;
            };
            std::deque<ReqCb> m_deqReqCb; //protected by m_cs;
            uv_async_t m_typeReq; //SendRequest events
            std::deque<std::shared_ptr<CNJFunc> > m_fBackup;
#endif
        private:
            CSpinLock m_csCb;
            CSpinLock m_cs;
            CUQueue &m_vCallback; //protected by m_cs;
            CUQueue &m_vBatching; //protected by m_cs;
            unsigned int m_nServiceId;
            CClientSocket *m_pClientSocket;
#ifndef NODE_JS_ADAPTER_PROJECT
            CUCriticalSection m_csSend;
#endif
            static CSpinLock m_csIndex;
            static UINT64 m_CallIndex; //protected by m_csIndex;
            friend class CClientSocket;
            template<typename THandler, typename TCS>
            friend class CSocketPool; // unbound friend class

            struct CRRImpl : public IUDelImpl<DResultReturned> {
                bool Invoke(CAsyncServiceHandler *ash, unsigned short reqId, CUQueue &buff);
            };
            CRRImpl m_rrImpl;
            IUDelImpl<DServerException> m_seImpl;
            IUDelImpl<DBaseRequestProcessed> m_brpImpl;

        public:
            IUDel<DResultReturned> &ResultReturned;
            IUDel<DServerException> &ServerException;
            IUDel<DBaseRequestProcessed> &BaseRequestProcessed;
        };

        typedef CAsyncServiceHandler* PAsyncServiceHandler;
        using DBaseRequestProcessed = CAsyncServiceHandler::DBaseRequestProcessed;
        using DServerException = CAsyncServiceHandler::DServerException;
        using DResultReturned = CAsyncServiceHandler::DResultReturned;
        using DDiscarded = CAsyncServiceHandler::DDiscarded;

#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE
#ifdef HAVE_COROUTINE
        template<typename R> using CWaiter = CAsyncServiceHandler::CWaiterBase<R>;
        using BWaiter = CAsyncServiceHandler::BWaiter;
        template<typename R> using RWaiter = CAsyncServiceHandler::RWaiter<R>;
#if HAVE_COROUTINE > 1

        struct CAwTask {

            struct promise_type {

                CAwTask get_return_object() {
                    return
                    {
                    };
                }

                std::suspend_never initial_suspend() {
                    return
                    {
                    };
                }

                std::suspend_never final_suspend() {
                    return
                    {
                    };
                }

                void return_void() {
                }

                void unhandled_exception() {
                }
            };
        };
#else
        typedef std::future<void> CAwTask;
#endif
#endif
#endif
#endif

        template<unsigned int serviceId>
        class CASHandler : public CAsyncServiceHandler {
        public:

            CASHandler(CClientSocket* cs) : CAsyncServiceHandler(serviceId, cs) {
            }
        };

        template<typename THandler, typename TCS = CClientSocket>
        class CSocketPool {
            const static unsigned int DEFAULT_QUEUE_TIME_TO_LIVE = 240 * 3600; //10 days

        public:
            typedef std::shared_ptr<THandler> PHandler;
            typedef std::shared_ptr<TCS> PClientSocket;
            typedef std::function<bool(CSocketPool*, TCS*) > DDoSslAuthentication;
            typedef THandler Handler;
            typedef std::function<void(CSocketPool*, tagSocketPoolEvent, THandler*) > DSocketPoolEvent;

        private:

            struct cs_hash : public std::hash < PClientSocket > {

                inline size_t operator()(const PClientSocket & key) const {
                    return (size_t) key->GetHandle();
                }
            };

        public:
            typedef std::unordered_map<PClientSocket, PHandler, cs_hash> CMapSocketHandler;
        public:

            CSocketPool(bool autoConn = true,
                    unsigned int recvTimeout = DEFAULT_RECV_TIMEOUT,
                    unsigned int connTimeout = DEFAULT_CONN_TIMEOUT,
                    unsigned int nServiceId = 0)
            : m_nPoolId(0),
            m_autoConn(autoConn),
            m_recvTimeout(recvTimeout),
            m_connTimeout(connTimeout),
            m_nServiceId(nServiceId),
            SocketPoolEvent(m_implSPE) {
                if (!ClientCoreLoader.IsLoaded()) {
                    throw CUExCode("Client core library not accessible!", MB_BAD_OPERATION);
                }
                m_implSPE.SetCS(&m_sl);
                CAutoLock al(g_csSpPool);
                m_vPool.push_back(this);
            }

            virtual ~CSocketPool() {
                ShutdownPool();
                CAutoLock al(g_csSpPool);
                if (m_vPool.size()) {
                    for (auto it = m_vPool.begin(), e = m_vPool.end(); it != e; ++it) {
                        if (this == *it) {
                            m_vPool.erase(it);
                            break;
                        }
                    }
                }
            }

        private:
            CSocketPool(const CSocketPool &sp) = delete;
            CSocketPool& operator=(const CSocketPool &sp) = delete;

        public:

            static unsigned int GetSocketPools() {
                return ClientCoreLoader.GetNumberOfSocketPools();
            }

            DDoSslAuthentication DoSslServerAuthentication;

            inline const CMapSocketHandler& GetSocketHandlerMap() const noexcept {
                return m_mapSocketHandler;
            }

            inline CUCriticalSection& GetCriticalSection() noexcept {
                return m_cs;
            }

            std::vector<PHandler> GetAsyncHandlers() {
                std::vector<PHandler> v;
                {
                    CAutoLock al(m_cs);
                    for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                        v.push_back(it->second);
                    }
                }
                return v;
            }

            std::vector<PClientSocket> GetSockets() {
                std::vector<PClientSocket> v;
                {
                    CAutoLock al(m_cs);
                    for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                        v.push_back(it->first);
                    }
                }
                return v;
            }

            /// <summary>
            /// Seek an async handler on the min number of requests queued in memory and its associated socket connection
            /// </summary>
            /// <returns>An async handler if found; and null or nothing if no connection is found</returns>

            virtual PHandler Seek() {
                PHandler h;
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetConnectionState() < tagConnectionState::csSwitched)
                        continue;
                    if (!h)
                        h = it->second;
                    else {
                        CClientSocket* cs = h->GetSocket();
                        IClientQueue& cq0 = cs->GetClientQueue();
                        UINT64 count0 = cq0.IsAvailable() ? cq0.GetMessageCount() : cs->GetCountOfRequestsInQueue();
                        cs = it->first.get();
                        IClientQueue& cq1 = cs->GetClientQueue();
                        UINT64 count1 = cq1.IsAvailable() ? cq1.GetMessageCount() : cs->GetCountOfRequestsInQueue();
                        if (count0 > count1)
                            h = it->second;
                        else if (count0 == count1) {
                            UINT64 sent0 = h->GetSocket()->GetBytesSent();
                            UINT64 sent1 = it->first->GetBytesSent();
                            if (sent0 >= sent1)
                                h = it->second;
                        }
                    }
                }
                return h;
            }

            unsigned int GetQueues() {
                unsigned int queues = 0;
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    queues += (it->first->GetClientQueue().IsAvailable() ? 1 : 0);
                }
                return queues;
            }

            virtual PHandler SeekByQueue() {
                PHandler h;
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    IClientQueue &cq = it->first->GetClientQueue();
                    if (!cq.IsAvailable() || cq.GetJobSize()/*queue is in transaction at this time*/)
                        continue;
                    if (!h)
                        h = it->second;
                    else {
                        UINT64 count0 = h->GetSocket()->GetClientQueue().GetMessageCount();
                        UINT64 count1 = cq.GetMessageCount();
                        if (count0 > count1 || (it->first->IsConnected() && !h->GetSocket()->IsConnected()))
                            h = it->second;
                    }
                }
                return h;
            }

            virtual PHandler SeekByQueue(const std::string &queueName) {
                PHandler h;
                std::string rawName;
                std::string qn(queueName);
                if (0 == qn.size())
                    return h;
#ifdef WIN32_64
                std::transform(qn.begin(), qn.end(), qn.begin(), ::tolower);
#endif
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    IClientQueue &cq = it->first->GetClientQueue();
                    if (!cq.IsAvailable())
                        continue;
                    if (cq.IsSecure())
                        rawName = qn + "_" + ClientCoreLoader.GetUClientAppName() + "_1.mqc";
                    else
                        rawName = qn + "_" + ClientCoreLoader.GetUClientAppName() + "_0.mqc";

                    std::string queueFileName = cq.GetQueueFileName();
                    size_t len = queueFileName.size();
                    size_t lenRaw = rawName.size();
                    if (lenRaw > len)
                        continue;
                    size_t pos = queueFileName.rfind(rawName);

                    //queue file name with full path
                    if (pos == 0)
                        return it->second;

                    //queue raw name only
                    if ((pos + lenRaw) == len)
                        return it->second;
                }
                return h;
            }

            inline void SetAutoConn(bool autoConn) {
                CAutoLock al(m_cs);
                m_autoConn = autoConn;
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    auto cs = it->first;
                    ClientCoreLoader.SetAutoConn(cs->GetHandle(), autoConn);
                }
            }

            inline bool GetAutoConn() noexcept {
                CAutoLock al(m_cs);
                return m_autoConn;
            }

            inline void SetRecvTimeout(unsigned int recvTimeout) {
                CAutoLock al(m_cs);
                if (!m_mapSocketHandler.size()) {
                    m_recvTimeout = recvTimeout;
                    if (m_recvTimeout < 1000)
                        m_recvTimeout = 1000;
                } else {
                    for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                        auto cs = it->first;
                        ClientCoreLoader.SetRecvTimeout(cs->GetHandle(), recvTimeout);
                        if (it == m_mapSocketHandler.begin())
                            m_recvTimeout = ClientCoreLoader.GetRecvTimeout(cs->GetHandle());
                    }
                }
            }

            inline unsigned int GetRecvTimeout() noexcept {
                CAutoLock al(m_cs);
                return m_recvTimeout;
            }

            inline void SetConnTimeout(unsigned int connTimeout) {
                CAutoLock al(m_cs);
                if (!m_mapSocketHandler.size()) {
                    m_connTimeout = connTimeout;
                    if (m_connTimeout < 1000)
                        m_connTimeout = 1000;
                } else {
                    for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                        auto cs = it->first;
                        ClientCoreLoader.SetConnTimeout(cs->GetHandle(), connTimeout);
                        if (it == m_mapSocketHandler.begin())
                            m_connTimeout = ClientCoreLoader.GetConnTimeout(cs->GetHandle());
                    }
                }
            }

            inline unsigned int GetConnTimeout() noexcept {
                CAutoLock al(m_cs);
                return m_connTimeout;
            }

            inline bool IsAvg() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.IsAvg(m_nPoolId);
            }

            inline bool GetQueueAutoMerge() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId);
            }

            inline void SetQueueAutoMerge(bool autoMerger) {
                CAutoLock al(m_cs);
                assert(m_nPoolId); //don't call the function before socket pool is started!
                ClientCoreLoader.SetQueueAutoMergeByPool(m_nPoolId, autoMerger);
            }

            inline unsigned int GetSocketsCreated() noexcept {
                CAutoLock al(m_cs);
                return (unsigned int) m_mapSocketHandler.size();
            }

            inline unsigned int GetPoolId() noexcept {
                return m_nPoolId;
            }

            bool IsStarted() {
                CAutoLock al(m_cs);
                unsigned int count = ClientCoreLoader.GetThreadCount(m_nPoolId);
                return (count > 0);
            }

            virtual bool StartSocketPool(CConnectionContext **ppCCs, unsigned int socketsPerThread, unsigned int threads, bool avg = true, tagThreadApartment ta = tagThreadApartment::taNone) {
                assert(ppCCs != nullptr);
                assert(*ppCCs != nullptr);
                assert(socketsPerThread != 0);
                assert(threads != 0);
                if (!StartSocketPool(socketsPerThread, threads, avg, ta))
                    return false;
                assert(threads == GetThreadsCreated());
                assert(socketsPerThread == GetSocketsPerThread());
                return PostProcess(ppCCs);
            }

            bool StartSocketPool(const CConnectionContext &cc, unsigned int socketsPerThread, unsigned int threads = 1, bool avg = true, tagThreadApartment ta = tagThreadApartment::taNone) {
                unsigned int n;
                assert(socketsPerThread > 0);
                if (!StartSocketPool(socketsPerThread, threads, avg, ta))
                    return false;
                if (threads == 0)
                    threads = GetThreadsCreated();
                assert(threads == GetThreadsCreated());
                assert(socketsPerThread == GetSocketsPerThread());
                typedef CConnectionContext* PCConnectionContext;
                PCConnectionContext *ppCCs = new PCConnectionContext[threads];
                for (n = 0; n < threads; ++n) {
                    ppCCs[n] = new CConnectionContext[socketsPerThread];
                    for (unsigned int j = 0; j < socketsPerThread; ++j) {
                        ppCCs[n][j] = cc;
                    }
                }
                bool ok = PostProcess(ppCCs);
                for (n = 0; n < threads; ++n) {
                    delete[](ppCCs[n]);
                }
                delete[]ppCCs;
                return ok;
            }

            PHandler Lock(unsigned int timeout = (~0)) {
                return Lock(timeout, nullptr);
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
                const CClientSocket *cs = h->GetSocket();
                Unlock(cs);
            }

            void Unlock(const PHandler &handler) {
                if (!handler)
                    return;
                const CClientSocket *cs = handler->GetSocket();
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
                ClientCoreLoader.UnlockASocket(poolId, h);
            }

            virtual void ShutdownPool() {
                ClientCoreLoader.DestroySocketPool(m_nPoolId);
                CAutoLock al(m_cs);
                m_nPoolId = 0;
            }

            const std::string& GetQueueName() noexcept {
                CAutoLock al(m_cs);
                return m_qName;
            }

            void SetQueueName(const char *qName) {
                std::string s(qName ? qName : "");
                Utilities::Trim(s);
#ifdef WIN32_64
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
#endif
                CAutoLock al(m_cs);
                if (m_qName != s) {
                    StopPoolQueue();
                    m_qName = s;
                    if (m_qName.size())
                        StartPoolQueue(m_qName.c_str());
                }
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

            PHandler FindClosedOne() noexcept {
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

            void StopPoolQueue() {
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    IClientQueue &cq = it->first->GetClientQueue();
                    if (cq.IsAvailable())
                        cq.StopQueue();
                }
            }

            void StartPoolQueue(const char *qname) {
                UINT64 index = 0;
                std::string s = qname;
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    IClientQueue &cq = it->first->GetClientQueue();
                    bool ok = cq.StartQueue((s + std::to_string(index)).c_str(), DEFAULT_QUEUE_TIME_TO_LIVE, it->first->GetEncryptionMethod() != tagEncryptionMethod::NoEncryption);
                    assert(ok);
                    ++index;
                }
            }

            void SetQueue(PClientSocket socket, unsigned int pos) {
                UINT64 index = pos;
                IClientQueue &cq = socket->GetClientQueue();
                if (m_qName.size()) {
                    if (!cq.IsAvailable()) {
                        cq.StartQueue((m_qName + std::to_string(index)).c_str(), DEFAULT_QUEUE_TIME_TO_LIVE, socket->GetEncryptionMethod() != tagEncryptionMethod::NoEncryption);
                    }
                }
            }

            bool PostProcess(CConnectionContext **ppCCs) {
                bool ok;
                unsigned int pos = 0;
                bool first = true;
                CAutoLock al(m_cs);
                unsigned int poolId = m_nPoolId;
                size_t size = m_mapSocketHandler.size();
                unsigned int socketsPerThread = ClientCoreLoader.GetSocketsPerThread(m_nPoolId);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    USocket_Client_Handle h = it->first->GetHandle();
                    if (ClientCoreLoader.IsOpened(h)) {
                        first = false;
                        continue;
                    }

                    PClientSocket cs = it->first;
                    cs->m_cc = ppCCs[pos / socketsPerThread][pos % socketsPerThread];

                    ClientCoreLoader.SetUserID(h, cs->m_cc.UserId.c_str());
                    ClientCoreLoader.SetEncryptionMethod(h, cs->m_cc.EncrytionMethod);

                    //we build connections asynchronously for all the sockets except the very first one
                    if (first) {
                        m_cs.unlock();
                        ok = (ClientCoreLoader.Connect(h, cs->m_cc.Host.c_str(), cs->m_cc.Port, true, cs->m_cc.V6) &&
                                ClientCoreLoader.WaitAll(h, (~0)) &&
                                ClientCoreLoader.GetConnectionState(h) > tagConnectionState::csConnected);
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
                    ++pos;
                }
                unsigned int count = ClientCoreLoader.GetConnectedSockets(poolId);
                return (count > 0);
            }

            bool StartSocketPool(unsigned int socketsPerThread, unsigned int threads, bool avg = true, tagThreadApartment ta = tagThreadApartment::taNone) {
                if (IsStarted())
                    return true;
                unsigned int poolId = ClientCoreLoader.CreateSocketPool(&CSocketPool::SPE, socketsPerThread, threads, avg, ta);
                CAutoLock al(m_cs);
                m_nPoolId = poolId;
                return true;
            }

            PHandler MapToHandler(USocket_Client_Handle h) noexcept {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetHandle() == h)
                        return it->second;
                }
                return PHandler();
            }

            PClientSocket MapToSocket(USocket_Client_Handle h, unsigned int &index) {
                index = 0;
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetHandle() == h)
                        return it->first;
                    ++index;
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
                CAutoLock al(g_csSpPool);
                if (!m_vPool.size()) {
                    return nullptr;
                }
                for (auto it = m_vPool.begin(), e = m_vPool.end(); it != e; ++it) {
                    if ((*it)->GetPoolId() == poolId) {
                        return *it;
                    }
                }
                return nullptr;
            }

            static void CALLBACK SPE(unsigned int poolId, tagSocketPoolEvent spe, USocket_Client_Handle h) {
                PClientSocket pcs;
                unsigned int index = 0;
                CSocketPool *sp = Seek(poolId);
                switch (spe) {
                    case tagSocketPoolEvent::speTimer:
                        if ((CScopeUQueue::GetMemoryConsumed() / 1024) > SHARED_BUFFER_CLEAN_SIZE) {
                            CScopeUQueue::DestroyUQueuePool();
                        }
                        break;
                    case tagSocketPoolEvent::speShutdown:
                        if (sp) {
                            CAutoLock al(sp->m_cs);
                            sp->m_mapSocketHandler.clear();
                            sp->m_nPoolId = 0;
                        }
                        break;
                    case tagSocketPoolEvent::speStarted:
                    {
                        CAutoLock al(g_csSpPool);
                        for (auto it = m_vPool.rbegin(), end = m_vPool.rend(); it != end; ++it) {
                            if ((*it)->m_nPoolId == 0) {
                                sp = *it;
                                break;
                            }
                        }
                        sp->m_nPoolId = poolId;
                    }
                        break;
                    case tagSocketPoolEvent::speUSocketCreated:
                        if (sp) {
                            PClientSocket clientSocket = CSocketPool<THandler, TCS>::CreateEmptySocket();
                            Set(clientSocket, h);
                            ClientCoreLoader.SetRecvTimeout(h, sp->m_recvTimeout);
                            ClientCoreLoader.SetConnTimeout(h, sp->m_connTimeout);
                            ClientCoreLoader.SetAutoConn(h, sp->m_autoConn);
                            PHandler handler(new THandler(clientSocket.get()));
                            if (handler->GetSvsID() == 0)
                                handler->m_nServiceId = sp->m_nServiceId;
                            CAutoLock al(sp->m_cs);
                            sp->m_mapSocketHandler[clientSocket] = handler;
                        }
                        break;
                    case tagSocketPoolEvent::speUSocketKilled:
                        if (sp) {
                            CAutoLock al(sp->m_cs);
                            for (auto it = sp->m_mapSocketHandler.begin(), end = sp->m_mapSocketHandler.end(); it != end; ++it) {
                                if (it->first->GetHandle() == h) {
                                    sp->m_mapSocketHandler.erase(it);
                                    break;
                                }
                            }
                        }
                        break;
                    case tagSocketPoolEvent::speConnected:
                        if (sp && ClientCoreLoader.IsOpened(h)) {
                            ClientCoreLoader.SetSockOpt(h, tagSocketOption::soRcvBuf, 116800, tagSocketLevel::slSocket);
                            ClientCoreLoader.SetSockOpt(h, tagSocketOption::soSndBuf, 116800, tagSocketLevel::slSocket);
                            pcs = sp->MapToSocket(h, index);
                            if (sp->DoSslServerAuthentication) {
                                if (ClientCoreLoader.GetEncryptionMethod(h) == tagEncryptionMethod::TLSv1 && !sp->DoSslServerAuthentication(sp, pcs.get()))
                                    return;
                            }
                            ClientCoreLoader.SetPassword(h, pcs->m_cc.Password.c_str());
                            bool ok = ClientCoreLoader.StartBatching(h);
                            ok = ClientCoreLoader.SwitchTo(h, sp->MapToHandler(h)->GetSvsID());
                            ok = ClientCoreLoader.TurnOnZipAtSvr(h, pcs->m_cc.Zip);
                            ok = ClientCoreLoader.SetSockOptAtSvr(h, tagSocketOption::soRcvBuf, 116800, tagSocketLevel::slSocket);
                            ok = ClientCoreLoader.SetSockOptAtSvr(h, tagSocketOption::soSndBuf, 116800, tagSocketLevel::slSocket);
                            ok = ClientCoreLoader.CommitBatching(h, false);
                        }
                        break;
                    case tagSocketPoolEvent::speQueueMergedFrom:
                        assert(sp);
                        sp->m_pHFrom = sp->MapToHandler(h);
                        break;
                    case tagSocketPoolEvent::speQueueMergedTo:
                        assert(sp);
                    {
                        PHandler to = sp->MapToHandler(h);
                        sp->m_pHFrom->AppendTo(*to);
                        sp->m_pHFrom.reset();
                    }
                        break;
                    default:
                        break;
                }
                if (!sp)
                    return;
                const PHandler &handler = sp->MapToHandler(h);
                sp->m_implSPE.Invoke(sp, spe, handler.get());
                sp->OnSocketPoolEvent(spe, handler);
                if (spe == tagSocketPoolEvent::speConnected && ClientCoreLoader.IsOpened(h)) {
                    sp->SetQueue(pcs, index);
                }
            }

        protected:
            CUCriticalSection m_cs;
            CMapSocketHandler m_mapSocketHandler; //locked by m_cs

        private:
            unsigned int m_nPoolId; //locked by m_cs
            bool m_autoConn;
            unsigned int m_recvTimeout;
            unsigned int m_connTimeout;
            unsigned int m_nServiceId;
            PHandler m_pHFrom;
            std::string m_qName;
            static std::vector<CSocketPool*> m_vPool; //protected by g_csSpPool
            CSpinLock m_sl;
            IUDelImpl<DSocketPoolEvent> m_implSPE;

        public:
            IUDel<DSocketPoolEvent>& SocketPoolEvent;
        };

        template<typename THandler, typename TCS>
        std::vector<CSocketPool<THandler, TCS>* > CSocketPool<THandler, TCS>::m_vPool;

        struct ReplicationSetting {
            static const unsigned int DEAFULT_TTL = 720 * 3600; //30 days

            ReplicationSetting()
            : QueueDir(CClientSocket::QueueConfigure::GetWorkDirectory()),
            NoAutoConn(false),
            TTL(DEAFULT_TTL),
            RecvTimeout(DEFAULT_RECV_TIMEOUT),
            ConnTimeout(DEFAULT_CONN_TIMEOUT) {

            }

            ReplicationSetting(const ReplicationSetting &rs)
            : QueueDir(rs.QueueDir),
            NoAutoConn(rs.NoAutoConn),
            TTL(rs.TTL),
            RecvTimeout(rs.RecvTimeout),
            ConnTimeout(rs.ConnTimeout) {

            }

            ReplicationSetting& operator=(const ReplicationSetting &rs) {
                if (this != &rs) {
                    QueueDir = rs.QueueDir;
                    NoAutoConn = rs.NoAutoConn;
                    TTL = rs.TTL;
                    RecvTimeout = rs.RecvTimeout;
                    ConnTimeout = rs.ConnTimeout;
                }
                return *this;
            }

            std::string QueueDir;
            bool NoAutoConn;
            unsigned int TTL;
            unsigned int RecvTimeout;
            unsigned int ConnTimeout;
        };

        typedef std::unordered_map<std::string, CConnectionContext> CMapQNameConn;

        template<typename THandler, typename TCS = CClientSocket>
        class CReplication {
            typedef CConnectionContext* PCConnectionContext;
            typedef std::shared_ptr<THandler> PHandler;
            typedef std::shared_ptr<TCS> PClientSocket;

        public:
#ifdef WIN32_64
            static const char DIR_SEP = '\\';
#else
            static const char DIR_SEP = '/';
#endif

            CReplication(const ReplicationSetting &rs)
            : m_pool(!rs.NoAutoConn, rs.RecvTimeout, rs.ConnTimeout),
            m_rs(rs) {
                assert(rs.QueueDir.size() > 0);
                assert(rs.QueueDir.find_first_not_of(" \n\r\t") == 0);
                assert(rs.QueueDir.rfind(DIR_SEP) == (rs.QueueDir.size() - 1));
                if (m_rs.TTL == 0)
                    m_rs.TTL = ReplicationSetting::DEAFULT_TTL;
            }

            virtual ~CReplication() {
                m_SourceHandler.reset();
                m_vTargetHandlers.clear();
            }

        public:

            inline unsigned int GetConnections() const {
                return m_pool.GetConnectedSockets();
            }

            inline bool IsReplicable() const noexcept {
                return m_mapQNameConn.size() > 1;
            }

            inline PHandler GetSourceHandler() const noexcept {
                return m_SourceHandler;
            }

            inline IClientQueue* GetSourceQueue() const {
                if (m_SourceHandler) {
                    return &(m_SourceHandler->GetSocket()->GetClientQueue());
                }
                return nullptr;
            }

            const std::vector<PHandler>& GetTargetHandlers() const noexcept {
                return m_vTargetHandlers;
            }

            const std::vector<IClientQueue*>& GetTargetQueues() const noexcept {
                return m_vTargetQueues;
            }

            unsigned int GetQueues() {
                return m_pool.GetQueues();
            }

            bool DoReplication() const {
                if (m_mapQNameConn.size() <= 1)
                    return false;
                std::vector<USocket_Client_Handle> vHandles;
                for (auto it = m_vTargetQueues.cbegin(), end = m_vTargetQueues.cend(); it != end; ++it) {
                    vHandles.push_back((*it)->GetHandle());
                }
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                return src->AppendTo(vHandles.data(), (unsigned int) vHandles.size());
            }

            inline size_t GetHosts() const noexcept {
                return m_mapQNameConn.size();
            }

            inline const ReplicationSetting& GetReplicationSetting() const noexcept {
                return m_rs;
            }

            virtual bool Send(unsigned short reqId, const unsigned char *buffer, unsigned int len) const {
                DResultHandler rh;
                PHandler src = GetSourceHandler();
                if (!src)
                    return false;
                IClientQueue *cq = &src->GetSocket()->GetClientQueue();
                if (!cq->IsAvailable())
                    return false;
                bool ok = src->SendRequest(reqId, buffer, len, rh);
                if (IsReplicable() && cq->GetJobSize() == 0)
                    ok = DoReplication();
                return ok;
            }

            bool Send(unsigned short reqId) const {
                return Send(reqId, (const unsigned char *) nullptr, (unsigned int) 0);
            }

            bool EndJob() const {
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                bool ok = src->EndJob();
                if (ok && IsReplicable())
                    ok = DoReplication();
                return ok;
            }

            bool StartJob() const {
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                return src->StartJob();
            }

            bool AbortJob() const {
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                return src->AbortJob();
            }

            template<typename ... Ts>
            bool Send(unsigned short reqId, const Ts& ...t) const {
                CScopeUQueue sb;
                sb->Save(t ...);
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            virtual bool Start(const CMapQNameConn &mapConnQueue, const char *rootQueueName = nullptr, tagThreadApartment ta = tagThreadApartment::taNone) {
                const wchar_t *rootUID = L"SOCKETPRO_UREPLICATION_ROOT_DUMMY_NAME";
                m_mapQNameConn.clear();
                m_pool.ShutdownPool();
                assert(mapConnQueue.size() > 0);
                for (auto it = mapConnQueue.cbegin(), end = mapConnQueue.cend(); it != end; ++it) {
                    assert(it->first.size() > 0);
                    assert(!DoesQueueExist(it->first));
                    assert(it->first.find_first_not_of(" \n\r\t/\\") == 0);
                    assert(it->first.find_last_not_of(" \n\r\t/\\") == (it->first.size() - 1));
                    m_mapQNameConn[it->first] = it->second;
                }
                size_t n = 0;
                bool secure = false;
                size_t all = m_mapQNameConn.size();
                if (all > 1)
                    ++all;
                std::vector<std::string> vDbFullName;
                PCConnectionContext ppCCs[1];
                ppCCs[0] = new CConnectionContext[all];
                for (auto it = m_mapQNameConn.cbegin(), end = m_mapQNameConn.cend(); it != end; ++it, ++n) {
                    ppCCs[0][n] = it->second;
                    if (!secure && it->second.EncrytionMethod == tagEncryptionMethod::TLSv1)
                        secure = true;
                    vDbFullName.push_back(m_rs.QueueDir + it->first);
                }
                if (all > 1) {
                    CConnectionContext last;
                    last.Host = "127.0.0.1";
                    last.Port = (~0);
                    last.UserId = rootUID;
                    last.Password = L"";
                    last.EncrytionMethod = secure ? tagEncryptionMethod::TLSv1 : tagEncryptionMethod::NoEncryption;
                    ppCCs[0][n] = last;
                    if (rootQueueName == nullptr || ::strlen(rootQueueName) == 0)
                        vDbFullName.push_back(m_rs.QueueDir + "urproot");
                    else
                        vDbFullName.push_back(m_rs.QueueDir + rootQueueName);
                }
                EndProcess(vDbFullName, secure, ppCCs, ta, rootUID);
                delete[](ppCCs[0]);
                return (m_pool.GetConnectedSockets() > 0);
            }

        protected:

            virtual bool DoSslServerAuthentication(TCS *cs) {
                return true;
            }

        private:
            //disable copy constructor and assignment operator
            CReplication(const CReplication &r) = delete;
            CReplication& operator=(const CReplication &r) = delete;

            bool DoesQueueExist(const std::string &qName) {
                std::string str1Cpy(qName);
#ifdef WIN32_64
                std::transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower);
#endif
                for (auto it = m_mapQNameConn.cbegin(), end = m_mapQNameConn.cend(); it != end; ++it) {
                    std::string str2Cpy(it->first);
#ifdef WIN32_64
                    std::transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower);
#endif
                    if (str2Cpy == str1Cpy)
                        return true;
                }
                return false;
            }

            void EndProcess(const std::vector<std::string>& vDbFullName, bool secure, PCConnectionContext *ppCCs, tagThreadApartment ta, const wchar_t *rootUID) {
                if (secure) {
                    m_pool.DoSslServerAuthentication = [this](CSocketPool<THandler, TCS> *sender, TCS * cs)->bool {
                        if (cs->GetConnectionState() < tagConnectionState::csConnected)
                            return true;
                        return DoSslServerAuthentication(cs);
                    };
                }
                int n = 0;
                bool ok = m_pool.StartSocketPool(ppCCs, (unsigned int) (vDbFullName.size()), 1);
                m_SourceHandler.reset();
                m_vTargetHandlers.clear();
                m_vTargetQueues.clear();
                auto map = m_pool.GetSocketHandlerMap();
                size_t size = map.size();
                for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
                    const std::string &qName = vDbFullName[n];
                    ok = it->first->GetClientQueue().StartQueue(qName.c_str(), m_rs.TTL, secure);
                    ++n;
                    if (size == 1) {
                        m_SourceHandler = it->second;
                        m_vTargetHandlers.push_back(m_SourceHandler);
                        m_vTargetQueues.push_back(&(it->first->GetClientQueue()));
                    } else {
                        if (it->first->GetUID() != rootUID) {
                            m_vTargetHandlers.push_back(it->second);
                            m_vTargetQueues.push_back(&(it->first->GetClientQueue()));
                        } else
                            m_SourceHandler = it->second;
                    }
                }
                if (size > 1) {
                    std::vector<USocket_Client_Handle> vHandles;
                    for (auto it = m_vTargetQueues.cbegin(), end = m_vTargetQueues.cend(); it != end; ++it) {
                        vHandles.push_back((*it)->GetHandle());
                    }
                    ok = GetSourceQueue()->EnsureAppending(vHandles.data(), (unsigned int) vHandles.size());
                }
                for (auto it = m_vTargetHandlers.begin(), end = m_vTargetHandlers.end(); it != end; ++it) {
                    ok = (*it)->GetSocket()->DoEcho();
                }
            }

        private:
            CSocketPool<THandler, TCS> m_pool;
            ReplicationSetting m_rs;
            CMapQNameConn m_mapQNameConn;
            PHandler m_SourceHandler;
            std::vector<PHandler> m_vTargetHandlers;
            std::vector<IClientQueue*> m_vTargetQueues;
        };
    }; //ClientSide
}; //SPA

#endif
