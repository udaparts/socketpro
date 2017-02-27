package SPA;

public enum tagEncryptionMethod {

    NoEncryption(0),
    TLSv1(1);

    private final int intValue;
    private static java.util.HashMap<Integer, tagEncryptionMethod> mappings;

    private static java.util.HashMap<Integer, tagEncryptionMethod> getMappings() {
        synchronized (tagEncryptionMethod.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagEncryptionMethod(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagEncryptionMethod forValue(int value) {
        return getMappings().get(value);
    }
}
