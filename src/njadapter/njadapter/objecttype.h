#ifndef __SOCKETPRO_NODEJS_ADAPTER_OBJECT_TYPES_H__
#define __SOCKETPRO_NODEJS_ADAPTER_OBJECT_TYPES_H__

namespace NJA {

	enum tagHandlerType {
		htJS,
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
		otSocketPool = 0,
		otReplication,
		otMasterPool,
		otSQLMasterPool,
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
		CSPAObject() : OT(otSocketPool), HT(htJS), Obj(nullptr) {
		}
		~CSPAObject() {
			ReleaseObject();
		}
		void ReleaseObject() {

		}
		CSPAObject(const CSPAObject &obj) = delete;
		CSPAObject& operator=(const CSPAObject &obj) = delete;

		tagObjectType OT;
		tagHandlerType HT;
		void *Obj;
	};
}

#endif
