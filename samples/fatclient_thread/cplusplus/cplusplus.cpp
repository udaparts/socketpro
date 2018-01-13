
#include "stdafx.h"

using namespace SPA::UDB;

typedef SPA::ClientSide::CSqliteBase CMyHandler;
typedef SPA::ClientSide::CSocketPool<SPA::ClientSide::CSqlite> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;

/*
//This is bad implementation for original SPA::ClientSide::CAsyncDBHandler::Open method!!!!
virtual bool Open(const wchar_t* strConnection, DResult handler, unsigned int flags = 0, DCanceled dicarded = nullptr) {
std::wstring s;
CAutoLock al(m_csDB); //start lock here
m_flags = flags;
if (strConnection) {
s = m_strConnection;
m_strConnection = strConnection;
}
//self cross SendRequest dead-lock here !!!!
if (SendRequest(UDB::idOpen, strConnection, flags, [handler, this](CAsyncResult & ar) {
int res, ms;
std::wstring errMsg;
ar >> res >> errMsg >> ms;
this->m_csDB.lock(); //self dead-lock !!!!
this->CleanRowset();
this->m_dbErrCode = res;
this->m_lastReqId = idOpen;
if (res == 0) {
this->m_strConnection = std::move(errMsg);
errMsg.clear();
}
else {
this->m_strConnection.clear();
}
this->m_dbErrMsg = errMsg;
this->m_ms = (tagManagementSystem)ms;
this->m_parameters = 0;
this->m_outputs = 0;
this->m_csDB.unlock();
if (handler) {
handler(*this, res, errMsg);
}
}, dicarded, nullptr)) {
return true;
}
if (strConnection) {
m_strConnection = s;
}
return false;
//end lock
}
*/

#define sample_database L"mysample.db"

void Demo_Cross_Request_Dead_Lock(CMyPool::PHandler sqlite) {
	unsigned int count = 10000;
	//uncomment the following call to remove potential cross SendRequest dead lock
	//sqlite->GetAttachedClientSocket()->GetClientQueue().StartQueue("cross_locking_0", 3600);
	do {
		bool ok = sqlite->Open(sample_database, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
			if (res != 0) {
				std::cout << "Open: res = " << res << ", errMsg: ";
				std::wcout << errMsg << std::endl;
			}
		});
		--count;
	} while (count > 0);
}

