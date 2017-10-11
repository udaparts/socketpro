
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"
#include "config.h"

std::chrono::seconds CYourPeerOne::m_timeout(30); //30 seconds

CYourPeerOne::CYourPeerOne() {

}

void CYourPeerOne::OnReleaseSource(bool bClosing, unsigned int info) {
    CYourServer::Slave->Remove((SPA::UINT64) this);
    CYourServer::Master->Remove((SPA::UINT64) this);
}

void CYourPeerOne::OnSwitchFrom(unsigned int nOldServiceId) {

}

void CYourPeerOne::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)
    M_I0_R2(idGetMasterSlaveConnectedSessions, GetMasterSlaveConnectedSessions, unsigned int, unsigned int)
    END_SWITCH
}

int CYourPeerOne::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)
    M_I1_R3(idQueryMaxMinAvgs, QueryPaymentMaxMinAvgs, std::wstring, int, std::wstring, CMaxMinAvg)
    M_I3_R2(SPA::UDB::idGetCachedTables, GetCachedTables, unsigned int, bool, SPA::UINT64, int, std::wstring)
    M_I1_R3(idUploadEmployees, UploadEmployees, SPA::UDB::CDBVariantArray, int, std::wstring, std::vector<SPA::INT64>)
    END_SWITCH
    return 0;
}

void CYourPeerOne::GetMasterSlaveConnectedSessions(unsigned int &m_connections, unsigned int &s_connections) {
    m_connections = CYourServer::Master->GetConnectedSockets();
    s_connections = CYourServer::Slave->GetConnectedSockets();
}

void CYourPeerOne::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, int &res, std::wstring &errMsg, std::vector<SPA::INT64> &vId) {
    res = 0;
    if (!vData.size())
        return;
    if ((vData.size() % 3)) {
        res = -1;
        errMsg = L"Data array size wrong";
        return;
    }

    const wchar_t *sql = L"INSERT INTO mysample.employee(CompanyId,Name,JoinDate)VALUES(?,?,?)";
    //use master for insert, update and delete
    auto handler = CYourServer::Master->Lock();
    if (!handler) {
        res = -1;
        errMsg = L"No connection to a master database";
        return;
    }
    bool ok;
    do {
        //subscribe for socket disconnection event during querying cached tables data
        CYourServer::Master->Subscribe((SPA::UINT64) this, [this, &res, &errMsg] {
            res = -4;
            errMsg = L"Master backend database disconnected during querying for cached tables data";
                    this->m_cv.notify_one();
        });
        CAutoLock al(m_mutex);
        ok = handler->Prepare(sql);
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
        ok = handler->BeginTrans();
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
        SPA::UDB::CDBVariantArray v;
        for (auto it = vData.cbegin(), end = vData.cend(); it != end;) {
            v.push_back(*it);
            v.push_back(*(it + 1));
            v.push_back(*(it + 2));
            ok = handler->Execute(v, [&res, &errMsg, &vId](CMySQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
                if (r && !res) {
                    res = r;
                    errMsg = err;
                    vId.push_back(-1);
                } else if (r)
                    vId.push_back(-1);
                else {
                    assert(affected == 1);
                    assert(!err.size());
                    assert(fail_ok == 1);
                    vId.push_back(vtId.llVal);
                }
            });
            if (!ok) {
                res = handler->GetAttachedClientSocket()->GetErrorCode();
                errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
                break;
            }
            v.clear();
            it += 3;
        }
        if (!ok)
            break;
        ok = handler->EndTrans(SPA::UDB::rpRollbackErrorAll);
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
        auto status = m_cv.wait_for(al, m_timeout); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
        if (status == std::cv_status::timeout) {
            res = -3;
            errMsg = L"Insert table data timeout";
        }
    } while (false);
    CYourServer::Master->Unlock(handler);
}

