#include "stdafx.h"
#include "sqlitehandler.h"
#include <iostream>
#include "config.h"

CSqliteHandler::CSqliteHandler(SPA::ClientSide::CClientSocket *cs)
: SPA::ClientSide::CSqlite(cs) {
}

bool CSqliteHandler::Open(const wchar_t* strConnection, DResult handler, unsigned int flags, DCanceled canceled) {
    std::wstring db((strConnection && ::wcslen(strConnection)) ? strConnection : g_config.m_master_default_db);
    SPA::CAutoLock al(m_sqliteOne);
    if (!SPA::ClientSide::CSqlite::Open(db.c_str(), [](CSQLHandler & dbHandler, int res, const std::wstring & errMsg) {
            if (res) {
                std::cout << "CSqliteHandler::Open error code = " << res << ", error message :";
                std::wcout << errMsg.c_str() << std::endl;
            }
        }, flags, nullptr))
    return false;
    if (!db.size())
        return true;
    std::wstring name = db;
    size_t pos = db.rfind(L".db");
    if (pos == std::wstring::npos)
        return true;
    name = db.substr(0, pos);
    std::wstring sql = L"ATTACH DATABASE '" + db + L"' AS " + name;
    return Execute(sql.c_str(), [handler](CSQLHandler &dbHandler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
        if (res) {
            std::cout << "CSqliteHandler::Open error code = " << res << ", error message :";
            std::wcout << errMsg.c_str() << std::endl;
        }
        if (handler) {
            handler(dbHandler, res, errMsg);
        }
    }, nullptr, nullptr, false, false, canceled);
}

bool CSqliteHandler::Execute(const wchar_t* sql, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta, bool lastInsertId, DCanceled canceled) {
    if (sql && ::wcslen(sql))
        return SPA::ClientSide::CSqlite::Execute(sql, handler, row, rh, meta, lastInsertId, canceled);
    std::wstring my_sql;
    for (auto it = g_config.m_vFrontCachedTable.cbegin(), end = g_config.m_vFrontCachedTable.cend(); it != end; ++it) {
        if (my_sql.size())
            my_sql += L";";
        my_sql += L"SELECT * FROM " + *it;
    }
    return SPA::ClientSide::CSqlite::Execute(my_sql.c_str(), handler, row, rh, meta, lastInsertId, canceled);
}

bool CSqliteHandler::Execute(SPA::UDB::CDBVariantArray &vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, bool meta, bool lastInsertId, DCanceled canceled) {
    return SPA::ClientSide::CSqlite::Execute(vParam, handler, row, rh, meta, lastInsertId, canceled);
}
