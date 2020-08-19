"use strict";

var SPA = require('njadapter');
var assert = require('assert');
const os = require('os');

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

    get PoolId() {
        return this.socket.getPoolId();
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

    get AutoConn() {
        return this.pool.getAutoConn();
    }

    set AutoConn(ac = true) {
        this.pool.setAutoConn(ac);
    }

    get RecvTimeout() {
        return this.pool.getRecvTimeout();
    }

    set RecvTimeout(timeout = 30000) {
        this.pool.setRecvTimeout(timeout);
    }

    get ConnTimeout() {
        return this.pool.getConnTimeout();
    }

    set ConnTimeout(timeout = 30000) {
        this.pool.setConnTimeout(timeout);
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
                    case exports.SID.sidOdbc:
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
                case exports.SID.sidOdbc:
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
                case exports.SID.sidOdbc:
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

    /**
     * A property {int} A service id
     */
    get SvsId() {
        return this.handler.getSvsId();
    }

    /**
     * A property {int} the number of requests queued
     */
    get RequestsQueued() {
        return this.handler.getRequestsQueued();
    }

    /**
     * A property {boolean} true if requests are beening batched
     */
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
    /**
     * Check if two handlers are the same
     * @param {CHandler} h An async request handler
     * @returns true if they are the same, and false if they are not the same
     */
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

    /**
     * Start to batch a number of requests manually
     * @returns true if successful, and false if failed
    */
    StartBatching() {
        return this.handler.StartBatching();
    }

    /**
     * Abort requests batched
     */
    AbortBatching() {
        return this.handler.AbortBatching();
    }

    /**
     * Commit batching a number of requests and send them to server in one single shot
     * @param {boolean} serverCommit true if returning results will be returned from server in batch, and false if not
     * @returns true if successful, and false if failed
     */
    CommitBatching(serverCommit = false) {
        return this.handler.CommitBatching(serverCommit);
    }

    Dispose() {
        return this.handler.Dispose();
    }

    /**
     * Send an interrupt request onto server with the fastest priority
     * @param {bigint} options bit-wise flags to be sent to server
     * @returns true if successful, and false if failed
     * @remarks The request is used to tell server side to stop loop like file file downloading and records fetching as soon as possible if implemented
     */
    Interrupt(options) {
        assert(Number.isSafeInteger(options));
        return this.handler.Interrupt(options);
    }

    /**
     *  Send a request onto a remote server for processing, and return immediately without blocking
     * @param {short} reqId An unique request id within a service handler
     * @param {CUQueue or null} buff null or an instance of CUQueue or None
     * @param {function} A callback for tracking an instance of CUQueue containing an expected result
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if successful, and false if communication channel is not sendable
     */
    SendRequest(reqId, buff, cb, discarded = null, serverException = null) {
        return this.handler.SendRequest(reqId, buff, cb, discarded, serverException);
    }

    //Promise version

    /**
     *  Send a request onto a remote server for processing, and return immediately without blocking
     * @param {short} reqId An unique request id within a service handler
     * @param {CUQueue or null} buff null or an instance of CUQueue or None
     * @param {function} A callback for tracking an instance of CUQueue containing an expected result
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns A promise
     * @throws A server or socket close exception
     */
    sendRequest(reqId, buff, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.SendRequest(reqId, buff, (q, id) => {
                var ret;
                if (cb) ret = cb(q, id);
                if (ret === undefined) ret = q;
                res(ret);
            }, (canceled, id) => {
                this.set_aborted(rej, 'SendRequest', reqId, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'SendRequest', reqId, discarded);
            }
        });
    }

    /**
     * set a server exception for promise reject
     * @param {reject} rej promise reject function
     * @param {string} errMsg an error message
     * @param {int} errCode an error code
     * @param {string} errWhere location where exception happens at server side
     * @param {short} id an unique request id within a service handler
     * @param {function} serverException an optional callback for tracking an exception from server
     */
    set_exception(rej, errMsg, errCode, errWhere, id, serverException) {
        var ret;
        if (serverException) ret = serverException(errMsg, errCode, errWhere, id);
        if (ret === undefined) ret = {
            ec: errCode,
            em: errMsg,
            where: errWhere,
            reqId: id
        };
        rej(ret);
    }

    /**
     * set a cancel or communication channel close exception for promise reject
     * @param {reject} rej promise reject function
     * @param {string} method_name a request method name
     * @param {short} req_id an unique request id within a service handler
     * @param {boolean} canceled true if the request is canceled, false if communication channel is closed
     * @param {function} discarded an optinal callback for tracking communication channel events, close and cancel
     */
    set_aborted(rej, method_name, req_id, canceled, discarded = null) {
        var ret;
        if (discarded) ret = discarded(canceled, req_id, method_name);
        if (ret === undefined) {
            if (canceled) {
                ret = { ec: -1002, em: 'Request ' + method_name + ' canceled', reqId: req_id, before: false };
            }
            else {
                var error = this.Socket.Error;
                if (!error.ec) {
                    error.ec = -1000;
                    error.em = 'Session closed after sending the request ' + method_name;
                }
                ret = { ec: error.ec, em: error.em, reqId: req_id, before: false };
            }
        }
        rej(ret);
    }

    /**
     * raise a communication channel close exception for promise reject
     * @param {reject} rej promise reject function
     * @param {string} method_name a request method name
     * @param {short} req_id An unique request id within a service handler
     * @param {function} discarded An optinal callback for tracking communication channel events, close and cancel
     */
    raise(rej, method_name, req_id, discarded = null) {
        var ret;
        if (discarded) ret = discarded(false, req_id, method_name);
        if (ret === undefined) {
            var error = this.Socket.Error;
            if (!error.ec) {
                error.ec = -1001;
                error.em = 'Session already closed before sending the request ' + method_name;
            }
            ret = { ec: error.ec, em: error.em, reqId: req_id, before: true };
        }
        rej(ret);
    }
}