void CYourPeerOne::QueryPaymentMaxMinAvgs(const std::wstring &filter, int &res, std::wstring &errMsg, CMaxMinAvg &mma) {
    res = 0;
    ::memset(&mma, 0, sizeof (mma));
    std::wstring sql = L"SELECT MAX(amount),MIN(amount),AVG(amount) FROM sakila.payment";
    if (filter.size())
        sql += (L" WHERE " + filter);
    do {
        //we are going to use slave for the query
        auto handler = CYourServer::Slave->Seek();
        if (!handler) {
            res = -1;
            errMsg = L"No connection to a slave database";
            break;
        }
        //subscribe for socket disconnection event during querying max, min and avg
        CYourServer::Slave->Subscribe((SPA::UINT64) this, [this, &res, &errMsg] {
            res = -4;
            errMsg = L"Slave backend database disconnected during querying";
                    this->m_cv.notify_one();
        });
        CAutoLock al(m_mutex);
        bool ok = handler->Execute(sql.c_str(), [this, &res, &errMsg](CMySQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
            res = r;
            errMsg = err;
                    this->m_cv.notify_one();
        }, [&mma, &res, &errMsg](CMySQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
            do {
                if (vData.size() != 3) {
                    res = -2;
                    errMsg = L"Sql statement doesn't generate max, min and average values as expected";
                    break;
                }
                CComVariant vt;
                HRESULT hr = ::VariantChangeType(&vt, &vData[0], 0, VT_R8);
                if (hr != S_OK) {
                    res = hr;
                    errMsg = L"Data type mismatch";
                    break;
                }
                mma.Max = vt.dblVal;
                hr = ::VariantChangeType(&vt, &vData[1], 0, VT_R8);
                if (hr != S_OK) {
                    res = hr;
                    errMsg = L"Data type mismatch";
                    break;
                }
                mma.Min = vt.dblVal;
                hr = ::VariantChangeType(&vt, &vData[2], 0, VT_R8);
                if (hr != S_OK) {
                    res = hr;
                    errMsg = L"Data type mismatch";
                    break;
                }
                mma.Avg = vt.dblVal;
            } while (false);
        }, [](CMySQLHandler & h) {

        });
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
        auto status = m_cv.wait_for(al, m_timeout); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
        if (status == std::cv_status::timeout) {
            res = -3;
            errMsg = L"Querying timeout";
            break;
        }
    } while (false);
}

unsigned int CYourPeerOne::SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index) {
    return SendResult(SPA::UDB::idRowsetHeader, meta, index);
}

unsigned int CYourPeerOne::SendRows(SPA::UDB::CDBVariantArray &vData) {
    SPA::CScopeUQueue sb;
#ifdef WIN32_64
    sb->TimeEx(true);
#endif
    sb << vData;
    unsigned int count;
    sb >> count; //remove data array size at head as a client is expecting an array of data without size ahead
    return SendResult(SPA::UDB::idEndRows, sb);
}

void CYourPeerOne::GetCachedTables(unsigned int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg) {
    res = 0;
    do {
        if (SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES == (flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES)) {
            unsigned int chatgroup[] = {SPA::UDB::CACHE_UPDATE_CHAT_GROUP_ID, SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID};
            if (!GetPush().Subscribe(chatgroup, 2)) {
                errMsg = L"Failed in subscribing for table events"; //warning message
            }
        }
        if (!rowset)
            break;
        auto handler = CYourServer::Master->Seek();
        if (!handler) {
            res = -1;
            errMsg = L"No connection to a master database";
            break;
        }
        if (!g_config.m_vFrontCachedTable.size())
            break;
        std::wstring sql;
        for (auto it = g_config.m_vFrontCachedTable.cbegin(), end = g_config.m_vFrontCachedTable.cend(); it != end; ++it) {
            if (sql.size())
                sql += L";";
            sql += L"SELECT * FROM " + SPA::Utilities::ToWide(it->c_str(), it->size());
        }
        //subscribe for socket disconnection event during querying cached tables data
        CYourServer::Master->Subscribe((SPA::UINT64) this, [this, &res, &errMsg] {
            res = -4;
            errMsg = L"Master backend database disconnected during querying for cached tables data";
                    this->m_cv.notify_one();
        });
        CAutoLock al(m_mutex);
        bool ok = handler->Execute(sql.c_str(), [this, &res, &errMsg](CMySQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
            res = r;
            errMsg = err;
                    this->m_cv.notify_one();
        }, [this](CMySQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
            this->SendRows(vData);
        }, [this, index](CMySQLHandler & h) {
            this->SendMeta(h.GetColumnInfo(), index);
        });
        if (!ok) {
            res = handler->GetAttachedClientSocket()->GetErrorCode();
            errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
            break;
        }
        auto status = m_cv.wait_for(al, m_timeout); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
        if (status == std::cv_status::timeout) {
            res = -3;
            errMsg = L"Querying cached table data timeout";
            break;
        }
    } while (false);
}
