
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"
#include "config.h"

#ifndef NDEBUG
SPA::CUCriticalSection CYourPeerOne::m_csConsole;
#endif

CYourPeerOne::CYourPeerOne() {

}

void CYourPeerOne::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
	if (reqId == idQueryMaxMinAvgs) {
		QueryPaymentMaxMinAvgs(m_UQueue, GetCurrentRequestIndex());
		return;
	}
	else if (reqId == idUploadEmployees) {
		UploadEmployees(m_UQueue, GetCurrentRequestIndex());
		return;
	}
	else if (reqId == idGetRentalDateTimes) {
		GetRentalDateTimes(m_UQueue, GetCurrentRequestIndex());
		return;
	}
	BEGIN_SWITCH(reqId)
		M_I0_R2(idGetMasterSlaveConnectedSessions, GetMasterSlaveConnectedSessions, unsigned int, unsigned int)
		END_SWITCH
}

int CYourPeerOne::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I3_R3(SPA::UDB::idGetCachedTables, GetCachedTables, std::wstring, unsigned int, SPA::UINT64, int, int, std::wstring)
		END_SWITCH
		return 0;
}

void CYourPeerOne::GetMasterSlaveConnectedSessions(unsigned int &m_connections, unsigned int &s_connections) {
	m_connections = CYourServer::Master->GetConnectedSockets();
	s_connections = CYourServer::Slave->GetConnectedSockets();
}

