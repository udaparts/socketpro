
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"
#include "config.h"

CYourPeerOne::CYourPeerOne() {

}

void CYourPeerOne::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)

    END_SWITCH
}

int CYourPeerOne::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)
    M_I1_R3(idQueryMaxMinAvgs, QueryMaxMinAvgs, std::wstring, CMaxMinAvg, int, std::wstring)
    M_I3_R2(SPA::UDB::idGetCachedTables, GetCachedTables, unsigned int, bool, SPA::UINT64, int, std::wstring)
    END_SWITCH
    return 0;
}

void CYourPeerOne::QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg) {

}

unsigned int CYourPeerOne::SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index) {
    return SendResult(SPA::UDB::idRowsetHeader, meta, index);
}

unsigned int CYourPeerOne::SendRows(SPA::UDB::CDBVariantArray &vData) {
    SPA::CScopeUQueue sb;
    sb << vData;
    unsigned int count;
    sb >> count; //remove data array size at head as a client is expecting an array of data without size ahead
    return SendResult(SPA::UDB::idEndRows, sb);
}

void CYourPeerOne::GetCachedTables(unsigned int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg) {
    res = 0;
    do {
        if (SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES == (flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES)) {
            unsigned int chatgroup[] = {SPA::UDB::CACHE_UPDATE_CHAT_GROUP_ID, SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID};
            if (!GetPush().Subscribe(chatgroup, 2)) {
                errMsg = L"Failed in subscribing for table events"; //warning message
            }
        }
        if (!rowset)
            break;
        auto handler = CYourServer::Master->Seek();
        if (!handler) {
            res = -1;
            errMsg = L"No connection to a master database";
            break;
        }
        if (!g_config.m_vFrontCachedTable.size())
            break;
        std::wstring sql;
        for (auto it = g_config.m_vFrontCachedTable.cbegin(), end = g_config.m_vFrontCachedTable.cend(); it != end; ++it) {
            if (sql.size())
                sql += L";";
            sql += L"SELECT * FROM " + SPA::Utilities::ToWide(it->c_str(), it->size());
        }
        bool ok = handler->Execute(sql.c_str(), [&res, &errMsg](CMySQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
            res = r;
            errMsg = err;
        }, [this](CMySQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
            this->SendRows(vData);
        }, [this, index](CMySQLHandler & h) {
            this->SendMeta(h.GetColumnInfo(), index);
        });
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
        ok = handler->WaitAll();
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
    } while (false);
}
