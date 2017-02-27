package SPA.ServerSide;

public enum tagTransport {

    tUnknown(-1),
    tWebSocket(0),
    tFlash(1),
    tAjax(2),
    tScript(3);

    private final int intValue;
    private static java.util.HashMap<Integer, tagTransport> mappings;

    private static java.util.HashMap<Integer, tagTransport> getMappings() {
        synchronized (tagTransport.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagTransport(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagTransport forValue(int value) {
        return getMappings().get(value);
    }
}
