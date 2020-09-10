#include "stdafx.h"
#include "ssserver.h"

CYourPeerOne::CYourPeerOne() {
}

void CYourPeerOne::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
    if (reqId == idQueryMaxMinAvgs) {
        QueryPaymentMaxMinAvgs(m_UQueue, GetCurrentRequestIndex());
    } else if (reqId == idUploadEmployees) {
        UploadEmployees(m_UQueue, GetCurrentRequestIndex());
    } else if (reqId == idGetRentalDateTimes) {
        GetRentalDateTimes(m_UQueue, GetCurrentRequestIndex());
    } else {
        BEGIN_SWITCH(reqId)
        M_I0_R2(idGetMasterSlaveConnectedSessions, GetMasterSlaveConnectedSessions, unsigned int, unsigned int);
        END_SWITCH
    }
}

int CYourPeerOne::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
    BEGIN_SWITCH(reqId)
    M_I3_R3(UDB::idGetCachedTables, GetCachedTables, CDBString, unsigned int, UINT64, int, int, wstring);
    END_SWITCH
    return 0;
}

void CYourPeerOne::GetMasterSlaveConnectedSessions(unsigned int& m_connections, unsigned int& s_connections) {
    m_connections = CYourServer::Master->GetConnectedSockets();
    s_connections = CYourServer::Slave->GetConnectedSockets();
}

void CYourPeerOne::GetRentalDateTimes(CUQueue& buffer, UINT64 reqIndex) {
    INT64 rental_id;
    //assuming slave pool has queue name set (request backup)
    assert(CYourServer::Slave->GetQueueName().size());
    buffer >> rental_id;
    wstring sql = L"SELECT rental_date,return_date,last_update FROM rental where rental_id=" + to_wstring(rental_id);
    shared_ptr<CMysql> handler = CYourServer::Slave->SeekByQueue();
    if (!handler) {
        CRentalDateTimes dates;
        unsigned int res = SendResultIndex(reqIndex, idGetRentalDateTimes, dates, (int)-2, L"No connection to anyone of slave databases");
        return;
    }
    auto peer_handle = GetSocketHandle();
    shared_ptr<CRentalDateTimes> dates(new CRentalDateTimes);
    dates->rental_id = rental_id;
    bool ok = handler->Execute(sql.c_str(), [this, reqIndex, dates, peer_handle](CMysql& h, int r, const wstring& err, INT64 affected, UINT64 fail_ok, UDB::CDBVariant & vtId) {
        //front peer not closed yet
        if (peer_handle == this->GetSocketHandle()) {
            this->SendResultIndex(reqIndex, idGetRentalDateTimes, *dates, r, err);
        }
    }, [dates](CMysql& h, UDB::CDBVariantArray& vData) {
        dates->Rental = vData[0].ullVal; //date time in high precision format
        if (vData[1].vt == VT_DATE)
            dates->Return = vData[1].ullVal;
        dates->LastUpdate = vData[2].ullVal;
    });
    assert(ok); //should always be true because slave pool has queue name set for request backup
}

void CYourPeerOne::QueryPaymentMaxMinAvgs(CUQueue& buffer, UINT64 reqIndex) {
    //assuming slave pool has queue name set (request backup)
    assert(CYourServer::Slave->GetQueueName().size());
    CDBString filter;
    buffer >> filter;
    CDBString sql = u"SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
    if (filter.size())
        sql += (u" WHERE " + filter);
    shared_ptr<CMysql> handler = CYourServer::Slave->SeekByQueue();
    if (!handler) {
        CMaxMinAvg mma;
        unsigned int ret = SendResultIndex(reqIndex, idQueryMaxMinAvgs, (int)-2, L"No connection to anyone of slave databases", mma);
        return;
    }
    shared_ptr<CMaxMinAvg> pmma(new CMaxMinAvg);
    auto peer_handle = GetSocketHandle();
    bool ok = handler->Execute(sql.c_str(), [reqIndex, peer_handle, pmma, this](CMysql& h, int r, const wstring& err, INT64 affected, UINT64 fail_ok, UDB::CDBVariant& vtId) {
        //front peer not closed yet
        if (peer_handle == this->GetSocketHandle()) {
            unsigned int ret = this->SendResultIndex(reqIndex, idQueryMaxMinAvgs, r, err, *pmma);
        }
    }, [pmma](CMysql& h, UDB::CDBVariantArray & vData) {
        CComVariant temp;
        ::VariantChangeType(&temp, &vData[0], 0, VT_R8);
        pmma->Max = temp.dblVal;
        ::VariantChangeType(&temp, &vData[1], 0, VT_R8);
        pmma->Min = temp.dblVal;
        ::VariantChangeType(&temp, &vData[2], 0, VT_R8);
        pmma->Avg = temp.dblVal;
    });
    assert(ok); //should always be true because slave pool has queue name set for request backup
}

