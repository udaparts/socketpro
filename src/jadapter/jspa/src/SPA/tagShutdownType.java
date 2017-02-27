package SPA;

public enum tagShutdownType {

    stReceive(0),
    stSend(1),
    stBoth(2);

    private final int intValue;
    private static java.util.HashMap<Integer, tagShutdownType> mappings;

    private static java.util.HashMap<Integer, tagShutdownType> getMappings() {
        synchronized (tagShutdownType.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagShutdownType(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagShutdownType forValue(int value) {
        return getMappings().get(value);
    }
}
