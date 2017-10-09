#pragma once

#include "../../shared/ss_defines.h"

class CSSPeer : public CClientPeer {
public:
    CSSPeer();
    ~CSSPeer();

public:
	unsigned int SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index);
	unsigned int SendRows(SPA::UDB::CDBVariantArray &vData);

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:
    void QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg);
	void GetCachedTables(unsigned int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg);

private:
    CSSPeer(const CSSPeer &p);
    CSSPeer& operator=(const CSSPeer &p);
};
