
#pragma once

#include <future>
#include "../../../../include/rdbcache.h" //relation DB cache

#if 0
#include "../../../../include/mysql/umysql.h" //MySQL constants
typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMySQLHandler;
typedef SPA::CSQLMasterPool<true, CMySQLHandler> CMySQLMasterPool;
#else
#include "../../../../include/async_sqlite.h"
typedef SPA::CSQLMasterPool<true, SPA::ClientSide::CSqlite> CMySQLMasterPool;
#endif
typedef CMySQLMasterPool::CSlavePool CMySQLSlavePool;
typedef CMySQLMasterPool::CSQLHandler CSQLHandler;

using namespace SPA::ServerSide;
using namespace SPA::ClientSide;
using namespace SPA::UDB;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
