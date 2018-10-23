var SPA = require('njadapter');
var assert = require('assert');

class CTable {
    constructor(t) {
        assert(t);
        this.table = t;
    }

    get Data() {
        return this.table.getData();
    }

    get Keys() {
        return this.table.getKeys();
    }

    get Meta() {
        return this.table.getMeta();
    }

    get Rows() {
        return this.table.getRows();
    }

    get Columns() {
        return this.table.getColumns();
    }

    Append(tbl) {
        return this.table.Append(tbl);
    }

    Dispose() {
        this.table.Dispose();
    }

    Sort(ordinal, desc = false) {
        return this.table.Sort(ordinal, desc);
    }

    FindOrdinal(colName) {
        return this.table.FindOrdinal(colName);
    }

    Find(odinal, data, op = exports.Cache.Op.eq, copy = false) {
        return new CTable(this.table.Find(odinal, op, data, copy));
    }

    FindNull(odinal, copy = false) {
        return new CTable(this.table.FindNull(odinal, copy));
    }

    Between(odinal, d0, d1, copy = false) {
        return new CTable(this.table.Between(odinal, d0, d1, copy));
    }

    In(odinal, arr, copy = false) {
        return new CTable(this.table.In(odinal, arr, copy));
    }

    NotIn(odinal, arr, copy = false) {
        return new CTable(this.table.NotIn(odinal, arr, copy));
    }
}

class CCache {
    constructor(c) {
        assert(c);
        this.cache = c;
    }

    get DbIp() {
        return this.cache.getDbIp();
    }

    get DbName() {
        return this.cache.getDbName();
    }

    get Updater() {
        return this.cache.getUpdater();
    }

    get DbMs() {
        return this.cache.getMS();
    }

    get Updater() {
        return this.cache.getUpdater();
    }

    get Empty() {
        return this.cache.isEmpty();
    }

    get DBNameCase() {
        return this.cache.getDBNameCase();
    }

    get TableNameCase() {
        return this.cache.getTableNameCase();
    }

    get FieldNameCase() {
        return this.cache.getFieldNameCase();
    }

    get DataCase() {
        return this.cache.getDataCase();
    }

    get DbTable() {
        return this.cache.getDbTable();
    }

    GetMeta(dbName, tblName) {
        return this.cache.GetMeta(dbName, tblName);
    }

    GetRowCount(dbName, tblName) {
        return this.cache.GetRowCount(dbName, tblName);
    }

    GetFields(dbName, tblName) {
        return this.cache.GetFields(dbName, tblName);
    }

    FindKeys(dbName, tblName) {
        return this.cache.FindKeys(dbName, tblName);
    }

    FindOrdinal(dbName, tblName, colName) {
        return this.cache.FindOrdinal(dbName, tblName, colName);
    }

    Find(dbName, tblName, odinal, data, op = exports.Cache.Op.eq, hint = '') {
        return new CTable(this.cache.Find(dbName, tblName, odinal, op, data, hint));
    }

    FindNull(dbName, tblName, odinal) {
        return new CTable(this.cache.FindNull(dbName, tblName, odinal));
    }

    Between(dbName, tblName, odinal, d0, d1) {
        return new CTable(this.cache.Between(dbName, tblName, odinal, d0, d1));
    }

    In(dbName, tblName, odinal, arr) {
        return new CTable(this.cache.In(dbName, tblName, odinal, arr));
    }

    NotIn(dbName, tblName, odinal, arr) {
        return new CTable(this.cache.NotIn(dbName, tblName, odinal, arr));
    }
}

class CClientQueue {
    constructor(cq) {
        assert(cq);
        this.clientQueue = cq;
    }

    get MessagesInDequeuing() {
        return this.clientQueue.getMessagesInDequeuing();
    }

    get MessageCount() {
        return this.clientQueue.getMessageCount();
    }

    get Size() {
        return this.clientQueue.getSize();
    }

    get Available() {
        return this.clientQueue.getAvailable();
    }

    get Secure() {
        return this.clientQueue.getSecure();
    }

    get FileName() {
        return this.clientQueue.getFileName();
    }

    get Name() {
        return this.clientQueue.getName();
    }

    get Enabled() {
        return this.clientQueue.getEnabled();
    }

    get JobSize() {
        return this.clientQueue.getJobSize();
    }

    get LastIndex() {
        return this.clientQueue.getLastIndex();
    }

    get Shared() {
        return this.clientQueue.getShared();
    }

    get TTL() {
        return this.clientQueue.getTTL();
    }

    get Status() {
        return this.clientQueue.getStatus();
    }

    get LastMsgTime() {
        return this.clientQueue.getLastMsgTime();
    }

    get RoutingIndex() {
        return this.clientQueue.getRoutingIndex();
    }
    set RoutingIndex(enabled = false) {
        this.clientQueue.setRoutingIndex(enabled);
    }

