package SPA.UDB;

public enum tagParameterDirection {
	pdUnknown(0),
    pdInput(1),
    pdOutput(2),
    pdInputOutput(3),
    pdReturnValue(4);

    private final int intValue;
    private static java.util.HashMap<Integer, tagParameterDirection> mappings;

    private static java.util.HashMap<Integer, tagParameterDirection> getMappings() {
        synchronized (tagParameterDirection.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagParameterDirection(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagParameterDirection forValue(int value) {
        return getMappings().get(value);
    }
}
