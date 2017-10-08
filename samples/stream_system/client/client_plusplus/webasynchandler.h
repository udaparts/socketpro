
#pragma once

#include "../../shared/ss_defines.h"

class CWebAsyncHandler : public CCachedBaseHandler<sidStreamSystem> {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
	typedef std::function<void(CWebAsyncHandler &sender, const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;

public:
    bool QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma);

protected:
	virtual void OnResultReturned(unsigned short reqId, SPA::CUQueue &mc);
};
