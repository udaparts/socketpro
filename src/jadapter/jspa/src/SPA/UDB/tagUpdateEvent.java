package SPA.UDB;

public enum tagUpdateEvent {

    ueUnknown(-1),
    /**
     * An event for inserting a record into a table
     */
    ueInsert(0),
    /**
     * An event for updating a record of a table
     */
    ueUpdate(1),
    /**
     * An event for deleting a record from a table
     */
    ueDelete(2);

    private final int intValue;
    private static java.util.HashMap<Integer, tagUpdateEvent> mappings;

    private static java.util.HashMap<Integer, tagUpdateEvent> getMappings() {
        synchronized (tagUpdateEvent.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagUpdateEvent(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagUpdateEvent forValue(int value) {
        return getMappings().get(value);
    }
}
