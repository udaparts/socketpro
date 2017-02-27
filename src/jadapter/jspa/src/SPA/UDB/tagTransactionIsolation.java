package SPA.UDB;

public enum tagTransactionIsolation {

    tiUnspecified(-1),
    tiChaos(0),
    tiReadUncommited(1),
    tiBrowse(2),
    tiCursorStability(3),
    tiReadCommited(3),
    tiRepeatableRead(4),
    tiSerializable(5),
    tiIsolated(6);

    private final int intValue;
    private static java.util.HashMap<Integer, tagTransactionIsolation> mappings;

    private static java.util.HashMap<Integer, tagTransactionIsolation> getMappings() {
        synchronized (tagTransactionIsolation.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagTransactionIsolation(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagTransactionIsolation forValue(int value) {
        return getMappings().get(value);
    }
}
