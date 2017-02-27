package SPA;

public enum tagOptimistic {

    oMemoryCached(0),
    oSystemMemoryCached(1),
    oDiskCommitted(2);

    private final int intValue;
    private static java.util.HashMap<Integer, tagOptimistic> mappings;

    private static java.util.HashMap<Integer, tagOptimistic> getMappings() {
        synchronized (tagOptimistic.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagOptimistic(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagOptimistic forValue(int value) {
        return getMappings().get(value);
    }
}
