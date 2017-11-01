#include "stdafx.h"
#include "sqlitehandler.h"
#include "config.h"

#ifndef NDEBUG
#include <iostream>
#endif

CSqliteHandler::CSqliteHandler(SPA::ClientSide::CClientSocket *cs)
	: SPA::ClientSide::CSqlite(cs) {
}

bool CSqliteHandler::Open(const wchar_t* strConnection, DResult handler, unsigned int flags, DCanceled canceled) {
	std::wstring db((strConnection && ::wcslen(strConnection)) ? strConnection : g_config.m_master_default_db);
	flags |= SPA::Sqlite::DATABASE_AUTO_ATTACHED;
	return SPA::ClientSide::CSqlite::Open(db.c_str(), handler, flags, canceled);
}

void CSqliteHandler::OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
#ifndef NDEBUG
	//for debug purpose
	std::cout << "Request id: " << requestId << ", error message: ";
	std::wcout << errMessage;
	std::cout << ", error where: " << errMessage << ", error code: " << errCode << std::endl;
#endif
	SPA::ClientSide::CSqlite::OnExceptionFromServer(requestId, errMessage, errWhere, errCode);
}
