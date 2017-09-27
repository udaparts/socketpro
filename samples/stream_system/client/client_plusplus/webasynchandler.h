
#pragma once

#include "../../shared/ss_defines.h"

class CWebAsyncHandler : public CAsyncServiceHandler {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
    typedef std::function<void(CWebAsyncHandler &sender, int res, const std::wstring &errMsg) > DResult;
    typedef std::function<void(CWebAsyncHandler &sender, const CMaxMinAvg &mma) > DMaxMinAvg;

public:
    /**
     * Subscribe update events (update, delete and insert) for cached tables at server side and get initial data of cached tables from database to this client
     * @param handler a callback for returning error message
     * @return true if request is successfully sent or queued; and false if failed
     */
    bool SubscribeAndGetInitialCachedTablesData(DResult handler = DResult());

    /**
     * Set default database
     * @param dbName a new database name which can be a null or empty string
     * @param handler a callback for returning error message
     * @param optimistic true for no checking dbName against a backend DB server; and false for checking it against a backend DB server 
     * @param slaveCheck true if checking dbName against one of slave DB server; false if check dbName against master DB server
     * @return true if request is successfully sent or queued; and false if failed
     */
    bool SetDefaultDatabaseName(const wchar_t *dbName, DResult handler = DResult(), bool optimistic = true, bool slaveCheck = false);
    
    /**
     * 
     * @param readonly
     * @param manualTrans
     * @param handler
     * @return true if request is successfully sent or queued; and false if failed
     */
    bool BeginBatchProcessing(bool readonly, bool manualTrans, DResult handler = DResult());
    
    /**
     * 
     * @param hints
     * @param handler
     * @return true if request is successfully sent or queued; and false if failed
     */
    bool EndBatchProcessing(unsigned int hints = 0, DResult handler = DResult());
    
    /**
     * 
     * @param sql
     * @param mma
     * @param handler
     * @return true if request is successfully sent or queued; and false if failed
     */
    bool QueryMaxMinAvgs(const wchar_t *sql, DMaxMinAvg mma, DResult handler = DResult());

};
