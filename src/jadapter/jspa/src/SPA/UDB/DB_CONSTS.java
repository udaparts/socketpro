package SPA.UDB;

public class DB_CONSTS {

    /**
     * Async database client/server just requires the following request
     * identification numbers
     */
    public static final short idOpen = 0x7E7F;
    public static final short idClose = idOpen + 1;
    public static final short idBeginTrans = idClose + 1;
    public static final short idEndTrans = idBeginTrans + 1;
    public static final short idExecute = idEndTrans + 1;
    public static final short idPrepare = idExecute + 1;
    public static final short idExecuteParameters = idPrepare + 1;

    /**
     * Request identification numbers used for message push from server to
     * client
     */
    public static final short idDBUpdate = idExecuteParameters + 1; //server ==> client only
    public static final short idRowsetHeader = idDBUpdate + 1; //server ==> client only
    public static final short idOutputParameter = idRowsetHeader + 1; //server ==> client only

    /**
     * Internal request/response identification numbers used for data
     * communication between client and server
     */
    public static final short idBeginRows = idOutputParameter + 1;
    public static final short idTransferring = idBeginRows + 1;
    public static final short idStartBLOB = idTransferring + 1;
    public static final short idChunk = idStartBLOB + 1;
    public static final short idEndBLOB = idChunk + 1;
    public static final short idEndRows = idEndBLOB + 1;
    public static final short idCallReturn = idEndRows + 1;

    public static final short idGetCachedTables = idCallReturn + 1;

    public static final short idSqlBatchHeader = idGetCachedTables + 1;
    public static final short idExecuteBatch = idSqlBatchHeader + 1;
    public static final short idParameterPosition = idExecuteBatch + 1;

    /**
     * Whenever a data size in bytes is about twice larger than the defined
     * second, the data will be treated in large object and transferred in
     * chunks for reducing memory foot print
     */
    public static final int DEFAULT_BIG_FIELD_CHUNK_SIZE = 16 * 1024; //16k

    /**
     * A record data size in bytes is approximately equal to or slightly larger
     * than the defined constant
     */
    public static final int DEFAULT_RECORD_BATCH_SIZE = 16 * 1024; //16k

    /**
     * A flag used with idOpen for tracing database table update events
     */
    public static final int ENABLE_TABLE_UPDATE_MESSAGES = 0x1;

    /**
     * A chat group id used at SocketPro server side for notifying database
     * events from server to connected clients
     */
    public static final int STREAMING_SQL_CHAT_GROUP_ID = 0x1fffffff;

    /**
     * A chat group id used at a SocketPro middle tier server side for notifying
     * database connecting events from the middle tier server to front clients
     */
    public static final int CACHE_UPDATE_CHAT_GROUP_ID = STREAMING_SQL_CHAT_GROUP_ID + 1;
}
