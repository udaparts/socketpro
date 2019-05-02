
#pragma once

#include "../../include/membuffer.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/unordered_map.hpp>
#include "../../include/userver.h"
#include "../../pinc/fastujson.h"
#include <boost/atomic.hpp>

namespace Connection {

    class CConnectionContextBase {
    public:
        CConnectionContextBase();

    public:
        boost::atomic<SPA::UINT64> RecvTime;
        boost::atomic<SPA::UINT64> SendTime;
        std::wstring UserId;
        std::wstring Password;
        std::vector<unsigned int> ChatGroups;
        CSvsContext SvsContext;
        SPA::UINT64 m_ulRead;
        SPA::UINT64 m_ulSent;
        bool m_bBatching;
        std::string Id;
        unsigned int Pt;
        bool IsOpera;
        bool IsGet;

    public:
        static void ChatGroupsAnd(const unsigned int *p0, unsigned int count0, const unsigned int *p1, unsigned int count1, std::vector<unsigned int> &vOut);
        static void ChatGroupsOr(const unsigned int *p0, unsigned int count0, const unsigned int *p1, unsigned int count1, std::vector<unsigned int> &vOut);
        static void ChatGroupsNew(const unsigned int *p0, unsigned int count0, const unsigned int *p1, unsigned int count1, std::vector<unsigned int> &vAdd, std::vector<unsigned int> &vExit);

        CConnectionContextBase& operator=(const CConnectionContextBase &ccb) {
            RecvTime = ccb.RecvTime.load();
            SendTime = ccb.SendTime.load();
            UserId = ccb.UserId;
            Password = ccb.Password;
            ChatGroups = ccb.ChatGroups;
            SvsContext = ccb.SvsContext;
            m_ulRead = ccb.m_ulRead;
            m_ulSent = ccb.m_ulSent;
            m_bBatching = ccb.m_bBatching;
            Id = ccb.Id;
            Pt = ccb.Pt;
            return *this;
        }

        void Initialize(const boost::posix_time::ptime &t) {
            RecvTime = (boost::posix_time::microsec_clock::local_time() - t).total_milliseconds();
            SendTime = RecvTime.load();
            UserId.clear();
            Password.resize(Password.size(), ' ');
            Password.clear();
            ChatGroups.clear();
            ::memset(&SvsContext, 0, sizeof (CSvsContext));
            m_ulRead = 0;
            m_ulSent = 0;
            m_bBatching = false;
            Id.clear();
        }

    private:
        CConnectionContextBase(const CConnectionContextBase &ccb);
    };

    class CConnectionContext : public CConnectionContextBase {
    public:
        CConnectionContext();
        typedef std::shared_ptr<CConnectionContext> SharedPtr;

    public:
        //SPA::UJsonDocument Responses;
        std::vector<std::string> Responses;

    public:
        static void AddConnectionContext(const std::string &id, SharedPtr cc);
        static void RemoveConnectionContext(const char *id);
        static SharedPtr SeekConnectionContext(const char *id);
        static void Enter(const std::string &id, const unsigned int *pGroups, unsigned int count);
        static void Enter(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count);
        static void Exit(const std::string &id, const unsigned int *pGroups, unsigned int count);
        static void Exit(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count);
        static void Speak(const std::string &id, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count);
        static void Speak(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const unsigned int *pGroups, unsigned int count);
        static void SendUserMessage(const std::string &id, const SPA::UVariant &vtMsg, const wchar_t *receiver, const unsigned int *pGroups, unsigned int count);
        static void SendUserMessage(const std::string &id, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const SPA::UVariant &vtMsg, const wchar_t *receiver, const unsigned int *pGroups, unsigned int count);
        static std::string ToString(SPA::UJsonValue &jv, unsigned int approSize);
        static void ToString(SPA::UJsonValue &jv, unsigned int approSize, SPA::CUQueue &q);
        static unsigned int SendResult(const std::string &id, unsigned short reqId, const char *res, unsigned int len, const char *callback);
        static unsigned int SendWSResult(unsigned short reqId, const char *res, unsigned int len, SPA::CUQueue &q, const char *callback);
        static unsigned int SendExceptionResult(const std::string &id, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode);

    private:
        static void SetSenderInfo(SPA::UJsonDocument &doc, const char *senderAddr, unsigned short senderClientPort, const wchar_t *sendUserId, unsigned int senderServiceId, const unsigned int *pGroups, unsigned int count);

    private:
        static SPA::CUCriticalSection m_cs;
        static boost::unordered_map<std::string, SharedPtr> m_mapCC;
    };
};



