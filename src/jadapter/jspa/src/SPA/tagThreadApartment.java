package SPA;

public enum tagThreadApartment {

    /**
     * no COM apartment involved
     */
    taNone(0),
    /**
     * STA apartment
     */
    taApartment(1),
    /**
     * MTA (free) or neutral apartments
     */
    taFree(2);

    private final int intValue;
    private static java.util.HashMap<Integer, tagThreadApartment> mappings;

    private static java.util.HashMap<Integer, tagThreadApartment> getMappings() {
        synchronized (tagThreadApartment.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagThreadApartment(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagThreadApartment forValue(int value) {
        return getMappings().get(value);
    }
}
