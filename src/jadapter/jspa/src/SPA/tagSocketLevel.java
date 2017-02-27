package SPA;

public enum tagSocketLevel {

    slTcp(6),
    slSocket(0xFFFF);

    private final int intValue;
    private static java.util.HashMap<Integer, tagSocketLevel> mappings;

    private static java.util.HashMap<Integer, tagSocketLevel> getMappings() {
        synchronized (tagSocketLevel.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagSocketLevel(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagSocketLevel forValue(int value) {
        return getMappings().get(value);
    }
}
