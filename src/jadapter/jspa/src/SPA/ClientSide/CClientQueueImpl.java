package SPA.ClientSide;

class CClientQueueImpl implements IClientQueue {

    CClientQueueImpl(CClientSocket cs) {
        m_cs = cs;
    }

    @Override
    public void StopQueue() {
        ClientCoreLoader.StopQueue(m_cs.getHandle(), false);
    }

    @Override
    public void StopQueue(boolean permanent) {
        ClientCoreLoader.StopQueue(m_cs.getHandle(), permanent);
    }

    @Override
    public long CancelQueuedMessages(long startIndex, long endIndex) {
        return ClientCoreLoader.CancelQueuedRequestsByIndex(m_cs.getHandle(), startIndex, endIndex);
    }

    @Override
    public long RemoveByTTL() {
        return ClientCoreLoader.RemoveQueuedRequestsByTTL(m_cs.getHandle());
    }

    @Override
    public boolean AbortJob() {
        return ClientCoreLoader.AbortJob(m_cs.getHandle());
    }

    @Override
    public boolean StartJob() {
        return ClientCoreLoader.StartJob(m_cs.getHandle());
    }

    @Override
    public boolean EndJob() {
        return ClientCoreLoader.EndJob(m_cs.getHandle());
    }

    @Override
    public void Reset() {
        ClientCoreLoader.ResetQueue(m_cs.getHandle());
    }

    @Override
    public int getMessagesInDequeuing() {
        return ClientCoreLoader.GetMessagesInDequeuing(m_cs.getHandle());
    }

    @Override
    public long getMessageCount() {
        return ClientCoreLoader.GetMessageCount(m_cs.getHandle());
    }

    @Override
    public long getQueueSize() {
        return ClientCoreLoader.GetQueueSize(m_cs.getHandle());
    }

    @Override
    public boolean getAvailable() {
        return ClientCoreLoader.IsQueueStarted(m_cs.getHandle());
    }

    @Override
    public boolean getSecure() {
        return ClientCoreLoader.IsQueueSecured(m_cs.getHandle());
    }

    @Override
    public String getQueueFileName() {
        return ClientCoreLoader.GetQueueFileName(m_cs.getHandle());
    }

    @Override
    public String getQueueName() {
        return ClientCoreLoader.GetQueueName(m_cs.getHandle());
    }

    @Override
    public long getJobSize() {
        return ClientCoreLoader.GetJobSize(m_cs.getHandle());
    }

    @Override
    public boolean getDequeueShared() {
        return ClientCoreLoader.IsDequeueShared(m_cs.getHandle());
    }

    @Override
    public long getLastIndex() {
        return ClientCoreLoader.GetQueueLastIndex(m_cs.getHandle());
    }

    @Override
    public SPA.tagQueueStatus getQueueStatus() {
        return SPA.tagQueueStatus.forValue(ClientCoreLoader.GetClientQueueStatus(m_cs.getHandle()));
    }

    @Override
    public int getTTL() {
        return ClientCoreLoader.GetTTL(m_cs.getHandle());
    }

    @Override
    public java.util.Date getLastMessageTime() {
        long seconds = ClientCoreLoader.GetLastQueueMessageTime(m_cs.getHandle());
        java.util.Calendar cal = java.util.Calendar.getInstance();
        cal.setTimeInMillis(0);
        cal.set(2013, 0, 1, 0, 0, 0); //.NET 2013, 1, 1, 0, 0, 0
        int days = (int) (seconds / 86400);
        cal.add(java.util.Calendar.DATE, days);
        cal.add(java.util.Calendar.SECOND, (int) (seconds % 86400));
        cal.add(java.util.Calendar.MILLISECOND, ClientCoreLoader.m_gc.getTimeZone().getDSTSavings());
        return cal.getTime();
    }

    @Override
    public boolean StartQueue(String qName, int ttl) {
        return StartQueue(qName, ttl, true, false);
    }