void CYourPeerOne::GetCachedTables(const CDBString& defaultDb, unsigned int flags, UINT64 index, int& dbMS, int& res, wstring& errMsg) {
    res = 0;
    dbMS = (int) UDB::msUnknown;
    if (!CYourServer::FrontCachedTables.size() || UDB::ENABLE_TABLE_UPDATE_MESSAGES != (flags & UDB::ENABLE_TABLE_UPDATE_MESSAGES))
        return;
    if (UDB::ENABLE_TABLE_UPDATE_MESSAGES == (flags & UDB::ENABLE_TABLE_UPDATE_MESSAGES)) {
        unsigned int chatgroup[] = {UDB::CACHE_UPDATE_CHAT_GROUP_ID, UDB::STREAMING_SQL_CHAT_GROUP_ID};
        if (!GetPush().Subscribe(chatgroup, 2)) {
            errMsg = L"Failed in subscribing for table events"; //warning message
        }
    }
    CDBString sql;
    for (auto it = CYourServer::FrontCachedTables.cbegin(), end = CYourServer::FrontCachedTables.cend(); it != end; ++it) {    
        if (sql.size())
            sql += u";";
        sql += u"SELECT * FROM " + *it;
    }
    //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
    auto handler = CYourServer::Master->Lock();
    if (!handler) {
        res = -2;
        errMsg = L"No connection to a master database";
        return;
    }
    dbMS = (int) handler->GetDBManagementSystem();
    try{
        auto f = handler->execute(sql.c_str(), [this](CMysql& h, UDB::CDBVariantArray& vData) {
            this->SendRows(vData);
        }, [this, index](CMysql & h) {
            this->SendMeta(h.GetColumnInfo(), index);
        });
        //put back locked handler and its socket back into pool for reuse as soon as possible
        CYourServer::Master->Unlock(handler);
        auto status = f.wait_for(chrono::seconds(30));
        if (status == future_status::timeout) {
            res = -3;
            errMsg = L"Querying cached table data timeout";
        } else {
            auto result = f.get();
            res = result.ec;
            errMsg = result.em;
        }
    }
    catch(CSocketError & ex) {
        res = ex.ec;
        errMsg = ex.em;
    }
}

void CYourPeerOne::UploadEmployees(CUQueue& buffer, UINT64 reqIndex) {
    unsigned int ret;
    shared_ptr<UDB::CDBVariantArray> pData(new UDB::CDBVariantArray);
    buffer >> *pData;
    //assuming there is no local queue (no request backup) for master
    assert(CYourServer::Master->GetQueueName().size() == 0);
    shared_ptr<pair<int, wstring> > pError(new pair<int, wstring>(0, L""));
    shared_ptr<CInt64Array> pId(new CInt64Array);
    if (!pData->size()) {
        ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
        return;
    } else if ((pData->size() % 3)) {
        pError->first = -1;
        pError->second = L"Data array size is wrong";
        ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
        return;
    }
    //use master for insert, update and delete
    //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
    auto handler = CYourServer::Master->Lock();
    if (!handler) {
        pError->first = -2;
        pError->second = L"No connection to a master database";
        ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
        return;
    }
    CClientSocket* cs = handler->GetSocket();
    do {
        if (!handler->BeginTrans() || !handler->Prepare(L"INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) break;
        bool ok = true;
        UDB::CDBVariantArray v;
        for (auto it = pData->cbegin(), end = pData->cend(); it != end;) {
            v.push_back(*it);
            v.push_back(*(it + 1));
            v.push_back(*(it + 2));
            ok = handler->Execute(v, [pError, pId](CMysql& h, int r, const wstring& err, INT64 affected, UINT64 fail_ok, UDB::CDBVariant& vtId) {
                if (r) {
                    if (!pError->first) {
                        //we only report the first error back to front caller
                        pError->first = r;
                        pError->second = err;
                    }
                    pId->push_back(-1);
                } else {
                    assert(affected == 1);
                    assert(!err.size());
                    assert(fail_ok == 1);
                    pId->push_back(vtId.llVal);
                }
            });
            if (!ok) break;
            v.clear();
            it += 3;
        }
        if (!ok) break;
        auto peer_handle = GetSocketHandle();
        if (!handler->EndTrans(UDB::rpRollbackErrorAll, [reqIndex, peer_handle, pError, pId, this](CMysql & h, int r, const wstring& err) {
                //send result if front socket is not closed yet
                if (peer_handle == this->GetSocketHandle()) {
                    if (r) {
                        if (!pError->first) {
                            //we only report the first error back to front caller
                            pError->first = r;
                            pError->second = err;
                        }
                    }
                    unsigned int ret = this->SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
                }
            }, [reqIndex, pId, this, peer_handle, pError](ClientSide::CAsyncServiceHandler* h, bool canceled) {
                if (peer_handle == this->GetSocketHandle()) {
                    CClientSocket* cs = h->GetSocket();
                    pError->first = cs->GetErrorCode();
                    string err_msg = cs->GetErrorMsg();
                    pError->second = Utilities::ToWide(err_msg.c_str(), err_msg.size());
                    unsigned int ret = this->SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
                }
            })) {
            break;
        }
        //put back locked handler and its socket back into pool for reuse as soon as possible
        CYourServer::Master->Unlock(handler);
        return;
    } while (false);
    pError->first = cs->GetErrorCode();
    string err_msg = cs->GetErrorMsg();
    pError->second = Utilities::ToWide(err_msg.c_str(), err_msg.size());
    ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
}
