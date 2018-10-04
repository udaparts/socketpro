var SPA=require('njadapter');

exports.newBuffer=function(initSize=4096, blockSize=4096) {
	return new SPA.CUQueue(initSize, blockSize);
};

//Base Request IDs
exports.BaseID={
	idUnknown : 0,
	idSwitchTo : 1,
	idRouteeChanged : 2,
	idEncrypted : 3,
	idBatchZipped : 4,
	idCancel : 5,
	idGetSockOptAtSvr : 6,
	idSetSockOptAtSvr : 7,
	idDoEcho : 8,
	idTurnOnZipAtSvr : 9,
	idStartBatching : 10,
	idCommitBatching : 11,
	idShrinkMemoryAtSvr : 12,
	idSetRouting : 13,
	idPing : 14,
	idEnableClientDequeue : 15,
	idServerException : 16,
	idAllMessagesDequeued : 17,
	idHttpClose : 18,
	idSetZipLevelAtSvr : 19,
	idStartJob : 20,
	idEndJob : 21,
	idRoutingData : 22,
	idDequeueConfirmed : 23,
	idMessageQueued : 24,
	idStartQueue : 25,
	idStopQueue : 26,
	idRoutePeerUnavailable : 27,
	idReservedOne : 0x100,
	idReservedTwo : 0x2001
};

//online message or chat request IDs
exports.ChatID={
	idEnter : 65,
	idSpeak : 66,
	idSpeakEx : 67,
	idExit : 68,
	idSendUserMessage : 69,
	idSendUserMessageEx : 70,
};

//reserved service IDs
exports.SID={
	sidReserved1:1,
	sidStartup:256,
	sidChat:257,
	sidQueue:257, //persistent message queue service
	sidHTTP: 258, //not supported at client side
	sidFile:259, //files streaming service
	sidOdbc:260, //ODBC SQL-streaming service
	sidReserved:0x10000000,
	sidSqlite:2147483632, //SQLite SQL-streaming services
    sidMysql:2147483633 //MySQL/Mariadb SQL-streaming services
};

//EM == EncryptionMethod
exports.EM={
	NoEncryption : 0,
	TLSv1 : 1
};

//socket shutdown type
exports.ShutdownType={
	receive:0,
	send:1,
	both:2
};

//compression options
exports.ZipLevel={
	zlDefault:0,
	zlBestSpeed:1,
	zlBestCompression:2
};

exports.OperationSystem={
	osWin:0,
	osApple:1,
	osMac:1,
	osIphone:1,
	osUnix:2,
	osLinux:2,
	osBSD:2,
	osAndroid:3,
	osWinCE:4, /**< Old window pocket pc, ce or smart phone devices*/
	osWinPhone:4
};

//CS == Client side namespace
exports.CS={
	version : SPA.getVersion(), //client core library version string, a static function
	getPools : function() {
		//return number of socket pools created, a static function
		return SPA.getPools();
	},
	
	//SSL/TLS server certificate authentication
	TLS : {
		setCA : function(caPath) {
			//set SSL/TLS CA certification store, a static function
			//return true if successful; Otherwise, false.

			//it works only on Linux to a pem file through openssl
			return SPA.setCA(caPath);
		},
		
		//authenticate server certificate by a public key (an array of bytes) in case certificate chain verification failed
		setKey : function(pk) {
			return SPA.setKey(pk);
		}
	},
	
	//client persistent message queue
	Queue : {
		getWorkingDir : function() {
			//find current working directory, a static function
			return SPA.getWorkingDir();
		},
		setWorkingDir : function(dir) {
			//set current working directory, a static function
			SPA.setWorkingDir(dir);
		},
		setPwd : function(pwd) {
			//set a password to protect client message queue, a static function 
			SPA.setPassword(pwd);
		},
		
		//queue flush options
		Optimistic : {
			oMemoryCached : 0,
			oSystemMemoryCached : 1,
			oDiskCommitted : 2
		},
		
		//reserved persistent queue request IDs
		ReqIds : {
			idEnqueue:0x2001 + 1,
			idDequeue:0x2001 + 2,
			idStartTrans:0x2001 + 3,
			idEndTrans:0x2001 + 4,
			idFlush:0x2001 + 5,
			idClose:0x2001 + 6,
			idGetKeys:0x2001 + 7,
			idEnqueueBatch:0x2001 + 8,
			idBatchSizeNotified:0x2001 + 20
		},
		
		//possible error codes from server persistent queue
		ErrorCode:{
			OK:0,
			TRANS_ALREADY_STARTED:1,
			TRANS_STARTING_FAILED:2,
			TRANS_NOT_STARTED_YET:3,
			TRANS_COMMITTING_FAILED:4,
			DEQUEUING:5,
			OTHER_WORKING_WITH_SAME_QUEUE:6,
			CLOSE_FAILED:7,
			ENQUEUING_FAILED:8
		},
		
		//persistent message queue status
		Status : {
			/// <summary>
			/// everything is fine
			/// </summary>
			qsNormal:0,

			/// <summary>
			/// Queued messages merged completely
			/// </summary>
			qsMergeComplete:1,

			/// <summary>
			/// Message replication started but not completed yet
			/// </summary>
			qsMergePushing:2,

			/// <summary>
			/// Message replicated incompletely from a source queue
			/// </summary>
			qsMergeIncomplete:3,

			/// <summary>
			/// A set of messages as a job are incompletely queued 
			/// </summary>
			qsJobIncomplete:4,

			/// <summary>
			/// A message queued incompletely because of application crash or unexpected termination
			/// </summary>
			qsCrash:5,

			/// <summary>
			/// Queue file open error
			/// </summary>
			qsFileError:6,

			/// <summary>
			/// Queue file opened but can not decrypt existing queued messages beacuse of bad password found
			/// </summary>
			qsBadPassword:7,

			/// <summary>
			/// Duplicate name error
			/// </summary>
			qsDuplicateName:8
		}	
	},
	
	newPool : function(svsId,defaulDb='') {
		//create a regular socket or master/slave pool.
		//you can create multiple pools for different services
		return new SPA.CSocketPool(	svsId, //a required unsigned int service id
									defaulDb //master/slave with real-time update cache
									);
	},
	//CC == Connection Context
	newCC : function(host,port,userId,pwd,em=0,zip=false,v6=false,anyData=null) {
		return {Host:host,Port:port,User:userId,Pwd:pwd,EM:em,Zip:zip,V6:v6,AnyData:anyData};
	},
	//Socket Connection State
	ConnState:{
		csClosed:0,
		csConnecting:1,
		csSslShaking:2,
		csClosing:3,
		csConnected:4,
		csSwitched:5
	},
	
	//Socket Pool Event
	PoolEvent:{
		speUnknown:-1,
		speStarted:0,
		speCreatingThread:1,
		speThreadCreated:2,
		speConnecting:3,
		speConnected:4,
		speKillingThread:5,
		speShutdown:6,
		speUSocketCreated:7,
		speHandShakeCompleted:8,
		speLocked:9,
		speUnlocked:10,
		speThreadKilled:11,
		speClosingSocket:12,
		speSocketClosed:13,
		speUSocketKilled:14,
		speTimer:15,
		speQueueMergedFrom:16,
		speQueueMergedTo:17,
	}
};