#if 1
void CYourPeerOne::UploadEmployees(SPA::CUQueue &q, SPA::UINT64 reqIndex) {
	unsigned int ret;
	std::shared_ptr<SPA::UDB::CDBVariantArray> pData(new SPA::UDB::CDBVariantArray);
	q >> *pData;
	//assuming there is no local queue (no request backup) for master
	assert(CYourServer::Master->GetQueueName().size() == 0);
	std::shared_ptr<std::pair<int, std::wstring> > pError(new std::pair<int, std::wstring>(0, L""));
	std::shared_ptr<CInt64Array> pId(new CInt64Array);
	if (!pData->size()) {
		ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
		return;
	}
	else if ((pData->size() % 3)) {
		pError->first = -1;
		pError->second = L"Data array size is wrong";
		ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
		return;
	}
	//use master for insert, update and delete
	auto handler = CYourServer::Master->Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
	if (!handler) {
		pError->first = -2;
		pError->second = L"No connection to a master database";
		ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
		return;
	}
	CClientSocket *cs = handler->GetAttachedClientSocket();
	do {
		if (!handler->BeginTrans() || !handler->Prepare(L"INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) break;
		bool ok = true;
		SPA::UDB::CDBVariantArray v;
		for (auto it = pData->cbegin(), end = pData->cend(); it != end;) {
			v.push_back(*it);
			v.push_back(*(it + 1));
			v.push_back(*(it + 2));
			ok = handler->Execute(v, [pError, pId](CSQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
				if (r) {
					if (!pError->first) {
						//we only report the first error back to front caller
						pError->first = r;
						pError->second = err;
					}
					pId->push_back(-1);
				}
				else {
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
		if (!handler->EndTrans(SPA::UDB::rpRollbackErrorAll, [reqIndex, peer_handle, pError, pId, this](CSQLHandler & h, int r, const std::wstring & err) {
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
		}, [reqIndex, pId, this, peer_handle, pError](SPA::ClientSide::CAsyncServiceHandler *h, bool canceled) {
			if (peer_handle == this->GetSocketHandle()) {
				CClientSocket *cs = h->GetAttachedClientSocket();
				pError->first = cs->GetErrorCode();
				std::string err_msg = cs->GetErrorMsg();
				pError->second = SPA::Utilities::ToWide(err_msg.c_str(), err_msg.size());
				unsigned int ret = this->SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
			}
		}))
			break;
		//put back locked handler and its socket back into pool for reuse as soon as possible
		CYourServer::Master->Unlock(handler);
		return;
	} while (false);
	pError->first = cs->GetErrorCode();
	std::string err_msg = cs->GetErrorMsg();
	pError->second = SPA::Utilities::ToWide(err_msg.c_str(), err_msg.size());
	ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
}
#else
//manual retry for better fault tolerance
void CYourPeerOne::UploadEmployees(SPA::CUQueue &q, SPA::UINT64 reqIndex) {
	unsigned int ret;
	std::shared_ptr<SPA::UDB::CDBVariantArray> pData(new SPA::UDB::CDBVariantArray);
	q >> *pData;
	//assuming there is no local queue (no request backup) for master
	assert(CYourServer::Master->GetQueueName().size() == 0);
	std::shared_ptr<std::pair<int, std::wstring> > pError(new std::pair<int, std::wstring>(0, L""));
	std::shared_ptr<CInt64Array> pId(new CInt64Array);
	if (!pData->size()) {
		ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
		return;
	}
	else if ((pData->size() % 3)) {
		pError->first = -1;
		pError->second = L"Data array size is wrong";
		ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
		return;
	}
	int redo = 0;
	do {
		//use master for insert, update and delete
		auto handler = CYourServer::Master->Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
		if (!handler) {
			pError->first = -2;
			pError->second = L"No connection to a master database";
			ret = SendResultIndex(reqIndex, idUploadEmployees, pError->first, pError->second, *pId);
			return;
		}
		++redo;
		do {
			bool ok = false;
			if (!handler->BeginTrans()) break;
			if (!handler->Prepare(L"INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) break;
			SPA::UDB::CDBVariantArray v;
			for (auto it = pData->cbegin(), end = pData->cend(); it != end;) {
				v.push_back(*it);
				v.push_back(*(it + 1));
				v.push_back(*(it + 2));
				ok = handler->Execute(v, [pError, pId](CSQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
					if (r) {
						if (!pError->first) {
							//we only report the first error back to front caller
							pError->first = r;
							pError->second = err;
						}
						pId->push_back(-1);
					}
					else {
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
			if (handler->EndTrans(SPA::UDB::rpRollbackErrorAll, [reqIndex, peer_handle, pError, pId, this](CSQLHandler & h, int r, const std::wstring & err) {
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
			}, [reqIndex, pData, this, peer_handle](SPA::ClientSide::CAsyncServiceHandler *h, bool canceled) {
				//we need to retry as long as front socket is not closed yet
				if (peer_handle == this->GetSocketHandle()) {
#ifndef NDEBUG
						{
							SPA::CAutoLock al(m_csConsole);
							//socket closed after sending
							std::cout << "Retrying UploadEmployees ......" << std::endl;
						}
#endif
						SPA::CScopeUQueue sq;
						//repack original request data and retry if socket is closed after sending
						sq << *pData;
						this->UploadEmployees(*sq, reqIndex); //this will not cause recursive stack-overflow exeption
				}
			})) {
				CYourServer::Master->Unlock(handler); //put back locked handler and its socket back into pool for reuse as soon as possible
				redo = 0; //disable redo only if all requests are successfully put onto wire
			}
			else {
				//socket closed when sending
			}
		} while (false);
	} while (redo);
}
#endif

void CYourPeerOne::GetRentalDateTimes(SPA::CUQueue &q, SPA::UINT64 reqIndex) {
	SPA::INT64 rental_id;
	//assuming slave pool has queue name set (request backup)
	assert(CYourServer::Slave->GetQueueName().size());
	q >> rental_id;
	std::wstring sql = L"SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=" + std::to_wstring(rental_id);
	std::shared_ptr<CSQLHandler> handler = CYourServer::Slave->SeekByQueue();
	if (!handler) {
		CRentalDateTimes dates;
		unsigned int res = SendResultIndex(reqIndex, idGetRentalDateTimes, dates, (int)-1, L"No connection to a slave database");
		return;
	}
	auto peer_handle = GetSocketHandle();
	std::shared_ptr<CRentalDateTimes> dates(new CRentalDateTimes);
	bool ok = handler->Execute(sql.c_str(), [this, reqIndex, dates, peer_handle](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
		//front peer not closed yet
		if (peer_handle == this->GetSocketHandle()) {
			this->SendResultIndex(reqIndex, idGetRentalDateTimes, *dates, r, err);
		}
	}, [dates](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
		dates->rental_id = vData[0].llVal;
		dates->Rental = vData[1].ullVal; //date time in high precision format
		dates->Return = vData[2].ullVal;
		dates->LastUpdate = vData[3].ullVal;
	});
	assert(ok); //should always be true because slave pool has queue name set for request backup
}

void CYourPeerOne::QueryPaymentMaxMinAvgs(SPA::CUQueue &q, SPA::UINT64 reqIndex) {
	std::wstring filter;
	//assuming slave pool has queue name set (request backup)
	assert(CYourServer::Slave->GetQueueName().size());
	q >> filter;
	std::wstring sql = L"SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
	if (filter.size())
		sql += (L" WHERE " + filter);
	std::shared_ptr<CSQLHandler> handler = CYourServer::Slave->SeekByQueue();
	if (!handler) {
		CMaxMinAvg mma;
		unsigned int ret = SendResultIndex(reqIndex, idQueryMaxMinAvgs, (int)-1, L"No connection to a slave database", mma);
		return;
	}
	std::shared_ptr<CMaxMinAvg> pmma(new CMaxMinAvg);
	auto peer_handle = GetSocketHandle();
	bool ok = handler->Execute(sql.c_str(), [reqIndex, peer_handle, pmma, this](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
		//front peer not closed yet
		if (peer_handle == this->GetSocketHandle()) {
			unsigned int ret = this->SendResultIndex(reqIndex, idQueryMaxMinAvgs, r, err, *pmma);
		}
	}, [pmma](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
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

void CYourPeerOne::GetCachedTables(const std::wstring &defaultDb, unsigned int flags, SPA::UINT64 index, int &dbMS, int &res, std::wstring &errMsg) {
	res = 0;
	dbMS = (int)SPA::UDB::msUnknown;
	do {
		if (!g_config.m_vFrontCachedTable.size() || SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES != (flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES))
			break;
		if (SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES == (flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES)) {
			unsigned int chatgroup[] = { SPA::UDB::CACHE_UPDATE_CHAT_GROUP_ID, SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID };
			if (!GetPush().Subscribe(chatgroup, 2)) {
				errMsg = L"Failed in subscribing for table events"; //warning message
			}
		}
		std::wstring sql;
		for (auto it = g_config.m_vFrontCachedTable.cbegin(), end = g_config.m_vFrontCachedTable.cend(); it != end; ++it) {
			if (sql.size())
				sql += L";";
			sql += L"SELECT * FROM " + *it;
		}
		auto handler = CYourServer::Master->Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
		if (!handler) {
			res = -1;
			errMsg = L"No connection to a master database";
			break;
		}
		dbMS = (int)handler->GetDBManagementSystem();
		std::shared_ptr<std::promise<void> > prom(new std::promise<void>, [](std::promise<void> *p) {
			delete p;
		});
		if (!handler->Execute(sql.c_str(), [prom, &res, &errMsg](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
			res = r;
			errMsg = err;
			prom->set_value();
		}, [this](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
			this->SendRows(vData);
		}, [this, index](CSQLHandler & h) {
			this->SendMeta(h.GetColumnInfo(), index);
		}, true, true, [prom, &res, &errMsg](SPA::ClientSide::CAsyncServiceHandler *h, bool canceled) {
			res = -2;
			errMsg = L"Request canceled or socket closed";
			prom->set_value();
		})) {
			res = handler->GetAttachedClientSocket()->GetErrorCode();
			errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
			break;
		}
		CYourServer::Master->Unlock(handler); //put back locked handler and its socket back into pool for reuse as soon as possible
		auto status = prom->get_future().wait_for(std::chrono::seconds(25)); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
		if (status == std::future_status::timeout) {
			res = -3;
			errMsg = L"Querying cached table data timeout";
			break;
		}
	} while (false);
}
