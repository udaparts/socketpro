
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
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/writer.h"

namespace PA {
	void Trim(std::string &str);

	enum tagPoolType {
		NotMS = 0,
		Slave = 1,
		Master = 2
	};
	extern const char *PHP_BUFFER;
	extern const char *PHP_CONN_CONTEXT;
	extern const char *PHP_FILE_HANDLER;
	extern const char *PHP_DB_HANDLER;
	extern const char *PHP_QUEUE_HANDLER;
	extern const char *PHP_ASYNC_HANDLER;
	extern const char *PHP_SOCKET_POOL;
	extern const char *PHP_CERT;
	extern const char *PHP_SOCKET;
	extern const char *PHP_CONSTRUCT;
	extern const char *PHP_MANAGER;

	//SendRequest
	extern const char *PHP_SENDREQUEST;
	extern const char *PHP_SENDREQUEST_REQID;
	extern const char *PHP_SENDREQUEST_BUFF;
	extern const char *PHP_SENDREQUEST_RH;
	extern const char *PHP_SENDREQUEST_CH;
	extern const char *PHP_SENDREQUEST_EX;

	extern const std::string SPA_NS;
	extern const std::string SPA_CS_NS;
	extern std::string SP_CONFIG;
	extern const char SYS_DIR;

	Php::Value GetManager();

	typedef SPA::ClientSide::CCachedBaseHandler<0> CAsyncHandler;
	typedef SPA::ClientSide::CClientSocket CClientSocket;
	typedef SPA::ClientSide::tagSocketPoolEvent tagSocketPoolEvent;
	typedef SPA::ClientSide::CConnectionContext CConnectionContext;
}
