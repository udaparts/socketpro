
#pragma once

#include "../../shared/ss_defines.h"
#include "../../../../include/generalcache.h"

using namespace SPA::ClientSide;

class CWebAsyncHandler : public CCachedBaseHandler < sidStreamSystem > {
public:
    CWebAsyncHandler(CClientSocket *pClientSocket = nullptr);

public:
    typedef std::function<void(SPA::UINT64 index) > DMyCanceled;
    typedef std::function<void(SPA::UINT64 index, const CMaxMinAvg &mma, int res, const std::wstring &errMsg) > DMaxMinAvg;
    typedef std::function<void(SPA::UINT64 index, unsigned int, unsigned int) > DConnectedSessions;
    typedef std::function<void(SPA::UINT64 index, int res, const std::wstring &errMsg, CInt64Array &vId) > DUploadEmployees;
    typedef std::function<void(SPA::UINT64 index, const CRentalDateTimes &dates, int res, const std::wstring &errMsg) > DRentalDateTimes;
    typedef std::function<void(SPA::UINT64 index, int res, const std::wstring &errMsg) > DSequeue;

public:
    SPA::UINT64 QueryPaymentMaxMinAvgs(const wchar_t *filter, DMaxMinAvg mma, DMyCanceled canceled = nullptr);
    SPA::UINT64 GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyCanceled canceled = nullptr);
    SPA::UINT64 UploadEmployees(const SPA::UDB::CDBVariantArray &vData, DUploadEmployees res, DMyCanceled canceled = nullptr);
    SPA::UINT64 StartSequence(DSequeue seq, DMyCanceled canceled = nullptr);
    SPA::UINT64 EndSequence(DSequeue seq, DMyCanceled canceled = nullptr);
    SPA::UINT64 GetRentalDateTimes(SPA::INT64 rentalId, DRentalDateTimes rdt, DMyCanceled canceled = nullptr);

private:
    CWebAsyncHandler(const CWebAsyncHandler &wah);
    CWebAsyncHandler& operator=(const CWebAsyncHandler &wah);

private:
    SPA::CUCriticalSection m_csSS;
    SPA::UINT64 m_ssIndex;
    std::unordered_map<SPA::UINT64, std::pair<DMaxMinAvg, DMyCanceled> > m_mapMMA;
    std::unordered_map<SPA::UINT64, std::pair<DConnectedSessions, DMyCanceled> > m_mapSession;
    std::unordered_map<SPA::UINT64, std::pair<DUploadEmployees, DMyCanceled> > m_mapUpload;
    std::unordered_map<SPA::UINT64, std::pair<DSequeue, DMyCanceled> > m_mapSequence;
    std::unordered_map<SPA::UINT64, std::pair<DRentalDateTimes, DMyCanceled> > m_mapRentalDateTimes;
};
