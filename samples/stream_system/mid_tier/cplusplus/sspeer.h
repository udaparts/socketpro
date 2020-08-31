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
    virtual void GetCachedTables(const SPA::CDBString &defaultDb, unsigned int flags, SPA::UINT64 index, int &dbMS, int &res, std::wstring &errMsg);

private:
    void GetMasterSlaveConnectedSessions(unsigned int &m_connections, unsigned int &s_connections);
    void QueryPaymentMaxMinAvgs(SPA::CUQueue &q, SPA::UINT64 reqIndex);
    void UploadEmployees(SPA::CUQueue &q, SPA::UINT64 reqIndex);
    void GetRentalDateTimes(SPA::CUQueue &q, SPA::UINT64 reqIndex);
};
