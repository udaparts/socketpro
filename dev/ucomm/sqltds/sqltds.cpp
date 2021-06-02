#include "tdschannel.h"
#include "prelogin.h"
#include "sqlbatch.h"
#include <iostream>

int main() {
	SPA::CSessionPool<tds::CTdsChannel> pool(1);
	auto handler = pool.FindAClosedHandler();
	bool ok = handler->Connect("windesk", 1433, SPA::tagEncryptionMethod::NoEncryption, false, true);

	char serverName[128];
	handler->GetServerName(serverName, sizeof(serverName));

	tds::CPrelogin pl(*handler);
	int res = pl.SendTDSMessage();

	tds::SqlLogin rec;
	rec.database = u"sakila";
	rec.userName = u"sa";
	rec.password = u"Smash123";
	rec.serverName = tds::CDBString(serverName, serverName + strlen(serverName));

	tds::CSqlBatch sqlbatch(*handler);
	tds::CSqlBatch::FeatureExtension fe;
	res = sqlbatch.SendTDSMessage(rec, fe);

	res = sqlbatch.SendTDSMessage(u"select * from actor where actor_id=10");

	unsigned int parameters;
	CParameterInfoArray vPInfo;
	SPA::CDBString errMsg;
	res = sqlbatch.Prepare(u"select * from actor where actor_id=? and first_name<>?", vPInfo, parameters);

	CDBVariantArray vParam;
	vParam.push_back(1);
	vParam.push_back("NICKTest");
	vParam.push_back(2);
	vParam.push_back(u"NICKTest");
	res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtBeginTrans, tds::CSqlBatch::tagIsolationLevel::ilReadCommitted);
	res = sqlbatch.SendTDSMessage(vParam);
	res = sqlbatch.SendTDSMessage(tds::CSqlBatch::tagRequestType::rtCommit);

	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}