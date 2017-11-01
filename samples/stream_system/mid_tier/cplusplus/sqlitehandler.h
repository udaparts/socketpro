#pragma once

#include "../../../../include/async_sqlite.h"

/*
SQLite supports multiple databases by statement ATTACH DATABASE ....
Therefore, we use the class for attaching multiple databases onto one session
*/
class CSqliteHandler : public SPA::ClientSide::CSqlite {
public:
    CSqliteHandler(SPA::ClientSide::CClientSocket *cs = nullptr);

	typedef SPA::ClientSide::CSqliteBase CSQLHandler;

public:
    virtual bool Open(const wchar_t* strConnection, DResult handler, unsigned int flags = 0, DCanceled canceled = nullptr);
    
protected:
	virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);

private:
    CSqliteHandler(const CSqliteHandler &s);
    CSqliteHandler& operator=(const CSqliteHandler &s);
};
