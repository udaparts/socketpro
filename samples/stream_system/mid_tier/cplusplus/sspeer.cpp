
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
		QueryPaymentMaxMinAvgs(m_UQueue);
		return;
	}
	else if (reqId == idUploadEmployees) {
		UploadEmployees(m_UQueue);
		return;
	}
	BEGIN_SWITCH(reqId)
		M_I1_R3(idGetMasterSlaveConnectedSessions, GetMasterSlaveConnectedSessions, SPA::UINT64, SPA::UINT64, unsigned int, unsigned int)
		END_SWITCH
}

int CYourPeerOne::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I4_R2(SPA::UDB::idGetCachedTables, GetCachedTables, std::wstring, unsigned int, bool, SPA::UINT64, int, std::wstring)
		M_I2_R4(idGetRentalDateTimes, GetRentalDateTimes, SPA::UINT64, SPA::INT64, SPA::UINT64, CRentalDateTimes, int, std::wstring)
		END_SWITCH
		return 0;
}

void CYourPeerOne::GetMasterSlaveConnectedSessions(SPA::UINT64 index, SPA::UINT64 &retIndex, unsigned int &m_connections, unsigned int &s_connections) {
	retIndex = index;
	m_connections = CYourServer::Master->GetConnectedSockets();
	s_connections = CYourServer::Slave->GetConnectedSockets();
}

