package SPA.ServerSide;

public class CClientPeer extends CSocketPeer {

    class CServerPushExImpl implements SPA.IUPushEx {

        private final CSocketPeer m_sp;

        public CServerPushExImpl(CSocketPeer sp) {
            m_sp = sp;
        }

        @Override
        public boolean Publish(byte[] Message, int[] Groups) {
            int size;
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            if (Message == null) {
                size = 0;
            } else {
                size = (int) Message.length;
            }
            return ServerCoreLoader.SpeakEx(m_sp.getHandle(), Message, size, Groups, len);
        }

        @Override
        public boolean SendUserMessage(String UserId, byte[] Message) {
            int size;
            if (Message == null) {
                size = 0;
            } else {
                size = (int) Message.length;
            }
            return ServerCoreLoader.SendUserMessageEx(m_sp.getHandle(), UserId, Message, size);
        }

        @Override
        public boolean Publish(Object Message, int[] Groups) {
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            SPA.CScopeUQueue su = new SPA.CScopeUQueue();
            SPA.CUQueue q = su.getUQueue();
            q.Save(Message);
            return ServerCoreLoader.Speak(m_sp.getHandle(), q.getIntenalBuffer(), q.GetSize(), Groups, len);

        }

        @Override
        public boolean Subscribe(int[] Groups) {
            int len;
            if (Groups == null) {
                len = 0;
            } else {
                len = (int) Groups.length;
            }
            return ServerCoreLoader.Enter(m_sp.getHandle(), Groups, len);
        }

        @Override
        public boolean Unsubscribe() {
            ServerCoreLoader.Exit(m_sp.getHandle());
            return true;
        }

        @Override
        public boolean SendUserMessage(Object Message, String UserId) {
            SPA.CScopeUQueue su = new SPA.CScopeUQueue();
            SPA.CUQueue q = su.getUQueue();
            q.Save(Message);
            return ServerCoreLoader.SendUserMessage(m_sp.getHandle(), UserId, q.getIntenalBuffer(), q.GetSize());
        }
    }

    final CServerPushExImpl m_PushImpl;

    protected CClientPeer() {
        m_PushImpl = new CServerPushExImpl(this);
    }

    public final SPA.IUPushEx getPush() {
        return m_PushImpl;
    }

    public final boolean StartBatching() {
        return ServerCoreLoader.StartBatching(getHandle());
    }

    public final boolean CommitBatching() {
        return ServerCoreLoader.CommitBatching(getHandle());
    }

    public final boolean AbortBatching() {
        return ServerCoreLoader.AbortBatching(getHandle());
    }

    protected final int SendResult(short reqId) {
        return SendResult(reqId, (byte[]) null, 0);
    }

    protected final int SendResultIndex(long reqIndex, short reqId) {
        return SendResultIndex(reqIndex, reqId, (byte[]) null, 0);
    }

    protected int SendResult(short reqId, byte[] data, int len) {
        if (data == null) {
            len = 0;
        } else if (len > data.length) {
            len = data.length;
        }
        if (getRandom()) {
            return ServerCoreLoader.SendReturnDataIndex(getHandle(), getCurrentRequestIndex(), reqId, len, data);
        }
        return ServerCoreLoader.SendReturnData(getHandle(), reqId, len, data);
    }

    protected int SendResultIndex(long reqIndex, short reqId, byte[] data, int len) {
        if (data == null) {
            len = 0;
        } else if (len > data.length) {
            len = data.length;
        }
        return ServerCoreLoader.SendReturnDataIndex(getHandle(), reqIndex, reqId, len, data);
    }

    protected final int SendResult(short reqId, SPA.CUQueue q) {
        if (q == null) {
            return SendResult(reqId, (byte[]) null, 0);
        } else if (q.getHeadPosition() == 0) {
            return SendResult(reqId, q.getIntenalBuffer(), q.GetSize());
        }
        byte[] bytes = q.GetBuffer();
        return SendResult(reqId, bytes, bytes.length);
    }

    protected final int SendResultIndex(long reqIndex, short reqId, SPA.CUQueue q) {
        if (q == null) {
            return SendResultIndex(reqIndex, reqId, (byte[]) null, 0);
        } else if (q.getHeadPosition() == 0) {
            return SendResultIndex(reqIndex, reqId, q.getIntenalBuffer(), q.GetSize());
        }
        byte[] bytes = q.GetBuffer();
        return SendResultIndex(reqIndex, reqId, bytes, bytes.length);
    }

    protected final int SendResult(short reqId, SPA.CScopeUQueue su) {
        if (su == null) {
            return SendResult(reqId, (byte[]) null, 0);
        }
        return SendResult(reqId, su.getUQueue());
    }

    protected final int SendResultIndex(long reqIndex, short reqId, SPA.CScopeUQueue su) {
        if (su == null) {
            return SendResultIndex(reqIndex, reqId, (byte[]) null, 0);
        }
        return SendResultIndex(reqIndex, reqId, su.getUQueue());
    }

