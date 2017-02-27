package SPA.ServerSide;

public enum tagContentMultiplax {

    cmUnknown(0),
    cmMixed(1),
    cmAlternative(2),
    cmDigest(3),
    cmParallel(4),
    cmFormData(5),
    cmReport(6),
    cmSigned(7),
    cmEncrypted(8),
    cmRelated(9),
    cmByteRanges(10);

    private final int intValue;
    private static java.util.HashMap<Integer, tagContentMultiplax> mappings;

    private static java.util.HashMap<Integer, tagContentMultiplax> getMappings() {
        synchronized (tagContentMultiplax.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagContentMultiplax(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagContentMultiplax forValue(int value) {
        return getMappings().get(value);
    }
}