    get Optimistic() {
        return this.clientQueue.getOptimistic();
    }
    set Optimistic(op = exports.CS.Queue.Optimistic.oMemoryCached) {
        this.clientQueue.setOptimistic(op);
    }

    StartQueue(qn, secure = false, ttl = 24 * 3600) {
        assert(typeof qn === 'string' && qn.length > 0);
        return this.clientQueue.StartQueue(qn, secure, ttl);
    }

    StopQueue(permanent = false) {
        this.clientQueue.StopQueue(permanent);
    }

    AbortJob() {
        return this.clientQueue.AbortJob();
    }

    StartJob() {
        return this.clientQueue.StartJob();
    }

    EndJob() {
        return this.clientQueue.EndJob();
    }

    RemoveByTTL() {
        return this.clientQueue.RemoveByTTL();
    }

    Reset() {
        this.clientQueue.Reset();
    }
}

class CSocket {
    constructor(s) {
        assert(s);
        this.socket = s;
    }

    get Zip() {
        return this.socket.getZip();
    }
    set Zip(zip = false) {
        this.socket.setZip(zip);
    }

    get ZipLevel() {
        return this.socket.getZipLevel();
    }
    set ZipLevel(zl = exports.ZipLevel.zlDefault) {
        this.socket.setZipLevel(zl);
    }

    get ConnTimeout() {
        return this.socket.getConnTimeout();
    }
    set ConnTimeout(timeout = 30000) {
        return this.socket.setConnTimeout(timeout);
    }

    get RecvTimeout() {
        return this.socket.getRecvTimeout();
    }
    set RecvTimeout(timeout = 30000) {
        return this.socket.setRecvTimeout(timeout);
    }

    get AutoConn() {
        return this.socket.getAutoConn();
    }
    set AutoConn(ac = false) {
        this.socket.setAutoConn(ac);
    }

    get Sendable() {
        return this.socket.getSendable();
    }

    get Cert() {
        return this.socket.getCert();
    }

    get ConnState() {
        return this.socket.getConnState();
    }

    get RouteeCount() {
        return this.socket.getRouteeCount();
    }

    get Routing() {
        return this.socket.isRouting();
    }

    get RequestsQueued() {
        return this.socket.getReqsQueued();
    }

    get CurrReqId() {
        return this.socket.getCurrReqId();
    }

    get CurrSvsId() {
        return this.socket.getCurrSvsId();
    }

    get ServerPingTime() {
        return this.socket.getServerPingTime();
    }

    get EncryptionMethod() {
        return this.socket.getEM();
    }

    get Error() {
        return this.socket.getError();
    }

    get Connected() {
        return this.socket.isConnected();
    }

    get ConnContext() {
        return this.socket.getConnContext();
    }

    get Random() {
        return this.socket.isRandom();
    }

    get BytesInSendingBuffer() {
        return this.socket.getBytesInSendingBuffer();
    }

    get BytesInRecvBuffer() {
        return this.socket.getBytesInRecvBuffer();
    }

    get BytesBatched() {
        return this.socket.getBytesBatched();
    }

    get BytesReceived() {
        return this.socket.getBytesReceived();
    }

    get BytesSent() {
        return this.socket.getBytesSent();
    }

    get UserId() {
        return this.socket.getUserId();
    }

    get PeerOs() {
        return this.socket.getPeerOs();
    }

    get PeerAddr() {
        return this.socket.getPeerAddr();
    }

    get Push() {
        return this.socket.getPush();
    }

    get Queue() {
        return new CClientQueue(this.socket.getQueue());
    }

    Dispose() {
        this.socket.Dispose();
    }

    Close() {
        this.socket.Close();
    }

    Shutdown(st = exports.ShutdownType.both) {
        this.socket.Shutdown(st);
    }

    IgnoreLastRequest(reqId) {
        return this.socket.IgnoreLastRequest(reqId);
    }

    Cancel() {
        return this.socket.Cancel();
    }

    Echo() {
        return this.socket.Echo();
    }

    TurnOnZipAtSvr(zip = true) {
        return this.socket.SetSvrZip(zip);
    }

    SetSvrZiplevel(zl = exports.ZipLevel.zlDefault) {
        return this.socket.SetSvrZiplevel(zl);
    }
}

class CSocketPool {
    constructor(p) {
        assert(p);
        assert(p.getSvsId() > 0);
        this.pool = p;
    }

    get PoolId() {
        return this.pool.getPoolId();
    }

    get ConnectedSockets() {
        return this.pool.getConnectedSockets();
    }

    get ClosedSockets() {
        return this.pool.getClosedSockets();
    }

    get SvsId() {
        return this.pool.getSvsId();
    }