exports.newBuffer = function (initSize = 4096, blockSize = 4096) {
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
    idStartMerge: 12,
    idEndMerge: 13,
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
    idDequeueBatchConfirmed: 28,
    idInterrupt: 29,
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
    sidQueue: 257, //server persistent message queue service
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
        assert(h.getSvsId() === exports.SID.sidQueue);
    }

    /**
     * A property{int} Batch dequeue size in bytes
     */
    get DequeueBatchSize() {
        return this.handler.getDeqBatchSize();
    }
    /**
     * A property{boolean} true if server is set to notify if there is a message available. Otherwise, it is false
     */
    get EnqueueNotified() {
        return this.handler.getEnqNotified();
    }

    /**
     * A write-only property{function} A callback for tracking dequeued messages or other returning results
     */
    set ResultReturned(rr) {
        return this.handler.setResultReturned(rr);
    }

    /**
     * Pack a message into the handler internal buffer to prepare for coming EqueueBatch or enqueueBatch
     * @param {any} reqId A request id for the message
     * @param {any} buff A instance of CUQueue containing a message, null or undefined
     */
    BatchMessage(reqId, buff) {
        this.handler.BatchMessage(reqId, buff);
    }

    /**
     * Send a request to server for querying all opened keys that are corresponding server queue files
     * @param {function} cb A callback for tracking an array of returning keys
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if succsessful, and false if communication channel is closed or not sendable
     */
    GetKeys(cb, discarded = null, serverException = null) {
        return this.handler.GetKeys(cb, discarded, serverException);
    }

    /**
     * Start enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
     * @param {string} key An ASCII string to identify a server queue file
     * @param {function} cb A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    StartTrans(key, cb = null, discarded = null, serverException = null) {
        return this.handler.StartTrans(key, cb, discarded, serverException);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
     * @param {boolean} rollback true for rollback, and false for committing
     * @param {function} cb A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    EndTrans(rollback = false, cb = null, discarded = null, serverException = null) {
        return this.handler.EndTrans(rollback, cb, discarded, serverException);
    }

    /**
     * Try to close or delete a persistent queue opened at server side
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {function} cb A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {boolean} permanent true for deleting a queue file, and false for closing a queue file
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    Close(key, cb = null, discarded = null, permanent = false, serverException = null) {
        return this.handler.Close(key, cb, discarded, permanent, serverException);
    }

    /**
     * Flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {function} cb A callback for tracking returning message count and queue file size in bytes
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {int} option one of tagOptimistic options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    Flush(key, cb = null, discarded = null, option = exports.CS.Queue.Optimistic.oMemoryCached, serverException = null) {
        return this.handler.Flush(key, cb, discarded, option, serverException);
    }

    /**
     * Dequeue messages from a persistent message queue file at server side in batch
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {function} cb A callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {int} timeout A time-out number in milliseconds
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    Dequeue(key, cb = null, discarded = null, timeout = 0, serverException = null) {
        return this.handler.Dequeue(key, cb, discarded, timeout, serverException);
    }

    /**
     *  Enqueue a message into a queue file identified by a key
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {short} reqId  A unsigned short number to identify a message
     * @param {CUQueue} buff an instance of SPA.CUQueue containing a message, null or undefined
     * @param {function} cb A callback for tracking returning index
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    Enqueue(key, reqId, buff, cb = null, discarded = null, serverException = null) {
        return this.handler.Enqueue(key, reqId, buff, cb, discarded, serverException);
    }

    /**
     * Enqueue a batch of messages into a queue file identified by a key
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {function} cb  A callback for tracking returning index
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {function} serverException A callback for tracking an exception from server
     * @returns true if communication channel is sendable, and false if communication channel is not sendable
     */
    EnqueueBatch(key, cb = null, discarded = null, serverException = null) {
        return this.handler.EnqueueBatch(key, cb, discarded, serverException);
    }

    //Promise
    /**
     * Query queue keys opened at server side
     * @param {function} cb An optinal callback for tracking an array of returning keys
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns An array of string keys by promise
     */
    getKeys(cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.GetKeys((vKeys) => {
                var ret;
                if (cb) ret = cb(vKeys);
                if (ret === undefined) ret = vKeys;
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'GetKeys', exports.CS.Queue.ReqIds.idGetKeys, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'GetKeys', exports.CS.Queue.ReqIds.idGetKeys, discarded);
            }
        });
    }

    //Promise
    /**
     * Try to close a persistent queue opened at server side
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {boolean} permanent true for deleting a queue file, and false for closing a queue file.
     * @param {function} cb An optional callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns An error code by promise, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
     */
    close(key, permanent = false, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.Close(key, (errCode) => {
                var ret;
                if (cb) ret = cb(errCode);
                if (ret === undefined) ret = errCode;
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Close', exports.CS.Queue.ReqIds.idClose, canceled, discarded);
            }, permanent, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Close', exports.CS.Queue.ReqIds.idClose, discarded);
            }
        });
    }

    //Promise
    /**
     * Start to enqueue messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {function} cb An optional callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns An error code by promise, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     */
    startTrans(key, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.StartTrans(key, (errCode) => {
                var ret;
                if (cb) ret = cb(errCode);
                if (ret === undefined) ret = errCode;
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'StartTrans', exports.CS.Queue.ReqIds.idStartTrans, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'StartTrans', exports.CS.Queue.ReqIds.idStartTrans, discarded);
            }
        });
    }

    //Promise
    /**
     * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
     * @param {boolean} rollback true for rollback, and false for committing.
     * @param {function} cb An optional callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns An error code by promise, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     */
    endTrans(rollback = false, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.EndTrans(rollback, (errCode) => {
                var ret;
                if (cb) ret = cb(errCode);
                if (ret === undefined) ret = errCode;
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'EndTrans', exports.CS.Queue.ReqIds.idEndTrans, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'EndTrans', exports.CS.Queue.ReqIds.idEndTrans, discarded);
            }
        });
    }

    //Promise
    /**
     * Just get message count and queue file size in bytes only
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {int} option one of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
     * @param {function} cb An optional callback for tracking returning message count and queue file size in bytes
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns message count and queue file size in bytes by promise
     */
    flush(key, option = exports.CS.Queue.Optimistic.oMemoryCached, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.Flush(key, (messages, fileSize) => {
                var ret;
                if (cb) ret = cb(messages, fileSize);
                if (ret === undefined) ret = {
                    msgs: messages,
                    fsize: fileSize
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Flush', exports.CS.Queue.ReqIds.idFlush, canceled, discarded);
            }, option, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Flush', exports.CS.Queue.ReqIds.idFlush, discarded);
            }
        });
    }

    //Promise disabled because it is not useful at current time
    /**
     * Enqueue a message to a server queue file
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {short} reqId A request id for a message
     * @param {CUQueue} buff An instance of CUQueue containing the message, null or undefined
     * @param {bigint} cb An optional callback for tracking returning index
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns An index by promise
     */
    /*
    enqueue(key, reqId, buff, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.Enqueue(key, reqId, buff, (index) => {
                var ret;
                if (cb) ret = cb(index);
                if (ret === undefined) ret = index;
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Enqueue', exports.CS.Queue.ReqIds.idEnqueue, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Enqueue', exports.CS.Queue.ReqIds.idEnqueue, discarded);
            }
        });
    }
    */

    //Promise
    /**
     * Enqueue a batch of messages into a server queue file
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {bigint} cb An optional callback for tracking returning index
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns An index by promise
     */
    enqueueBatch(key, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.EnqueueBatch(key, (index) => {
                var ret;
                if (cb) ret = cb(index);
                if (ret === undefined) ret = index;
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'EnqueueBatch', exports.CS.Queue.ReqIds.idEnqueueBatch, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'EnqueueBatch', exports.CS.Queue.ReqIds.idEnqueueBatch, discarded);
            }
        });
    }

    //Promise
    /**
     * 
     * @param {string} key An ASCII string for identifying a queue at server side
     * @param {int} timeout A time-out number in millseconds
     * @param {function} cb An optional callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns Remaining message count within a server queue file, queue file size in bytes, messages dequeued and bytes dequeued within this batch by promise
     */
    dequeue(key, timeout = 0, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.Dequeue(key, (messages, fileSize, messagesDequeued, bytes) => {
                var ret;
                if (cb) ret = cb(messages, fileSize, messagesDequeued, bytes);
                if (ret === undefined) ret = {
                    msgs: messages,
                    fsize: fileSize,
                    msgsDequeued: messagesDequeued,
                    bytes: bytes
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Dequeue', exports.CS.Queue.ReqIds.idDequeue, canceled, discarded);
            }, timeout, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Dequeue', exports.CS.Queue.ReqIds.idDequeue, discarded);
            }
        });
    }
}

