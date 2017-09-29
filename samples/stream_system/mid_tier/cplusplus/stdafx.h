
#pragma once


#include "../../../../include/aserverw.h"


#include "../../shared/tablecache.h"
#include "../../../../include/mysql/umysql.h"
typedef SPA::CMasterPool<SPA::Mysql::sidMysql> CMySQLMasterPool;
typedef CMySQLMasterPool::CAsyncSQLPool CMySQLSlavePool;
typedef CMySQLMasterPool::CAsyncSQLHandler CMySQLHandler;

using namespace SPA::ClientSide;
using namespace SPA::ServerSide;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
