
#include "stdafx.h"
#include "../../../../include/mysql/umysql.h"
using namespace SPA::UDB;

typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMyHandler;
typedef SPA::ClientSide::CSocketPool<CMyHandler> CMyPool;
typedef SPA::ClientSide::CConnectionContext CMyConnContext;


int main(int argc, char* argv[]) {
    
    CMyConnContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20902;
    cc.UserId = L"root";
    cc.Password = L"Smash123";
    CMyPool spMysql(true, (~0));
    
    return 0;
}