void TestCreateTables(CMyPool::PHandler sqlite) {
	const wchar_t *sql = L"CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,NAME CHAR(64)NOT NULL)";
	bool ok = sqlite->Execute(sql, [](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
		if (res != 0) {
			std::cout << "affected = " << affected << ", fails = " << (fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
			std::wcout << errMsg << std::endl;
		}
	});
	sql = L"CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
	ok = sqlite->Execute(sql, [](CMyHandler &handler, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
		if (res != 0) {
			std::cout << "affected = " << affected << ", fails = " << (fail_ok >> 32) << ", oks = " << (unsigned int)fail_ok << ", res = " << res << ", errMsg: ";
			std::wcout << errMsg << std::endl;
		}
	});
}

SPA::CUCriticalSection m_csConsole;

void StreamSQLsWithManualTransaction(CMyPool::PHandler sqlite) {
	bool ok = sqlite->BeginTrans(tiReadCommited, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
		if (res != 0) {
			SPA::CAutoLock al(m_csConsole);
			std::wcout << L"BeginTrans: res = " << res << L", errMsg: ";
			std::wcout << errMsg << std::endl;
		}
	});
	ok = sqlite->Execute(L"delete from EMPLOYEE;delete from COMPANY", [](CMyHandler &h, int res, const std::wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
		if (res != 0) {
			SPA::CAutoLock al(m_csConsole);
			std::wcout << L"Execute_Delete: affected=" << affected << L", fails=" << (fail_ok >> 32) << L", res=" << res << L", errMsg=" << errMsg << std::endl;
		}
	});
	ok = sqlite->Prepare(L"INSERT INTO COMPANY(ID,NAME)VALUES(?,?)");
	CDBVariantArray vData;
	vData.push_back(1);
	vData.push_back("Google Inc.");
	vData.push_back(2);
	vData.push_back("Microsoft Inc.");
	//send two sets of parameterized data in one shot for processing
	ok = sqlite->Execute(vData, [](CMyHandler &h, int res, const std::wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
		if (res != 0) {
			SPA::CAutoLock al(m_csConsole);
			std::wcout << L"INSERT COMPANY: affected=" << affected << L", fails=" << (fail_ok >> 32) << L", res=" << res << L", errMsg=" << errMsg << std::endl;
		}
	});
	ok = sqlite->Prepare(L"INSERT INTO EMPLOYEE(EMPLOYEEID,CompanyId,name,JoinDate)VALUES(?,?,?,?)");
	vData.clear();
	SYSTEMTIME st;
	vData.push_back(1);
	vData.push_back(1); //Google company id
	vData.push_back("Ted Cruz");
#ifdef WIN32_64
	::GetLocalTime(&st);
#else
	::gettimeofday(&st, nullptr);
#endif
	vData.push_back(st);

	vData.push_back(2);
	vData.push_back(1); //Google company id
	vData.push_back("Donald Trump");
#ifdef WIN32_64
	::GetLocalTime(&st);
#else
	::gettimeofday(&st, nullptr);
#endif
	vData.push_back(st);

	vData.push_back(3);
	vData.push_back(2); //Microsoft company id
	vData.push_back("Hillary Clinton");
#ifdef WIN32_64
	::GetLocalTime(&st);
#else
	::gettimeofday(&st, nullptr);
#endif
	vData.push_back(st);
	//send three sets of parameterized data in one shot for processing
	ok = sqlite->Execute(vData, [](CMyHandler &h, int res, const std::wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
		if (res != 0) {
			SPA::CAutoLock al(m_csConsole);
			std::wcout << L"INSERT EMPLOYEE: affected=" << affected << L", fails=" << (fail_ok >> 32) << L", res=" << res << L", errMsg=" << errMsg << std::endl;
		}
	});
	ok = sqlite->EndTrans(rpDefault, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
		if (res != 0) {
			SPA::CAutoLock al(m_csConsole);
			std::wcout << L"EndTrans: res = " << res << L", errMsg: ";
			std::wcout << errMsg << std::endl;
		}
	});
}

#define m_cycle ((unsigned int) 100)

void Demo_Multiple_SendRequest_MultiThreaded_Wrong(CMyPool& sp) {
	unsigned int cycle = m_cycle;
	while (cycle > 0) {
		//Seek an async handler on the min number of requests queued in memory and its associated socket connection
		CMyPool::PHandler sqlite = sp.Seek();
		StreamSQLsWithManualTransaction(sqlite);
		--cycle;
	}
	auto vSqlite = sp.GetAsyncHandlers();
	for (auto s = vSqlite.begin(), e = vSqlite.end(); s != e; ++s) {
		(*s)->WaitAll();
	}
}

void Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(CMyPool& sp) {
	unsigned int cycle = m_cycle;
	while (cycle > 0) {
		//Take an async handler infinitely from socket pool for sending multiple requests from current thread
		CMyPool::PHandler sqlite = sp.Lock();
		StreamSQLsWithManualTransaction(sqlite);
		//Put back a previously locked async handler to pool for reuse.
		sp.Unlock(sqlite);
		--cycle;
	}
	auto vSqlite = sp.GetAsyncHandlers();
	for (auto s = vSqlite.begin(), e = vSqlite.end(); s != e; ++s) {
		(*s)->WaitAll();
	}
}

