
#pragma once

#include "../../include/udatabase.h"

class CAsyncDBUpdate : public SPA::ClientSide::CAsyncServiceHandler {
public:

    CAsyncDBUpdate(SPA::ClientSide::CClientSocket *cs = nullptr) : CAsyncServiceHandler(0, cs) {
    }
};

class CDBUpdateImpl {
    CDBUpdateImpl();
    CDBUpdateImpl(const CDBUpdateImpl &impl);
    CDBUpdateImpl& operator=(const CDBUpdateImpl &impl);

public:
    ~CDBUpdateImpl();

public:
    unsigned int SetSocketProConnectionString(const wchar_t *connectionString);
    SPA::UINT64 NotifySocketProDatabaseEvent(unsigned int *group, unsigned int count, SPA::UDB::tagUpdateEvent dbEvent, const wchar_t *queryFilter, unsigned int *index, unsigned int size);
    SPA::UINT64 GetSocketProConnections(unsigned int *index, unsigned int size);

private:
    SPA::CUCriticalSection m_cs;
    std::vector<std::pair<SPA::ClientSide::CConnectionContext, unsigned int>> m_vCC;
    std::vector<int> m_vZip;
    std::string m_cert;
    std::shared_ptr<SPA::ClientSide::CSocketPool<CAsyncDBUpdate>> m_pPool;

public:
    static CDBUpdateImpl DBUpdate;

private:
    void Parse(const wchar_t *s);
    static void Trim(std::wstring &s);
};
