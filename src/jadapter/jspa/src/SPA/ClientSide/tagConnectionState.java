package SPA.ClientSide;

public enum tagConnectionState {

    csClosed(0),
    csConnecting(1),
    csSslShaking(2),
    csClosing(3),
    csConnected(4),
    csSwitched(5);

    private final int intValue;
    private static java.util.HashMap<Integer, tagConnectionState> mappings;

    private static java.util.HashMap<Integer, tagConnectionState> getMappings() {
        synchronized (tagConnectionState.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagConnectionState(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagConnectionState forValue(int value) {
        return getMappings().get(value);
    }
}
