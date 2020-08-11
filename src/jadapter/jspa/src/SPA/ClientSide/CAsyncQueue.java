package SPA.ClientSide;

import SPA.*;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

/**
 * A client side class for easy accessing remote persistent message queues by
 * use of SocketPro communication framework
 *
 * @author UDAParts
 */
public class CAsyncQueue extends CAsyncServiceHandler {

    public final static int sidQueue = BaseServiceID.sidChat;

    public CAsyncQueue() {
        super(sidQueue);
    }

    public CAsyncQueue(int sid) {
        super(sid);
    }

    public static final short idEnqueue = tagBaseRequestID.idReservedTwo + 1;
    public static final short idDequeue = tagBaseRequestID.idReservedTwo + 2;
    public static final short idStartTrans = tagBaseRequestID.idReservedTwo + 3;
    public static final short idEndTrans = tagBaseRequestID.idReservedTwo + 4;
    public static final short idFlush = tagBaseRequestID.idReservedTwo + 5;
    public static final short idClose = tagBaseRequestID.idReservedTwo + 6;
    public static final short idGetKeys = tagBaseRequestID.idReservedTwo + 7;
    public static final short idEnqueueBatch = tagBaseRequestID.idReservedTwo + 8;

    //this id is designed for notifying dequeue batch size from server to client
    public static final short idBatchSizeNotified = tagBaseRequestID.idReservedTwo + 20;

    //error code
    public final static int QUEUE_OK = 0;
    public final static int QUEUE_TRANS_ALREADY_STARTED = 1;
    public final static int QUEUE_TRANS_STARTING_FAILED = 2;
    public final static int QUEUE_TRANS_NOT_STARTED_YET = 3;
    public final static int QUEUE_TRANS_COMMITTING_FAILED = 4;
    public final static int QUEUE_DEQUEUING = 5;
    public final static int QUEUE_OTHER_WORKING_WITH_SAME_QUEUE = 6;
    public final static int QUEUE_CLOSE_FAILED = 7;
    public final static int QUEUE_ENQUEUING_FAILED = 8;

    //callback definitions
    public interface DQueueTrans {

        void invoke(CAsyncQueue aq, int errCode);
    }

    public interface DGetKeys {

        void invoke(CAsyncQueue aq, String[] vKey);
    }

    public interface DFlush {

        void invoke(CAsyncQueue aq, long messageCount, long fileSize);
    }

    public interface DEnqueue {

        void invoke(CAsyncQueue aq, long indexMessage);
    }

    public interface DClose {

        void invoke(CAsyncQueue aq, int errCode);
    }

    public interface DDequeue {

        void invoke(CAsyncQueue aq, long messageCount, long fileSize, int messagesDequeuedInBatch, int bytesDequeuedInBatch);
    }

    public interface DMessageQueued {

        void invoke(CAsyncQueue aq);
    }

    private int m_nBatchSize = 0;
    final private Object m_csQ = new Object();
    private byte[] m_keyDequeue = new byte[0]; //protected by m_csQ
    private DDequeue m_dDequeue = null; //protected by m_csQ

    /**
     * An event for tracking message queued notification from server side
     */
    public DMessageQueued MessageQueued;

    /**
     * Query dequeue batch size in bytes
     *
     * @return dequeue batch size in bytes
     */
    public final int getDequeueBatchSize() {
        return (m_nBatchSize & 0xffffff);
    }

    /**
     * Check if remote queue server is able to automatically notify a client
     * when a message is enqueued at server side
     *
     * @return true if remote queue server will automatically notify a client,
     * and false if remote queue server will not
     */
    public final boolean getEnqueueNotified() {
        return ((m_nBatchSize >> 24) == 0);
    }

    /**
     * Get last dequeue callback
     *
     * @return last dequeue callback
     */
    public final DDequeue getLastDequeueCallback() {
        synchronized (m_csQ) {
            return m_dDequeue;
        }
    }

