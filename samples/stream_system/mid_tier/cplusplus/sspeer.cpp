
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"
#include "config.h"


std::chrono::seconds CYourPeerOne::m_timeout(10); //10 seconds

CYourPeerOne::CYourPeerOne() {

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
		M_I4_R2(SPA::UDB::idGetCachedTables, GetCachedTables, std::wstring, unsigned int, bool, SPA::UINT64, int, std::wstring)
		M_I1_R3(idUploadEmployees, UploadEmployees, SPA::UDB::CDBVariantArray, int, std::wstring, CInt64Array)
		END_SWITCH
		return 0;
}

void CYourPeerOne::GetMasterSlaveConnectedSessions(unsigned int &m_connections, unsigned int &s_connections) {
	m_connections = CYourServer::Master->GetConnectedSockets();
	s_connections = CYourServer::Slave->GetConnectedSockets();
}

void CYourPeerOne::UploadEmployees(const SPA::UDB::CDBVariantArray &vData, int &res, std::wstring &errMsg, CInt64Array &vId) {
	res = 0;
	if (!vData.size()) return;
	if ((vData.size() % 3)) {
		res = -1;
		errMsg = L"Data array size wrong";
		return;
	}
	int redo = 0;
	do {
		//use master for insert, update and delete
		auto handler = CYourServer::Master->Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
		if (!handler) {
			res = -2;
			errMsg = L"No connection to a master database";
			return;
		}
		++redo;
		do {
			bool ok = false;
			if (!handler->Prepare(L"INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) break;
			if (!handler->BeginTrans()) break;
			SPA::UDB::CDBVariantArray v;
			for (auto it = vData.cbegin(), end = vData.cend(); it != end;) {
				v.push_back(*it);
				v.push_back(*(it + 1));
				v.push_back(*(it + 2));
				ok = handler->Execute(v, [&redo, &res, &errMsg, &vId](CSQLHandler &h, int r, const std::wstring &err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
					if (r && !res) {
						res = r;
						errMsg = err;
						vId.push_back(-1);
					}
					else if (r)
						vId.push_back(-1);
					else {
						assert(affected == 1);
						assert(!err.size());
						assert(fail_ok == 1);
						vId.push_back(vtId.llVal);
					}
				});
				if (!ok) break;
				v.clear();
				it += 3;
			}
			if (!ok) break;
			std::shared_ptr<std::promise<void> > prom(new std::promise<void>, [](std::promise<void> *p) {
				delete p;
			});
			if (handler->EndTrans(SPA::UDB::rpRollbackErrorAll, [&redo, prom](CSQLHandler & h, int r, const std::wstring & err) {
				redo = 0; //we will not redo only if the final callback is called
				prom->set_value();
			}, [prom, &res, &errMsg]() {
				//socket closed after sending
				prom->set_value();
			})) {
				CYourServer::Master->Unlock(handler); //put back locked handler and its socket back into pool for reuse as soon as possible
				auto status = prom->get_future().wait_for(m_timeout); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
			}
			else {
				//socket closed when sending
			}
		} while (false);
		if (redo) {
			std::cout << "Going to retry ......" << std::endl;
		}
	} while (redo);
}

void CYourPeerOne::QueryPaymentMaxMinAvgs(const std::wstring &filter, int &res, std::wstring &errMsg, CMaxMinAvg &mma) {
	res = 0;
	::memset(&mma, 0, sizeof(mma));
	std::wstring sql = L"SELECT MAX(amount),MIN(amount),AVG(amount) FROM sakila.payment";
	if (filter.size())
		sql += (L" WHERE " + filter);
	int redo = 0;
	do {
		//we are going to use slave for the query
		auto handler = CYourServer::Slave->Seek();
		if (!handler) {
			res = -1;
			errMsg = L"No connection to a slave database";
			return;
		}
		++redo;
		std::shared_ptr<std::promise<void> > prom(new std::promise<void>, [](std::promise<void> *p) {
			delete p;
		});
		if (handler->Execute(sql.c_str(), [&redo, prom, &res, &errMsg](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
			res = r;
			errMsg = err;
			redo = 0; //we will not redo only if the final callback is called
			prom->set_value();
		}, [&mma, &res, &errMsg](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
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
		}, [](CSQLHandler & h) {
			assert(h.GetColumnInfo().size() == 3);
		}, true, true, [prom]() {
			prom->set_value(); //socket closed after sending
		})) {
			auto status = prom->get_future().wait_for(m_timeout); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
		}
		else {
			//socket closed when sending
		}
		if (redo) {
			std::cout << "Going to retry ......" << std::endl;
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
		auto status = prom->get_future().wait_for(m_timeout); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
		if (status == std::future_status::timeout) {
			res = -3;
			errMsg = L"Querying cached table data timeout";
			break;
		}
	} while (false);
}
