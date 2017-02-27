package SPA.ServerSide;

public enum tagHttpRequestID {

    idGet((short) 129),
    idPost((short) 130),
    idHead((short) 131),
    idPut((short) 132),
    idDelete((short) 133),
    idOptions((short) 134),
    idTrace((short) 135),
    idConnect((short) 136),
    idMultiPart((short) 137), //HTTP POST MUTIPLE PART
    idUserRequest((short) 138); //SocketPro HTTP User Request

    private final short intValue;
    private static java.util.HashMap<Short, tagHttpRequestID> mappings;

    private static java.util.HashMap<Short, tagHttpRequestID> getMappings() {
        synchronized (tagHttpRequestID.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagHttpRequestID(short value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public short getValue() {
        return intValue;
    }

    public static tagHttpRequestID forValue(short value) {
        return getMappings().get(value);
    }
}
//#endif
