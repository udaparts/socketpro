#pragma once

#include "../../shared/ss_defines.h"

class CYourPeerOne : public CClientPeer {
public:
    CYourPeerOne();

public:
    unsigned int SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index);
    unsigned int SendRows(SPA::UDB::CDBVariantArray &vData);

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:
	void GetCachedTables(unsigned int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg);
    void QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg);
	void GetMasterSlaveConnectedSessions(unsigned int &m_connections, unsigned int &s_connections);

private:
    CYourPeerOne(const CYourPeerOne &p);
    CYourPeerOne& operator=(const CYourPeerOne &p);
};