    get Error() {
        return this.pool.getError();
    }

    get Queues() {
        return this.pool.getQueues();
    }

    get Count() {
        return this.pool.getTotalSockets();
    }

    get Started() {
        return this.pool.getStarted();
    }

    get Cache() {
        var cache = this.pool.getCache();
        if (cache)
            return new CCache(cache);
        return cache;
    }

    get AutoMerge() {
        return this.pool.getAutoMerge();
    }

    set AutoMerge(am = false) {
        this.pool.setAutoMerge(am);
    }

    get QueueName() {
        return this.pool.getQueueName();
    }

    set QueueName(qn = '') {
        this.pool.setQueueName(qn);
    }

    set PoolEvent(pe) {
        assert(pe === null || pe === undefined || typeof pe === 'function');
        this.pool.setPoolEvent(pe);
    }

    set ResultReturned(rr) {
        assert(rr === null || rr === undefined || typeof rr === 'function');
        this.pool.setReturned(rr);
    }

    set AllProcessed(ap) {
        assert(ap === null || ap === undefined || typeof ap === 'function');
        this.pool.setAllProcessed(ap);
    }

    set Push(p) {
        assert(p === null || p === undefined || typeof p === 'function');
        this.pool.setPush(p);
    }

    set BaseReqProcessed(brp) {
        assert(brp === null || brp === undefined || typeof brp === 'function');
        this.pool.setBaseReqProcessed(brp);
    }

    set ServerException(se) {
        assert(se === null || se === undefined || typeof se === 'function');
        this.pool.setServerException(se);
    }

    get Sockets() {
        var vs = [];
        var arr = this.pool.getSockets();
        for (var n = 0; n < arr.length; ++n) {
            var s = arr[n];
            if (s) vs.push(new CSocket(s));
        }
        return vs;
    }

    get Handlers() {
        var svsId = this.pool.getSvsId();
        var arr = this.pool.getHandlers();
        var vh = [];
        for (var n = 0; n < arr.length; ++n) {
            var h = arr[n];
            if (h) {
                switch (svsId) {
                    case exports.SID.sidFile:
                        vh.push(new CAsyncFile(h));
                        break;
                    case exports.SID.sidQueue:
                        vh.push(new CAsyncQueue(h));
                        break;
                    case exports.SID.sidODBC:
                    case exports.SID.sidSqlite:
                    case exports.SID.sidMysql:
                        vh.push(new CDb(h));
                        break;
                    default:
                        vh.push(new CHandler(h));
                        break;
                }
            }
        }
        return vh;
    }

    Seek() {
        var h = this.pool.Seek();
        if (h) {
            switch (h.getSvsId()) {
                case exports.SID.sidFile:
                    h = new CAsyncFile(h);
                    break;
                case exports.SID.sidQueue:
                    h = new CAsyncQueue(h);
                    break;
                case exports.SID.sidODBC:
                case exports.SID.sidSqlite:
                case exports.SID.sidMysql:
                    h = new CDb(h);
                    break;
                default:
                    h = new CHandler(h);
                    break;
            }
        }
        return h;
    }

    SeekByQueue(qn) {
        assert(qn === null || qn === undefined || typeof qn === 'string');
        var h = this.pool.SeekByQueue(qn);
        if (h) {
            switch (h.getSvsId()) {
                case exports.SID.sidFile:
                    h = new CAsyncFile(h);
                    break;
                case exports.SID.sidQueue:
                    h = new CAsyncQueue(h);
                    break;
                case exports.SID.sidODBC:
                case exports.SID.sidSqlite:
                case exports.SID.sidMysql:
                    h = new CDb(h);
                    break;
                default:
                    h = new CHandler(h);
                    break;
            }
        }
        return h;
    }

    Start(cc, sessions) {
        assert(typeof sessions === 'number');
        return this.pool.Start(cc, sessions);
    }

    Shutdown() {
        return this.pool.Shutdown();
    }

    NewSlave(defaultDb = '') {
        assert(defaultDb === null || defaultDb === undefined || typeof defaultDb === 'string');
        var p = this.pool.NewSlave(defaultDb);
        if (p)
            return new CSocketPool(p);
        return p;
    }

    Dispose() {
        return this.pool.Dispose();
    }
}

class CHandler {
    constructor(h) {
        assert(h);
        assert(h.getSvsId() > 0);
        this.handler = h;
    }

    get Socket() {
        return new CSocket(this.handler.getSocket());
    }

    get SvsId() {
        return this.handler.getSvsId();
    }

    get RequestsQueued() {
        return this.handler.getRequestsQueued();
    }

    get Batching() {
        return this.handler.isBatching();
    }

    get DequeuedMessageAborted() {
        return this.handler.isDequeuedMessageAborted();
    }

