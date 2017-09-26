
#include "stdafx.h"
#include "webasynchandler.h"


CWebAsyncHandler::CWebAsyncHandler(CClientSocket *pClientSocket) 
	: CAsyncServiceHandler(sidStreamSystem, pClientSocket)
{
}

bool CWebAsyncHandler::SubscribeAndGetInitialCachedTablesData(DResult handler) {
	return false;
}

bool CWebAsyncHandler::SetDefaultDatabaseName(const wchar_t *dbName, DResult handler, bool optimistic) {
	return false;
}

bool CWebAsyncHandler::BeginBatchProcessing(bool readonly, bool manualTrans, DResult handler) {
	return false;
}

bool CWebAsyncHandler::EndBatchProcessing(unsigned int hints, DResult handler) {
	return false;
}

bool CWebAsyncHandler::QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma, DResult handler) {
	return false;
}