class CAsyncFile extends CHandler {
    constructor(h) {
        super(h);
        assert(h.getSvsId() === exports.SID.sidFile);
    }

    /**
     * A property{int} the number of files queued for transferring
     */
    get FilesQueued() {
        return this.handler.getFilesQueued();
    }

    /**
     * A property{int} the number of files streamed at max. It defaults to 1
     */
    get FilesStreamed() {
        return this.handler.getFilesStreamed();
    }

    /**
     * A property{int} set a max number of  files streamed. The max number could be 32 at hard-coded
     * @remarks Increasing the property could speed up files downloading if there are a lot of small files to be downloaded from remote server
     */
    set FilesStreamed(max) {
        return this.handler.setFilesStreamed(max);
    }

    /**
     * Post a message for uploading a file onto a remote server
     * @param {string} localFile A non-empty string path to a local file
     * @param {string} remoteFile A non-empty string path to a remote file at server side
     * @param {function} cb A callback for tracking the final result of file uploading containing an error code and an error message
     * @param {function} progress A callback for file uploading progress
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {int} flags A bit-wsie int flags for opening file options. It could be one or more of these options: exports.File.OpenOption.TRUNCACTED, APPENDED, SHARE_READ and SHARE_WRITE
     * @param {function} serverException A callback for tracking an exception from server
     * @returns It always returns true
     */
    Upload(localFile, remoteFile, cb = null, progress = null, discarded = null, flags = exports.File.OpenOption.TRUNCACTED, serverException = null) {
        return this.handler.Upload(localFile, remoteFile, cb, progress, discarded, flags, serverException);
    }

    /**
     * Post a message for downloading a file from a remote server
     * @param {string} localFile A non-empty string path to a local file
     * @param {string} remoteFile A non-empty string path to a remote file at server side
     * @param {function} cb A callback for tracking the final result of file downloading containing an error code and an error message
     * @param {function} progress A callback for file downloading progress
     * @param {function} discarded A callback for tracking communication channel events, close and cancel
     * @param {int} flags A bit-wsie int flags for opening file options. It could be one or more of these options: exports.File.OpenOption.TRUNCACTED, APPENDED, SHARE_READ and SHARE_WRITE
     * @param {function} serverException A callback for tracking an exception from server
     * @returns It always returns true
     */
    Download(localFile, remoteFile, cb = null, progress = null, discarded = null, flags = exports.File.OpenOption.TRUNCACTED, serverException = null) {
        return this.handler.Download(localFile, remoteFile, cb, progress, discarded, flags, serverException);
    }

    /**
     * Cancel file transferrings queued
     * @returns the number of file transferrings canceled
     */
    Cancel() {
        return this.handler.Cancel();
    }

