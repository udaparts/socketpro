
#pragma once


#include "../../../../include/aserverw.h"


#include "../../../../include/servercache.h"
#include "../../../../include/mysql/umysql.h"

typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMySQLHandler;
typedef SPA::CMasterPool<CMySQLHandler> CMySQLMasterPool;
typedef CMySQLMasterPool::CSlavePool CMySQLSlavePool;

//#include "../../../../include/async_sqlite.h"
//typedef SPA::CMasterPool<SPA::ClientSide::CSqlite> CSqliteMasterPool;
//typedef CSqliteMasterPool::CSlavePool CSqliteSlavePool;
//typedef SPA::ClientSide::CSqlite CSqliteHandler;
//
//#include "../../../../include/async_odbc.h"
//typedef SPA::CMasterPool<SPA::ClientSide::COdbc> COdbcMasterPool;
//typedef COdbcMasterPool::CSlavePool COdbcSlavePool;
//typedef SPA::ClientSide::COdbc COdbcHandler;

using namespace SPA::ClientSide;
using namespace SPA::ServerSide;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
