
#include "stdafx.h"

using namespace SPA::UDB;

typedef SPA::ClientSide::CSqliteBase CMyHandler;
typedef SPA::ClientSide::CSocketPool<SPA::ClientSide::CSqlite> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;
typedef std::pair<CDBColumnInfoArray, CDBVariantArray> CPColumnRowset;
typedef std::vector<CPColumnRowset> CRowsetArray;

int main(int argc, char* argv[])
{
	CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    CMyPool spSqlite;

	return 0;
}

