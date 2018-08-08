
#pragma once

#include "../../../include/aserverw.h"

class CHttpPeer : public SPA::ServerSide::CHttpPeerBase {
protected:
    bool DoAuthentication(const wchar_t *userId, const wchar_t *password);
    void OnFastRequestArrive(unsigned short requestId, unsigned int len);
    int OnSlowRequestArrive(unsigned short requestId, unsigned int len);
};
