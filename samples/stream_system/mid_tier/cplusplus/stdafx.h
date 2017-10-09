
#pragma once

#include "../../../../include/aserverw.h"
#include "../../../../include/rdbcache.h"

#include "../../../../include/mysql/umysql.h"
typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMySQLHandler;
typedef SPA::CMasterPool<CMySQLHandler> CMySQLMasterPool;
typedef CMySQLMasterPool::CSlavePool CMySQLSlavePool;

using namespace SPA::ServerSide;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
