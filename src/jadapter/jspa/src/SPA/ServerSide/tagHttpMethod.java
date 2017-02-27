package SPA.ServerSide;

public enum tagHttpMethod {

    hmUnknown(0),
    hmGet(1),
    hmPost(2),
    hmHead(3),
    hmPut(4),
    hmDelete(5),
    hmOptions(6),
    hmTrace(7),
    hmConnect(8);

    private final int intValue;
    private static java.util.HashMap<Integer, tagHttpMethod> mappings;

    private static java.util.HashMap<Integer, tagHttpMethod> getMappings() {
        synchronized (tagHttpMethod.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagHttpMethod(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagHttpMethod forValue(int value) {
        return getMappings().get(value);
    }
}
