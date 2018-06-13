package SPA.ClientSide;

import SPA.*;
import SPA.UDB.*;
import java.nio.charset.Charset;

public class CAsyncDBHandler extends CAsyncServiceHandler {

    private static final int ONE_MEGA_BYTES = 0x100000;
    private static final int BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0;

    public CAsyncDBHandler(int sid) {
        super(sid);
    }

    public interface DResult {

        void invoke(CAsyncDBHandler dbHandler, int res, String errMsg);
    }

    public interface DExecuteResult {

        void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId);
    }

    public interface DRowsetHeader {

        void invoke(CAsyncDBHandler dbHandler);
    }

    public interface DRows {

        void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData);
    }

    protected final Object m_csDB = new Object();
    protected CDBColumnInfoArray m_vColInfo = new CDBColumnInfoArray();

    private String m_strConnection;
    protected long m_affected = -1;
    protected int m_dbErrCode = 0;
    protected String m_dbErrMsg = "";
    protected short m_lastReqId = 0;

    protected final java.util.HashMap<Long, Pair<DRowsetHeader, DRows>> m_mapRowset = new java.util.HashMap<>();
    private final java.util.HashMap<Long, CDBVariantArray> m_mapParameterCall = new java.util.HashMap<>();

    protected long m_indexRowset = 0;
    private final CUQueue m_Blob = new CUQueue();
    private final CDBVariantArray m_vData = new CDBVariantArray();
    private tagManagementSystem m_ms = tagManagementSystem.msUnknown;

    private int m_flags = 0;
    private int m_parameters = 0;
    private int m_indexProc = 0;
    private int m_output = 0;
    private boolean m_bCallReturn = false;
    private boolean m_queueOk = false;

    public final String getConnection() {
        synchronized (m_csDB) {
            return m_strConnection;
        }
    }

    public final int getParameters() {
        synchronized (m_csDB) {
            return m_parameters;
        }
    }

    public final boolean getCallReturn() {
        synchronized (m_csDB) {
            return m_bCallReturn;
        }
    }

    public final int getOutputs() {
        synchronized (m_csDB) {
            return m_output;
        }
    }

    public final int getLastDBErrorCode() {
        synchronized (m_csDB) {
            return m_dbErrCode;
        }
    }

    public final tagManagementSystem getDBManagementSystem() {
        synchronized (m_csDB) {
            return m_ms;
        }

    }

    public final boolean getOpened() {
        synchronized (m_csDB) {
            return (m_strConnection != null && m_strConnection.length() > 0 && m_lastReqId > 0);
        }
    }

    public final long geLastAffected() {
        synchronized (m_csDB) {
            return m_affected;
        }
    }

    public final String getLastDBErrorMessage() {
        synchronized (m_csDB) {
            return m_dbErrMsg;
        }
    }

    public CDBColumnInfoArray getColumnInfo() {
        synchronized (m_csDB) {
            return m_vColInfo;
        }
    }

    @Override
    public int CleanCallbacks() {
        synchronized (m_csDB) {
            Clean();
        }
        return super.CleanCallbacks();
    }

    private void Clean() {
        m_strConnection = "";
        m_mapRowset.clear();
        m_vColInfo.clear();
        m_mapParameterCall.clear();
        m_lastReqId = 0;
        m_Blob.SetSize(0);
        m_deqExecuteResult.clear();
        m_deqResult.clear();
        if (m_Blob.getMaxBufferSize() > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
            m_Blob.Realloc(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
        }
        m_vData.clear();
    }

    @Override
    protected void OnMergeTo(CAsyncServiceHandler to) {
        CAsyncDBHandler dbTo = (CAsyncDBHandler) to;
        synchronized (dbTo.m_csDB) {
            synchronized (m_csDB) {
                dbTo.m_deqResult.addAll(m_deqResult);
                dbTo.m_deqExecuteResult.addAll(m_deqExecuteResult);
                m_deqExecuteResult.clear();
                for (long callIndex : m_mapRowset.keySet()) {
                    dbTo.m_mapRowset.put(callIndex, m_mapRowset.get(callIndex));
                }
                m_mapRowset.clear();
                for (long callIndex : m_mapParameterCall.keySet()) {
                    dbTo.m_mapParameterCall.put(callIndex, m_mapParameterCall.get(callIndex));
                }
                m_mapParameterCall.clear();
            }
        }
    }

    @Override
    protected void OnAllProcessed() {
        synchronized (m_csDB) {
            Object[] arr = m_mapRowset.keySet().toArray();
            int count = arr.length - 16;
            for (int n = 0; n < count; ++n) {
                m_mapRowset.remove((Long) (arr[n]));
            }
            m_mapParameterCall.clear();
        }
    }

    private boolean Send(CUQueue q, boolean[] firstRow) {
        if (q.GetSize() > 0) {
            if (firstRow[0]) {
                firstRow[0] = false;
                if (!SendRequest(DB_CONSTS.idBeginRows, q.GetBuffer(), q.GetSize(), null)) {
                    return false;
                }
            } else {
                if (!SendRequest(DB_CONSTS.idTransferring, q.GetBuffer(), q.GetSize(), null)) {
                    return false;
                }
            }
            q.SetSize(0);
        } else if (firstRow[0]) {
            firstRow[0] = false;
            if (!SendRequest(DB_CONSTS.idBeginRows, null)) {
                return false;
            }
        }
        return true;
    }

    private boolean SendBlob(CUQueue q) {
        if (q.GetSize() > 0) {
            byte[] bytes = new byte[DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE];
            boolean start = true;
            while (q.GetSize() > 0) {
                int send = (q.GetSize() >= DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) ? DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE : q.GetSize();
                q.PopBytes(bytes, send);
                if (start) {
                    if (!SendRequest(DB_CONSTS.idStartBLOB, bytes, send, null)) {
                        return false;
                    }
                    start = false;
                } else {
                    if (!SendRequest(DB_CONSTS.idChunk, bytes, send, null)) {
                        return false;
                    }
                }
                if (q.GetSize() < DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    break;
                }
            }
            if (!SendRequest(DB_CONSTS.idEndBLOB, q.GetBuffer(), q.GetSize(), null)) {
                return false;
            }
            q.SetSize(0);
        }
        return true;
    }

    static java.text.SimpleDateFormat m_f = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");

    private boolean SendParametersData(CDBVariantArray vParam) {
        if (vParam == null) {
            return true;
        }
        int size = vParam.size();
        if (size == 0) {
            return true;
        }
        boolean[] firstRow = new boolean[1];
        firstRow[0] = true;
        CUQueue sb = CScopeUQueue.Lock();
        for (int n = 0; n < size; ++n) {
            Object vt = vParam.get(n);
            if (vt == null) {
                sb.Save(vt);
                continue;
            }
            String type = vt.getClass().getName();
            switch (type) {
                case "java.lang.String": {
                    String s = (String) vt;
                    int len = s.length();
                    if (len < DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                        sb.Save(vt);
                    } else {
                        if (!Send(sb, firstRow)) {
                            CScopeUQueue.Unlock(sb);
                            return false;
                        }
                        int bytes = len;
                        bytes *= 2;
                        bytes += 6; //sizeof(ushort) + sizeof(uint)
                        sb.Save(bytes).Save(vt);
                        if (!SendBlob(sb)) {
                            CScopeUQueue.Unlock(sb);
                            return false;
                        }
                    }
                }
                break;
                case "[B": {
                    byte[] bytes = (byte[]) vt;
                    int len = bytes.length;
                    if (len < 2 * DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                        sb.Save(vt);
                    } else {
                        if (!Send(sb, firstRow)) {
                            CScopeUQueue.Unlock(sb);
                            return false;
                        }
                        len += 6; //sizeof(ushort) + sizeof(uint)
                        sb.Save(len).Save(vt);
                        if (!SendBlob(sb)) {
                            CScopeUQueue.Unlock(sb);
                            return false;
                        }
                    }
                }
                break;
                default:
                    sb.Save(vt);
                    break;
            }
            if (sb.GetSize() >= DB_CONSTS.DEFAULT_RECORD_BATCH_SIZE) {
                if (!Send(sb, firstRow)) {
                    CScopeUQueue.Unlock(sb);
                    return false;
                }
            }
        }
        boolean ok = Send(sb, firstRow);
        CScopeUQueue.Unlock(sb);
        return ok;
    }

    protected class MyCallback<CB> {

        public short ReqId;
        public CB Callback;

        public MyCallback(short reqId, CB cb) {
            ReqId = reqId;
            Callback = cb;
        }
    }
    private final java.util.ArrayDeque<MyCallback<DResult>> m_deqResult = new java.util.ArrayDeque<>();

    private MyCallback<DResult> GetResultHandler(short reqId) {
        if (m_ClientSocket.getRandom()) {
            synchronized (m_csDB) {
                for (MyCallback<DResult> kv : m_deqResult) {
                    if (kv.ReqId == reqId) {
                        m_deqResult.remove(kv);
                        return kv;
                    }
                }
            }
        } else {
            synchronized (m_csDB) {
                if (m_deqResult.size() > 0 && m_deqResult.getFirst().ReqId == reqId) {
                    return m_deqResult.removeFirst();
                }
            }
        }
        return new MyCallback<>((short) 0, null);
    }

    /**
     * Notify connected remote server to close database connection string
     * asynchronously
     *
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Close() {
        return Close(null, null);
    }

    /**
     * Notify connected remote server to close database connection string
     * asynchronously
     *
     * @param handler a callback for closing result, which should be OK always
     * as long as there is network or queue available
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Close(DResult handler) {
        return Close(handler, null);
    }

    /**
     * Notify connected remote server to close database connection string
     * asynchronously
     *
     * @param handler a callback for closing result, which should be OK always
     * as long as there is network or queue available
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Close(DResult handler, DDiscarded discarded) {
        MyCallback<DResult> cb = new MyCallback<>(DB_CONSTS.idClose, handler);
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_deqResult.add(cb);
            }
            if (!SendRequest(DB_CONSTS.idClose, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqResult.remove(cb);
                }
                return false;
            }
        }
        return true;
    }

    /**
     * Start a manual transaction with a given isolation
     * tagTransactionIsolation.tiReadCommited asynchronously. Note the
     * transaction will be associated with SocketPro client message queue if
     * available to avoid possible transaction lose
     *
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean BeginTrans() {
        return BeginTrans(tagTransactionIsolation.tiReadCommited, null, null);
    }

    /**
     * Start a manual transaction with a given isolation asynchronously. Note
     * the transaction will be associated with SocketPro client message queue if
     * available to avoid possible transaction lose
     *
     * @param isolation a second for transaction isolation. It defaults to
     * tagTransactionIsolation.tiReadCommited
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean BeginTrans(tagTransactionIsolation isolation) {
        return BeginTrans(isolation, null, null);
    }

    /**
     * Start a manual transaction with a given isolation asynchronously. Note
     * the transaction will be associated with SocketPro client message queue if
     * available to avoid possible transaction lose
     *
     * @param isolation a second for transaction isolation. It defaults to
     * tagTransactionIsolation.tiReadCommited
     * @param handler a callback for tracking its response result
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean BeginTrans(tagTransactionIsolation isolation, DResult handler) {
        return BeginTrans(isolation, handler, null);
    }

    /**
     * Start a manual transaction with a given isolation asynchronously. Note
     * the transaction will be associated with SocketPro client message queue if
     * available to avoid possible transaction lose
     *
     * @param isolation a second for transaction isolation. It defaults to
     * tagTransactionIsolation.tiReadCommited
     * @param handler a callback for tracking its response result
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean BeginTrans(tagTransactionIsolation isolation, DResult handler, DDiscarded discarded) {
        MyCallback<DResult> cb = new MyCallback<>(DB_CONSTS.idBeginTrans, handler);
        CUQueue sb = CScopeUQueue.Lock();
        //make sure BeginTrans sending and underlying client persistent message queue as one combination sending
        //to avoid possible request sending/client message writing overlapping within multiple threading environment
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.

            //associate begin transaction with underlying client persistent message queue
            m_queueOk = getAttachedClientSocket().getClientQueue().StartJob();

            synchronized (m_csDB) {
                sb.Save(isolation.getValue()).Save(m_strConnection).Save(m_flags);
                m_deqResult.add(cb);
            }
            if (!SendRequest(DB_CONSTS.idBeginTrans, sb, null, discarded)) {
                CScopeUQueue.Unlock(sb);
                synchronized (m_csDB) {
                    m_deqResult.remove(cb);
                    if (m_queueOk) {
                        getAttachedClientSocket().getClientQueue().AbortJob();
                    }
                }
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    /**
     * End a manual transaction with a given rollback plan
     * tagRollbackPlan.rpDefault. Note the transaction will be associated with
     * SocketPro client message queue if available to avoid possible transaction
     * lose
     *
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean EndTrans() {
        return EndTrans(tagRollbackPlan.rpDefault, null, null);
    }

    /**
     * End a manual transaction with a given rollback plan. Note the transaction
     * will be associated with SocketPro client message queue if available to
     * avoid possible transaction lose
     *
     * @param plan a second for computing how included transactions should be
     * rollback at server side. It defaults to tagRollbackPlan.rpDefault
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean EndTrans(tagRollbackPlan plan) {
        return EndTrans(plan, null, null);
    }

    /**
     * End a manual transaction with a given rollback plan. Note the transaction
     * will be associated with SocketPro client message queue if available to
     * avoid possible transaction lose
     *
     * @param plan a second for computing how included transactions should be
     * rollback at server side. It defaults to tagRollbackPlan.rpDefault
     * @param handler a callback for tracking its response result
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean EndTrans(tagRollbackPlan plan, DResult handler) {
        return EndTrans(plan, handler, null);
    }

    /**
     * End a manual transaction with a given rollback plan. Note the transaction
     * will be associated with SocketPro client message queue if available to
     * avoid possible transaction lose
     *
     * @param plan a second for computing how included transactions should be
     * rollback at server side. It defaults to tagRollbackPlan.rpDefault
     * @param handler a callback for tracking its response result
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean EndTrans(tagRollbackPlan plan, DResult handler, DDiscarded discarded) {
        boolean ok = true;
        MyCallback<DResult> cb = new MyCallback<>(DB_CONSTS.idEndTrans, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(plan.getValue());
        //make sure EndTrans sending and underlying client persistent message queue as one combination sending
        //to avoid possible request sending/client message writing overlapping within multiple threading environment
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_deqResult.add(cb);
            }
            ok = SendRequest(DB_CONSTS.idEndTrans, sb, null, discarded);
            if (ok) {
                if (m_queueOk) {
                    //associate end transaction with underlying client persistent message queue
                    getAttachedClientSocket().getClientQueue().EndJob();
                    m_queueOk = false;
                }
            } else {
                synchronized (m_csDB) {
                    m_deqResult.remove(cb);
                }
            }
        }
        CScopeUQueue.Unlock(sb);
        return ok;
    }

    private final Charset UTF8_CHARSET = Charset.forName("UTF-8");

    protected final java.util.ArrayDeque<MyCallback<DExecuteResult>> m_deqExecuteResult = new java.util.ArrayDeque<>();

    protected MyCallback<DExecuteResult> GetExecuteResultHandler(short reqId) {
        if (m_ClientSocket.getRandom()) {
            synchronized (m_csDB) {
                for (MyCallback<DExecuteResult> kv : m_deqExecuteResult) {
                    if (kv.ReqId == reqId) {
                        m_deqExecuteResult.remove(kv);
                        return kv;
                    }
                }
            }
        } else {
            synchronized (m_csDB) {
                if (m_deqExecuteResult.size() > 0 && m_deqExecuteResult.getFirst().ReqId == reqId) {
                    return m_deqExecuteResult.removeFirst();
                }
            }
        }
        return null;
    }

    /**
     * Open a database connection at server side asynchronously
     *
     * @param strConnection a database connection string. The database
     * connection string can be an empty string if its server side supports
     * global database connection string
     * @param handler a callback for database connecting result
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Open(String strConnection, DResult handler) {
        return Open(strConnection, handler, 0, null);
    }

    /**
     * Open a database connection at server side asynchronously
     *
     * @param strConnection a database connection string. The database
     * connection string can be an empty string if its server side supports
     * global database connection string
     * @param handler a callback for database connecting result
     * @param flags a set of flags transferred to server to indicate how to
     * build database connection at server side. It defaults to zero
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Open(String strConnection, DResult handler, int flags) {
        return Open(strConnection, handler, flags, null);
    }

    /**
     * Open a database connection at server side asynchronously
     *
     * @param strConnection a database connection string. The database
     * connection string can be an empty string if its server side supports
     * global database connection string
     * @param handler a callback for database connecting result
     * @param flags a set of flags transferred to server to indicate how to
     * build database connection at server side. It defaults to zero
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Open(String strConnection, DResult handler, int flags, DDiscarded discarded) {
        String str = null;
        MyCallback<DResult> cb = new MyCallback<>(DB_CONSTS.idOpen, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(strConnection).Save(flags);
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_flags = flags;
                if (strConnection != null) {
                    str = m_strConnection;
                    m_strConnection = strConnection;
                }
                m_deqResult.add(cb);
            }
            if (SendRequest(DB_CONSTS.idOpen, sb, null, discarded)) {
                CScopeUQueue.Unlock(sb);
                return true;
            } else {
                synchronized (m_csDB) {
                    m_deqResult.remove(cb);
                    if (strConnection != null) {
                        m_strConnection = str;
                    }
                }
            }
        }
        CScopeUQueue.Unlock(sb);
        return false;
    }

    /**
     * Send a parameterized SQL statement for preparing asynchronously
     *
     * @param sql a parameterized SQL statement
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Prepare(String sql) {
        return Prepare(sql, null, null, null);
    }

    /**
     * Send a parameterized SQL statement for preparing asynchronously
     *
     * @param sql a parameterized SQL statement
     * @param handler a callback for SQL preparing result
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Prepare(String sql, DResult handler) {
        return Prepare(sql, handler, null, null);
    }

    /**
     * Send a parameterized SQL statement for preparing with a given array of
     * parameter informations asynchronously
     *
     * @param sql a parameterized SQL statement
     * @param handler a callback for SQL preparing result
     * @param vParameterInfo a given array of parameter informations
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Prepare(String sql, DResult handler, CParameterInfo[] vParameterInfo) {
        return Prepare(sql, handler, vParameterInfo, null);
    }

    /**
     * Send a parameterized SQL statement for preparing with a given array of
     * parameter informations asynchronously
     *
     * @param sql a parameterized SQL statement
     * @param handler a callback for SQL preparing result
     * @param vParameterInfo a given array of parameter informations
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Prepare(String sql, DResult handler, CParameterInfo[] vParameterInfo, DDiscarded discarded) {
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(sql);
        int count = 0;
        if (vParameterInfo != null) {
            count = vParameterInfo.length;
        }
        sb.Save(count);
        if (count > 0) {
            for (CParameterInfo info : vParameterInfo) {
                info.SaveTo(sb);
            }
        }
        boolean ok;
        MyCallback<DResult> cb = new MyCallback<>(DB_CONSTS.idPrepare, handler);
        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                m_deqResult.add(cb);
            }
            if (SendRequest(DB_CONSTS.idPrepare, sb, null, discarded)) {
                ok = true;
            } else {
                ok = false;
                synchronized (m_csDB) {
                    m_deqResult.remove(cb);
                }
            }
        }
        CScopeUQueue.Unlock(sb);
        return ok;
    }

    /**
     * Asynchronously process a complex SQL statement which may be combined with
     * multiple basic SQL statements, and don't expect any data returned
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(String sql) {
        return Execute(sql, null, null, null, true, true, null);
    }

    /**
     * Asynchronously process a complex SQL statement which may be combined with
     * multiple basic SQL statements, and don't expect any records returned
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @param handler a callback for tracking final result
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(String sql, DExecuteResult handler) {
        return Execute(sql, handler, null, null, true, true, null);
    }

    /**
     * Process a complex SQL statement which may be combined with multiple basic
     * SQL statements asynchronously
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @param handler a callback for tracking final result
     * @param row a callback for receiving records of data
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(String sql, DExecuteResult handler, DRows row) {
        return Execute(sql, handler, row, null, true, true, null);
    }

    /**
     * Process a complex SQL statement which may be combined with multiple basic
     * SQL statements asynchronously
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @param handler a callback for tracking final result
     * @param row a callback for receiving records of data
     * @param rh a callback for tracking row set of header column informations.
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(String sql, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return Execute(sql, handler, row, rh, true, true, null);
    }

    /**
     * Process a complex SQL statement which may be combined with multiple basic
     * SQL statements asynchronously
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @param handler a callback for tracking final result
     * @param row a callback for receiving records of data
     * @param rh a callback for tracking row set of header column informations
     * @param meta a boolean second for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults
     * to true
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(String sql, DExecuteResult handler, DRows row, DRowsetHeader rh, boolean meta) {
        return Execute(sql, handler, row, rh, meta, true, null);
    }

    /**
     * Process a complex SQL statement which may be combined with multiple basic
     * SQL statements asynchronously
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @param rh a callback for tracking row set of header column informations
     * @param meta a boolean second for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults
     * to true
     * @param lastInsertId a boolean second for last insert record
     * identification number. It defaults to true
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(String sql, DExecuteResult handler, DRows row, DRowsetHeader rh, boolean meta, boolean lastInsertId) {
        return Execute(sql, handler, row, rh, meta, lastInsertId, null);
    }

    /**
     * Process a complex SQL statement which may be combined with multiple basic
     * SQL statements asynchronously
     *
     * @param sql a complex SQL statement which may be combined with multiple
     * basic SQL statements
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @param rh a callback for tracking row set of header column informations
     * @param meta a boolean second for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults
     * to true
     * @param lastInsertId a boolean second for last insert record
     * identification number. It defaults to true
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Execute(String sql, DExecuteResult handler, DRows row, DRowsetHeader rh, boolean meta, boolean lastInsertId, DDiscarded discarded) {
        boolean rowset = (rh != null || row != null);
        if (!rowset) {
            meta = false;
        }
        MyCallback<DExecuteResult> cb = new MyCallback<>(DB_CONSTS.idExecute, handler);
        CUQueue sb = CScopeUQueue.Lock();
        sb.Save(sql);
        sb.Save(rowset);
        sb.Save(meta);
        sb.Save(lastInsertId);
        final long index = GetCallIndex();
        sb.Save(index);

        synchronized (m_csOneSending) {
            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                if (rowset) {
                    m_mapRowset.put(index, new Pair<>(rh, row));
                }
                m_deqExecuteResult.add(cb);
            }
            if (!SendRequest(DB_CONSTS.idExecute, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_deqExecuteResult.remove(cb);
                    if (rowset) {
                        m_mapRowset.remove(index);
                    }
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
        }
        CScopeUQueue.Unlock(sb);
        return true;
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously, and don't expect any data returned
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(CDBVariantArray vParam) {
        return Execute(vParam, null, null, null, true, true, null);
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously, and don't expect any row set or output
     * returned
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @param handler a callback for tracking final result
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(CDBVariantArray vParam, DExecuteResult handler) {
        return Execute(vParam, handler, null, null, true, true, null);
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously, and don't expect any row set returned
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row) {
        return Execute(vParam, handler, row, null, true, true, null);
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @param rh a callback for tracking row set of header column informations
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh) {
        return Execute(vParam, handler, row, rh, true, true, null);
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @param rh a callback for tracking row set of header column informations
     * @param meta a boolean second for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults
     * to true
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, boolean meta) {
        return Execute(vParam, handler, row, rh, meta, true, null);
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @param rh a callback for tracking row set of header column informations
     * @param meta a boolean second for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults
     * to true
     * @param lastInsertId a boolean second for last insert record
     * identification number. It defaults to true
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public final boolean Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, boolean meta, boolean lastInsertId) {
        return Execute(vParam, handler, row, rh, meta, lastInsertId, null);
    }

    /**
     * Process one or more sets of prepared statements with an array of
     * parameter data asynchronously
     *
     * @param vParam an array of parameter data which will be bounded to
     * previously prepared parameters
     * @param handler a callback for tracking final result
     * @param row a callback for tracking record or output parameter returned
     * data
     * @param rh a callback for tracking row set of header column informations
     * @param meta a boolean second for better or more detailed column meta
     * details such as unique, not null, primary first, and so on. It defaults
     * to true
     * @param lastInsertId a boolean second for last insert record
     * identification number. It defaults to true
     * @param discarded a callback for tracking cancel or socket closed event
     * @return true if request is successfully sent or queued; and false if
     * request is NOT successfully sent or queued
     */
    public boolean Execute(CDBVariantArray vParam, DExecuteResult handler, DRows row, DRowsetHeader rh, boolean meta, boolean lastInsertId, DDiscarded discarded) {
        boolean rowset = (rh != null || row != null);
        if (!rowset) {
            meta = false;
        }
        MyCallback<DExecuteResult> cb = new MyCallback<>(DB_CONSTS.idExecuteParameters, handler);

        //make sure all parameter data sendings and ExecuteParameters sending as one combination sending
        //to avoid possible request sending overlapping within multiple threading environment
        synchronized (m_csOneSending) {
            boolean queueOk = getAttachedClientSocket().getClientQueue().StartJob();
            if (!SendParametersData(vParam)) {
                Clean();
                return false;
            }
            final long index = GetCallIndex();
            CUQueue sb = CScopeUQueue.Lock();
            sb.Save(rowset);
            sb.Save(meta);
            sb.Save(lastInsertId);
            sb.Save(index);

            //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
            //in case a client asynchronously sends lots of requests without use of client side queue.
            synchronized (m_csDB) {
                if (rowset) {
                    m_mapRowset.put(index, new Pair<>(rh, row));
                }
                m_deqExecuteResult.add(cb);
                m_mapParameterCall.put(index, vParam);
            }
            if (!SendRequest(DB_CONSTS.idExecuteParameters, sb, null, discarded)) {
                synchronized (m_csDB) {
                    m_mapParameterCall.remove(index);
                    m_deqExecuteResult.remove(cb);
                    if (rowset) {
                        m_mapRowset.remove(index);
                    }
                }
                CScopeUQueue.Unlock(sb);
                return false;
            }
            if (queueOk) {
                getAttachedClientSocket().getClientQueue().EndJob();
            }
            CScopeUQueue.Unlock(sb);
        }
        return true;
    }

    final protected Object m_csOneSending = new Object();

    @Override
    protected void OnResultReturned(short reqId, CUQueue mc) {
        switch (reqId) {
            case DB_CONSTS.idExecuteParameters:
            case DB_CONSTS.idExecute: {
                long affected = mc.LoadLong();
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                Object vtId = mc.LoadObject();
                long fail_ok = mc.LoadLong();
                MyCallback<DExecuteResult> t = GetExecuteResultHandler(reqId);
                synchronized (m_csDB) {
                    m_lastReqId = reqId;
                    m_affected = affected;
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    if (m_mapRowset.containsKey(m_indexRowset)) {
                        m_mapRowset.remove(m_indexRowset);
                    }

                    m_indexProc = 0;
                    if (m_mapParameterCall.containsKey(m_indexRowset)) {
                        m_mapParameterCall.remove(m_indexRowset);
                    } else if (getAttachedClientSocket().getCountOfRequestsInQueue() == 1) {
                        m_mapParameterCall.clear();
                    }
                }
                if (t != null) {
                    if (t.Callback != null) {
                        t.Callback.invoke(this, res, errMsg, affected, fail_ok, vtId);
                    }
                }
            }
            break;
            case DB_CONSTS.idPrepare: {
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                int parameters = mc.LoadInt();
                MyCallback<DResult> t = GetResultHandler(reqId);
                synchronized (m_csDB) {
                    m_lastReqId = DB_CONSTS.idPrepare;
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    m_parameters = (parameters & 0xffff);
                    m_output = (parameters >> 16);
                    m_indexProc = 0;
                    m_bCallReturn = false;
                }
                if (t != null && t.Callback != null) {
                    t.Callback.invoke(this, res, errMsg);
                }
            }
            break;
            case DB_CONSTS.idOpen: {
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                int ms = mc.LoadInt();
                MyCallback<DResult> t = GetResultHandler(reqId);
                synchronized (m_csDB) {
                    m_dbErrCode = res;
                    m_lastReqId = DB_CONSTS.idOpen;
                    if (res == 0) {
                        m_strConnection = errMsg;
                        errMsg = "";
                    } else {
                        m_strConnection = "";
                    }
                    m_dbErrMsg = errMsg;
                    m_ms = tagManagementSystem.forValue(ms);
                    m_parameters = 0;
                    m_output = 0;
                    m_indexProc = 0;
                    if (getAttachedClientSocket().getCountOfRequestsInQueue() == 1) {
                        m_mapParameterCall.clear();
                    }
                }
                if (t != null && t.Callback != null) {
                    t.Callback.invoke(this, res, errMsg);
                }
            }
            break;
            case DB_CONSTS.idEndTrans: {
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                MyCallback<DResult> t = GetResultHandler(reqId);
                synchronized (m_csDB) {
                    m_lastReqId = DB_CONSTS.idEndTrans;
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    if (getAttachedClientSocket().getCountOfRequestsInQueue() == 1) {
                        m_mapParameterCall.clear();
                    }
                }
                if (t != null && t.Callback != null) {
                    t.Callback.invoke(this, res, errMsg);
                }
            }
            break;
            case DB_CONSTS.idBeginTrans: {
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                int ms = mc.LoadInt();
                MyCallback<DResult> t = GetResultHandler(reqId);
                synchronized (m_csDB) {
                    if (res == 0) {
                        m_strConnection = errMsg;
                        errMsg = "";
                    }
                    m_lastReqId = DB_CONSTS.idBeginTrans;
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    m_ms = tagManagementSystem.forValue(ms);
                }
                if (t != null && t.Callback != null) {
                    t.Callback.invoke(this, res, errMsg);
                }
            }
            break;
            case DB_CONSTS.idClose: {
                int res = mc.LoadInt();
                String errMsg = mc.LoadString();
                MyCallback<DResult> t = GetResultHandler(reqId);
                synchronized (m_csDB) {
                    m_lastReqId = DB_CONSTS.idClose;
                    m_strConnection = "";
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    m_parameters = 0;
                    m_output = 0;
                    m_indexProc = 0;
                    if (getAttachedClientSocket().getCountOfRequestsInQueue() == 1) {
                        m_mapParameterCall.clear();
                    }
                }
                if (t != null && t.Callback != null) {
                    t.Callback.invoke(this, res, errMsg);
                }
            }
            break;
            case DB_CONSTS.idRowsetHeader: {
                m_Blob.SetSize(0);
                if (m_Blob.getMaxBufferSize() > ONE_MEGA_BYTES) {
                    m_Blob.Realloc(ONE_MEGA_BYTES);
                }
                m_vData.clear();
                DRowsetHeader header = null;
                synchronized (m_csDB) {
                    m_vColInfo = new CDBColumnInfoArray();
                    m_vColInfo.LoadFrom(mc);
                    m_indexRowset = mc.LoadLong();
                    if (mc.GetSize() > 0) {
                        m_output = mc.LoadInt();
                    } else {
                        m_output = 0;
                    }
                    if (m_output == 0 && m_vColInfo.size() > 0) {
                        if (m_mapRowset.containsKey(m_indexRowset)) {
                            header = m_mapRowset.get(m_indexRowset).first;
                        }
                    }
                }
                if (header != null) {
                    header.invoke(this);
                }
            }
            break;
            case DB_CONSTS.idCallReturn: {
                Object vt = mc.LoadObject();
                synchronized (m_csDB) {
                    if (m_mapParameterCall.containsKey(m_indexRowset)) {
                        CDBVariantArray vParam = m_mapParameterCall.get(m_indexRowset);
                        int pos = m_parameters * m_indexProc;
                        vParam.set(pos, vt);
                    }
                    m_bCallReturn = true;
                }
            }
            break;
            case DB_CONSTS.idBeginRows:
                m_Blob.SetSize(0);
                m_vData.clear();
                if (mc.GetSize() > 0) {
                    synchronized (m_csDB) {
                        m_indexRowset = mc.LoadLong();
                    }
                }
                break;
            case DB_CONSTS.idTransferring:
                while (mc.GetSize() > 0) {
                    Object vt = mc.LoadObject();
                    m_vData.add(vt);
                }
                break;
            case DB_CONSTS.idOutputParameter:
            case DB_CONSTS.idEndRows:
                if (mc.GetSize() > 0 || m_vData.size() > 0) {
                    Object vt;
                    while (mc.GetSize() > 0) {
                        vt = mc.LoadObject();
                        m_vData.add(vt);
                    }
                    if (reqId == DB_CONSTS.idOutputParameter) {
                        synchronized (m_csDB) {
                            m_output = m_vData.size() + (m_bCallReturn ? 1 : 0);
                            if (m_mapParameterCall.containsKey(m_indexRowset)) {
                                CDBVariantArray vParam = m_mapParameterCall.get(m_indexRowset);
                                int pos = m_parameters * m_indexProc + m_parameters - m_output;
                                for (int n = 0; n < m_output; ++n) {
                                    vParam.set(pos, m_vData.get(n));
                                    ++pos;
                                }
                            }
                            ++m_indexProc;
                        }
                    } else {
                        DRows row = null;
                        synchronized (m_csDB) {
                            if (m_mapRowset.containsKey(m_indexRowset)) {
                                row = m_mapRowset.get(m_indexRowset).second;
                            }
                        }
                        if (row != null) {
                            row.invoke(this, m_vData);
                        }
                    }
                }
                m_vData.clear();
                break;
            case DB_CONSTS.idStartBLOB:
                if (mc.GetSize() > 0) {
                    m_Blob.SetSize(0);
                    int len = mc.LoadInt();
                    if (len != -1 && len > m_Blob.getMaxBufferSize()) {
                        m_Blob.Realloc(len);
                    }
                    m_Blob.Push(mc.getIntenalBuffer(), mc.getHeadPosition(), mc.GetSize());
                    mc.SetSize(0);
                }
                break;
            case DB_CONSTS.idChunk:
                if (mc.GetSize() > 0) {
                    m_Blob.Push(mc.getIntenalBuffer(), mc.GetSize());
                    mc.SetSize(0);
                }
                break;
            case DB_CONSTS.idEndBLOB:
                if (mc.GetSize() > 0 || m_Blob.GetSize() > 0) {
                    m_Blob.Push(mc.getIntenalBuffer(), mc.GetSize());
                    mc.SetSize(0);
                    int len = m_Blob.PeekInt(m_Blob.getHeadPosition() + 2);
                    if (len < 0 && len >= BLOB_LENGTH_NOT_AVAILABLE) {
                        //length should be reset if BLOB length not available from server side at beginning
                        len = m_Blob.GetSize() - 6; //sizeof(short) - sizeof(int);
                        m_Blob.ResetInt(len, 2); //sizeof(short)
                    }
                    Object vt = m_Blob.LoadObject();
                    m_vData.add(vt);
                }
                break;
            default:
                super.OnResultReturned(reqId, mc);
                break;
        }
    }
}
