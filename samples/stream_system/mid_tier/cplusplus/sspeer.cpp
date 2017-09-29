
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"
#include "../../shared/tablecache.h"


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
    M_I3_R2(idSetDefaultDatabaseName, SetDefaultDatabaseName, std::wstring, bool, bool, int, std::wstring)
    M_I1_R2(idEndBatchProcessing, EndBatchProcessing, unsigned int, int, std::wstring)

    M_I1_R3(idQueryMaxMinAvgs, QueryMaxMinAvgs, std::wstring, CMaxMinAvg, int, std::wstring)
    END_SWITCH
    return 0;
}

void CSSPeer::SubscribeAndGetInitialCachedTablesData(int &res, std::wstring &errMsg) {

}

void CSSPeer::SetDefaultDatabaseName(const std::wstring &dbName, bool optimistic, bool slaveCheck, int &res, std::wstring &errMsg) {
    res = 0;
    m_dbDefaultName = dbName;
    if (!optimistic && m_dbDefaultName.size()) {
        do {
            auto mysql = slaveCheck ? CSSServer::Slave->Seek() : CSSServer::Master->Seek();
            if (!mysql) {
                //????
                break;
            }
            std::wstring sql = L"USE " + dbName;
			if (!mysql->Execute(sql.c_str(), [&res, &errMsg](CMySQLHandler & sender, int errCode, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & vtId) {
                    res = errCode;
                    errMsg = err;
                })) {
            //????
            break;
        }
            if (!mysql->WaitAll()) {

            }
        } while (false);
    }
}

void CSSPeer::BeginBatchProcessing(bool readonly, bool manualTrans, int &res, std::wstring &errMsg) {

}

void CSSPeer::EndBatchProcessing(unsigned int hints, int &res, std::wstring &errMsg) {

}

void CSSPeer::QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg) {

}
