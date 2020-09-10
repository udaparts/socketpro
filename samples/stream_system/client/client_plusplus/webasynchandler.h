#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

class CWebAsyncHandler : public CCachedBaseHandler < sidStreamSystem > {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);
};
