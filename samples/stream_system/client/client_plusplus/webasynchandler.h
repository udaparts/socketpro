
#pragma once
class CWebAsyncHandler : public CAsyncServiceHandler
{
public:
	CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
	typedef std::function<void(CWebAsyncHandler &sender, int res, const std::wstring &errMsg) > DResult;
	typedef std::function<void(CWebAsyncHandler &sender, const CMaxMinAvg &mma) > DMaxMinAvg;

public:
	/**
	* Subscribe update events (update, delete and insert) for cached tables at server side and get initial data of cached tables from database to this client
	* @param handler a callback for returning error message
	* @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
	*/
	bool SubscribeAndGetInitialCachedTablesData(DResult handler = DResult());

	/**
	* Set default database
	* @param dbName a new database name
	* @param handler a callback for returning error message
	* @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
	*/
	bool SetDefaultDatabaseName(const wchar_t *dbName, DResult handler = DResult(), bool optimistic = true);

	bool BeginBatchProcessing(bool readonly, bool manualTrans, DResult handler = DResult());

	bool EndBatchProcessing(unsigned int hints = 0, DResult handler = DResult());

	bool QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma, DResult handler = DResult());

};
