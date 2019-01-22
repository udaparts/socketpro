
#pragma once

#include "../../../include/definebase.h"

#ifdef WIN32_64

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

#define SPA_PHP_EXPORT __declspec(dllexport)
#else
#define SPA_PHP_EXPORT __attribute__ ((visibility ("default")))
#endif

#include "../../../include/aqhandler.h"
#include "../../../include/streamingfile.h"
#include "../../../include/udb_client.h"
#include "../../../include/masterpool.h"
#include "../../../include/rdbcache.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/sqlite/usqlite.h"
#include "../../../include/odbc/uodbc.h"
#include <algorithm>

namespace PA {
	void Trim(std::string &str);

	enum tagPoolType {
		NotMS = 0,
		Slave = 1,
		Master = 2
	};
	extern const char *PHP_BUFFER;
	extern const char *PHP_FILE_HANDLER;
	extern const char *PHP_DB_HANDLER;
	extern const char *PHP_QUEUE_HANDLER;
	extern const char *PHP_ASYNC_HANDLER;
	extern const char *PHP_SOCKET_POOL;
	extern const char *PHP_CONSTRUCT;

	//SendRequest
	extern const char *PHP_SENDREQUEST;
	extern const char *PHP_SENDREQUEST_REQID;
	extern const char *PHP_SENDREQUEST_BUFF;
	extern const char *PHP_SENDREQUEST_RH;
	extern const char *PHP_SENDREQUEST_CH;
	extern const char *PHP_SENDREQUEST_EX;

	typedef SPA::ClientSide::CCachedBaseHandler<0> CAsyncHandler;
	typedef SPA::ClientSide::CClientSocket CClientSocket;
}