std::future<bool> DoFuture(CMyPool &sp) {
	std::shared_ptr<std::promise<bool> > prom(new std::promise<bool>, [](std::promise<bool> *p) {
		delete p;
	});
	CMyPool::PHandler sqlite = sp.Lock();
	if (!sqlite) {
		SPA::CAutoLock al(m_csConsole);
		std::cout << "All connections are disconnected" << std::endl;
		prom->set_value(false);
		return prom->get_future();
	}
	bool ok = false;
	do {
		if (!sqlite->BeginTrans(tiReadCommited, [](CMyHandler & handler, int res, const std::wstring & errMsg) {
			if (res != 0) {
				SPA::CAutoLock al(m_csConsole);
				std::wcout << L"BeginTrans: res = " << res << L", errMsg: ";
				std::wcout << errMsg << std::endl;
			}
		})) break;
		if (!sqlite->Execute(L"delete from EMPLOYEE;delete from COMPANY", [](CMyHandler & h, int res, const std::wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
			if (res != 0) {
				SPA::CAutoLock al(m_csConsole);
				std::wcout << L"Execute_Delete: affected=" << affected << L", fails=" << (fail_ok >> 32) << L", res=" << res << L", errMsg=" << errMsg << std::endl;
			}
		})) break;
		if (!sqlite->Prepare(L"INSERT INTO COMPANY(ID,NAME)VALUES(?,?)")) break;
		CDBVariantArray vData;
		vData.push_back(1);
		vData.push_back("Google Inc.");
		vData.push_back(2);
		vData.push_back("Microsoft Inc.");
		//send two sets of parameterised data in one shot for processing
		if (!sqlite->Execute(vData, [](CMyHandler & h, int res, const std::wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
			if (res != 0) {
				SPA::CAutoLock al(m_csConsole);
				std::wcout << L"INSERT COMPANY: affected=" << affected << L", fails=" << (fail_ok >> 32) << L", res=" << res << L", errMsg=" << errMsg << std::endl;
			}
		})) break;
		if (!sqlite->Prepare(L"INSERT INTO EMPLOYEE(EMPLOYEEID,CompanyId,name,JoinDate)VALUES(?,?,?,?)")) break;
		vData.clear();
		SYSTEMTIME st;
		vData.push_back(1);
		vData.push_back(1); //Google company id
		vData.push_back("Ted Cruz");
#ifdef WIN32_64
		::GetLocalTime(&st);
#else
		::gettimeofday(&st, nullptr);
#endif
		vData.push_back(st);

		vData.push_back(2);
		vData.push_back(1); //Google company id
		vData.push_back("Donald Trump");
#ifdef WIN32_64
		::GetLocalTime(&st);
#else
		::gettimeofday(&st, nullptr);
#endif
		vData.push_back(st);

		vData.push_back(3);
		vData.push_back(2); //Microsoft company id
		vData.push_back("Hillary Clinton");
#ifdef WIN32_64
		::GetLocalTime(&st);
#else
		::gettimeofday(&st, nullptr);
#endif
		vData.push_back(st);
		//send three sets of parameterised data in one shot for processing
		if (!sqlite->Execute(vData, [](CMyHandler & h, int res, const std::wstring & errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, CDBVariant & id) {
			if (res != 0) {
				SPA::CAutoLock al(m_csConsole);
				std::wcout << L"INSERT EMPLOYEE: affected=" << affected << L", fails=" << (fail_ok >> 32) << L", res=" << res << L", errMsg=" << errMsg << std::endl;
			}
		})) break;
		if (!sqlite->EndTrans(rpDefault, [prom](CMyHandler & handler, int res, const std::wstring & errMsg) {
			if (res != 0) {
				SPA::CAutoLock al(m_csConsole);
				std::wcout << L"EndTrans: res = " << res << L", errMsg: ";
				std::wcout << errMsg << std::endl;
			}
			prom->set_value(true);
		}, [prom](SPA::ClientSide::CAsyncServiceHandler *h, bool canceled) {
			{
				SPA::CAutoLock al(m_csConsole);
				std::cout << (canceled ? "Request canceled" : "Socket closed") << std::endl;
			}
			//std::exception ex("Canceled");
			//prom->set_exception(std::make_exception_ptr(ex));
			prom->set_value(false);
		})) break;
		ok = true;
		sp.Unlock(sqlite); //put handler back into pool for reuse 
	} while (false);
	if (!ok) {
		prom->set_value(false);
		//Socket is closed at server side and the above locked handler is automatically unlocked
		SPA::CAutoLock al(m_csConsole);
		std::cout << "Socket closed with error code: " << sqlite->GetAttachedClientSocket()->GetErrorCode() << ", error message: ";
		std::wcout << sqlite->GetAttachedClientSocket()->GetErrorMsg().c_str() << std::endl;
	}
	return prom->get_future();
}

