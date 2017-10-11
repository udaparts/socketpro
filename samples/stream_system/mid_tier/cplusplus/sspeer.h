#pragma once

#include "../../shared/ss_defines.h"
#include <condition_variable>
#include <chrono>

using namespace SPA::ServerSide;

class CYourPeerOne : public CClientPeer {
public:
    CYourPeerOne();

public:
    unsigned int SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index);
    unsigned int SendRows(SPA::UDB::CDBVariantArray &vData);

protected:
    virtual void OnReleaseSource(bool bClosing, unsigned int info);
    virtual void OnSwitchFrom(unsigned int nOldServiceId);
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:
    void GetCachedTables(unsigned int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg);
    void QueryPaymentMaxMinAvgs(const std::wstring &filter, int &res, std::wstring &errMsg, CMaxMinAvg &mma);
    void GetMasterSlaveConnectedSessions(unsigned int &m_connections, unsigned int &s_connections);
    void UploadEmployees(const SPA::UDB::CDBVariantArray &vData, int &res, std::wstring &errMsg, std::vector<SPA::INT64> &vId);

private:
    CYourPeerOne(const CYourPeerOne &p);
    CYourPeerOne& operator=(const CYourPeerOne &p);

public:
    std::mutex m_mutex;
    typedef std::unique_lock<std::mutex> CAutoLock;
    std::condition_variable m_cv;
    static std::chrono::seconds m_timeout;
};
