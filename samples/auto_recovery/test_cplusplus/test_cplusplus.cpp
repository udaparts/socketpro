
#include "stdafx.h"

int main(int argc, char* argv[])
{
	const int THREADS = 1;
	const int cycles = 10000;
	const int sessions_per_host = 2;
	const int HOSTS = 2;
	std::string vHost[HOSTS] = { "localhost", "192.168.2.172" };

	typedef CConnectionContext* PCConnectionContext;
	PCConnectionContext ppCc[THREADS];
	CSocketPool<CSql> sp;
	for (int t = 0; t < THREADS; ++t) {
		ppCc[t] = new CConnectionContext[HOSTS * sessions_per_host];
		for (int n = 0; n < HOSTS; ++n) {
			for (int j = 0; j < sessions_per_host; ++j) {
				CConnectionContext &cc = ppCc[t][n * sessions_per_host + j];
				cc.Host = vHost[n];
				cc.Port = 20901;
				cc.UserId = L"AClientUserId";
				cc.Password = L"MyPassword";
			}
		}
	}
	sp.SetQueueName("ar_cpp"); //set a local message queue to backup requests for auto fault recovery
	bool ok = sp.StartSocketPool(ppCc, THREADS, HOSTS * sessions_per_host);
	do
	{
		if (!ok) {
			std::cout << "There is no connection and press any key to close the application ......" << std::endl;
			::getchar(); break;
		}
		std::wstring sql = L"SELECT max(amount), min(amount), avg(amount) FROM payment";
		std::cout << "Input a filter for payment_id" << std::endl;
		std::wstring filter;
		std::getline(std::wcin, filter);
		while (filter.size() && ::isspace(filter.back())) {
			filter.pop_back();
		}
		while (filter.size() && ::isspace(filter.front())) {
			filter.erase(filter.begin());
		}
		if (filter.size()) sql += (L" WHERE " + filter);
		auto v = sp.GetAsyncHandlers();
		for (auto it = v.begin(), end = v.end(); it != end; ++it) {
			ok = (*it)->Open(L"sakila.db", [](CSql &h, int res, const std::wstring &errMsg){
				if (res)
					std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
			});
			assert(ok);
		}
		int returned = 0;
        double dmax = 0.0, dmin = 0.0, davg = 0.0;
		SPA::UDB::CDBVariantArray row;
		CSql::DExecuteResult er = [&dmax, &dmin, &davg, &returned, &row](CSql &h, int res, const std::wstring &errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant lastId) {
			if (res)
				std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
			else {
				dmax += row[0].dblVal;
				dmin += row[1].dblVal;
				davg += row[2].dblVal;
			}
			++returned;
		};
		CSql::DRows r = [&row](CSql &h, SPA::UDB::CDBVariantArray &r) {
			row = r;
		};
		auto asql = sp.SeekByQueue(); //get one handler for querying one record
		ok = asql->Execute(sql.c_str(), er, r);
		assert(ok);
		ok = asql->WaitAll();
		std::cout << "Result: max = " << dmax << ", min = " << dmin << ", avg = " << davg << std::endl;
		returned = 0;
        dmax = 0.0; dmin = 0.0; davg = 0.0;
		std::cout << "Going to get " << cycles << " queries for max, min and avg" << std::endl;
		for (int n = 0; n < cycles; ++n)
        {
            asql = sp.SeekByQueue();
            ok = asql->Execute(sql.c_str(), er, r);
			assert(ok);
        }
		for (auto it = v.begin(), end = v.end(); it != end; ++it) {
			ok = (*it)->WaitAll();
		}
		std::cout << "Retured = " << returned << ", max = " << dmax << ", min = " << dmin << ", avg = " << davg << std::endl;
		std::cout << "Press any key to close the application ......" << std::endl;
		::getchar();
	}while (false);
	for (int t = 0; t < THREADS; ++t) {
		PCConnectionContext p = ppCc[t];
		delete []p;
	}
	sp.ShutdownPool();
	return 0;
}

