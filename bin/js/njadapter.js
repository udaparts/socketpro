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

//Online message or chat request IDs
exports.ChatID={
	idEnter : 65,
	idSpeak : 66,
	idSpeakEx : 67,
	idExit : 68,
	idSendUserMessage : 69,
	idSendUserMessageEx : 70,
};

//Service IDs
exports.BaseSID={
	sidReserved1:1,
	sidStartup:256,
	sidChat:257,
	sidQueue:257,
	sidHTTP: 258,
	sidFile:259,
	sidOdbc:260,
	sidReserved:0x10000000,
	sidSqlite:(0x10000000+0x6FFFFFF0),
    sidMysql:(0x10000000+0x6FFFFFF1)
};

//EM == EncryptionMethod
exports.EM={
	NoEncryption : 0,
	TLSv1 : 1
};

//Persistent message queue status
exports.QueueStatus={
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
	qsDuplicateName:8,
};

exports.Optimistic={
	oMemoryCached : 0,
	oSystemMemoryCached : 1,
	oDiskCommitted : 2
};

//CS == ClientSide namespace
exports.CS={
	newPool : function(svsId,defaulDb='') {
		return new SPA.CSocketPool(	svsId, //a required unsigned int service id
									defaulDb //master/slave with real-time update cache
									);
	},
	//CC == Connection Context
	newCC : function(host,port,userId,pwd,em=0,zip=false,v6=false,anyData=null) {
		return {Host:host,Port:port,User:userId,Pwd:pwd,EM:em,Zip:zip,V6:v6,AnyData:anyData};
	},
	//Connection State
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
