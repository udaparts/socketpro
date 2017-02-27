package SPA;

public enum tagQueueStatus {

    /**
     * everything is fine
     */
    qsNormal(0),
    /**
     * Queued messages merged completely
     */
    qsMergeComplete(1),
    /**
     * Message replication started but not completed yet
     */
    qsMergePushing(2),
    /**
     * Message replicated incompletely from a source queue
     */
    qsMergeIncomplete(3),
    //job incomplete (crash or endjob not found)

    /**
     * A set of messages as a job are incompletely queued
     */
    qsJobIncomplete(4),
    /**
     * A message queued incompletely because of application crash or unexpected
     * termination
     */
    qsCrash(5),
    /**
     * Queue file open error
     */
    qsFileError(6),
    /**
     * Queue file opened but can not decrypt existing queued messages because of
     * bad password found
     */
    qsBadPassword(7),
    /**
     * Duplicate name error
     */
    qsDuplicateName(8);

    private final int intValue;
    private static java.util.HashMap<Integer, tagQueueStatus> mappings;

    private static java.util.HashMap<Integer, tagQueueStatus> getMappings() {
        synchronized (tagQueueStatus.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagQueueStatus(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagQueueStatus forValue(int value) {
        return getMappings().get(value);
    }
}
