#pragma once

#include "../../../../include/async_sqlite.h"

/*
SQLite supports multiple databases by statement ATTACH DATABASE ....
Therefore, we use the class for attaching multiple databases onto one session
*/
class CSqliteHandler : public SPA::ClientSide::CSqlite {
public:
    CSqliteHandler(SPA::ClientSide::CClientSocket *cs = nullptr);

public:
    virtual bool Open(const wchar_t* strConnection, DResult handler, unsigned int flags = 0, DCanceled canceled = nullptr);
    virtual bool Execute(const wchar_t* sql, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader(), bool meta = true, bool lastInsertId = true, DCanceled canceled = nullptr);
    virtual bool Execute(SPA::UDB::CDBVariantArray &vParam, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader(), bool meta = true, bool lastInsertId = true, DCanceled canceled = nullptr);

protected:
	virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);

private:
    CSqliteHandler(const CSqliteHandler &s);
    CSqliteHandler& operator=(const CSqliteHandler &s);

    SPA::CUCriticalSection m_sqliteOne;
};