    //Promise version
    /**
     * Post a message for uploading a file onto a remote server
     * @param {string} localFile A non-empty string path to a local file
     * @param {string} remoteFile A non-empty string path to a remote file at server side
     * @param {function} progress A callback for file uploading progress
     * @param {int} flags A bit-wsie int flags for opening file options. It could be one or more of these options: exports.File.OpenOption.TRUNCACTED, APPENDED, SHARE_READ and SHARE_WRITE
     * @param {function} cb An optional callback for tracking the final result of file uploading containing an error code and an error message
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns A structure for final result of file uploading containing an error code and an error message by promise
     */
    upload(localFile, remoteFile, progress = null, flags = exports.File.OpenOption.TRUNCACTED, cb = null, discarded = null, serverException = null) {
        assert(localFile && typeof localFile === 'string');
        assert(remoteFile && typeof remoteFile === 'string');
        assert(progress === null || progress === undefined || typeof progress === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            this.handler.Upload(localFile, remoteFile, (errMsg, errCode, download) => {
                var ret;
                if (cb) ret = cb(errMsg, errCode, download);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, progress, (canceled, download) => {
                this.set_aborted(rej, 'Upload', exports.File.ReqIds.idUpload, canceled, discarded);
            }, flags, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
        });
    }

    //Promise version
    /**
     * Post a message for downloading a file from a remote server
     * @param {string} localFile A non-empty string path to a local file
     * @param {string} remoteFile A non-empty string path to a remote file at server side
     * @param {function} progress A callback for file downloading progress
     * @param {int} flags A bit-wsie int flags for opening file options. It could be one or more of these options: exports.File.OpenOption.TRUNCACTED, APPENDED, SHARE_READ and SHARE_WRITE
     * @param {function} cb An optional callback for tracking the final result of file uploading containing an error code and an error message
     * @param {function} discarded An optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException An optional callback for tracking an exception from server
     * @returns A structure for final result of file downloading containing an error code and an error message by promise
     */
    download(localFile, remoteFile, progress = null, flags = exports.File.OpenOption.TRUNCACTED, cb = null, discarded = null, serverException = null) {
        assert(localFile && typeof localFile === 'string');
        assert(remoteFile && typeof remoteFile === 'string');
        assert(progress === null || progress === undefined || typeof progress === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            this.handler.Download(localFile, remoteFile, (errMsg, errCode, download) => {
                var ret;
                if (cb) ret = cb(errMsg, errCode, download);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, progress, (canceled, download) => {
                this.set_aborted(rej, 'Download', exports.File.ReqIds.idDownload, canceled, discarded);
            }, flags, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
        });
    }
}

class CDb extends CHandler {
    constructor(h) {
        super(h);
        var sid = h.getSvsId();
        assert(sid === exports.SID.sidOdbc || sid === exports.SID.sidSqlite || sid === exports.SID.sidMysql);
    }

    /**
     * A property{int} A value for database management system. one of exports.DB.ManagementSystem.Sqlite, Mysql, ODBC, MsSQL, Oracle, DB2, PostgreSQL, and MongoDB
     */
    get DbMS() {
        return this.handler.getDbMS();
    }

    /**
     * A property{boolean} true if database is connected, and false if database is not opened
     */
    get Opened() {
        return this.handler.isOpened();
    }

    /**
     * Start a manual transaction with a given isolation asynchronously. Note
     * the transaction will be associated with SocketPro client message queue if
     * available to avoid possible transaction lose
     * 
     * @param {int} isolation a transaction isolation. It defaults to exports.DB.tagTransactionIsolation.tiReadCommited
     * @param {function} cb a callback for tracking its response result
     * @param {function} discarded an callback for tracking communication channel events, close and cancel
     * @param {function} serverException a callback for tracking an exception from server
     * @returns true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
     */
    BeginTrans(isolation = exports.DB.TransIsolation.ReadCommited, cb = null, discarded = null, serverException = null) {
        return this.handler.BeginTrans(isolation, cb, discarded, serverException);
    }

    /**
     * Notify connected remote server to close database connection string
     * asynchronously
     * @param {function} cb a callback for closing result, which should be OK always as long as there is network or queue available
     * @param {function} discarded a callback for tracking communication channel events, close and cancel
     * @param {function} serverException a callback for tracking an exception from server
     * @returns true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
     */
    Close(cb = null, discarded = null, serverException = null) {
        return this.handler.Close(cb, discarded, serverException);
    }

    /**
     * End a manual transaction with a given rollback plan. Note the transaction
     * will be associated with SocketPro client message queue if available to
     * avoid possible transaction lose
     * @param {int} rp plan a schedule about how included transactions should be rollback
     * at server side. It defaults to exports.DB.tagRollbackPlan.rpDefault
     * @param {function} cb a callback for tracking its response result
     * @param {function} discarded an callback for tracking communication channel events, close and cancel
     * @param {function} serverException a callback for tracking an exception from server
     * @returns true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
     */
    EndTrans(rp = exports.DB.RollbackPlan.rpDefault, cb = null, discarded = null, serverException = null) {
        return this.handler.EndTrans(rp, cb, discarded, serverException);
    }

    /**
     * Open a database connection at server side asynchronously
     * @param {string} conn a database connection string. The database
     * connection string can be null, undefined or an empty string if its server side supports
     * global database connection string
     * @param {function} cb a callback for database connecting result
     * @param {function} discarded a callback for tracking communication channel events, close and cancel
     * @param {int} flags a set of flags transferred to server to indicate how to
     * build database connection at server side. It defaults to zero
     * @param {function} serverException a callback for tracking an exception from server
     */
    Open(conn, cb = null, discarded = null, flags = 0, serverException = null) {
        return this.handler.Open(conn, cb, discarded, flags, serverException);
    }

    /**
     * Send a parameterized SQL statement for preparing with a given array of
     * parameter informations asynchronously
     * @param {string} sql a parameterized SQL statement
     * @param {function} cb a callback for SQL preparing result
     * @param {function} discarded a callback for tracking communication channel events, close and cancel
     * @param {a[]} arrP a given array of parameter informations. It defaults to an empty array
     * @param {function} serverException a callback for tracking an exception from server
     * @returns true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    Prepare(sql, cb = null, discarded = null, arrP = [], serverException = null) {
        return this.handler.Prepare(sql, cb, discarded, arrP, serverException);
    }

    /**
     * Process a complex SQL statement which may be combined with multiple basic
     * SQL statements asynchronously
     * @param {string, [] or CUQueue} sql_or_arrParam an SQL statement, an array of parameter data, or an instance of CUQueue containing an array of parameters
     * @param {function} cb a callback for tracking final result
     * @param {function} rows a callback for tracking record or output parameter returned data
     * @param {function} rh a callback for tracking row set of header column meta informations
     * @param {boolean} meta a boolean value for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults to true
     * @param {function} discarded a callback for tracking communication channel events, close and cancel
     *  @param {function} serverException a callback for tracking an exception from server
     * @returns true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
     */
    Execute(sql_or_arrParam, cb = null, rows = null, rh = null, meta = true, discarded = null, serverException = null) {
        return this.handler.Execute(sql_or_arrParam, cb, rows, rh, discarded, meta, serverException);
    }

    /**
     * Execute a batch of SQL statements on one single call
     * @param {int} isolation a value for manual transaction isolation. Specifically,
     * there is no manual transaction around the batch SQL statements if it is tiUnspecified
     * @param {string} sql a SQL statement having a batch of individual SQL statements
     * @param {[]} paramBuff an array of parameter data which will be bounded to previously prepared parameters.
     * The array size can be 0 if the given batch SQL statement doesn't having any prepared statement
     * @param {function} cb a callback for tracking final result
     * @param {function} rows a callback for receiving records of data
     * @param {function} rh a callback for tracking row set of header column meta informations
     * @param {string} delimiter a delimiter string used for separating the batch SQL statements into individual SQL statements at server side
     * for processing. It defaults to ";"
     * @param {function} batchHeader a callback for tracking returning batch start error messages
     * @param {function} discarded a callback for tracking communication channel events, close and cancel
     * @param {boolean} meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true
     * @param {int} rp a value for computing how included transactions should be rollback. It defaults to exports.DB.tagRollbackPlan.rpDefault
     * @param {[]} arrP a given array of parameter informations which may be empty to most of database management systems
     * @param {boolean} lastInsertId true if last insert id is hoped, and false if last insert id is not expected
     * @param {function} serverException a callback for tracking an exception from server
     * @returns true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
     */
    ExecuteBatch(isolation, sql, paramBuff, cb = null, rows = null, rh = null, delimiter = ';', batchHeader = null, discarded = null, meta = true, rp = exports.DB.RollbackPlan.rpDefault, arrP = [], lastInsertId = true, serverException = null) {
        return this.handler.ExecuteBatch(isolation, sql, paramBuff, cb, rows, rh, batchHeader, discarded, rp, delimiter, arrP, meta, lastInsertId, serverException);
    }

    //Promise
    /**
     * Notify connected remote server to close database connection string asynchronously
     * @param {function} cb an optional callback for tracking returning result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns a future for execution error information
     */
    close(cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.Close((errCode, errMsg) => {
                var ret;
                if (cb) ret = cb(errCode, errMsg);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Close', exports.DB.ReqIds.idClose, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Close', exports.DB.ReqIds.idClose, discarded);
            }
        });
    }

