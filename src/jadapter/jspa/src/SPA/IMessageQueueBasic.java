package SPA;

public interface IMessageQueueBasic {

    /**
     * Stop message queue without removing message queue file
     */
    void StopQueue();

    /**
     * Stop message queue
     *
     * @param permanent A boolean value to determine if the message queue file
     * is permanently removed
     */
    void StopQueue(boolean permanent);

    /**
     * Remove queued messages according to given message indexes
     *
     * @param startIndex A start index
     * @param endIndex An end index
     * @return The number of messages removed
     */
    long CancelQueuedMessages(long startIndex, long endIndex);

    /**
     * Remove messages according to time-to-live
     *
     * @return The number of messages removed
     */
    long RemoveByTTL();

    /**
     * Abort current transaction messages
     *
     * @return True if successful; and false if failed
     */
    boolean AbortJob();

    /**
     * Start a message transaction
     *
     * @return True if successful; and false if failed
     */
    boolean StartJob();

    /**
     * Commit a message transaction
     *
     * @return True if successful; and false if failed
     */
    boolean EndJob();

    /**
     * Discard all of persistent messages
     */
    void Reset();

    int getMessagesInDequeuing();

    long getMessageCount();

    long getQueueSize();

    boolean getAvailable();

    boolean getSecure();

    String getQueueFileName();

    String getQueueName();

    long getJobSize();

    boolean getDequeueShared();

    long getLastIndex();

    tagQueueStatus getQueueStatus();

    int getTTL();

    java.util.Date getLastMessageTime();

    /**
     *
     * @return a value for how to flush message into hard disk. It defaults to the value oSystemMemoryCached.
     */
    tagOptimistic getOptimistic();

    /**
     *
     * @param optimistic Enable (oMemoryCached) or disable (oSystemMemoryCached
     * or oDiskCommitted) message writing with delay
     */
    void setOptimistic(tagOptimistic optimistic);
}
