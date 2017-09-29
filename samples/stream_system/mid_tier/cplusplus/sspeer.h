#pragma once

#include "../../shared/ss_defines.h"

class CSSPeer : public CClientPeer {
public:
    CSSPeer();
    ~CSSPeer();

protected:
    virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
    virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);

private:
    void SubscribeAndGetInitialCachedTablesData(int &res, std::wstring &errMsg);
    void SetDefaultDatabaseName(const std::wstring &dbName, bool optimistic, bool slaveCheck, int &res, std::wstring &errMsg);
    void BeginBatchProcessing(bool readonly, bool manualTrans, int &res, std::wstring &errMsg);
    void EndBatchProcessing(unsigned int hints, int &res, std::wstring &errMsg);


    void QueryMaxMinAvgs(const std::wstring &sql, CMaxMinAvg &mma, int &res, std::wstring &errMsg);

private:
    std::wstring m_dbDefaultName;

private:
    CSSPeer(const CSSPeer &p);
    CSSPeer& operator=(const CSSPeer &p);
};
