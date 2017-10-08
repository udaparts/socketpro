
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
	M_I3_R2(SPA::UDB::idGetCachedTables, GetCachedTables, unsigned int, bool, SPA::UINT64, int, std::wstring)
    END_SWITCH
    return 0;
}

void CSSPeer::QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg) {

}

void CSSPeer::GetCachedTables(unsigned int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg) {
	res = 0;
	if (SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES == (flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES)) {
		GetPush().Subscribe(&SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
		res = -1;
		errMsg = L"Failed in subscribing for table events";
	}
	if (rowset) {
		do {
			auto handler = CSSServer::Slave->Seek();
			if (!handler) {
				res = -1;
				errMsg = L"No connection to anyone of slave databases";
				break;
			}
			
		} while (false);
	}
}