int main(int argc, char* argv[]) {
	CMyConnContext cc;
	std::cout << "Remote host: " << std::endl;
	std::getline(std::cin, cc.Host);
	//cc.Host = "localhost";
	cc.Port = 20901;
	cc.UserId = L"MyUserId";
	cc.Password = L"MyPassword";
	CMyPool spSqlite;
	//start socket pool having 1 worker thread which hosts two non-block sockets
	bool ok = spSqlite.StartSocketPool(cc, 2, 1);
	if (!ok) {
		std::cout << "No connection to sqlite server and press any key to close the demo ......" << std::endl;
		::getchar();
		return 0;
	}
	CMyPool::PHandler sqlite = spSqlite.GetAsyncHandlers()[0];
	//Use the above bad implementation to replace original SPA::ClientSide::CAsyncDBHandler::Open method
	//at file socketpro/include/udb_client.h for cross SendRequest dead lock demonstration
	std::cout << "Doing Demo_Cross_Request_Dead_Lock ......" << std::endl;
	Demo_Cross_Request_Dead_Lock(sqlite);

	//create two tables, COMAPNY and EMPLOYEE
	TestCreateTables(sqlite);
	ok = sqlite->WaitAll();

	//make sure all other handlers/sockets to open the same database mysample.db
	auto vHandle = spSqlite.GetAsyncHandlers();
	size_t count = vHandle.size();
	for (size_t n = 1; n < count; ++n) {
		sqlite = vHandle[n];
		sqlite->Open(sample_database, [](CMyHandler &handler, int res, const std::wstring & errMsg) {
			if (res != 0) {
				std::cout << "Open: res = " << res << ", errMsg: ";
				std::wcout << errMsg << std::endl;
			}
		});
		ok = sqlite->WaitAll();
	}
	//execute manual transactions concurrently with transaction overlapping on the same session at client side
	auto f0 = std::async(std::launch::async, [&spSqlite]() {
		Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
	});
	auto f1 = std::async(std::launch::async, [&spSqlite]() {
		Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
	});
	auto f2 = std::async(std::launch::async, [&spSqlite]() {
		Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
	});
	Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
	f0.get();
	f1.get();
	f2.get(); //wait until all streamed SQL statements are processed
	std::cout << "Demo_Multiple_SendRequest_MultiThreaded_Wrong completed" << std::endl << std::endl;

	//execute manual transactions concurrently without transaction overlapping on the same session at client side by lock/unlock
	auto f3 = std::async(std::launch::async, [&spSqlite]() {
		Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
	});
	auto f4 = std::async(std::launch::async, [&spSqlite]() {
		Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
	});
	auto f5 = std::async(std::launch::async, [&spSqlite]() {
		Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
	});
	Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
	f3.get();
	f4.get();
	f5.get(); //wait until all streamed SQL statements are processed
	std::cout << "Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock" << std::endl << std::endl;

	std::cout << "Demonstration of DoFuture ....." << std::endl;
	auto f = DoFuture(spSqlite);
	if (f.wait_for(std::chrono::seconds(5)) == std::future_status::timeout || !f.get())
		std::cout << "The requests within the function DoFuture are not completed in 5 seconds" << std::endl;
	else {
		std::cout << "All requests within the function DoFuture are completed" << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Press any key to close the application ......" << std::endl;
	::getchar();
	return 0;
}
