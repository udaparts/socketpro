#pragma once

#include "../../shared/ss_defines.h"

class CSSPeer : public CClientPeer {
public:
    CSSPeer();
    ~CSSPeer();

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:
    void QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg);

private:
    CSSPeer(const CSSPeer &p);
    CSSPeer& operator=(const CSSPeer &p);
};