//DB namespace
exports.DB={
	
	//defined DB management systems
	ManagementSystem : {
		Unknown:-1,
		Sqlite:0,
		Mysql:1,
		ODBC:2,
		MsSQL:3,
		Oracle:4,
		DB2:5,
		PostgreSQL:6,
		MongoDB:7
	},
	
	//rollback hints defined for ending a manual transaction asynchronously
	RollbackPlan:{
		/// <summary>
		/// Manual transaction will rollback whenever there is an error by default
		/// </summary>
		rpDefault:0,
		
		/// <summary>
		/// Manual transaction will rollback whenever there is an error by default
		/// </summary>
		rpErrorAny:0,

		/// <summary>
		/// Manual transaction will rollback as long as the number of errors is less than the number of ok processing statements
		/// </summary>
		rpErrorLess:1,

		/// <summary>
		/// Manual transaction will rollback as long as the number of errors is less or equal than the number of ok processing statements
		/// </summary>
		rpErrorEqual:2,

		/// <summary>
		/// Manual transaction will rollback as long as the number of errors is more than the number of ok processing statements
		/// </summary>
		rpErrorMore:3,

		/// <summary>
		/// Manual transaction will rollback only if all the processing statements are failed
		/// </summary>
		rpErrorAll:4,

		/// <summary>
		/// Manual transaction will rollback always no matter what happens.
		/// </summary>
		rpAlways:5
	},
	
	//DB transaction isolation levels
	TransIsolation : {
		Unspecified:-1,
		Chaos:0,
		ReadUncommited:1,
		Browse:2,
		CursorStability:3,
		ReadCommited:3,
		RepeatableRead:4,
		Serializable:5,
		Isolated:6
	},
	
	//reserved DB operational request IDs
	ReqIds:{
		/// <summary>
		/// Async database client/server just requires the following request identification numbers 
		/// </summary>
		idOpen:0x7E7F,
		idClose:0x7E80,
		idBeginTrans:0x7E81,
		idEndTrans:0x7E82,
		idExecute:0x7E83,
		idPrepare:0x7E84,
		idExecuteParameters:0x7E85,

		/// <summary>
		/// the request identification numbers used for message push from server to client
		/// </summary>
		idDBUpdate:0x7E86, //server ==> client only
		idRowsetHeader:0x7E87, //server ==> client only
		idOutputParameter:0x7E88, //server ==> client only

		/// <summary>
		/// Internal request/response identification numbers used for data communication between client and server
		/// </summary>
		idBeginRows:0x7E89,
		idTransferring:0x7E8A,
		idStartBLOB:0x7E8B,
		idChunk:0x7E8C,
		idEndBLOB:0x7E8D,
		idEndRows:0x7E8E,
		idCallReturn:0x7E8F, //server ==> client only

		idGetCachedTables:0x7E90,

		idSqlBatchHeader:0x7E91, //server ==> client only
		idExecuteBatch:0x7E92,
		idParameterPosition:0x7E93 //server ==> client only
	}
};

//real-time updateable data cache
exports.Cache={
	/// <summary>
	/// A flag used with idOpen for tracing database table update events
	/// </summary>
	ENABLE_TABLE_UPDATE_MESSAGES:1,
	
	/// <summary>
	/// A chat group id used at SocketPro server side for notifying database events from server to connected clients
	/// </summary>
	STREAMING_SQL_CHAT_GROUP_ID:0x1fffffff,
	
	CACHE_UPDATE_CHAT_GROUP_ID:0x20000000,
	
	//operator
	Op : {
		eq:0, //equal
		gt:1, //great
		lt:2, //less
		ge:3, //great_equal
		le:4, //less_equal
		ne:5, //not_equal
		is_null:6 //is_null
	}
};
