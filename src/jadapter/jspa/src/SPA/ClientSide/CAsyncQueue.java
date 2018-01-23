package SPA.ClientSide;

import SPA.*;

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

        void invoke(int errCode);
    }

    public interface DGetKeys {

        void invoke(String[] vKey);
    }

    public interface DFlush {

        void invoke(long messageCount, long fileSize);
    }

    public interface DEnqueue {

        void invoke(long indexMessage);
    }

    public interface DClose {

        void invoke(int errCode);
    }

    public interface DDequeue {

        void invoke(long messageCount, long fileSize, int messagesDequeuedInBatch, int bytesDequeuedInBatch);
    }

    public interface DMessageQueued {

        void invoke();
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
        return m_dDequeue;
    }

    private DAsyncResultHandler GetRH(final DEnqueue e) {
        if (e != null) {
            return new CAsyncServiceHandler.DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    e.invoke(ar.LoadLong());
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
            byte[] data = q.getIntenalBuffer();
            int n = (int) ((0xff & data[3]) << 24
                    | (0xff & data[2]) << 16
                    | (0xff & data[1]) << 8
                    | (0xff & data[0]));
            n += 1;
            data[3] = (byte) (n >>> 24);
            data[2] = (byte) (n >>> 16);
            data[1] = (byte) (n >>> 8);
            data[0] = (byte) n;
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
        } else if (src.getHeadPosition() > 0) {
            message = src.GetBuffer();
            len = src.getSize();
            src.SetSize(0);
        } else {
            message = src.getIntenalBuffer();
            len = src.getSize();
            src.SetSize(0);
        }
        BatchMessage(idMessage, message, len, q);
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q, DEnqueue e, DDiscarded discarded) {
        if (key == null) {
            key = new byte[0];
        }
        if (q == null || q.getSize() < 8) {
            throw new java.lang.IllegalArgumentException("Bad operation");
        }
        CUQueue b = CScopeUQueue.Lock();
        b.Save(key).Push(q.getIntenalBuffer(), q.getHeadPosition(), q.getSize());
        q.SetSize(0);
        boolean ok = SendRequest(idEnqueueBatch, b, GetRH(e), discarded);
        CScopeUQueue.Unlock(b);
        return ok;
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q, DEnqueue e) {
        return EnqueueBatch(key, q, e, null);
    }

    public boolean EnqueueBatch(byte[] key, CUQueue q) {
        return EnqueueBatch(key, q, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage) {
        return Enqueue(key, idMessage, (byte[]) null, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, DEnqueue e) {
        return Enqueue(key, idMessage, (byte[]) null, e, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, DEnqueue e, DDiscarded discarded) {
        return Enqueue(key, idMessage, (byte[]) null, e, discarded);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes) {
        return Enqueue(key, idMessage, bytes, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes, DEnqueue e) {
        return Enqueue(key, idMessage, bytes, e, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, byte[] bytes, DEnqueue e, DDiscarded discarded) {
        if (key == null) {
            key = new byte[0];
        }
        CUQueue q = CScopeUQueue.Lock();
        q.Save(key).Save(idMessage).Push(bytes);
        boolean ok = SendRequest(idEnqueue, q, GetRH(e), discarded);
        CScopeUQueue.Unlock(q);
        return ok;
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q) {
        return Enqueue(key, idMessage, q, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q, DEnqueue e) {
        return Enqueue(key, idMessage, q, e, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CUQueue q, DEnqueue e, DDiscarded discarded) {
        if (key == null) {
            key = new byte[0];
        }
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key).Save(idMessage).Push(q.getIntenalBuffer(), q.getHeadPosition(), q.getSize());
        boolean ok = SendRequest(idEnqueue, sq, GetRH(e), discarded);
        CScopeUQueue.Unlock(sq);
        return ok;
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q) {
        return Enqueue(key, idMessage, q, null, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q, DEnqueue e) {
        return Enqueue(key, idMessage, q, e, null);
    }

    public boolean Enqueue(byte[] key, short idMessage, CScopeUQueue q, DEnqueue e, DDiscarded discarded) {
        if (key == null) {
            key = new byte[0];
        }
        CUQueue src = q.getUQueue();
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key).Save(idMessage).Push(src.getIntenalBuffer(), src.getHeadPosition(), src.getSize());
        boolean ok = SendRequest(idEnqueue, sq, GetRH(e), discarded);
        CScopeUQueue.Unlock(sq);
        return ok;
    }

    static private final java.nio.charset.Charset UTF8_CHARSET = java.nio.charset.Charset.forName("UTF-8");

    /**
     * Start enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param key An ASCII string for identifying a queue at server side
     * @return true for sending the request successfully, and false for failure
     */
    public boolean StartQueueTrans(byte[] key) {
        return StartQueueTrans(key, null, null);
    }

    /**
     * Start enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
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
     * Start enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean StartQueueTrans(byte[] key, final DQueueTrans qt, DDiscarded discarded) {
        if (key == null) {
            key = new byte[0];
        }
        IClientQueue cq = this.getAttachedClientSocket().getClientQueue();
        if (cq.getAvailable()) {
            cq.StartJob();
        }
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key);
        boolean ok = SendRequest(idStartTrans, sq, new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                if (qt != null) {
                    qt.invoke(ar.LoadInt());
                } else {
                    ar.getUQueue().SetSize(0);
                }
            }
        }, discarded);
        CScopeUQueue.Unlock(sq);
        return ok;
    }

    /**
     * Commit enqueuing messages with transaction style. Currently, total size
     * of queued messages must be less than 4 G bytes
     *
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans() {
        return EndQueueTrans(false, null, null);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param rollback true for rollback, and false for committing
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans(boolean rollback) {
        return EndQueueTrans(rollback, null, null);
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
        return EndQueueTrans(rollback, qt, null);
    }

    /**
     * End enqueuing messages with transaction style. Currently, total size of
     * queued messages must be less than 4 G bytes
     *
     * @param rollback true for rollback, and false for committing
     * @param qt A callback for tracking returning error code, which can be one
     * of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean EndQueueTrans(boolean rollback, final DQueueTrans qt, DDiscarded discarded) {
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(rollback);
        boolean ok = SendRequest(idEndTrans, sq, new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                if (qt != null) {
                    qt.invoke(ar.LoadInt());
                } else {
                    ar.getUQueue().SetSize(0);
                }
            }
        }, discarded);
        CScopeUQueue.Unlock(sq);
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
    public boolean GetKeys(final DGetKeys gk, DDiscarded discarded) {
        return SendRequest(idGetKeys, new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                if (gk != null) {
                    int size = ar.LoadInt();
                    String[] v = new String[size];
                    for (int n = 0; n < size; ++n) {
                        byte[] bytes = ar.LoadBytes();
                        v[n] = new String(bytes, UTF8_CHARSET);
                    }
                    gk.invoke(v);
                } else {
                    ar.getUQueue().SetSize(0);
                }
            }
        }, discarded);
    }

    /**
     * *
     * Try to close a persistent queue opened at server side
     *
     * @param key An ASCII string for identifying a queue at server side
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key) {
        return CloseQueue(key, null, null, false);
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
        return CloseQueue(key, c, null, false);
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
        return CloseQueue(key, c, discarded, false);
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
     * queue file
     * @return true for sending the request successfully, and false for failure
     */
    public boolean CloseQueue(byte[] key, final DClose c, DDiscarded discarded, boolean permanent) {
        if (key == null) {
            key = new byte[0];
        }
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key).Save(permanent);
        boolean ok = SendRequest(idClose, sq, new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                if (c != null) {
                    c.invoke(ar.LoadInt());
                } else {
                    ar.getUQueue().SetSize(0);
                }
            }
        }, discarded);
        CScopeUQueue.Unlock(sq);
        return ok;
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
     * oDiskCommitted
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
     * oDiskCommitted
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean FlushQueue(byte[] key, final DFlush f, tagOptimistic option, DDiscarded discarded) {
        if (key == null) {
            key = new byte[0];
        }
        CUQueue sq = CScopeUQueue.Lock();
        sq.Save(key).Save(option.getValue());
        boolean ok = SendRequest(idFlush, new CAsyncServiceHandler.DAsyncResultHandler() {
            @Override
            public void invoke(CAsyncResult ar) {
                if (f != null) {
                    long messageCount = ar.LoadLong();
                    long fileSize = ar.LoadLong();
                    f.invoke(messageCount, fileSize);
                } else {
                    ar.getUQueue().SetSize(0);
                }
            }
        }, discarded);
        CScopeUQueue.Unlock(sq);
        return ok;
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
        return Dequeue(key, d, 0, null);
    }

    /**
     * Dequeue messages from a persistent message queue file at server side in
     * batch
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param d A callback for tracking data like remaining message count within
     * a server queue file, queue file size in bytes, message dequeued within
     * this batch and bytes dequeued within this batch
     * @param timeout A time-out number in milliseconds
     * @return true for sending the request successfully, and false for failure
     */
    public boolean Dequeue(byte[] key, DDequeue d, int timeout) {
        return Dequeue(key, d, timeout, null);
    }

    /**
     * *
     * Dequeue messages from a persistent message queue file at server side in
     * batch
     *
     * @param key An ASCII string for identifying a queue at server side
     * @param d A callback for tracking data like remaining message count within
     * a server queue file, queue file size in bytes, message dequeued within
     * this batch and bytes dequeued within this batch
     * @param timeout A time-out number in milliseconds
     * @param discarded a callback for tracking socket closed or request cancel
     * event
     * @return true for sending the request successfully, and false for failure
     */
    public boolean Dequeue(byte[] key, final DDequeue d, int timeout, DDiscarded discarded) {
        DAsyncResultHandler rh = null;
        if (key == null) {
            key = new byte[0];
        }
        synchronized (m_csQ) {
            m_keyDequeue = key;
            if (d != null) {
                rh = new CAsyncServiceHandler.DAsyncResultHandler() {
                    @Override
                    public void invoke(CAsyncResult ar) {
                        long messageCount = ar.LoadLong(), fileSize = ar.LoadLong(), ret = ar.LoadLong();
                        int messages = (int) ret;
                        int bytes = (int) (ret >> 32);
                        d.invoke(messageCount, fileSize, messages, bytes);
                    }
                };
                m_dDequeue = d;
            } else {
                m_dDequeue = null;
            }
            CUQueue sq = CScopeUQueue.Lock();
            sq.Save(key).Save(timeout);
            boolean ok = SendRequest(idDequeue, sq, rh, discarded);
            CScopeUQueue.Unlock(sq);
            return ok;
        }
    }

    @Override
    protected void OnBaseRequestProcessed(short reqId) {
        switch (reqId) {
            case 24: //tagBaseRequestID.idMessageQueued.getValue()
                synchronized (m_csQ) {
                    if (m_dDequeue != null) {
                        //we send a request to dequeue messages after a notification message arrives that a new message is enqueued at server side
                        Dequeue(m_keyDequeue, m_dDequeue, 0);
                    }
                }
                if (MessageQueued != null) {
                    MessageQueued.invoke();
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
