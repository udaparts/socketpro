package SPA;

public enum tagOperationSystem {

    osWin((byte) 0),
    osApple((byte) 1),
    osUnix((byte) 2),
    osAndroid((byte) 3),
    osWinCE((byte) 4); //*< Old window pocket pc, ce or smart phone devices

    private final byte intValue;
    private static java.util.HashMap<Byte, tagOperationSystem> mappings;

    private static java.util.HashMap<Byte, tagOperationSystem> getMappings() {
        synchronized (tagOperationSystem.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagOperationSystem(byte value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public byte getValue() {
        return intValue;
    }

    public static tagOperationSystem forValue(byte value) {
        switch (value) {
            case 0:
                return osWin;
            case 1:
                return osApple;
            case 2:
                return osUnix;
            case 3:
                return osAndroid;
            default:
                return osWinCE;
        }
    }
}
