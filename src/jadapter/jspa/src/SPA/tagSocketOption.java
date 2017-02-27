package SPA;

public enum tagSocketOption {

    soTcpNoDelay(1),
    soReuseAddr(4),
    soKeepAlive(8),
    soSndBuf(0x1001), // send buffer size
    soRcvBuf(0x1002); // receive buffer size

    private final int intValue;
    private static java.util.HashMap<Integer, tagSocketOption> mappings;

    private static java.util.HashMap<Integer, tagSocketOption> getMappings() {
        synchronized (tagSocketOption.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagSocketOption(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagSocketOption forValue(int value) {
        return getMappings().get(value);
    }
}
