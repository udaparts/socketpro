package SPA;

public enum tagZipLevel {

    zlDefault(0),
    zlBestSpeed(1),
    zlBestCompression(2);

    private final int intValue;
    private static java.util.HashMap<Integer, tagZipLevel> mappings;

    private static java.util.HashMap<Integer, tagZipLevel> getMappings() {
        synchronized (tagZipLevel.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagZipLevel(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagZipLevel forValue(int value) {
        return getMappings().get(value);
    }
}
