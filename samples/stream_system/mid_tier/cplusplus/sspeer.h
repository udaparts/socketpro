#pragma once

#include "../../shared/ss_defines.h"

using namespace SPA::ServerSide;

class CYourPeerOne : public CClientPeer {
public:
    CYourPeerOne();

public:
    unsigned int SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index);
    unsigned int SendRows(SPA::UDB::CDBVariantArray &vData);

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
	virtual void OnSwitchFrom(unsigned int nOldServiceId);

private:
    void GetCachedTables(const std::wstring &defaultDb, int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg);
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
