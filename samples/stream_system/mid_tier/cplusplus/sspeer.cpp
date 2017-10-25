
#include "stdafx.h"
#include "sspeer.h"
#include "ssserver.h"
#include "config.h"

#ifndef NDEBUG
SPA::CUCriticalSection CYourPeerOne::m_csConsole;
#endif

CYourPeerOne::CYourPeerOne() {

}

void CYourPeerOne::OnSwitchFrom(unsigned int nOldServiceId) {
	if (m_seqHandler) {
		CYourServer::Slave->Unlock(m_seqHandler);
		m_seqHandler = nullptr;
	}
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
	else if (reqId == idGetRentalDateTimes) {
		GetRentalDateTimes(m_UQueue);
		return;
	}
	BEGIN_SWITCH(reqId)
		M_I1_R3(idGetMasterSlaveConnectedSessions, GetMasterSlaveConnectedSessions, SPA::UINT64, SPA::UINT64, unsigned int, unsigned int)
		M_I1_R3(idStartSequence, StartSequence, SPA::UINT64, SPA::UINT64, int, std::wstring)
		M_I1_R3(idEndSequence, EndSequence, SPA::UINT64, SPA::UINT64, int, std::wstring)
		END_SWITCH
}

int CYourPeerOne::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
	BEGIN_SWITCH(reqId)
		M_I4_R2(SPA::UDB::idGetCachedTables, GetCachedTables, std::wstring, unsigned int, bool, SPA::UINT64, int, std::wstring)
		END_SWITCH
		return 0;
}

void CYourPeerOne::StartSequence(SPA::UINT64 index, SPA::UINT64 &retIndex, int &res, std::wstring &errMsg) {
	retIndex = index;
	do {
		if (m_seqHandler) {
			res = -1;
			errMsg = L"Wrong call sequence";
			break;
		}
		m_seqHandler = CYourServer::Slave->Lock();
		if (!m_seqHandler) {
			res = -2;
			errMsg = L"No connection to slave servers";
			break;
		}
		res = 0;
	} while (false);
}

