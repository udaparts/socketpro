package SPA.ServerSide;

public enum tagTransferEncoding {

    teUnknown(0),
    teChunked(1),
    teCompress(2),
    teDeflate(3),
    teGZip(4),
    teIdentity(5);

    private final int intValue;
    private static java.util.HashMap<Integer, tagTransferEncoding> mappings;

    private static java.util.HashMap<Integer, tagTransferEncoding> getMappings() {
        synchronized (tagTransferEncoding.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagTransferEncoding(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagTransferEncoding forValue(int value) {
        return getMappings().get(value);
    }
}
