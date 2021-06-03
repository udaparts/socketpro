#include "tdschannel.h"
#include "prelogin.h"
#include "sqlbatch.h"
#include <iostream>
#include <chrono>
using namespace std::chrono;
typedef std::chrono::milliseconds ms;

const unsigned int CYCLES = 50000;

int main() {
	SPA::CSessionPool<tds::CTdsChannel> pool(1);
	auto handler = pool.FindAClosedHandler();
	bool ok = handler->Connect("windesk", 1433, SPA::tagEncryptionMethod::NoEncryption, false, true);

	char serverName[128];
	handler->GetServerName(serverName, sizeof(serverName));

	tds::CPrelogin pl(*handler);
	int res = pl.SendTDSMessage();

	tds::SqlLogin rec;
	rec.database = u"sqltestdb";
	rec.userName = u"sa";
	rec.password = u"Smash123";
	rec.serverName = tds::CDBString(serverName, serverName + strlen(serverName));

	tds::CSqlBatch sqlbatch(*handler);
	tds::CSqlBatch::FeatureExtension fe;
	res = sqlbatch.SendTDSMessage(rec, fe);
	SPA::CDBString errMsg;
	res = sqlbatch.GetSQLError(errMsg);
#if 0
	SPA::CDBString sql;
	system_clock::time_point start = system_clock::now();
	for (unsigned int n = 1; n <= CYCLES; ++n) {
		if (sql.size()) {
			sql.push_back(';');
		}
		sql += u"select * from actor where actor_id between 11 and 20";
		if ((n % 10) == 0) {
			res = sqlbatch.SendTDSMessage(sql.c_str());
			sql.clear();
			if (res) {
				break;
			}
		}
	}
	system_clock::time_point stop = system_clock::now();
	ms d = duration_cast<ms>(stop - start);
	std::cout << "Time required: " << d.count() << " ms\n\n";
#endif
	//res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtBeginTrans, tds::CSqlBatch::tagIsolationLevel::ilReadCommitted);
	unsigned int parameters;
	CParameterInfoArray vPInfo;
	res = sqlbatch.Prepare(u"select * from actor where actor_id=? and first_name<>?", vPInfo, parameters);
	CDBVariantArray vParam;
	vParam.push_back(1);
	vParam.push_back("NICKTest");
	vParam.push_back(2);
	vParam.push_back(u"NICKTest");
	res = sqlbatch.SendTDSMessage(vParam);
	//res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtCommit);

	//res = sqlbatch.SendTDSMessage(u"select * from test_rare1;select * from pet;select * from SpatialTable;select * from vtest");

	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}
