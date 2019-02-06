
#ifndef PHP_ADAPTER_STDAFX_H
#define PHP_ADAPTER_STDAFX_H

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

	enum tagRequestReturnStatus {
		rrsServerException = -3,
		rrsCanceled = -2,
		rrsTimeout = -1,
		rrsClosed = 0,
		rrsOk = 1,
	};

	extern const int64_t PHP_ADAPTER_SECRET;
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
	extern const char *PHP_KEY;
	extern const char *PHP_TIMEOUT;
	extern const char *PHP_LEN;
	extern const char *PHP_OBJ;
	extern const char *PHP_SIZE;
	extern const char *PHP_ERR_CODE;
	extern const char *PHP_ERR_MSG;

	//SendRequest
	extern const char *PHP_SENDREQUEST;
	extern const char *PHP_SENDREQUEST_REQID;
	extern const char *PHP_SENDREQUEST_BUFF;
	extern const char *PHP_SENDREQUEST_SYNC;
	extern const char *PHP_SENDREQUEST_CH;
	extern const char *PHP_SENDREQUEST_EX;

	extern const char *PHP_SOCKET_CLOSED;
	extern const char *PHP_REQUEST_CANCELED;
	extern const char *PHP_SERVER_EXCEPTION;
	extern const char *PHP_REQUEST_TIMEOUT;

	extern const std::string SPA_NS;
	extern const std::string SPA_CS_NS;
	extern std::string SP_CONFIG;
	extern const char SYS_DIR;

	Php::Value GetManager();
	Php::Value GetSpPool(Php::Parameters &params);
	Php::Value GetSpHandler(Php::Parameters &params);
	Php::Value LockSpHandler(Php::Parameters &params);
	Php::Value SpBuff(Php::Parameters &params);

	typedef SPA::ClientSide::CCachedBaseHandler<0> CAsyncHandler;
	typedef SPA::ClientSide::CClientSocket CClientSocket;
	typedef SPA::ClientSide::tagSocketPoolEvent tagSocketPoolEvent;
	typedef SPA::ClientSide::CConnectionContext CConnectionContext;
}

#define KEY_CERT_STORE			"CertStore"
#define KEY_WORKING_DIR			"WorkingDir"
#define KEY_QUEUE_PASSWORD		"QueuePassword"
#define KEY_HOSTS				"Hosts"
#define KEY_SVS_ID				"SvsId"
#define KEY_THREADS				"Threads"
#define KEY_AUTO_CONN			"AutoConn"
#define KEY_QUEUE_NAME			"Queue"
#define KEY_AUTO_MERGE			"AutoMerge"
#define KEY_RECV_TIMEOUT		"RecvTimeout"
#define KEY_CONN_TIMEOUT		"ConnTimeout"
#define KEY_DEFAULT_DB			"DefaultDb"
#define KEY_POOL_TYPE			"PoolType"
#define KEY_SLAVES				"Slaves"
#define KEY_POOLS				"Pools"
#define KEY_PORT				"Port"
#define KEY_USER_ID				"UserId"
#define KEY_PASSWORD			"Password"
#define KEY_ENCRYPTION_METHOD	"EncrytionMethod"
#define KEY_ZIP					"Zip"
#define KEY_V6					"V6"
#define KEY_HOST				"Host"
#define KEY_ANY_DATA			"AnyData"


#endif