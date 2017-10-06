
#pragma once

#include "../../../../include/aserverw.h"
#include "../../../../include/rdbcache.h"

#include "../../../../include/mysql/umysql.h"
typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMySQLHandler;
typedef SPA::CMasterPool<CMySQLHandler> CMySQLMasterPool;
typedef CMySQLMasterPool::CSlavePool CMySQLSlavePool;

//#include "../../../../include/async_sqlite.h"
//typedef SPA::ClientSide::CSqlite CSqliteHandler;
//typedef SPA::CMasterPool<CSqliteHandler> CSqliteMasterPool;
//typedef CSqliteMasterPool::CSlavePool CSqliteSlavePool;

//#include "../../../../include/async_odbc.h"
//typedef SPA::ClientSide::COdbc COdbcHandler;
//typedef SPA::CMasterPool<COdbcHandler> COdbcMasterPool;
//typedef COdbcMasterPool::CSlavePool COdbcSlavePool;

using namespace SPA::ServerSide;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
