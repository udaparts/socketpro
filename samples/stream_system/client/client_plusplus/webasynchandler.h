
#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

using namespace SPA::ClientSide;

class CWebAsyncHandler : public CCachedBaseHandler<sidStreamSystem> {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
    typedef std::function<void(const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;
    typedef std::function<void(unsigned int, unsigned int) > DConnectedSessions;

public:
    bool QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma);
    bool GetMasterSlaveConnectedSessions(DConnectedSessions cs);

protected:
    virtual void OnResultReturned(unsigned short reqId, SPA::CUQueue &mc);
};
