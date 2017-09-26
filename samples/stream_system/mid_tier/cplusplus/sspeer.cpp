
#include "stdafx.h"
#include "sspeer.h"

CSSPeer::CSSPeer() {

}

CSSPeer::~CSSPeer() {
}

void CSSPeer::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I2_R2(idBeginBatchProcessing, BeginBatchProcessing, bool, bool, int, std::wstring)
	END_SWITCH
}

int CSSPeer::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I0_R2(idSubscribeAndGetInitialCachedTablesData, SubscribeAndGetInitialCachedTablesData, int, std::wstring)
		M_I2_R2(idSetDefaultDatabaseName, SetDefaultDatabaseName, std::wstring, bool, int, std::wstring)
		M_I1_R2(idEndBatchProcessing, EndBatchProcessing, unsigned int, int, std::wstring)

		M_I1_R3(idQueryMaxMinAvgs, QueryMaxMinAvgs, std::wstring, CMaxMinAvg, int, std::wstring)
	END_SWITCH
    return 0;
}

void CSSPeer::SubscribeAndGetInitialCachedTablesData(int &res, std::wstring &errMsg) {

}

void CSSPeer::SetDefaultDatabaseName(const std::wstring &dbName, bool optimistic, int &res, std::wstring &errMsg) {

}

void CSSPeer::BeginBatchProcessing(bool readonly, bool manualTrans, int &res, std::wstring &errMsg) {

}

void CSSPeer::EndBatchProcessing(unsigned int hints, int &res, std::wstring &errMsg) {

}

void CSSPeer::QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg) {

}