    get DequeuedResult() {
        return this.handler.isDequeuedResult();
    }

    get RouteeResult() {
        return this.handler.isRouteeResult();
    }

    IsSame(h) {
        if (!h) return false;
        return this.handler.isSame(h.handler);
    }

    CleanCallbacks() {
        return this.handler.CleanCallbacks();
    }

    AbortDequeuedMessage() {
        this.handler.AbortDequeuedMessage();
    }

    AbortBatching() {
        return this.handler.AbortBatching();
    }

    CommitBatching(serverCommit = false) {
        return this.handler.CommitBatching(serverCommit);
    }

    Dispose() {
        return this.handler.Dispose();
    }

    StartBatching() {
        return this.handler.StartBatching();
    }

    SendRequest(reqId, buff, cb, discarded = null, serverException = null) {
        return this.handler.SendRequest(reqId, buff, cb, discarded, serverException);
    }

    //Promise version
    sendRequest(reqId, buff, cb, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.SendRequest(reqId, buff, (q, h, id) => {
                if (cb) ret = cb(q, id, h);
                res(ret);
            }, (canceled, h, id) => {
                if (discarded) ret = discarded(canceled, id, h);
                if (ret === undefined) ret = (canceled ? 'Request canceled' : 'Connection closed');
                rej(ret);
            }, (errMsg, errCode, errWhere, h, id) => {
                if (serverException) ret = serverException(errMsg, errCode, errWhere, id, h);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg,
                    stack: errWhere,
                    reqId: id,
                    handler: h
                };
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, reqId, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }
}

exports.newBuffer = function(initSize = 4096, blockSize = 4096) {
    return new SPA.CUQueue(initSize, blockSize);
};

//Base Request IDs
exports.BaseID = {
    idUnknown: 0,
    idSwitchTo: 1,
    idRouteeChanged: 2,
    idEncrypted: 3,
    idBatchZipped: 4,
    idCancel: 5,
    idGetSockOptAtSvr: 6,
    idSetSockOptAtSvr: 7,
    idDoEcho: 8,
    idTurnOnZipAtSvr: 9,
    idStartBatching: 10,
    idCommitBatching: 11,
    idShrinkMemoryAtSvr: 12,
    idSetRouting: 13,
    idPing: 14,
    idEnableClientDequeue: 15,
    idServerException: 16,
    idAllMessagesDequeued: 17,
    idHttpClose: 18,
    idSetZipLevelAtSvr: 19,
    idStartJob: 20,
    idEndJob: 21,
    idRoutingData: 22,
    idDequeueConfirmed: 23,
    idMessageQueued: 24,
    idStartQueue: 25,
    idStopQueue: 26,
    idRoutePeerUnavailable: 27,
    idReservedOne: 0x100,
    idReservedTwo: 0x2001
};

//online message or chat request IDs
exports.ChatID = {
    idEnter: 65,
    idSpeak: 66,
    idSpeakEx: 67,
    idExit: 68,
    idSendUserMessage: 69,
    idSendUserMessageEx: 70,
};

//reserved service IDs
exports.SID = {
    sidReserved1: 1,
    sidStartup: 256,
    sidChat: 257,
    sidQueue: 257, //persistent message queue service
    sidHTTP: 258, //not supported at client side
    sidFile: 259, //files streaming service
    sidOdbc: 260, //ODBC SQL-streaming service
    sidReserved: 0x10000000,
    sidSqlite: 2147483632, //SQLite SQL-streaming services
    sidMysql: 2147483633 //MySQL/Mariadb SQL-streaming services
};

class CAsyncQueue extends CHandler {
    constructor(h) {
        super(h);
        assert(h.getSvsId() == exports.SID.sidQueue);
    }

    get DequeueBatchSize() {
        return this.handler.getDeqBatchSize();
    }

    get EnqueueNotified() {
        return this.handler.getEnqNotified();
    }

    set ResultReturned(rr) {
        return this.handler.setResultReturned(rr);
    }

    BatchMessage(reqId, buff) {
        return this.handler.BatchMessage(reqId, buff);
    }

    GetKeys(cb, discarded = null) {
        return this.handler.GetKeys(cb, discarded);
    }

    StartTrans(key, cb = null, discarded = null) {
        return this.handler.StartTrans(key, cb, discarded);
    }

    EndTrans(rollback = false, cb = null, discarded = null) {
        return this.handler.EndTrans(rollback, cb, discarded);
    }

    Close(key, cb = null, discarded = null, permanent = false) {
        return this.handler.Close(key, cb, discarded, permanent);
    }

    Flush(key, cb = null, discarded = null, option = exports.CS.Queue.Optimistic.oMemoryCached) {
        return this.handler.Flush(key, cb, discarded, option);
    }

    Dequeue(key, cb = null, discarded = null, timeout = 0) {
        return this.handler.Dequeue(key, cb, discarded, timeout);
    }

