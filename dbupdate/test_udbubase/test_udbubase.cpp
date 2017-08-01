// test_udbubase.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../include/udatabase.h"

int main(int argc, char* argv[])
{
	unsigned int index[2];

	//a list of remote SocketPro server connection strings separated by the char '*'
	//2147483633 -- async mysql service id
	std::wstring connectionString = L"host=localhost;port=20901;userid=udatabase_update;pwd=dbupdate_pass_1;serviceid=2147483633 * server=yyetouch;port=20901;uid=udatabase_update;password=dbupdate_pass_0;zip=1;service-id=2147483633";
	
	//the number of remote socketpro servers
	unsigned int count = SetSocketProConnectionString(connectionString.c_str());
	
	//higher bits (32) for the number of disconnected sockets and lower bits (32) for the number of connected sockets
	SPA::UINT64 res = GetSocketProConnections(index, sizeof(index)/sizeof(unsigned int));
	::Sleep(1000);
	
	res = GetSocketProConnections(index, sizeof(index)/sizeof(unsigned int));

	unsigned int group[2] = {1, 2};
	//receiver will obtain the following string like "1/rental_id=12@localhost.mysql.sakila.rental"
	SPA::UINT64 ret = NotifySocketProDatabaseEvent(group, sizeof(group)/sizeof(unsigned int), SPA::UDB::ueUpdate, L"rental_id=12@localhost.mysql.sakila.rental", index, sizeof(index)/sizeof(unsigned int));
	
	//all sockets will be closed if connection string is null or empty
	count = SetSocketProConnectionString(nullptr);
	return 0;
}