void CYourPeerOne::EndSequence(SPA::UINT64 index, SPA::UINT64 &retIndex, int &res, std::wstring &errMsg) {
	retIndex = index;
	do {
		if (!m_seqHandler) {
			res = -1;
			errMsg = L"Wrong call sequence";
			break;
		}
		CYourServer::Slave->Unlock(m_seqHandler);
		m_seqHandler = nullptr;
		res = 0;
	} while (false);
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

void CYourPeerOne::GetRentalDateTimes(SPA::CUQueue &q) {
	unsigned int ret;
	SPA::UINT64 index;
	SPA::INT64 rental_id;
	q >> index >> rental_id;
	std::shared_ptr<CRentalDateTimes> prdt(new CRentalDateTimes);
	std::shared_ptr<std::pair<int, std::wstring> > pError(new std::pair<int, std::wstring>(0, L""));
	std::wstring sql = L"SELECT rental_date,return_date,last_update FROM sakila.rental where rental_id=" + std::to_wstring(rental_id);
	int redo = 0;
	do {
		bool seq;
		std::shared_ptr<CSQLHandler> handler;
		if (m_seqHandler) {
			handler = m_seqHandler;
			seq = true;
		}
		else {
			seq = false;
			handler = CYourServer::Slave->Lock();
			if (!handler) {
				pError->first = -1;
				pError->second = L"No connection to a slave database";
				ret = SendResult(idGetRentalDateTimes, index, pError->first, pError->second, *prdt);
				return;
			}
		}
		++redo;
		auto peer_handle = seq ? 0 : GetSocketHandle();
		if (handler->Execute(sql.c_str(), [index, pError, prdt, this](CSQLHandler & h, int r, const std::wstring & err, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
			if (r) {
#ifndef NDEBUG
				SPA::CAutoLock al(m_csConsole);
				std::cout << __FUNCTION__ << "@LINE@" << __LINE__ << ": error code = " << r << ", error message = ";
				std::wcout << err.c_str() << std::endl;
#endif
				pError->first = r;
				pError->second = err;
			}
			unsigned int ret = this->SendResult(idGetRentalDateTimes, index, pError->first, pError->second, *prdt);
		}, [prdt](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
			prdt->Rental = vData[0].ullVal; //date time in high precision format
			prdt->Return = vData[1].ullVal; //date time in high precision format
			prdt->LastUpdate = vData[2].ullVal; //date time in high precision format
		}, [](CSQLHandler & h) {
			assert(h.GetColumnInfo().size() == 3);
		}, true, true, [index, prdt, peer_handle, rental_id, this]() {
			//socket closed after sending
			unsigned int ret;
			if (!peer_handle) {
				//sequence
				ret = this->SendResult(idGetRentalDateTimes, index, (int)-2, L"Socket closed after sending and no sequence possible", *prdt);
			}
			else {
				//no sequence
#ifndef NDEBUG
					{
						SPA::CAutoLock al(m_csConsole);
						//socket closed after sending
						std::cout << "Retrying QueryPaymentMaxMinAvgs ......" << std::endl;
					}
#endif
					//we need to retry as long as front socket is not closed yet
					if (peer_handle == this->GetSocketHandle()) {
						SPA::CScopeUQueue sq;
						//repack original request data and retry if socket is closed after sending
						sq << index << rental_id;
						this->GetRentalDateTimes(*sq); //this will not cause recursive stack-overflow exeption
					}
			}
		})) {
			redo = 0; //disable redo
			if (!seq) {
				//handler is locally locked so that we need to unlock it
				CYourServer::Slave->Unlock(handler);
			}
		}
		else if (seq) {
			//sequence and socket closed when sending
			pError->first = handler->GetAttachedClientSocket()->GetErrorCode();
			pError->second = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
			ret = SendResult(idGetRentalDateTimes, index, pError->first, pError->second, *prdt);
			redo = 0; //disable redo
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
		bool seq;
		std::shared_ptr<CSQLHandler> handler;
		if (m_seqHandler) {
			handler = m_seqHandler;
			seq = true;
		}
		else {
			seq = false;
			//we are going to use slave for the query
			handler = CYourServer::Slave->Lock();
			if (!handler) {
				pError->first = -1;
				pError->second = L"No connection to a slave database";
				ret = SendResult(idQueryMaxMinAvgs, index, pError->first, pError->second, *pmma);
				return;
			}
		}
		++redo;
		auto peer_handle = seq ? 0 : GetSocketHandle();
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
		}, [pError, pmma](CSQLHandler &h, SPA::UDB::CDBVariantArray & vData) {
			HRESULT hr = -1;
			do {
				if (vData.size() != 3) {
					if (!pError->first) {
						pError->first = -2;
						pError->second = L"Sql statement doesn't generate max, min and average values as expected";
					}
					hr = -2;
					break;
				}
				CComVariant vt;
				hr = ::VariantChangeType(&vt, &vData[0], 0, VT_R8);
				if (hr != S_OK) {
					if (!pError->first) {
						pError->first = hr;
						pError->second = L"Data type mismatch";
					}
					break;
				}
				pmma->Max = vt.dblVal;
				hr = ::VariantChangeType(&vt, &vData[1], 0, VT_R8);
				if (hr != S_OK) {
					if (!pError->first) {
						pError->first = hr;
						pError->second = L"Data type mismatch";
					}
					break;
				}
				pmma->Min = vt.dblVal;
				hr = ::VariantChangeType(&vt, &vData[2], 0, VT_R8);
				if (hr != S_OK) {
					if (!pError->first) {
						pError->first = hr;
						pError->second = L"Data type mismatch";
					}
					break;
				}
				pmma->Avg = vt.dblVal;
			} while (false);
#ifndef NDEBUG
			if (hr) {
				SPA::CAutoLock al(m_csConsole);
				std::cout << __FUNCTION__ << "@LINE@" << __LINE__ << ": error code = " << hr << ", error message = ";
				std::wcout << pError->second.c_str() << std::endl;
			}
#endif
		}, [](CSQLHandler & h) {
			assert(h.GetColumnInfo().size() == 3);
		}, true, true, [peer_handle, index, filter, pmma, this]() {
#ifndef NDEBUG
			{
				SPA::CAutoLock al(m_csConsole);
				//socket closed after sending
				std::cout << "Retrying QueryPaymentMaxMinAvgs ......" << std::endl;
			}
#endif
			if (!peer_handle) {
				//sequence
				unsigned int ret = this->SendResult(idQueryMaxMinAvgs, index, (int)-2, L"Socket closed after sending and no sequence possible", *pmma);
			}                //we need to retry as long as front socket is not closed yet
			else if (peer_handle == this->GetSocketHandle()) {
				SPA::CScopeUQueue sq;
				//repack original request data and retry if socket is closed after sending
				sq << index << filter;
				this->QueryPaymentMaxMinAvgs(*sq); //this will not cause recursive stack-overflow exeption
			}
			//socket closed after sending
		})) {
			redo = 0;
			if (!seq) {
				//handler is locally locked so that we need to unlock it
				CYourServer::Slave->Unlock(handler);
			}
		}
		else if (seq) {
			//sequence and socket closed when sending
			pError->first = handler->GetAttachedClientSocket()->GetErrorCode();
			pError->second = SPA::Utilities::ToWide(handler->GetAttachedClientSocket()->GetErrorMsg().c_str());
			ret = SendResult(idGetRentalDateTimes, index, pError->first, pError->second, *pmma);
			redo = 0; //disable redo
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
		auto status = prom->get_future().wait_for(std::chrono::seconds(5)); //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
		if (status == std::future_status::timeout) {
			res = -3;
			errMsg = L"Querying cached table data timeout";
			break;
		}
	} while (false);
}