    Enqueue(key, reqId, buff, cb = null, discarded = null) {
        return this.handler.Enqueue(key, reqId, buff, cb, discarded);
    }

    EnqueueBatch(key, cb = null, discarded = null) {
        return this.handler.EnqueueBatch(key, cb, discarded);
    }

    //Promise
    getKeys(cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.GetKeys((vKeys, queue) => {
                if (cb) ret = cb(vKeys, queue);
                if (ret === undefined) ret = vKeys;
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'GetKeys canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    close(key, permanent = false, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Close(key, (errCode, queue) => {
                if (cb) ret = cb(errCode, queue);
                if (ret === undefined) ret = errCode;
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'Close canceled' : 'Connection closed');
                rej(ret);
            }, permanent);
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    startTrans(key, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.StartTrans(key, (errCode, queue) => {
                if (cb) ret = cb(errCode, queue);
                if (ret === undefined) ret = errCode;
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'StartTrans canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    endTrans(rollback = false, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.EndTrans(rollback, (errCode, queue) => {
                if (cb) ret = cb(errCode, queue);
                if (ret === undefined) ret = errCode;
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'EndTrans canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    flush(key, option = exports.CS.Queue.Optimistic.oMemoryCached, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Flush(key, (messages, fileSize, queue) => {
                if (cb) ret = cb(messages, fileSize, queue);
                if (ret === undefined) ret = {
                    msgs: messages,
                    fsize: fileSize
                };
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'Flush canceled' : 'Connection closed');
                rej(ret);
            }, option);
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    enqueue(key, reqId, buff, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Enqueue(key, reqId, buff, (index, queue) => {
                if (cb) ret = cb(index, queue);
                if (ret === undefined) ret = index;
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'Enqueue canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    enqueueBatch(key, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.EnqueueBatch(key, (index, queue) => {
                if (cb) ret = cb(index, queue);
                if (ret === undefined) ret = index;
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'EnqueueBatch canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    dequeue(key, timeout = 0, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Dequeue(key, (messages, fileSize, messagesDequeued, bytes, queue) => {
                if (cb) ret = cb(messages, fileSize, messagesDequeued, bytes, queue);
                if (ret === undefined) ret = {
                    msgs: messages,
                    fsize: fileSize,
                    msgsDequeued: messagesDequeued,
                    bytes: bytes
                };
                res(ret);
            }, (canceled, queue) => {
                if (discarded) ret = discarded(canceled, queue);
                if (ret === undefined) ret = (canceled ? 'Dequeue canceled' : 'Connection closed');
                rej(ret);
            }, timeout);
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }
}

class CAsyncFile extends CHandler {
    constructor(h) {
        super(h);
        assert(h.getSvsId() == exports.SID.sidFile);
    }

    get FilesQueued() {
        return this.handler.getFilesQueued();
    }

    Upload(localFile, remoteFile, cb = null, progress = null, discarded = null) {
        return this.handler.Upload(localFile, remoteFile, cb, progress, discarded);
    }

    Download(localFile, remoteFile, cb = null, progress = null, discarded = null) {
        return this.handler.Download(localFile, remoteFile, cb, progress, discarded);
    }

