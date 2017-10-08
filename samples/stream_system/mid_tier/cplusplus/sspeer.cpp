
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"

CSSPeer::CSSPeer() {

}

CSSPeer::~CSSPeer() {
}

void CSSPeer::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)
    
    END_SWITCH
}

int CSSPeer::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)
    M_I1_R3(idQueryMaxMinAvgs, QueryMaxMinAvgs, std::wstring, CMaxMinAvg, int, std::wstring)
    END_SWITCH
    return 0;
}

void CSSPeer::QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg) {

}