    @Override
    public boolean StartQueue(String qName, int ttl, boolean secure) {
        return StartQueue(qName, ttl, secure, false);
    }

    @Override
    public boolean StartQueue(String qName, int ttl, boolean secure, boolean dequeueShared) {
        if (qName == null || qName.length() == 0) {
            throw new java.lang.IllegalArgumentException("Invalid queue file name");
        }
        byte[] bytes = qName.getBytes();
        return ClientCoreLoader.StartQueue(m_cs.getHandle(), bytes, bytes.length, secure, dequeueShared, ttl);
    }

    @Override
    public boolean getDequeueEnabled() {
        return ClientCoreLoader.IsDequeueEnabled(m_cs.getHandle());
    }

    @Override
    public boolean AppendTo(IClientQueue clientQueue) {
        if (clientQueue == null) {
            return true;
        }
        long[] qh = {clientQueue.getHandle()};
        return AppendTo(qh);
    }

    @Override
    public boolean AppendTo(IClientQueue[] clientQueues) {
        if (clientQueues == null || clientQueues.length == 0) {
            return true;
        }
        long[] qh = new long[clientQueues.length];
        int n = 0;
        for (IClientQueue cq : clientQueues) {
            qh[n] = cq.getHandle();
            ++n;
        }
        return AppendTo(qh);
    }

    @Override
    public boolean AppendTo(long[] queueHandles) {
        if (queueHandles == null || queueHandles.length == 0) {
            return true;
        }
        return ClientCoreLoader.PushQueueTo(m_cs.getHandle(), queueHandles, queueHandles.length);
    }

    @Override
    public boolean EnsureAppending(IClientQueue clientQueue) {
        if (clientQueue == null) {
            return true;
        }
        long[] qh = {clientQueue.getHandle()};
        return EnsureAppending(qh);
    }

    @Override
    public boolean EnsureAppending(IClientQueue[] clientQueues) {
        if (clientQueues == null || clientQueues.length == 0) {
            return true;
        }
        long[] qh = new long[clientQueues.length];
        int n = 0;
        for (IClientQueue cq : clientQueues) {
            qh[n] = cq.getHandle();
            ++n;
        }
        return EnsureAppending(qh);
    }

    @Override
    public boolean EnsureAppending(long[] queueHandles) {
        if (!getAvailable()) {
            return false;
        }
        if (getQueueStatus() != SPA.tagQueueStatus.qsMergePushing) {
            return true;
        }
        if (queueHandles == null || queueHandles.length == 0) {
            return true;
        }
        java.util.ArrayList<Long> vHandles = new java.util.ArrayList<>();
        for (long h : queueHandles) {
            if (ClientCoreLoader.GetClientQueueStatus(h) != SPA.tagQueueStatus.qsMergeComplete.getValue()) {
                vHandles.add(h);
            }
        }
        if (vHandles.size() > 0) {
            int n = 0;
            long[] qh = new long[vHandles.size()];
            for (long h : vHandles) {
                qh[n] = h;
                ++n;
            }
            return AppendTo(qh);
        }
        Reset();
        return true;
    }

    @Override
    public boolean getRoutingQueueIndex() {
        return ClientCoreLoader.IsRoutingQueueIndexEnabled(m_cs.getHandle());
    }

    @Override
    public void setRoutingQueueIndex(boolean value) {
        ClientCoreLoader.EnableRoutingQueueIndex(m_cs.getHandle(), value);
    }

    @Override
    public long getHandle() {
        return m_cs.getHandle();
    }

    @Override
    public SPA.tagOptimistic getOptimistic() {
        return SPA.tagOptimistic.forValue(ClientCoreLoader.GetOptimistic(m_cs.getHandle()));
    }

    @Override
    public void setOptimistic(SPA.tagOptimistic optimistic) {
        ClientCoreLoader.SetOptimistic(m_cs.getHandle(), optimistic.getValue());
    }

    private final CClientSocket m_cs;
}
