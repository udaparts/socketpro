package SPA.ServerSide;

import SPA.CScopeUQueue;

public class CServerQueue implements IServerQueue {

    private int m_nHandle = 0;

    @Override
    protected void finalize() throws Throwable {
        Clean();
        super.finalize();
    }

    public CServerQueue() {
    }

    public CServerQueue(int handle) {
        m_nHandle = handle;
    }

    private void Clean() {
        if (m_nHandle != 0) {
            StopQueue(false);
            m_nHandle = 0;
        }
    }

    @Override
    public final int getHandle() {
        return m_nHandle;
    }

    public final void setHandle(int value) {
        m_nHandle = value;
    }

    @Override
    public final boolean AppendTo(IServerQueue serverQueue) {
        if (serverQueue == null) {
            return true;
        }
        int[] queueHandles = {serverQueue.getHandle()};
        return ServerCoreLoader.PushQueueTo(getHandle(), queueHandles, (int) queueHandles.length);
    }

    @Override
    public final boolean AppendTo(int[] queueHandles) {
        if (queueHandles == null || queueHandles.length == 0) {
            return true;
        }
        return ServerCoreLoader.PushQueueTo(getHandle(), queueHandles, (int) queueHandles.length);
    }

    @Override
    public final boolean AppendTo(IServerQueue[] serverQueues) {
        if (serverQueues == null || serverQueues.length == 0) {
            return true;
        }
        int[] targetQueues = new int[serverQueues.length];
        int n = 0;
        for (IServerQueue sq : serverQueues) {
            targetQueues[n] = sq.getHandle();
            ++n;
        }
        return ServerCoreLoader.PushQueueTo(getHandle(), targetQueues, (int) targetQueues.length);
    }

    @Override
    public final boolean EnsureAppending(IServerQueue serverQueue) {
        if (serverQueue == null) {
            return true;
        }
        int[] queueHandles = {serverQueue.getHandle()};
        return EnsureAppending(queueHandles);
    }

    @Override
    public final boolean EnsureAppending(IServerQueue[] serverQueues) {
        if (serverQueues == null || serverQueues.length == 0) {
            return true;
        }
        int n = 0;
        int[] targetQueues = new int[serverQueues.length];

        for (IServerQueue sq : serverQueues) {
            targetQueues[n] = sq.getHandle();
            ++n;
        }
        return EnsureAppending(targetQueues);
    }

    @Override
    public final boolean EnsureAppending(int[] queueHandles) {
        if (!getAvailable()) {
            return false;
        }
        if (getQueueStatus() != SPA.tagQueueStatus.qsMergePushing) {
            return true;
        }
        if (queueHandles == null || queueHandles.length == 0) {
            return true;
        }
        java.util.ArrayList<Integer> vHandles = new java.util.ArrayList<>();
        for (int h : queueHandles) {
            if (ServerCoreLoader.GetServerQueueStatus(h) != SPA.tagQueueStatus.qsMergeComplete.getValue()) {
                vHandles.add(h);
            }
        }
        if (vHandles.size() > 0) {
            int[] handles = new int[vHandles.size()];
            int n = 0;
            for (int h : vHandles) {
                handles[n] = h;
                ++n;
            }
            return AppendTo(handles);
        }
        Reset();
        return true;
    }

    @Override
    public final void StopQueue() {
        ServerCoreLoader.StopQueueByHandle(m_nHandle, false);
    }

    @Override
    public final void StopQueue(boolean permanent) {
        ServerCoreLoader.StopQueueByHandle(m_nHandle, permanent);
    }

    @Override
    public final void Reset() {
        ServerCoreLoader.ResetQueue(m_nHandle);
    }

    @Override
    public final long CancelQueuedMessages(long startIndex, long endIndex) {
        return ServerCoreLoader.CancelQueuedRequestsByIndex(m_nHandle, startIndex, endIndex);
    }

    @Override
    public final int getMessagesInDequeuing() {
        return ServerCoreLoader.GetMessagesInDequeuing(m_nHandle);
    }

    @Override
    public final long getMessageCount() {
        return ServerCoreLoader.GetMessageCount(m_nHandle);
    }

    @Override
    public final long getQueueSize() {
        return ServerCoreLoader.GetQueueSize(m_nHandle);
    }

