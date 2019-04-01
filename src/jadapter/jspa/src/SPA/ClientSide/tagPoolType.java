package SPA.ClientSide;

public enum tagPoolType {

    Regular(0),
    Slave(1),
    Master(2);
    private final int intValue;
    private static java.util.HashMap<Integer, tagPoolType> mappings;

    private static java.util.HashMap<Integer, tagPoolType> getMappings() {
        synchronized (tagConnectionState.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagPoolType(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagPoolType forValue(int value) {
        return getMappings().get(value);
    }
}
