package SPA.ClientSide;

public enum tagSocketPoolEvent {

    speUnknown(-1),
    speStarted(0),
    speCreatingThread(1),
    speThreadCreated(2),
    speConnecting(3),
    speConnected(4),
    speKillingThread(5),
    speShutdown(6),
    speUSocketCreated(7),
    speHandShakeCompleted(8),
    speLocked(9),
    speUnlocked(10),
    speThreadKilled(11),
    speClosingSocket(12),
    speSocketClosed(13),
    speUSocketKilled(14),
    speTimer(15),
    speQueueMergedFrom(16),
    speQueueMergedTo(17);

    private final int intValue;
    private static java.util.HashMap<Integer, tagSocketPoolEvent> mappings;

    private static java.util.HashMap<Integer, tagSocketPoolEvent> getMappings() {
        synchronized (tagSocketPoolEvent.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagSocketPoolEvent(int value) {
        intValue = value;
        getMappings().put(value, this);
    }

    public int getValue() {
        return intValue;
    }

    public static tagSocketPoolEvent forValue(int value) {
        return getMappings().get(value);
    }
}
