package SPA.ClientSide;

public interface IClientQueue extends SPA.IMessageQueueBasic {

    /**
     * Open a persistent file for a message queue
     *
     * @param qName Message queue name or a full path to message queue file
     * @param ttl Time-to-live in seconds
     * @return True if successful and false if failed To reopen an existing
     * secure message queue file, the method may fail if current password is
     * different from original one. There are a number of situations leading the
     * failures of this method
     */
    boolean StartQueue(String qName, int ttl);

    /**
     * Open a persistent file for a message queue
     *
     * @param qName Message queue name or a full path to message queue file
     * @param ttl Time-to-live in seconds
     * @param secure A boolean value default to true to indicate if queued
     * messages should be encrypted by password
     * @return True if successful and false if failed To reopen an existing
     * secure message queue file, the method may fail if current password is
     * different from original one. There are a number of situations leading the
     * failures of this method
     */
    boolean StartQueue(String qName, int ttl, boolean secure);

    /**
     * Open a persistent file for a message queue
     *
     * @param qName Message queue name or a full path to message queue file
     * @param ttl Time-to-live in seconds
     * @param secure A boolean value default to true to indicate if queued
     * messages should be encrypted by password
     * @param dequeueShared A boolean value default to false to indicate if
     * there are two or more sessions to dequeue messages
     * @return True if successful and false if failed To reopen an existing
     * secure message queue file, the method may fail if current password is
     * different from original one. There are a number of situations leading the
     * failures of this method
     */
    boolean StartQueue(String qName, int ttl, boolean secure, boolean dequeueShared);

    boolean getDequeueEnabled();

    /**
     * Replicate all messages within this queue onto one target queue
     *
     * @param clientQueue A target queue for appending messages from this queue
     * @return True for success; and false for fail. To make the call success, a
     * target queue should be already opened and available
     */
    boolean AppendTo(IClientQueue clientQueue);

    /**
     * Replicate all messages within this queue onto an array of queues
     *
     * @param clientQueues An array of target queues for appending messages from
     * this queue
     * @return True for success; and false for fail. To make the call success,
     * all of target queues should be already opened and available
     */
    boolean AppendTo(IClientQueue[] clientQueues);

    /**
     * Replicate all messages within this queue onto an array of queues
     *
     * @param queueHandles An array of target queues for appending messages from
     * this queue
     * @return True for success; and false for fail. To make the call success,
     * all of target queues should be already opened and available
     */
    boolean AppendTo(long[] queueHandles);

    /**
     * Ensure previous replication in case an application was crashed
     * previously. Call this method one time only and as early as possible
     *
     * @param clientQueue A target queue for appending messages from this queue
     * @return True for success; and false for fail. To make the call success, a
     * target queue should be already opened and available
     */
    boolean EnsureAppending(IClientQueue clientQueue);

    /**
     * Ensure previous replication in case an application was crashed
     * previously. Call this method one time only and as early as possible
     *
     * @param clientQueues An array of target queues for appending messages from
     * this queue
     * @return True for success; and false for fail. To make the call success,
     * all of target queues should be already opened and available
     */
    boolean EnsureAppending(IClientQueue[] clientQueues);

    /**
     * Ensure previous replication in case an application was crashed
     * previously. Call this method one time only and as early as possible
     *
     * @param queueHandles An array of target queues for appending messages from
     * this queue
     * @return True for success; and false for fail. To make the call success,
     * all of target queues should be already opened and available
     */
    boolean EnsureAppending(long[] queueHandles);

    boolean getRoutingQueueIndex();

    void setRoutingQueueIndex(boolean value);

    long getHandle();
}
