

#pragma once

#include "../../../../include/generalcache.h"
#include "../../shared/ss_defines.h"

#include "../../../../include/rdbcache.h" //relation DB cache

#include "../../../../include/aserverw.h"
#include "../../../../include/mysql/umysql.h" //MySQL constants
typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CMySQLHandler;
typedef SPA::CMasterPool<CMySQLHandler> CMySQLMasterPool;
typedef CMySQLMasterPool::CSlavePool CMySQLSlavePool;

using namespace SPA::ClientSide;

#ifdef WIN32_64
#include "targetver.h"
#else

#endif

#include <stdio.h>
#include <iostream>