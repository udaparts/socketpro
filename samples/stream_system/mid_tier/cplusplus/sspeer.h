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
    virtual void GetCachedTables(const std::wstring &defaultDb, unsigned int flags, SPA::UINT64 index, int &dbMS, int &res, std::wstring &errMsg);

private:
    void GetRentalDateTimes(SPA::UINT64 index, SPA::INT64 rentalId, SPA::UINT64 &retIndex, CRentalDateTimes &dates, int &res, std::wstring &errMsg);
    void GetMasterSlaveConnectedSessions(SPA::UINT64 index, SPA::UINT64 &retIndex, unsigned int &m_connections, unsigned int &s_connections);
    void QueryPaymentMaxMinAvgs(SPA::CUQueue &q);
    void UploadEmployees(SPA::CUQueue &q);

private:
    CYourPeerOne(const CYourPeerOne &p);
    CYourPeerOne& operator=(const CYourPeerOne &p);

#ifndef NDEBUG
    static SPA::CUCriticalSection m_csConsole;
#endif
};
