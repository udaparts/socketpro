package SPA.ServerSide;

public enum tagAuthenticationMethod {

    amOwn(0),
    amMixed(1),
    amIntegrated(2),
    amTrusted(3);

    private final int intValue;
    private static java.util.HashMap<Integer, tagAuthenticationMethod> mappings;

    private static java.util.HashMap<Integer, tagAuthenticationMethod> getMappings() {
        synchronized (tagAuthenticationMethod.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagAuthenticationMethod(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagAuthenticationMethod forValue(int value) {
        return getMappings().get(value);
    }
}