    //Promise
    /**
     * Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
     * @param {string} sql a parameterized SQL statement
     * @param {[]} arrP a given array of parameter informations. It defaults to empty array
     * @param {function} cb an optional callback for tracking returning result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns a future for execution error information
     */
    prepare(sql, arrP = [], cb = null, discarded = null, serverException = null) {
        assert(sql && typeof sql === 'string');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ret;
            var ok = this.handler.Prepare(sql, (errCode, errMsg) => {
                if (cb) ret = cb(errCode, errMsg);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Prepare', exports.DB.ReqIds.idPrepare, canceled, discarded);
            }, arrP, (errMsg, errCode, errWhere, id) => {
                ithis.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Prepare', exports.DB.ReqIds.idPrepare, discarded);
            }
        });
    }

    //Promise
    /**
     * Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro
     * client message queue if available to avoid possible transaction lose
     * @param {int} isolation a transaction isolation. It defaults to exports.DB.TransactionIsolation.tiReadCommited
     * @param {function} cb an optional callback for tracking returning result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns a future for execution error information
     */
    beginTrans(isolation = exports.DB.TransIsolation.ReadCommited, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.BeginTrans(isolation, (errCode, errMsg) => {
                var ret;
                if (cb) ret = cb(errCode, errMsg);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'BeginTrans', exports.DB.ReqIds.idBeginTrans, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'BeginTrans', exports.DB.ReqIds.idBeginTrans, discarded);
            }
        });
    }

    //Promise
    /**
     * End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue
     * if available to avoid possible transaction lose
     * @param {int} rp a schedule about how included transactions should be rollback at server side. It defaults to exports.DB.RollbackPlan.rpDefault
     * @param {function} cb an optional callback for tracking returning result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns a future for execution error information
     */
    endTrans(rp = exports.DB.RollbackPlan.rpDefault, cb = null, discarded = null, serverException = null) {
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.EndTrans(rp, (errCode, errMsg) => {
                var ret;
                if (cb) ret = cb(errCode, errMsg);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'EndTrans', exports.DB.ReqIds.idEndTrans, canceled, discarded);
            }, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'EndTrans', exports.DB.ReqIds.idEndTrans, discarded);
            }
        });
    }

    //Promise
    /**
     * Open a database connection at server side asynchronously
     * @param {string} conn a database connection string. The database connection string can be null, undefined or an empty string
     * if its server side supports global database connection string
     * @param {int} flags a set of flags transferred to server to indicate how to build database connection at server side. It defaults to zero
     * @param {function} cb an optional callback for tracking returning result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns a future for execution error information
     */
    open(conn, flags = 0, cb = null, discarded = null, serverException = null) {
        assert(conn === null || conn === undefined || typeof conn === 'string');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.Open(conn, (errCode, errMsg) => {
                var ret;
                if (cb) ret = cb(errCode, errMsg);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg
                };
                res(ret);
            }, (canceled) => {
                this.set_aborted(rej, 'Open', exports.DB.ReqIds.idOpen, canceled, discarded);
            }, flags, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'Open', exports.DB.ReqIds.idOpen, discarded);
            }
        });
    }

    //Promise
    /**
     * Execute one SQL complex statement having multiple basic SQL statements or
     * one or more sets of prepared statements with an array of parameter data asynchronously
     * @param {string, [], or CUQueue} sql_or_arrParam an SQL statement, an array of parameter data or an instance of CUQueue containing an array of parameters
     * @param {function} rows a callback for tracking record data or output parameter returned data
     * @param {function} rh a callback for tracking row set of header column meta informations
     * @param {boolean} meta a boolean value for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults to true
     * @param {function} cb a callback for tracking SQL final result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns a future for SQL execution error information
     */
    execute(sql_or_arrParam, rows = null, rh = null, meta = true, cb = null, discarded = null, serverException = null) {
        assert(rows === null || rows === undefined || typeof rows === 'function');
        assert(rh === null || rh === undefined || typeof rh === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        var sql = (typeof sql_or_arrParam === 'string');
        return new Promise((res, rej) => {
            var ok = this.handler.Execute(sql_or_arrParam, (errCode, errMsg, affected, fails, oks, id) => {
                var ret;
                if (cb) ret = cb(errCode, errMsg, affected, fails, oks, id);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg,
                    aff: affected,
                    oks: oks,
                    fails: fails,
                    lastId: id
                };
                res(ret);
            }, rows, rh, (canceled) => {
                this.set_aborted(rej, sql ? 'ExecuteSQL' : 'ExecuteParameters', sql ? exports.DB.ReqIds.idExecute : exports.DB.ReqIds.idExecuteParameters, canceled, discarded);
            }, meta, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, sql ? 'ExecuteSQL' : 'ExecuteParameters', sql ? exports.DB.ReqIds.idExecute : exports.DB.ReqIds.idExecuteParameters, discarded);
            }
        });
    }

    //Promise
    /**
     * Execute a batch of SQL statements on one single call
     * @param {int} isolation a value for manual transaction isolation. Specifically,
     * there is no manual transaction around the batch SQL statements if it is exports.DB.TransIsolation.Unspecified
     * @param {string} sql a SQL statement having a batch of individual SQL statements
     * @param {[]} paramBuff an array of parameter data which will be bounded to previously prepared parameters.
     * The array size can be 0 if the given batch SQL statement doesn't having any prepared statement
     * @param {function} rows a callback for tracking final result
     * @param {function} rh a callback for tracking row set of header column informations
     * @param {string} delimiter delimiter a delimiter string used for separating the batch SQL statements into individual SQL statements
     * at server side for processing. It defaults to ";"
     * @param {function} batchHeader a callback for tracking batch start event
     * @param {boolean} meta a boolean value for better or more detailed column meta details such as unique, not null, primary key, and so on. It defaults to true
     * @param {int} rp a value for computing how included transactions should be rollback. It defaults to exports.DB.RollbackPlan.rpDefault
     * @param {[]} arrP a given array of parameter informations which may be empty to some of database management systems
     * @param {boolean} lastInsertId true if last insert id is hoped, and false if last insert id is not expected
     * @param {function} cb an optional callback for tracking final execution result
     * @param {function} discarded an optional callback for tracking communication channel events, close and cancel
     * @param {function} serverException an optional callback for tracking an exception from server
     * @returns final execution result by promise
     */
    executeBatch(isolation, sql, paramBuff, rows = null, rh = null, delimiter = ';', batchHeader = null, meta = true, rp = exports.DB.RollbackPlan.rpDefault, arrP = [], lastInsertId = true, cb = null, discarded = null, serverException = null) {
        assert(sql && typeof sql === 'string');
        assert(rows === null || rows === undefined || typeof rows === 'function');
        assert(rh === null || rh === undefined || typeof rh === 'function');
        assert(batchHeader === null || batchHeader === undefined || typeof batchHeader === 'function');
        assert(cb === null || cb === undefined || typeof cb === 'function');
        assert(discarded === null || discarded === undefined || typeof discarded === 'function');
        assert(serverException === null || serverException === undefined || typeof serverException === 'function');
        return new Promise((res, rej) => {
            var ok = this.handler.ExecuteBatch(isolation, sql, paramBuff, (errCode, errMsg, affected, fails, oks, id) => {
                var ret;
                if (cb) ret = cb(errCode, errMsg, affected, fails, oks, id);
                if (ret === undefined) ret = {
                    ec: errCode,
                    em: errMsg,
                    aff: affected,
                    oks: oks,
                    fails: fails,
                    lastId: id
                };
                res(ret);
            }, rows, rh, batchHeader, (canceled) => {
                this.set_aborted(rej, 'ExecuteBatch', exports.DB.ReqIds.idExecuteBatch, canceled, discarded);
            }, rp, delimiter, arrP, meta, lastInsertId, (errMsg, errCode, errWhere, id) => {
                this.set_exception(rej, errMsg, errCode, errWhere, id, serverException);
            });
            if (!ok) {
                this.raise(rej, 'ExecuteBatch', exports.DB.ReqIds.idExecuteBatch, discarded);
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
    wrap: function (h) {
        if (h) {
            switch (h.getSvsId()) {
                case exports.SID.sidFile:
                    h = new CAsyncFile(h);
                    break;
                case exports.SID.sidQueue:
                    h = new CAsyncQueue(h);
                    break;
                case exports.SID.sidOdbc:
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

        //set an array of public key strings
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

    newPool: function (svsId, defaulDb = '') {
        //create a regular socket or master/slave pool.
        //you can create multiple pools for different services
        var pool = new SPA.CSocketPool(svsId /*a required unsigned int service id*/, defaulDb /*master/slave with real-time updateable cache*/);
        if (pool)
            return new CSocketPool(pool);
        return pool;
    },
    //CC == Connection Context
    newCC: function (host, port, userId, pwd, em = 0, zip = false, v6 = false, anyData = null) {
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

exports.File = {
    OpenOption: {
        TRUNCACTED: 1,
        APPENDED: 2,
        SHARE_READ: 4,
        SHARE_WRITE: 8
    },

    ReqIds: {
        idDownload: 0x7F70,
        idStartDownloading: 0x7F71,
        idDownloading: 0x7F72,
        idUpload: 0x7F73,
        idUploading: 0x7F74,
        idUploadCompleted: 0x7F75,
        idUploadBackup: 0x7F76
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

exports.CHandler = CHandler;

class CJsManager {
    constructor(jsonConfig) {
        this.ph = {};
        var poolKeys = [];
        assert(typeof jsonConfig === 'string' && jsonConfig.length > 0);
        var conf = require(jsonConfig);
        this.jc = {};
        var jcObject = this.jc;
        if (conf.WorkingDir !== undefined) {
            if (typeof conf.WorkingDir === 'string') {
                SPA.setWorkingDir(conf.WorkingDir);
            }
            else {
                throw 'A string expected for WorkingDir';
            }
        }
        this.jc['Hosts'] = {};
        this.jc['Pools'] = {};
        this.jc['CertStore'] = '';
        if (conf.CertStore !== undefined) {
            if (typeof conf.CertStore === 'string') {
                var cs = conf.CertStore.trim();
                if (cs.length > 0) {
                    SPA.setCA(cs);
                    this.jc['CertStore'] = cs;
                }
                else if (os.platform() == 'win32') {
                    SPA.setCA('root');
                    this.jc['CertStore'] = 'root';
                }
            }
            else {
                throw 'A string expected for CertStore';
            }
        }
        this.jc['QueuePassword'] = 0;
        if (conf.QueuePassword !== undefined) {
            if (typeof conf.QueuePassword === 'string') {
                var pwd = conf.QueuePassword.trim();
                if (pwd.length > 0) {
                    SPA.setPassword(pwd);
                    this.jc['QueuePassword'] = 1;
                }
            }
            else {
                throw 'A string expected for QueuePassword';
            }
        }
        if (conf.KeysAllowed !== undefined) {
            if (Array.isArray(conf.KeysAllowed)) {
                SPA.setKey(conf.KeysAllowed);
                this.jc['KeysAllowed'] = conf.KeysAllowed;
            }
            else {
                throw 'An array of public key strings expected for KeysAllowed';
            }
        }
        else {
            this.jc['KeysAllowed'] = [];
        }
        function existsConn(conn) {
            var arr = Object.keys(jcObject.Hosts);
            for (var n = 0; n < arr.length; ++n) {
                var obj = jcObject.Hosts[arr[n]];
                if (conn.Host == obj.Host && conn.Port === obj.Port) {
                    throw 'Connection context cannot be duplicate with the sample ip address and port';
                }
            }
        }
        if (typeof conf.Hosts !== 'object') {
            throw 'A map of key/host connection context expected for Hosts';
        }
        var keys = Object.keys(conf.Hosts);
        if (keys.length === 0) {
            throw 'A Hosts map cannot be empty';
        }
        for (var n = 0; n < keys.length; ++n) {
            var key = keys[n];
            if (!key) {
                throw 'Host key cannot be empty';
            }
            var obj = conf.Hosts[key];
            if (!obj || typeof obj !== 'object') {
                throw 'A pair of key/host connection context object expected';
            }
            var host = '';
            if (typeof obj.Host === 'string') {
                host = obj.Host.trim().toLowerCase();
            }
            if (!host) {
                throw 'A string expected for connection context Host';
            }
            var port = 0;
            if (typeof obj.Port === 'number') {
                port = Number(obj.Port);
            }
            if (port <= 0) {
                throw 'A positive integer expected for connection context Port';
            }
            var userid = '';
            if (obj.UserId !== undefined) {
                if (typeof obj.UserId === 'string') {
                    userid = obj.UserId.trim();
                }
                else {
                    throw 'A string expected for connection context UserId';
                }
            }
            var pwd = '';
            if (obj.Password !== undefined) {
                if (typeof obj.Password === 'string') {
                    pwd = obj.Password.trim();
                }
                else {
                    throw 'A string expected for connection context Password';
                }
            }
            var em = 0;
            if (obj.EncrytionMethod !== undefined) {
                if (typeof obj.EncrytionMethod === 'number') {
                    em = Number(obj.EncrytionMethod);
                }
                else {
                    throw 'A number is expected connection context EncrytionMethod'
                }
                if (em < 0 || em > 1) {
                    throw 'EncrytionMethod must be 0 or 1 for connection context EncrytionMethod';
                }
            }
            var zip = (!!obj.Zip);
            var v6 = (!!obj.V6);
            var anyData = obj.AnyData;
            var conn = exports.CS.newCC(host, port, userid, pwd, em, zip, v6, anyData);
            existsConn(conn);
            this.jc.Hosts[key] = conn;
        }
        function existsHost(k) {
            var arr = Object.keys(jcObject.Hosts);
            for (var n = 0; n < arr.length; ++n) {
                if (arr[n] == k) return;
            }
            throw 'Host ' + k + ' not found from the array of Hosts';
        }
        function existsPool(k) {
            var arr = poolKeys;
            for (var n = 0; n < arr.length; ++n) {
                var key = arr[n];
                if (key == k) {
                    throw 'Pool key ' + key + ' is duplicated';
                }
            }
        }
        function existsQueue(qn) {
            var arr = Object.keys(jcObject.Pools);
            for (var n = 0; n < arr.length; ++n) {
                var key = arr[n];
                var obj = jcObject.Pools[key];
                if (obj.Queue == qn) {
                    throw 'Pool queue name ' + qn + ' duplicated';
                }
                if (obj.DefaultDb && (typeof obj.Slaves === 'object')) {
                    var a = Object.keys(obj.Slaves);
                    for (var j = 0; j < a.length; ++j) {
                        obj = obj.Slaves[a[j]];
                        if (obj.Queue == qn) {
                            throw 'Pool queue name ' + qn + ' duplicated';
                        }
                    }
                }
            }
        }
        if (typeof conf.Pools !== 'object') {
            throw 'A map of key/pool context expected for Pools';
        }
        keys = Object.keys(conf.Pools);
        if (keys.length === 0) {
            throw 'A Pools map cannot be empty';
        }
        Object.assign(poolKeys, keys);
        for (var n = 0; n < keys.length; ++n) {
            var key = keys[n];
            if (!key) {
                throw 'Pool key cannot be empty';
            }
            var obj = conf.Pools[key];
            if (!obj || typeof obj !== 'object') {
                throw 'A pair of key/Pool context expected';
            }
            obj.Threads = 1;
            var queue = '';
            if (obj.Queue !== undefined) {
                if (typeof obj.Queue === 'string') {
                    queue = obj.Queue.trim();
                    if (os.platform() == 'win32') {
                        queue = queue.toLowerCase();
                    }
                    obj.Queue = queue;
                }
                else {
                    throw 'A string expected for client queue name';
                }
            }
            if (queue) {
                existsQueue(queue);
            }
            var defaultDb = '';
            if (obj.DefaultDb !== undefined) {
                if (typeof obj.DefaultDb === 'string') {
                    defaultDb = obj.DefaultDb.trim();
                    obj.DefaultDb = defaultDb;
                }
                else {
                    throw 'A string expected for default db name';
                }
            }
            obj.PoolType = 0; //regular
            if (obj.DefaultDb) {
                obj.PoolType = 2; //master
            }
            var svsId = 0;
            if (typeof obj.SvsId === 'number') {
                svsId = Number(obj.SvsId);
            }
            else {
                throw 'A number expected for service identification number';
            }
            var connTimeout = 30000;
            if (obj.ConnTimeout !== undefined) {
                if (typeof obj.ConnTimeout === 'number' && obj.ConnTimeout > 0) {
                    connTimeout = Number(obj.ConnTimeout);
                }
                else {
                    throw 'A number expected for connection timeout in milliseconds';
                }
            }
            obj.ConnTimeout = connTimeout;
            var recvTimeout = 30000;
            if (obj.RecvTimeout !== undefined) {
                if (typeof obj.RecvTimeout === 'number' && obj.RecvTimeout > 0) {
                    recvTimeout = Number(obj.RecvTimeout);
                }
                else {
                    throw 'A number expected for request receiving timeout in milliseconds';
                }
            }
            obj.RecvTimeout = recvTimeout;
            var autoConn = true;
            if (obj.AutoConn !== undefined) {
                autoConn = (!!obj.AutoConn);
            }
            obj.AutoConn = autoConn;
            var automerge = ((!!obj.Queue) && (!!obj.AutoMerge));
            obj.AutoMerge = automerge;
            switch (svsId) {
                case exports.SID.sidHTTP:
                    throw 'Client side does not support HTTP/websocket service';
                case exports.SID.sidQueue:
                    if (defaultDb || typeof obj.Slaves === 'object')
                        throw 'Server queue service does not support master or slave pool';
                    break;
                case exports.SID.sidFile:
                    if (defaultDb || typeof obj.Slaves === 'object')
                        throw 'Remote file service does not support master or slave pool';
                    break;
                case exports.SID.sidMysql:
                case exports.SID.sidOdbc:
                case exports.SID.sidSqlite:
                    break;
                default:
                    if (svsId <= exports.SID.sidReserved)
                        throw 'User defined service id must be larger than ' + exports.SID.sidReserved;
                    break;
            }
            if (!defaultDb && typeof obj.Slaves === 'object')
                throw 'Slave array is not empty but DefaultDb string is empty';
            var master = {};
            Object.assign(master, obj);
            delete master.Master;
            this.jc.Pools[key] = master;
            if (obj.DefaultDb && typeof obj.Slaves === 'object') {
                master.Slaves = {};
                var skeys = Object.keys(obj.Slaves);
                for (var j = 0; j < skeys.length; ++j) {
                    var k = skeys[j];
                    existsPool(k);
                    poolKeys.push(k);
                    var one = obj.Slaves[k];
                    var s = {};
                    s.Master = key;
                    s.PoolType = 1;
                    s.SvsId = obj.SvsId;
                    s.DefaultDb = obj.DefaultDb;
                    s.Threads = 1;
                    s.Queue = '';
                    if (one.Queue !== undefined) {
                        if (typeof one.Queue === 'string') {
                            var qn = one.Queue.trim();
                            if (os.platform() == 'win32') {
                                qn = qn.toLowerCase();
                            }
                            s.Queue = qn;
                        }
                        else {
                            throw 'A string expected for client queue name';
                        }
                    }
                    if (s.Queue) {
                        existsQueue(s.Queue);
                    }
                    if (one.DefaultDb !== undefined) {
                        if (typeof one.DefaultDb === 'string') {
                            s.DefaultDb = one.DefaultDb.trim();
                        }
                        else {
                            throw 'A string expected for default db name';
                        }
                    }
                    s.ConnTimeout = 30000;
                    if (one.ConnTimeout !== undefined) {
                        if (typeof one.ConnTimeout === 'number' && one.ConnTimeout >= 0) {
                            s.ConnTimeout = Number(one.ConnTimeout);
                        }
                        else {
                            throw 'A number expected for connection timeout in milliseconds';
                        }
                    }
                    s.RecvTimeout = 30000;
                    if (one.RecvTimeout !== undefined) {
                        if (typeof one.RecvTimeout === 'number' && one.RecvTimeout >= 0) {
                            s.RecvTimeout = Number(one.RecvTimeout);
                        }
                        else {
                            throw 'A number expected for request timeout in milliseconds';
                        }
                    }
                    s.AutoConn = true;
                    if (one.AutoConn !== undefined) {
                        s.AutoConn = (!!one.AutoConn);
                    }
                    s.AutoMerge = (s.Queue && (!!one.AutoMerge));
                    s.Hosts = [];
                    if (Array.isArray(one.Hosts) && one.Hosts.length > 0) {
                        for (var m = 0; m < one.Hosts.length; ++m) {
                            var h = one.Hosts[m].trim();
                            existsHost(h);
                            s.Hosts.push(h);
                        }
                    }
                    else {
                        throw 'An array of host key strings expected for pool context Hosts';
                    }
                    master.Slaves[k] = s;
                }
            }
            if (Array.isArray(obj.Hosts) && obj.Hosts.length > 0) {
                for (var j = 0; j < obj.Hosts.length; ++j) {
                    var h = obj.Hosts[j].trim();
                    obj.Hosts[j] = h;
                    existsHost(h);
                }
            }
            else {
                throw 'An array of host key strings expected for pool context Hosts';
            }
        }
    }
    get Config() {
        var conf = {};
        Object.assign(conf, this.jc);
        conf.WorkingDir = SPA.getWorkingDir(); //working directory hoilding client queues for client requests backups
        return conf;
    }
    get Pools() {
        return SPA.getPools(); //return the number of running socket pools
    }
    get Version() {
        return SPA.getVersion(); //return SocketPro client core version
    }
    GetPool(keyPool) {
        var jh = this.ph;
        var jcObject = this.jc;
        function existsPool(k) {
            var arr = Object.keys(jcObject.Pools);
            for (var n = 0; n < arr.length; ++n) {
                var key = arr[n];
                if (key == k) {
                    jh[key] = jcObject.Pools[key];
                    return jh[key];
                }
                var obj = jcObject.Pools[key];
                if (obj.DefaultDb && (typeof obj.Slaves === 'object')) {
                    var a = Object.keys(obj.Slaves);
                    for (var j = 0; j < a.length; ++j) {
                        if (a[j] == k) {
                            jh[k] = obj.Slaves[k];
                            return jh[k];
                        }
                    }
                }
            }
            return null;
        }
        var pc = existsPool(keyPool);
        if (!pc) {
            throw 'Pool not found by key = ' + keyPool;
        }
        var pool = jh[keyPool].Pool;
        if (pool) {
            return pool;
        }
        if (pc.Master) {
            pool = this.GetPool(pc.Master);
            pool = pool.NewSlave(pc.DefaultDb);
        }
        else {
            pool = exports.CS.newPool(pc.SvsId, pc.DefaultDb);
        }
        pool.AutoConn = pc.AutoConn;
        pool.QueueName = pc.Queue;
        pool.AutoMerge = pc.AutoMerge;
        pool.ConnTimeout = pc.ConnTimeout;
        pool.RecvTimeout = pc.RecvTimeout;
        jh[keyPool].Pool = pool;
        var sessions = [];
        for (var n = 0; n < pc.Hosts.length; ++n) {
            var key = pc.Hosts[n];
            sessions.push(jcObject.Hosts[key]);
        }
        var ok = pool.Start(sessions, sessions.length);
        if (!ok) {
            throw JSON.stringify(pool.Error);
        }
        return pool;
    }
    GetHandler(keyPool) {
        var pool = this.GetPool(keyPool);
        return pool.Seek();
    }
    GetHandlerByQueue(keyPool) {
        var pool = this.GetPool(keyPool);
        if (!pool.Queue) {
            throw 'No queue name available';
        }
        return pool.SeekByQueue(pool.Queue);
    }
}

exports.Manager = null;
exports.GetManager = function (jsonConfig = '') {
    if (!exports.Manager) {
        if (!jsonConfig) {
            jsonConfig = process.cwd();
            if (os.platform() == 'win32')
                jsonConfig += '\\';
            else
                jsonConfig += '/';
            jsonConfig += 'sp_config.json';
        }
        exports.Manager = new CJsManager(jsonConfig);
    }
    return exports.Manager;
};

exports.GetSpManager = exports.GetManager;

exports.GetSpPool = function (keyPool) {
    return exports.GetManager().GetPool(keyPool);
};

exports.GetSpHandler = function (keyPool) {
    return exports.GetManager().GetHandler(keyPool);
};

exports.SeekSpHandler = exports.GetSpHandler;

exports.GetSpHandlerByQueue = function (keyPool) {
    return exports.GetManager().GetHandlerByQueue(keyPool);
};

exports.SeekSpHandlerByQueue = exports.GetSpHandlerByQueue;