    /**
     * Set dequeue callback
     *
     * @param deq a callback for monitoring queue attributes at a server queue
     * file
     */
    public final void setLastDequeueCallback(DDequeue deq) {
        synchronized (m_csQ) {
            m_dDequeue = deq;
        }
    }

    private static DAsyncResultHandler GetRH(final DEnqueue e) {
        if (e != null) {
            return new CAsyncServiceHandler.DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    e.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), ar.LoadLong());
                }
            };
        }
        return null;
    }

    public static void BatchMessage(short idMessage, byte[] message, int len, CUQueue q) {
        if (message == null) {
            message = new byte[0];
            len = 0;
        } else if (len > message.length) {
            len = message.length;
        }
        if (q.GetSize() == 0) {
            int count = 1;
            q.Save(count);
        } else {
            int n = q.PeekInt(0);
            n += 1;
            q.ResetInt(n, 0);
        }
        q.Save(idMessage).Save(len);
        q.Push(message, len);
    }

    public static void BatchMessage(short idMessage, byte[] message, CUQueue q) {
        int len;
        if (message == null) {
            message = new byte[0];
            len = 0;
        } else {
            len = message.length;
        }
        BatchMessage(idMessage, message, len, q);
    }

    public static void BatchMessage(short idMessage, CUQueue q) {
        BatchMessage(idMessage, (byte[]) null, 0, q);
    }

    public static void BatchMessage(short idMessage, CUQueue src, CUQueue q) {
        byte[] message;
        int len;
        if (src == null) {
            message = new byte[0];
            len = 0;
        } else {
            message = src.GetBuffer();
            len = src.getSize();
            src.SetSize(0);
        }
        BatchMessage(idMessage, message, len, q);
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q, DEnqueue e, DDiscarded discarded, DOnExceptionFromServer se) {
        if (q == null || q.getSize() < 8) {
            throw new java.lang.IllegalArgumentException("Bad operation");
        }
        CUQueue b = CScopeUQueue.Lock();
        b.Save(key).Push(q.getIntenalBuffer());
        q.SetSize(0);
        boolean ok = SendRequest(idEnqueueBatch, b, GetRH(e), discarded, se);
        CScopeUQueue.Unlock(b);
        return ok;
    }

    public Future<Long> enqueueBatch(byte[] key, short idMessage, CUQueue q) throws ExecutionException {
        UFuture<Long> f = new UFuture<>();
        DEnqueue e = new DEnqueue() {
            @Override
            public void invoke(CAsyncQueue aq, long indexMessage) {
                f.set(indexMessage);
            }
        };
        if (!EnqueueBatch(key, q, e, getAborted(f, "EnqueueBatch", idEnqueueBatch), getSE(f))) {
            raise("EnqueueBatch", idEnqueueBatch);
        }
        return f;
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q, DEnqueue e, DDiscarded discarded) {
        return EnqueueBatch(key, q, e, discarded, null);
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q, DEnqueue e) {
        return EnqueueBatch(key, q, e, null, null);
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q) {
        return EnqueueBatch(key, q, null, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage) {
        return Enqueue(key, idMessage, (byte[]) null, null, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, DEnqueue e) {
        return Enqueue(key, idMessage, (byte[]) null, e, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, DEnqueue e, DDiscarded discarded) {
        return Enqueue(key, idMessage, (byte[]) null, e, discarded, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, DEnqueue e, DDiscarded discarded, DOnExceptionFromServer se) {
        return Enqueue(key, idMessage, (byte[]) null, e, discarded, se);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes) {
        return Enqueue(key, idMessage, bytes, null, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes, DEnqueue e) {
        return Enqueue(key, idMessage, bytes, e, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes, DEnqueue e, DDiscarded discarded) {
        return Enqueue(key, idMessage, bytes, e, discarded, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes, DEnqueue e, DDiscarded discarded, DOnExceptionFromServer se) {
        CUQueue q = CScopeUQueue.Lock();
        q.Save(key).Save(idMessage).Push(bytes);
        boolean ok = SendRequest(idEnqueue, q, GetRH(e), discarded, se);
        CScopeUQueue.Unlock(q);
        return ok;
    }

    public Future<Long> enqueue(byte[] key, short idMessage, byte[] bytes) throws ExecutionException {
        UFuture<Long> f = new UFuture<>();
        DEnqueue e = new DEnqueue() {
            @Override
            public void invoke(CAsyncQueue aq, long indexMessage) {
                f.set(indexMessage);
            }
        };
        if (!Enqueue(key, idMessage, bytes, e, getAborted(f, "Enqueue", idEnqueue), getSE(f))) {
            raise("Enqueue", idEnqueue);
        }
        return f;
    }

    public Future<Long> enqueue(byte[] key, short idMessage) throws ExecutionException {
        return enqueue(key, idMessage, (byte[]) null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q) {
        return Enqueue(key, idMessage, q, null, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q, DEnqueue e) {
        return Enqueue(key, idMessage, q, e, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q, DEnqueue e, DDiscarded discarded) {
        return Enqueue(key, idMessage, q, e, discarded, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q, DEnqueue e, DDiscarded discarded, DOnExceptionFromServer se) {
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key).Save(idMessage).Push(q.getIntenalBuffer());
        boolean ok = SendRequest(idEnqueue, sq, GetRH(e), discarded, se);
        CScopeUQueue.Unlock(sq);
        return ok;
    }

    public Future<Long> enqueue(byte[] key, short idMessage, CUQueue q) throws ExecutionException {
        UFuture<Long> f = new UFuture<>();
        DEnqueue e = new DEnqueue() {
            @Override
            public void invoke(CAsyncQueue aq, long indexMessage) {
                f.set(indexMessage);
            }
        };
        if (!Enqueue(key, idMessage, q, e, getAborted(f, "Enqueue", idEnqueue), getSE(f))) {
            raise("Enqueue", idEnqueue);
        }
        return f;
    }

    public Future<Long> enqueue(byte[] key, short idMessage, CScopeUQueue q) throws ExecutionException {
        return enqueue(key, idMessage, q.getUQueue());
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q) {
        return Enqueue(key, idMessage, q.getUQueue(), null, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q, DEnqueue e) {
        return Enqueue(key, idMessage, q.getUQueue(), e, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q, DEnqueue e, DDiscarded discarded) {
        return Enqueue(key, idMessage, q.getUQueue(), e, discarded, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q, DEnqueue e, DDiscarded discarded, DOnExceptionFromServer se) {
        return Enqueue(key, idMessage, q.getUQueue(), e, discarded, se);
    }

    static private final java.nio.charset.Charset UTF8_CHARSET = java.nio.charset.Charset.forName("UTF-8");

    /**
     * Start to enqueue messages with transaction style. Currently, total size
     * of queued messages must be less than 4 G bytes
     *
     * @param key An ASCII string for identifying a queue at server side
     * @return true for sending the request successfully, and false for failure
     */
    public boolean StartQueueTrans(byte[] key) {
        return StartQueueTrans(key, null, null);
    }

    /**
     * Start to enqueue messages with transaction style. Currently, total size
     * of queued messages must be less than 4 G bytes
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     * @return true for sending the request successfully, and false for failure
     */
    public boolean StartQueueTrans(byte[] key, DQueueTrans qt) {
        return StartQueueTrans(key, qt, null);
    }

    /**
     * Start to enqueue messages with transaction style. Currently, total size
     * of queued messages must be less than 4 G bytes
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean StartQueueTrans(byte[] key, DQueueTrans qt, DDiscarded discarded) {
        return StartQueueTrans(key, qt, discarded, null);
    }

    /**
     * Start to enqueue messages with transaction style. Currently, total size
     * of queued messages must be less than 4 G bytes
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param se A callback for tracking an exception from server
     * @return true for sending the request successfully, and false for failure
     */
    public boolean StartQueueTrans(byte[] key, DQueueTrans qt, DDiscarded discarded, DOnExceptionFromServer se) {
        IClientQueue cq = this.getAttachedClientSocket().getClientQueue();
        if (cq.getAvailable()) {
            cq.StartJob();
        }
        try (CScopeUQueue sq = new CScopeUQueue()) {
            return SendRequest(idStartTrans, sq.Save(key), new DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    if (qt != null) {
                        qt.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), ar.LoadInt());
                    } else {
                        ar.getUQueue().SetSize(0);
                    }
                }
            }, discarded, null);
        }
    }

    public Future<Integer> startQueueTrans(byte[] key) throws ExecutionException {
        UFuture<Integer> f = new UFuture<>();
        DQueueTrans qt = new DQueueTrans() {
            @Override
            public void invoke(CAsyncQueue aq, int errCode) {
                f.set(errCode);
            }
        };
        if (!StartQueueTrans(key, qt, getAborted(f, "StartQueueTrans", idStartTrans), getSE(f))) {
            raise("StartQueueTrans", idStartTrans);
        }
        return f;
    }

    /**
     * Commit messages with transaction style. Currently, total size of queued
     * messages must be less than 4 G bytes
     *
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans() {
        return EndQueueTrans(false, null, null, null);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param rollback true for rollback, and false for committing
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans(boolean rollback) {
        return EndQueueTrans(rollback, null, null, null);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param rollback true for rollback, and false for committing
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans(boolean rollback, DQueueTrans qt) {
        return EndQueueTrans(rollback, qt, null, null);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param rollback true for rollback, and false for committing. It defaults
     * to false
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans(boolean rollback, DQueueTrans qt, DDiscarded discarded) {
        return EndQueueTrans(rollback, qt, discarded, null);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param rollback true for rollback, and false for committing. It defaults
     * to false
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param se A callback for tracking an exception from server
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans(boolean rollback, DQueueTrans qt, DDiscarded discarded, DOnExceptionFromServer se) {
        try (CScopeUQueue sq = new CScopeUQueue()) {
            boolean ok = SendRequest(idEndTrans, sq.Save(rollback), new DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    if (qt != null) {
                        qt.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), ar.LoadInt());
                    } else {
                        ar.getUQueue().SetSize(0);
                    }
                }
            }, discarded, se);
            IClientQueue cq = this.getAttachedClientSocket().getClientQueue();
            if (cq.getAvailable()) {
                if (rollback) {
                    cq.AbortJob();
                } else {
                    cq.EndJob();
                }
            }
            return ok;
        }
    }

    public Future<Integer> endQueueTrans() throws ExecutionException {
        return endQueueTrans(false);
    }

    public Future<Integer> endQueueTrans(boolean rollback) throws ExecutionException {
        UFuture<Integer> f = new UFuture<>();
        DQueueTrans qt = new DQueueTrans() {
            @Override
            public void invoke(CAsyncQueue aq, int errCode) {
                f.set(errCode);
            }
        };
        if (!EndQueueTrans(rollback, qt, getAborted(f, "EndQueueTrans", idEndTrans), getSE(f))) {
            raise("EndQueueTrans", idEndTrans);
        }
        return f;
    }

    /**
     * Query queue keys opened at server side
     *
     * @param gk A callback for tracking a list of key names
     * @return true for sending the request successfully, and false for failure
     */
    public boolean GetKeys(DGetKeys gk) {
        return GetKeys(gk, null);
    }

    /**
     * Query queue keys opened at server side
     *
     * @param gk A callback for tracking a list of key names
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean GetKeys(DGetKeys gk, DDiscarded discarded) {
        return GetKeys(gk, discarded, null);
    }

    /**
     * Query queue keys opened at server side
     *
     * @param gk A callback for tracking a list of key names
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param se A callback for tracking an exception from server
     * @return true for sending the request successfully, and false for failure
     */
    public boolean GetKeys(DGetKeys gk, DDiscarded discarded, DOnExceptionFromServer se) {
        return SendRequest(idGetKeys, new DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                if (gk != null) {
                    int size = ar.LoadInt();
                    String[] v = new String[size];
                    for (int n = 0; n < size; ++n) {
                        byte[] bytes = ar.LoadBytes();
                        v[n] = new String(bytes, UTF8_CHARSET);
                    }
                    gk.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), v);
                } else {
                    ar.getUQueue().SetSize(0);
                }
            }
        }, discarded, null);
    }

    public Future<String[]> getKeys() throws ExecutionException {
        UFuture<String[]> f = new UFuture<>();
        DGetKeys gk = new DGetKeys() {
            @Override
            public void invoke(CAsyncQueue aq, String[] vKey) {
                f.set(vKey);
            }
        };
        if (!GetKeys(gk, getAborted(f, "GetKeys", idGetKeys), getSE(f))) {
            raise("GetKeys", idGetKeys);
        }
        return f;
    }

    /**
     * *
     * Try to close a persistent queue opened at server side
     *
     * @param key An ASCII string for identifying a queue at server side
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key) {
        return CloseQueue(key, null, null, false, null);
    }

    /**
     * *
     * Try to close a persistent queue opened at server side
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param c A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_DEQUEUING, and so on
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key, DClose c) {
        return CloseQueue(key, c, null, false, null);
    }

    /**
     * Try to close or delete a persistent queue opened at server side
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param c A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_DEQUEUING, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key, DClose c, DDiscarded discarded) {
        return CloseQueue(key, c, discarded, false, null);
    }

    /**
     * Try to close or delete a persistent queue opened at server side
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param c A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_DEQUEUING, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param permanent true for deleting a queue file, and false for closing a
     * queue file. It defaults to false
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key, DClose c, DDiscarded discarded, boolean permanent) {
        return CloseQueue(key, c, discarded, permanent, null);
    }

    /**
     * Try to close or delete a persistent queue opened at server side
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param c A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_DEQUEUING, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param permanent true for deleting a queue file, and false for closing a
     * queue file. It defaults to false
     * @param se A callback for tracking an exception from server
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key, final DClose c, DDiscarded discarded, boolean permanent, DOnExceptionFromServer se) {
        try (CScopeUQueue sq = new CScopeUQueue()) {
            sq.Save(key).Save(permanent);
            return SendRequest(idClose, sq, new DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    if (c != null) {
                        c.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), ar.LoadInt());
                    } else {
                        ar.getUQueue().SetSize(0);
                    }
                }
            }, discarded, se);
        }
    }

    public Future<Integer> closeQueue(byte[] key, boolean permanent) throws ExecutionException {
        UFuture<Integer> f = new UFuture<>();
        DClose c = new DClose() {
            @Override
            public void invoke(CAsyncQueue aq, int errCode) {
                f.set(errCode);
            }
        };
        if (!CloseQueue(key, c, getAborted(f, "CloseQueue", idClose), permanent, getSE(f))) {
            raise("CloseQueue", idClose);
        }
        return f;
    }

    public Future<Integer> closeQueue(byte[] key) throws ExecutionException {
        return closeQueue(key, false);
    }

    /**
     * Just get message count and queue file size in bytes only
     *
     * @param key An ASCII string for identifying a queue at server side
     * @return true for sending the request successfully, and false for failure
     */
    public boolean FlushQueue(byte[] key) {
        return FlushQueue(key, null, tagOptimistic.oMemoryCached, null);
    }

    /**
     * Just get message count and queue file size in bytes only
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param f A callback for tracking returning message count and queue file
     * size in bytes
     * @return true for sending the request successfully, and false for failure
     */
    public boolean FlushQueue(byte[] key, DFlush f) {
        return FlushQueue(key, f, tagOptimistic.oMemoryCached, null);
    }

    /**
     * May flush memory data into either operation system memory or hard disk,
     * and return message count and queue file size in bytes. Note the method
     * only returns message count and queue file size in bytes if the option is
     * oMemoryCached
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param f A callback for tracking returning message count and queue file
     * size in bytes
     * @param option one of options, oMemoryCached, oSystemMemoryCached and
     * oDiskCommitted. It defaults to tagOptimistic.oMemoryCached
     * @return true for sending the request successfully, and false for failure
     */
    public boolean FlushQueue(byte[] key, DFlush f, tagOptimistic option) {
        return FlushQueue(key, f, option, null);
    }

    /**
     * May flush memory data into either operation system memory or hard disk,
     * and return message count and queue file size in bytes. Note the method
     * only returns message count and queue file size in bytes if the option is
     * oMemoryCached
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param f A callback for tracking returning message count and queue file
     * size in bytes
     * @param option one of options, oMemoryCached, oSystemMemoryCached and
     * oDiskCommitted. It defaults to tagOptimistic.oMemoryCached
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean FlushQueue(byte[] key, DFlush f, tagOptimistic option, DDiscarded discarded) {
        return FlushQueue(key, f, option, discarded, null);
    }

    /**
     * May flush memory data into either operation system memory or hard disk,
     * and return message count and queue file size in bytes. Note the method
     * only returns message count and queue file size in bytes if the option is
     * oMemoryCached
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param f A callback for tracking returning message count and queue file
     * size in bytes
     * @param option one of options, oMemoryCached, oSystemMemoryCached and
     * oDiskCommitted. It defaults to tagOptimistic.oMemoryCached
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param se A callback for tracking an exception from server
     * @return true for sending the request successfully, and false for failure
     */
    public boolean FlushQueue(byte[] key, DFlush f, tagOptimistic option, DDiscarded discarded, DOnExceptionFromServer se) {
        try (CScopeUQueue sq = new CScopeUQueue()) {
            sq.Save(key).Save(option.getValue());
            return SendRequest(idFlush, new DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    if (f != null) {
                        long messageCount = ar.LoadLong();
                        long fileSize = ar.LoadLong();
                        f.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), messageCount, fileSize);
                    } else {
                        ar.getUQueue().SetSize(0);
                    }
                }
            }, discarded, se);
        }
    }

    public class QueueInfo {

        /**
         * The messages remaining in server message queue file
         */
        public long messages;

        /**
         * server message queue file in bytes
         */
        public long fSize;

        QueueInfo(long message_count, long file_size) {
            messages = message_count;
            fSize = file_size;
        }
    }

    public Future<QueueInfo> flushQueue(byte[] key, tagOptimistic option) throws ExecutionException {
        UFuture<QueueInfo> f = new UFuture<>();
        DFlush df = new DFlush() {
            @Override
            public void invoke(CAsyncQueue aq, long messageCount, long fileSize) {
                f.set(new QueueInfo(messageCount, fileSize));
            }
        };
        if (!FlushQueue(key, df, option, getAborted(f, "FlushQueue", idFlush), getSE(f))) {
            raise("FlushQueue", idFlush);
        }
        return f;
    }

    public Future<QueueInfo> flushQueue(byte[] key) throws ExecutionException {
        return flushQueue(key, tagOptimistic.oMemoryCached);
    }

    /**
     * Dequeue messages from a persistent message queue file at server side in
     * batch without waiting
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param d A callback for tracking data like remaining message count within
     * a server queue file, queue file size in bytes, message dequeued within
     * this batch and bytes dequeued within this batch
     * @return true for sending the request successfully, and false for failure
     */
    public boolean Dequeue(byte[] key, DDequeue d) {
        return Dequeue(key, d, 0, null, null);
    }

    /**
     * Dequeue messages from a persistent message queue file at server side in
     * batch
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param d A callback for tracking data like remaining message count within
     * a server queue file, queue file size in bytes, message dequeued within
     * this batch and bytes dequeued within this batch
     * @param timeout A time-out number in milliseconds. It defaults to zero
     * @return true for sending the request successfully, and false for failure
     */
    public boolean Dequeue(byte[] key, DDequeue d, int timeout) {
        return Dequeue(key, d, timeout, null, null);
    }

    /**
     * Dequeue messages from a persistent message queue file at server side in
     * batch
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param d A callback for tracking data like remaining message count within
     * a server queue file, queue file size in bytes, message dequeued within
     * this batch and bytes dequeued within this batch
     * @param timeout A time-out number in milliseconds. It defaults to zero
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean Dequeue(byte[] key, DDequeue d, int timeout, DDiscarded discarded) {
        return Dequeue(key, d, timeout, discarded, null);
    }

    /**
     * Dequeue messages from a persistent message queue file at server side in
     * batch
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param d A callback for tracking data like remaining message count within
     * a server queue file, queue file size in bytes, message dequeued within
     * this batch and bytes dequeued within this batch
     * @param timeout A time-out number in milliseconds. It defaults to zero
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @param se A callback for tracking an exception from server
     * @return true for sending the request successfully, and false for failure
     */
    public boolean Dequeue(byte[] key, DDequeue d, int timeout, DDiscarded discarded, DOnExceptionFromServer se) {
        DAsyncResultHandler rh = null;
        synchronized (m_csQ) {
            m_keyDequeue = key;
            if (d != null) {
                rh = new DAsyncResultHandler() {
                    @Override
                    public void invoke(CAsyncResult ar) {
                        long messageCount = ar.LoadLong(), fileSize = ar.LoadLong(), ret = ar.LoadLong();
                        int messages = (int) ret;
                        int bytes = (int) (ret >> 32);
                        d.invoke((CAsyncQueue) ar.getAsyncServiceHandler(), messageCount, fileSize, messages, bytes);
                    }
                };
                m_dDequeue = d;
            } else {
                m_dDequeue = null;
            }
        }
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key).Save(timeout);
        boolean ok = SendRequest(idDequeue, sq, rh, discarded, se);
        CScopeUQueue.Unlock(sq);
        return ok;
    }

    public class DeqInfo extends QueueInfo {

        /**
         * messages dequeued from server by this request Dequeue
         */
        public int DeMessages;

        /**
         * bytes dequeued from server by this request Dequeue
         */
        public int DeBytes;

        DeqInfo(long messages, long fSize, int msgs, int bytes) {
            super(messages, fSize);
            DeMessages = msgs;
            DeBytes = bytes;
        }
    }

    public Future<DeqInfo> dequeue(byte[] key, int timeout) throws ExecutionException {
        UFuture<DeqInfo> f = new UFuture<>();
        DDequeue d = new DDequeue() {
            @Override
            public void invoke(CAsyncQueue aq, long messageCount, long fileSize, int messages, int bytes) {
                f.set(new DeqInfo(messageCount, fileSize, messages, bytes));
            }
        };
        if (!Dequeue(key, d, timeout, getAborted(f, "Dequeue", idDequeue), getSE(f))) {
            raise("Dequeue", idDequeue);
        }
        return f;
    }

    @Override
    protected void OnBaseRequestProcessed(short reqId) {
        switch (reqId) {
            case 24: //tagBaseRequestID.idMessageQueued.getValue()
                DDequeue deq;
                byte[] key;
                synchronized (m_csQ) {
                    deq = m_dDequeue;
                    key = m_keyDequeue;
                }
                if (deq != null) {
                    //we send a request to dequeue messages after a notification message arrives that a new message is enqueued at server side
                    Dequeue(key, deq, 0);
                }
                if (MessageQueued != null) {
                    MessageQueued.invoke(this);
                }
                break;
            default:
                break;
        }
    }

    @Override
    protected void OnResultReturned(short reqId, CUQueue mc) {
        switch (reqId) {
            case idClose:
            case idEnqueue:
                mc.SetSize(0);
                break;
            case idBatchSizeNotified:
                m_nBatchSize = mc.LoadInt();
                break;
            default:
                break;
        }
    }
}