    //Promise version
    upload(localFile, remoteFile, progress = null, cb = null, discarded = null) {
        assert(localFile && typeof localFile === 'string');
        assert(remoteFile && typeof remoteFile === 'string');
        assert(progress === null || progress === undefined || typeof progress === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Upload(localFile, remoteFile, (errMsg, errCode, download, file) => {
                if (cb) ret = cb(errMsg, errCode, download, file);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, progress, (canceled, download, file) => {
                if (discarded) ret = discarded(canceled, download, file);
                if (ret === undefined) ret = (canceled ? 'Upload canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise version
    download(localFile, remoteFile, progress = null, cb = null, discarded = null) {
        assert(localFile && typeof localFile === 'string');
        assert(remoteFile && typeof remoteFile === 'string');
        assert(progress === null || progress === undefined || typeof progress === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Download(localFile, remoteFile, (errMsg, errCode, download, file) => {
                if (cb) ret = cb(errMsg, errCode, download, file);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, progress, (canceled, download, file) => {
                if (discarded) ret = discarded(canceled, download, file);
                if (ret === undefined) ret = (canceled ? 'Download canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, true, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }
}

class CDb extends CHandler {
    constructor(h) {
        super(h);
        var sid = h.getSvsId();
        assert(sid == exports.SID.sidOdbc || sid == exports.SID.sidSqlite || sid == exports.SID.sidMysql);
    }

    get DbMS() {
        return this.handler.getDbMS();
    }

    get Opened() {
        return this.handler.isOpened();
    }

    BeginTrans(isolation = exports.DB.TransIsolation.ReadCommited, cb = null, discarded = null) {
        return this.handler.BeginTrans(isolation, cb, discarded);
    }

    Close(cb = null, discarded = null) {
        return this.handler.Close(cb, discarded);
    }

    EndTrans(rp = exports.DB.RollbackPlan.rpDefault, cb = null, discarded = null) {
        return this.handler.EndTrans(rp, cb, discarded);
    }

    Open(conn, cb = null, discarded = null, flags = 0) {
        return this.handler.Open(conn, cb, discarded, flags);
    }

    Prepare(sql, cb = null, discarded = null) {
        return this.handler.Prepare(sql, cb, discarded);
    }

    Execute(sql_or_arrParam, cb = null, rows = null, meta = null, discarded = null) {
        return this.handler.Execute(sql_or_arrParam, cb, rows, meta, discarded);
    }

    ExecuteBatch(isolation, sql, paramBuff, cb = null, rows = null, meta = null, batchHeader = null, discarded = null, rp = exports.DB.RollbackPlan.rpDefault, delimiter = ';', arrP = []) {
        return this.handler.ExecuteBatch(isolation, sql, paramBuff, cb, rows, meta, batchHeader, discarded, rp, delimiter, arrP);
    }

    //Promise
    close(cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Close((errCode, errMsg, db) => {
                if (cb) ret = cb(errCode, errMsg, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'Close canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    prepare(sql, cb = null, discarded = null) {
        assert(sql && typeof sql === 'string');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Prepare(sql, (errCode, errMsg, db) => {
                if (cb) ret = cb(errCode, errMsg, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'Prepare canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    beginTrans(isolation = exports.DB.TransIsolation.ReadCommited, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.BeginTrans(isolation, (errCode, errMsg, db) => {
                if (cb) ret = cb(errCode, errMsg, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'BeginTrans canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    endTrans(rp = exports.DB.RollbackPlan.rpDefault, cb = null, discarded = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.EndTrans(rp, (errCode, errMsg, db) => {
                if (cb) ret = cb(errCode, errMsg, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'EndTrans canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    open(conn, cb = null, discarded = null) {
        assert(conn === null || conn === undefined || typeof conn === 'string');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Open(conn, (errCode, errMsg, db) => {
                if (cb) ret = cb(errCode, errMsg, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'Open canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    execute(sql_or_arrParam, rows = null, meta = null, cb = null, discarded = null) {
        assert(rows === null || rows === undefined || typeof rows === 'function');
        assert(meta === null || meta === undefined || typeof meta === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Execute(sql_or_arrParam, (errCode, errMsg, affected, fails, oks, id, db) => {
                if (cb) ret = cb(errCode, errMsg, affected, fails, oks, id, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg,
                    aff: affected,
                    oks: oks,
                    fails: fails,
                    lastId: id
                };
                res(ret);
            }, rows, meta, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'Execute canceled' : 'Connection closed');
                rej(ret);
            });
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }

    //Promise
    executeBatch(isolation, sql, paramBuff, rows = null, meta = null, rp = exports.DB.RollbackPlan.rpDefault, delimiter = ';', batchHeader = null, cb = null, discarded = null, arrP = []) {
        assert(sql && typeof sql === 'string');
        assert(rows === null || rows === undefined || typeof rows === 'function');
        assert(meta === null || meta === undefined || typeof meta === 'function');
        assert(batchHeader === null || batchHeader === undefined || typeof batchHeader === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.ExecuteBatch(isolation, sql, paramBuff, (errCode, errMsg, affected, fails, oks, id, db) => {
                if (cb) ret = cb(errCode, errMsg, affected, fails, oks, id, db);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg,
                    aff: affected,
                    oks: oks,
                    fails: fails,
                    lastId: id
                };
                res(ret);
            }, rows, meta, batchHeader, (canceled, db) => {
                if (discarded) ret = discarded(canceled, db);
                if (ret === undefined) ret = (canceled ? 'ExecuteBatch canceled' : 'Connection closed');
                rej(ret);
            }, rp, delimiter, arrP);
            if (!ok) {
                if (discarded) ret = discarded(false, this.handler);
                if (ret === undefined) ret = this.Socket.Error;
                rej(ret);
            }
        });
    }
}

//EM == EncryptionMethod
exports.EM = {
    NoEncryption: 0,
    TLSv1: 1
};

//socket shutdown type
exports.ShutdownType = {
    receive: 0,
    send: 1,
    both: 2
};

//compression options
exports.ZipLevel = {
    zlDefault: 0,
    zlBestSpeed: 1,
    zlBestCompression: 2
};

exports.OperationSystem = {
    osWin: 0,
    osApple: 1,
    osMac: 1,
    osIphone: 1,
    osUnix: 2,
    osLinux: 2,
    osBSD: 2,
    osAndroid: 3,
    osWinCE: 4,
    /**< Old window pocket pc, ce or smart phone devices*/
    osWinPhone: 4
};

//CS == Client side namespace
exports.CS = {
    get Version() {
        return SPA.getVersion(); //client core library version string, a static function
    },
    get Pools() {
        //return number of socket pools created, a static function
        return SPA.getPools();
    },
    //wrap native handler into accessable handler
    wrap: function(h) {
        if (h) {
            switch (h.getSvsId()) {
                case exports.SID.sidFile:
                    h = new CAsyncFile(h);
                    break;
                case exports.SID.sidQueue:
                    h = new CAsyncQueue(h);
                    break;
                case exports.SID.sidODBC:
                case exports.SID.sidSqlite:
                case exports.SID.sidMysql:
                    h = new CDb(h);
                    break;
                default:
                    h = new CHandler(h);
                    break;
            }
        }
        return h;
    },

    //SSL/TLS server certificate authentication
    TLS: {
        set CA(caPath) {
            //set SSL/TLS CA certification store, a static function
            //return true if successful; Otherwise, false.

            //it works only on Linux to a pem file through openssl
            return SPA.setCA(caPath);
        },

        //authenticate server certificate by a public key (an array of bytes) in case certificate chain verification failed
        set Key(pk) {
            return SPA.setKey(pk);
        }
    },

    //client persistent message queue
    Queue: {
        get WorkingDir() {
            //find current working directory, a static function
            return SPA.getWorkingDir();
        },
        set WorkingDir(dir) {
            //set current working directory, a static function
            SPA.setWorkingDir(dir);
        },
        set Pwd(pwd) {
            //set a password to protect client message queue, a static function 
            SPA.setPassword(pwd);
        },

        //queue flush options for server persistent queue, CAsyncQueue
        Optimistic: {
            oMemoryCached: 0,
            oSystemMemoryCached: 1,
            oDiskCommitted: 2
        },

        //reserved persistent queue request IDs
        ReqIds: {
            idEnqueue: 0x2001 + 1,
            idDequeue: 0x2001 + 2,
            idStartTrans: 0x2001 + 3,
            idEndTrans: 0x2001 + 4,
            idFlush: 0x2001 + 5,
            idClose: 0x2001 + 6,
            idGetKeys: 0x2001 + 7,
            idEnqueueBatch: 0x2001 + 8,
            idBatchSizeNotified: 0x2001 + 20
        },

        //possible error codes from server persistent queue, which are used within CAsyncQueue
        ErrorCode: {
            OK: 0,
            TRANS_ALREADY_STARTED: 1,
            TRANS_STARTING_FAILED: 2,
            TRANS_NOT_STARTED_YET: 3,
            TRANS_COMMITTING_FAILED: 4,
            DEQUEUING: 5,
            OTHER_WORKING_WITH_SAME_QUEUE: 6,
            CLOSE_FAILED: 7,
            ENQUEUING_FAILED: 8
        },

        //persistent message queue status used within CClientQueue
        Status: {
            /// <summary>
            /// everything is fine
            /// </summary>
            qsNormal: 0,

            /// <summary>
            /// Queued messages merged completely
            /// </summary>
            qsMergeComplete: 1,

            /// <summary>
            /// Message replication started but not completed yet
            /// </summary>
            qsMergePushing: 2,

            /// <summary>
            /// Message replicated incompletely from a source queue
            /// </summary>
            qsMergeIncomplete: 3,

            /// <summary>
            /// A set of messages as a job are incompletely queued 
            /// </summary>
            qsJobIncomplete: 4,

            /// <summary>
            /// A message queued incompletely because of application crash or unexpected termination
            /// </summary>
            qsCrash: 5,

            /// <summary>
            /// Queue file open error
            /// </summary>
            qsFileError: 6,

            /// <summary>
            /// Queue file opened but can not decrypt existing queued messages beacuse of bad password found
            /// </summary>
            qsBadPassword: 7,

            /// <summary>
            /// Duplicate name error
            /// </summary>
            qsDuplicateName: 8
        }
    },

    newPool: function(svsId, defaulDb = '') {
        //create a regular socket or master/slave pool.
        //you can create multiple pools for different services
        var pool = new SPA.CSocketPool(svsId /*a required unsigned int service id*/ , defaulDb /*master/slave with real-time updateable cache*/ );
        if (pool)
            return new CSocketPool(pool);
        return pool;
    },
    //CC == Connection Context
    newCC: function(host, port, userId, pwd, em = 0, zip = false, v6 = false, anyData = null) {
        return {
            Host: host,
            Port: port,
            User: userId,
            Pwd: pwd,
            EM: em,
            Zip: zip,
            V6: v6,
            AnyData: anyData
        };
    },
    //Socket Connection State
    ConnState: {
        csClosed: 0,
        csConnecting: 1,
        csSslShaking: 2,
        csClosing: 3,
        csConnected: 4,
        csSwitched: 5
    },

    //Socket Pool Events
    PoolEvent: {
        speUnknown: -1,
        speStarted: 0,
        speCreatingThread: 1,
        speThreadCreated: 2,
        speConnecting: 3,
        speConnected: 4,
        speKillingThread: 5,
        speShutdown: 6,
        speUSocketCreated: 7,
        speHandShakeCompleted: 8,
        speLocked: 9,
        speUnlocked: 10,
        speThreadKilled: 11,
        speClosingSocket: 12,
        speSocketClosed: 13,
        speUSocketKilled: 14,
        speTimer: 15,
        speQueueMergedFrom: 16,
        speQueueMergedTo: 17,
    }
};

//DB namespace
exports.DB = {
    //pre-defined DB management systems
    ManagementSystem: {
        Unknown: -1,
        Sqlite: 0,
        Mysql: 1,
        ODBC: 2,
        MsSQL: 3,
        Oracle: 4,
        DB2: 5,
        PostgreSQL: 6,
        MongoDB: 7
    },

    //rollback hints defined for ending a manual transaction asynchronously
    RollbackPlan: {
        /// <summary>
        /// Manual transaction will rollback whenever there is an error by default
        /// </summary>
        rpDefault: 0,

        /// <summary>
        /// Manual transaction will rollback whenever there is an error by default
        /// </summary>
        rpErrorAny: 0,

        /// <summary>
        /// Manual transaction will rollback as long as the number of errors is less than the number of ok processing statements
        /// </summary>
        rpErrorLess: 1,

        /// <summary>
        /// Manual transaction will rollback as long as the number of errors is less or equal than the number of ok processing statements
        /// </summary>
        rpErrorEqual: 2,

        /// <summary>
        /// Manual transaction will rollback as long as the number of errors is more than the number of ok processing statements
        /// </summary>
        rpErrorMore: 3,

        /// <summary>
        /// Manual transaction will rollback only if all the processing statements are failed
        /// </summary>
        rpErrorAll: 4,

        /// <summary>
        /// Manual transaction will rollback always no matter what happens.
        /// </summary>
        rpAlways: 5
    },

    //DB transaction isolation levels
    TransIsolation: {
        Unspecified: -1,
        Chaos: 0,
        ReadUncommited: 1,
        Browse: 2,
        CursorStability: 3,
        ReadCommited: 3,
        RepeatableRead: 4,
        Serializable: 5,
        Isolated: 6
    },

    //reserved DB operational request IDs
    ReqIds: {
        /// <summary>
        /// Async database client/server just requires the following request identification numbers 
        /// </summary>
        idOpen: 0x7E7F,
        idClose: 0x7E80,
        idBeginTrans: 0x7E81,
        idEndTrans: 0x7E82,
        idExecute: 0x7E83,
        idPrepare: 0x7E84,
        idExecuteParameters: 0x7E85,

        /// <summary>
        /// the request identification numbers used for message push from server to client
        /// </summary>
        idDBUpdate: 0x7E86, //server ==> client only
        idRowsetHeader: 0x7E87, //server ==> client only
        idOutputParameter: 0x7E88, //server ==> client only

        /// <summary>
        /// Internal request/response identification numbers used for data communication between client and server
        /// </summary>
        idBeginRows: 0x7E89,
        idTransferring: 0x7E8A,
        idStartBLOB: 0x7E8B,
        idChunk: 0x7E8C,
        idEndBLOB: 0x7E8D,
        idEndRows: 0x7E8E,
        idCallReturn: 0x7E8F, //server ==> client only

        idGetCachedTables: 0x7E90,

        idSqlBatchHeader: 0x7E91, //server ==> client only
        idExecuteBatch: 0x7E92,
        idParameterPosition: 0x7E93 //server ==> client only
    }
};

//real-time updateable data cache
exports.Cache = {
    /// <summary>
    /// A flag used with idOpen for tracing database table update events
    /// </summary>
    ENABLE_TABLE_UPDATE_MESSAGES: 1,

    /// <summary>
    /// A chat group id used at SocketPro server side for notifying database events from server to connected clients
    /// </summary>
    STREAMING_SQL_CHAT_GROUP_ID: 0x1fffffff,

    CACHE_UPDATE_CHAT_GROUP_ID: 0x20000000,

    //operators used within CCache and CTable
    Op: {
        eq: 0, //equal
        gt: 1, //great
        lt: 2, //less
        ge: 3, //great_equal
        le: 4, //less_equal
        ne: 5, //not_equal
        is_null: 6 //is_null
    }
};