void CYourPeerOne::UploadEmployees(SPA::CUQueue &q) {
	unsigned int ret;
	SPA::UINT64 index;
	std::shared_ptr<SPA::UDB::CDBVariantArray> pData(new SPA::UDB::CDBVariantArray);
	q >> index >> *pData;
	std::shared_ptr<std::pair<int, std::wstring> > pError(new std::pair<int, std::wstring>(0, L""));
	std::shared_ptr<CInt64Array> pId(new CInt64Array);
	if (!pData->size()) {
		ret = SendResult(idUploadEmployees, index, pError->first, pError->second, *pId);
		return;
	}
	else if ((pData->size() % 3)) {
		pError->first = -1;
		pError->second = L"Data array size is wrong";
		ret = SendResult(idUploadEmployees, index, pError->first, pError->second, *pId);
		return;
	}
	int redo = 0;
	do {
		//use master for insert, update and delete
		auto handler = CYourServer::Master->Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
		if (!handler) {
			pError->first = -2;
			pError->second = L"No connection to a master database";
			ret = SendResult(idUploadEmployees, index, pError->first, pError->second, *pId);
			return;
		}
		++redo;
		do {
			bool ok = false;
			if (!handler->Prepare(L"INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) break;
			if (!handler->BeginTrans()) break;
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
#ifndef NDEBUG
						{
							SPA::CAutoLock al(m_csConsole);
							std::cout << __FUNCTION__ << "@LINE@" << __LINE__ << ": error code = " << r << ", error message = ";
							std::wcout << err.c_str() << std::endl;
						}
#endif
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
			if (handler->EndTrans(SPA::UDB::rpRollbackErrorAll, [index, pError, pId, this](CSQLHandler & h, int r, const std::wstring & err) {
				if (r) {
#ifndef NDEBUG
						{
							SPA::CAutoLock al(m_csConsole);
							std::cout << __FUNCTION__ << "@LINE@" << __LINE__ << ": error code = " << r << ", error message = ";
							std::wcout << err.c_str() << std::endl;
						}
#endif
						if (!pError->first) {
							//we only report the first error back to front caller
							pError->first = r;
							pError->second = err;
						}
				}
				unsigned int ret = this->SendResult(idUploadEmployees, index, pError->first, pError->second, *pId);
			}, [index, pData, this, peer_handle]() {
#ifndef NDEBUG
					{
						SPA::CAutoLock al(m_csConsole);
						//socket closed after sending
						std::cout << "Retrying UploadEmployees ......" << std::endl;
					}
#endif
					//we need to retry as long as front socket is not closed yet
					if (peer_handle == this->GetSocketHandle()) {
						SPA::CScopeUQueue sq;
						//repack original request data and retry if socket is closed after sending
						sq << index << *pData;
						this->UploadEmployees(*sq); //this will not cause recursive stack-overflow exeption
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

void CYourPeerOne::GetRentalDateTimes(SPA::UINT64 index, SPA::INT64 rental_id, SPA::UINT64 &retIndex, CRentalDateTimes &dates, int &res, std::wstring &errMsg) {
	retIndex = index;
	res = 0;
	::memset(&dates, 0, sizeof(dates));
	std::wstring sql = L"SELECT rental_date,return_date,last_update FROM sakila.rental where rental_id=" + std::to_wstring(rental_id);
	int redo = 0;
	do {
		std::shared_ptr<CSQLHandler> handler = CYourServer::Slave->Seek();
		if (!handler) {
			res = -1;
			errMsg = L"No connection to a slave database";
			return;
		}
		++redo;
		std::shared_ptr<std::promise<bool> > prom(new std::promise<bool>, [](std::promise<bool> *p) {
			delete p;
		});
		if (handler->Execute(sql.c_str(), [&res, &errMsg, prom](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
			res = r;
			errMsg = err;
			prom->set_value(true);
		}, [&dates](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
			dates.Rental = vData[0].ullVal; //date time in high precision format
			dates.Return = vData[1].ullVal;
			dates.LastUpdate = vData[2].ullVal;
		}, [](CSQLHandler & h) {
			assert(h.GetColumnInfo().size() == 3);
		}, true, true, [prom]() {
			//socket closed after sending
			prom->set_value(false);
		})) {
			auto status = prom->get_future().wait_for(std::chrono::seconds(20)); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
			if (status == std::future_status::timeout) {
				res = -2;
				errMsg = L"Querying rental date times timed out";
				redo = 0; //no redo because of timed-out
			}
			else if (prom->get_future().get()) {
				redo = 0; //disable redo after result returned
			}
			else {
#ifndef NDEBUG
				SPA::CAutoLock al(m_csConsole);
				//socket closed after sending
				std::cout << "Retry rental date times ......" << std::endl;
#endif
			}
		}
		else {
			//socket closed when sending		
		}
	} while (redo);
}

void CYourPeerOne::QueryPaymentMaxMinAvgs(SPA::CUQueue &q) {
	unsigned int ret;
	SPA::UINT64 index;
	std::shared_ptr<CMaxMinAvg> pmma(new CMaxMinAvg);
	std::wstring filter;
	q >> index >> filter;
	std::shared_ptr<std::pair<int, std::wstring> > pError(new std::pair<int, std::wstring>(0, L""));
	std::wstring sql = L"SELECT MAX(amount),MIN(amount),AVG(amount) FROM sakila.payment";
	if (filter.size())
		sql += (L" WHERE " + filter);
	int redo = 0;
	do {
		std::shared_ptr<CSQLHandler> handler = CYourServer::Slave->Seek();
		if (!handler) {
			pError->first = -1;
			pError->second = L"No connection to a slave database";
			ret = SendResult(idQueryMaxMinAvgs, index, pError->first, pError->second, *pmma);
			return;
		}
		++redo;
		auto peer_handle = GetSocketHandle();
		if (handler->Execute(sql.c_str(), [index, pError, pmma, this](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
			if (r) {
#ifndef NDEBUG
				SPA::CAutoLock al(m_csConsole);
				std::cout << __FUNCTION__ << "@LINE@" << __LINE__ << ": error code = " << r << ", error message = ";
				std::wcout << err.c_str() << std::endl;
#endif
				pError->first = r;
				pError->second = err;
			}
			unsigned int ret = this->SendResult(idQueryMaxMinAvgs, index, pError->first, pError->second, *pmma);
		}, [pmma](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
			pmma->Max = vData[0].dblVal;
			pmma->Min = vData[1].dblVal;
			pmma->Avg = vData[2].dblVal;
		}, [](CSQLHandler & h) {
			assert(h.GetColumnInfo().size() == 3);
		}, true, true, [peer_handle, index, filter, this]() {
			//socket closed after sending
#ifndef NDEBUG
			{
				SPA::CAutoLock al(m_csConsole);
				//socket closed after sending
				std::cout << "Retrying QueryPaymentMaxMinAvgs ......" << std::endl;
			}
#endif
			//front peer not closed yet
			if (peer_handle == this->GetSocketHandle()) {
				SPA::CScopeUQueue sq;
				//repack original request data and retry
				sq << index << filter;
				this->QueryPaymentMaxMinAvgs(*sq); //this will not cause recursive stack-overflow exeption
			}
		})) {
			redo = 0; //disable redo once request is put on wire
		}
	} while (redo);
}

unsigned int CYourPeerOne::SendMeta(const SPA::UDB::CDBColumnInfoArray &meta, SPA::UINT64 index) {
	//A client expects a rowset meta data and call index
	return SendResult(SPA::UDB::idRowsetHeader, meta, index);
}

unsigned int CYourPeerOne::SendRows(SPA::UDB::CDBVariantArray &vData) {
	SPA::CScopeUQueue sb;
	//The serialization will be fine as long as the data array doesn't contain huge BLOB or text
	sb << vData;
	unsigned int count;
	sb >> count; //remove data array size at head as a client is expecting an array of data without size ahead
	return SendResult(SPA::UDB::idEndRows, sb);
}

void CYourPeerOne::GetCachedTables(const std::wstring &defaultDb, int flags, bool rowset, SPA::UINT64 index, int &res, std::wstring &errMsg) {
	res = 0;
	do {
		if (SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES == (flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES)) {
			unsigned int chatgroup[] = { SPA::UDB::CACHE_UPDATE_CHAT_GROUP_ID, SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID };
			if (!GetPush().Subscribe(chatgroup, 2)) {
				errMsg = L"Failed in subscribing for table events"; //warning message
			}
		}
		if (!rowset)
			break;
		if (!g_config.m_vFrontCachedTable.size())
			break;
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
		std::shared_ptr<std::promise<void> > prom(new std::promise<void>, [](std::promise<void> *p) {
			delete p;
		});
		if (!handler->Execute(sql.c_str(), [prom, &res, &errMsg](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
			res = r;
			errMsg = err;
			if (res) {
				std::cout << "CYourPeerOne::GetCachedTables: error code = " << res << ", error message = ";
				std::wcout << errMsg.c_str() << std::endl;
			}
			prom->set_value();
		}, [this](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
			this->SendRows(vData);
		}, [this, index](CSQLHandler & h) {
			this->SendMeta(h.GetColumnInfo(), index);
		}, true, true, [prom, &res, &errMsg]() {
			res = -4;
			errMsg = L"Request canceled or socket closed";
			prom->set_value();
		})) {
			res = handler->GetAttachedClientSocket()->GetErrorCode();
			errMsg = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
			break;
		}
		CYourServer::Master->Unlock(handler); //put back locked handler and its socket back into pool for reuse as soon as possible
		auto status = prom->get_future().wait_for(std::chrono::seconds(100)); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
		if (status == std::future_status::timeout) {
			res = -3;
			errMsg = L"Querying cached table data timeout";
			break;
		}
	} while (false);
}
