package SPA;

public enum tagChatRequestID {

    idEnter((short) 65),
    idSpeak((short) 66),
    idSpeakEx((short) 67),
    idExit((short) 68),
    idSendUserMessage((short) 69),
    idSendUserMessageEx((short) 70);

    private final short intValue;
    private static java.util.HashMap<Short, tagChatRequestID> mappings;

    private static java.util.HashMap<Short, tagChatRequestID> getMappings() {
        synchronized (tagChatRequestID.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagChatRequestID(short value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public short getValue() {
        return intValue;
    }

    public static tagChatRequestID forValue(short value) {
        return getMappings().get(value);
    }
}
