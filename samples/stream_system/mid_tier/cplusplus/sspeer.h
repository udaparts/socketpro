#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/gencachepeer.h"

using namespace SPA::ServerSide;

class CYourPeerOne : public CCacheBasePeer {
public:
    CYourPeerOne();

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:
    void GetCachedTables(const std::wstring &defaultDb, int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg);
    void GetMasterSlaveConnectedSessions(SPA::UINT64 index, SPA::UINT64 &retIndex, unsigned int &m_connections, unsigned int &s_connections);
    void QueryPaymentMaxMinAvgs(SPA::CUQueue &q);
    void UploadEmployees(SPA::CUQueue &q);
    void GetRentalDateTimes(SPA::UINT64 index, SPA::INT64 rentalId, SPA::UINT64 &retIndex, CRentalDateTimes &dates, int &res, std::wstring &errMsg);

private:
    CYourPeerOne(const CYourPeerOne &p);
    CYourPeerOne& operator=(const CYourPeerOne &p);

#ifndef NDEBUG
    static SPA::CUCriticalSection m_csConsole;
#endif
};
