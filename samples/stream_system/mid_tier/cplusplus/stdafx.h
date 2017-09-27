
#pragma once

#include "../../../../include/mysql/umysql.h"
#include "../../../../include/aserverw.h"
#include "../../../../include/udb_client.h"
#include "../../shared/ss_defines.h"

using namespace SPA::ClientSide;
using namespace SPA::ServerSide;
using namespace SPA::UDB;

typedef SPA::ClientSide::CAsyncDBHandler<SPA::Mysql::sidMysql> CAsyncSQLHandler;

typedef SPA::ClientSide::CSocketPool<CAsyncSQLHandler> CAsyncSQLPool;

#ifdef WIN32_64

#include "targetver.h"

#else

#endif

#include <stdio.h>
#include <iostream>
