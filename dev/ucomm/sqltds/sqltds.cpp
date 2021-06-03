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
	bool ok = handler->Connect("acer", 1433, SPA::tagEncryptionMethod::NoEncryption, false, true);

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
	CParameterInfo pi;
	pi.DataType = VT_I4;
	pi.ParameterName = u"@n";
	vPInfo.push_back(pi);
	pi.Direction = tagParameterDirection::pdInputOutput;
	pi.ParameterName = u"@nout";
	vPInfo.push_back(pi);
	pi.DataType = VT_DECIMAL;
	pi.Direction = tagParameterDirection::pdOutput;
	pi.ParameterName = u"@dec";
	pi.Precision = 19;
	pi.Scale = 2;
	vPInfo.push_back(pi);
	res = sqlbatch.Prepare(u"call sqltestdb.dbo.GetSomeData(?, ?, ?)", vPInfo, parameters);
	CDBVariantArray vParam;
	vParam.push_back(10);
	vParam.push_back(12);
	vParam.push_back((const char*)nullptr);
	res = sqlbatch.SendTDSMessage(vParam);
	//res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtCommit);

	//res = sqlbatch.SendTDSMessage(u"select * from test_rare1;select * from pet;select * from SpatialTable;select * from vtest");

	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}
