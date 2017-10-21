
#pragma once

#include <future>
#include "../../../../include/rdbcache.h" //relation DB cache

#include "../../../../include/mysql/umysql.h" //MySQL constants
typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMySQLHandler;
typedef SPA::CSQLMasterPool<true, CMySQLHandler> CMySQLMasterPool;
typedef CMySQLMasterPool::CSlavePool CMySQLSlavePool;

using namespace SPA::ServerSide;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
