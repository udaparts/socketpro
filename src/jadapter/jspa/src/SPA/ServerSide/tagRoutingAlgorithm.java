package SPA.ServerSide;

public enum tagRoutingAlgorithm {

    raDefault(0),
    raRandom(1),
    raAverage(2);

    private final int intValue;
    private static java.util.HashMap<Integer, tagRoutingAlgorithm> mappings;

    private static java.util.HashMap<Integer, tagRoutingAlgorithm> getMappings() {
        synchronized (tagRoutingAlgorithm.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagRoutingAlgorithm(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagRoutingAlgorithm forValue(int value) {
        return getMappings().get(value);
    }
}