    @Override
    public final boolean getAvailable() {
        return ServerCoreLoader.IsQueueStartedByHandle(m_nHandle);
    }

    @Override
    public final boolean getSecure() {
        return ServerCoreLoader.IsQueueSecuredByHandle(m_nHandle);
    }

    @Override
    public final String getQueueFileName() {
        return ServerCoreLoader.GetQueueFileName(m_nHandle);
    }

    @Override
    public final String getQueueName() {
        return ServerCoreLoader.GetQueueName(m_nHandle);
    }

    @Override
    public final long getLastIndex() {
        return ServerCoreLoader.GetQueueLastIndex(m_nHandle);
    }

    @Override
    public final boolean AbortJob() {
        return ServerCoreLoader.AbortJob(m_nHandle);
    }

    @Override
    public final boolean StartJob() {
        return ServerCoreLoader.StartJob(m_nHandle);
    }

    @Override
    public final boolean EndJob() {
        return ServerCoreLoader.EndJob(m_nHandle);
    }

    @Override
    public final long getJobSize() {
        return ServerCoreLoader.GetJobSize(m_nHandle);
    }

    @Override
    public final java.util.Date getLastMessageTime() {
        long seconds = ServerCoreLoader.GetLastQueueMessageTime(m_nHandle);
        java.util.Calendar cal = java.util.Calendar.getInstance();
        cal.setTimeInMillis(0);
        cal.set(2013, 0, 1, 0, 0, 0); //.NET 2013, 1, 1, 0, 0, 0
        int days = (int) (seconds / 86400);
        cal.add(java.util.Calendar.DATE, days);
        cal.add(java.util.Calendar.SECOND, (int) (seconds % 86400));
        cal.add(java.util.Calendar.MILLISECOND, ServerCoreLoader.m_gc.getTimeZone().getDSTSavings());
        return cal.getTime();
    }

    @Override
    public final boolean getDequeueShared() {
        return ServerCoreLoader.IsDequeueShared(m_nHandle);
    }

    @Override
    public final long RemoveByTTL() {
        return ServerCoreLoader.RemoveQueuedRequestsByTTL(m_nHandle);
    }

    @Override
    public final SPA.tagQueueStatus getQueueStatus() {
        return SPA.tagQueueStatus.forValue(ServerCoreLoader.GetServerQueueStatus(m_nHandle));
    }

    @Override
    public final int getTTL() {
        return ServerCoreLoader.GetTTL(m_nHandle);
    }

    @Override
    public SPA.tagOptimistic getOptimistic() {
        return SPA.tagOptimistic.forValue(ServerCoreLoader.GetOptimistic(m_nHandle));
    }

    @Override
    public void setOptimistic(SPA.tagOptimistic optimistic) {
        ServerCoreLoader.SetOptimistic(m_nHandle, optimistic.getValue());
    }

    public long Enqueue(short reqId, java.nio.ByteBuffer data) {
        int size = 0;
        if (data != null) {
            size = data.limit() - data.position();
        }
        return Enqueue(reqId, data, size);
    }

    public long Enqueue(short reqId, byte[] data, int len) {
        SPA.CUQueue q = CScopeUQueue.Lock();
        q.Push(data, len);
        long res = Enqueue(reqId, q);
        CScopeUQueue.Unlock(q);
        return res;
    }

    public long Enqueue(short reqId, java.nio.ByteBuffer data, int size) {
        int position = 0;
        if (data == null) {
            size = 0;
        } else {
            position = data.position();
        }
        return ServerCoreLoader.Enqueue(m_nHandle, reqId, data, size, position);
    }

    public final long Enqueue(short reqId, SPA.CUQueue q) {
        if (q == null || q.GetSize() == 0) {
            return Enqueue(reqId, (java.nio.ByteBuffer) null, 0);
        }
        return Enqueue(reqId, q.getIntenalBuffer(), q.GetSize());
    }

    public final long Enqueue(short reqId) {
        return Enqueue(reqId, (java.nio.ByteBuffer) null, (int) 0);
    }

    public final long Enqueue(short reqId, SPA.CScopeUQueue q) {
        if (q == null) {
            return Enqueue(reqId, (java.nio.ByteBuffer) null, 0);
        }
        return Enqueue(reqId, q.getUQueue());
    }
}