    /**
     * Dequeue messages from a persistent message queue
     *
     * @param qHandle A handle representing a server persistent message queue
     * @param messageCount An expected count of messages
     * @param bNotifiedWhenAvailable A boolean value if this peer client will be
     * notified once a message is available
     * @return A 8-byte long value. Its high-order 4-byte integer represents the
     * actual bytes of dequeued messages; and its low-order 4-byte integer is
     * the number of dequeued messages
     */
    public final long Dequeue(int qHandle, int messageCount, boolean bNotifiedWhenAvailable) {
        return Dequeue(qHandle, messageCount, bNotifiedWhenAvailable, (int) 0);
    }

    /**
     * Dequeue messages from a persistent message queue
     *
     * @param qHandle A handle representing a server persistent message queue
     * @param messageCount An expected count of messages
     * @param bNotifiedWhenAvailable A boolean value if this peer client will be
     * notified once a message is available
     * @param waitTime A time-out value in ms for waiting for a message. It
     * defaults to zero.
     * @return A 8-byte long value. Its high-order 4-byte integer represents the
     * actual bytes of dequeued messages; and its low-order 4-byte integer is
     * the number of dequeued messages
     */
    public long Dequeue(int qHandle, int messageCount, boolean bNotifiedWhenAvailable, int waitTime) {
        return ServerCoreLoader.Dequeue(qHandle, getHandle(), messageCount, bNotifiedWhenAvailable, waitTime);
    }

    /**
     * Dequeue messages from a persistent message queue
     *
     * @param qHandle A handle representing a server persistent message queue
     * @param bNotifiedWhenAvailable A boolean value if this peer client will be
     * notified once a message is available
     * @return A 8-byte long value. Its high-order 4-byte integer represents the
     * actual bytes of dequeued messages; and its low-order 4-byte integer is
     * the number of dequeued messages
     */
    public final long Dequeue(int qHandle, boolean bNotifiedWhenAvailable) {
        return Dequeue(qHandle, bNotifiedWhenAvailable, 8 * 1024, (int) 0);
    }

    /**
     * Dequeue messages from a persistent message queue
     *
     * @param qHandle A handle representing a server persistent message queue
     * @param bNotifiedWhenAvailable A boolean value if this peer client will be
     * notified once a message is available
     * @param maxBytes The max number of message bytes. It defaults to 8
     * kilobytes
     * @return A 8-byte long value. Its high-order 4-byte integer represents the
     * actual bytes of dequeued messages; and its low-order 4-byte integer is
     * the number of dequeued messages
     */
    public final long Dequeue(int qHandle, boolean bNotifiedWhenAvailable, int maxBytes) {
        return Dequeue(qHandle, bNotifiedWhenAvailable, maxBytes, (int) 0);
    }

    /**
     * Dequeue messages from a persistent message queue
     *
     * @param qHandle A handle representing a server persistent message queue
     * @param bNotifiedWhenAvailable A boolean value if this peer client will be
     * notified once a message is available
     * @param maxBytes The max number of message bytes. It defaults to 8
     * kilobytes
     * @param waitTime A time-out value in ms for waiting for a message. It
     * defaults to zero
     * @return A 8-byte long value. Its high-order 4-byte integer represents the
     * actual bytes of dequeued messages; and its low-order 4-byte integer is
     * the number of dequeued messages
     */
    public long Dequeue(int qHandle, boolean bNotifiedWhenAvailable, int maxBytes, int waitTime) {
        return ServerCoreLoader.Dequeue2(qHandle, getHandle(), maxBytes, bNotifiedWhenAvailable, waitTime);
    }

    /**
     * Enable or disable client side dequeue from server side
     *
     * @param enable True for enabling client side dequeue; and false for
     * disabling client side dequeue
     */
    public final void EnableClientDequeue(boolean enable) {
        ServerCoreLoader.EnableClientDequeue(getHandle(), enable);
    }

    public final SPA.tagZipLevel getZipLevel() {
        return SPA.tagZipLevel.forValue(ServerCoreLoader.GetZipLevel(getHandle()));
    }

    public final void setZipLevel(SPA.tagZipLevel value) {
        ServerCoreLoader.SetZipLevel(getHandle(), value.getValue());
    }

    public final boolean getZip() {
        return ServerCoreLoader.GetZip(getHandle());
    }

    public final void setZip(boolean value) {
        ServerCoreLoader.SetZip(getHandle(), value);
    }

    public final boolean getDequeuedMessageAborted() {
        return ServerCoreLoader.IsDequeuedMessageAborted(getHandle());
    }

    public final void AbortDequeuedMessage() {
        ServerCoreLoader.AbortDequeuedMessage(getHandle());
    }

    public final boolean getIsDequeueRequest() {
        return ServerCoreLoader.IsDequeueRequest(getHandle());
    }

    public final SPA.tagOperationSystem GetPeerOs(SPA.RefObject<Boolean> bigEndian) {
        if (bigEndian != null) {
            bigEndian.Value = m_endian;
        }
        return m_os;
    }

    public final SPA.tagOperationSystem GetPeerOs() {
        return m_os;
    }

    public final int getBytesBatched() {
        return ServerCoreLoader.GetBytesBatched(getHandle());
    }

    @Override
    protected void OnPublishEx(int[] groups, byte[] message) {

    }

    @Override
    protected void OnSendUserMessageEx(String receiver, byte[] message) {

    }

    @Override
    protected void OnFastRequestArrive(short requestId, int len) {

    }

    @Override
    protected int OnSlowRequestArrive(short requestId, int len) {
        return 0;
    }
}
