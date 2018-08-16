#ifndef __SOCKETPRO_NODEJS_ADAPTER_OBJECT_TYPES_H__
#define __SOCKETPRO_NODEJS_ADAPTER_OBJECT_TYPES_H__

namespace NJA {

	enum tagHandlerType {
		htJS = 0,
		htCachedJS,
		htSqlite,
		htMySQL,
		htODBC,
		htFile,
	};

	enum tagPool {
		pSocketPool = 0,
		pReplication,
		pMasterPool,
		pSQLMasterPool,
	};

	enum tagObjectType {
		otPool = 0,
		otBuffer,
		otAsyncResult,
		otColumnInfo,
		otParameterInfo,
		otTable,
		otDataSet,
		otPush,
		otQueue,
		otClientSocket,
		otHandler,
	};

	struct CSPAObject {
		CSPAObject() : Object(nullptr), ObjectType(otPool) {
		}
		void *Object;
		tagObjectType ObjectType;
	};
}

#endif
