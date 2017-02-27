package SPA;

public enum tagBaseRequestID {

    idUnknown((short) 0),
    idSwitchTo((short) 1),
    idRouteeChanged((short) 2),
    idEncrypted((short) 3),
    idBatchZipped((short) 4),
    idCancel((short) 5),
    idGetSockOptAtSvr((short) 6),
    idSetSockOptAtSvr((short) 7),
    idDoEcho((short) 8),
    idTurnOnZipAtSvr((short) 9),
    idStartBatching((short) 10),
    idCommitBatching((short) 11),
    idShrinkMemoryAtSvr((short) 12),
    idSetRouting((short) 13),
    idPing((short) 14),
    idEnableClientDequeue((short) 15),
    idServerException((short) 16),
    idAllMessagesDequeued((short) 17),
    idHttpClose((short) 18), //SocketPro HTTP Close
    idSetZipLevelAtSvr((short) 19),
    idStartJob((short) 20),
    idEndJob((short) 21),
    idRoutingData((short) 22),
    idDequeueConfirmed((short) 23),
    idMessageQueued((short) 24),
    idStartQueue((short) 25),
    idStopQueue((short) 26),
    idRoutePeerUnavailable((short) 27),
    idReservedOne((short) 0x100),
    idStartReqId((short) 0x2001);

    public final static short idReservedTwo = 0x2001;

    private final short sValue;
    private static java.util.HashMap<Short, tagBaseRequestID> mappings;

    private static java.util.HashMap<Short, tagBaseRequestID> getMappings() {
        synchronized (tagBaseRequestID.class) {
            if (mappings == null) {
                mappings = new java.util.HashMap<>();
            }
        }
        return mappings;
    }

    @SuppressWarnings("LeakingThisInConstructor")
    private tagBaseRequestID(short value) {
        sValue = value;
        getMappings().put(value, this);
    }

    public short getValue() {
        return sValue;
    }

    public static tagBaseRequestID forValue(short value) {
        return getMappings().get(value);
    }
}
