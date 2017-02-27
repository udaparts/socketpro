package hello_world;

public final class hwConst {

    public static final int sidHelloWorld = (SPA.BaseServiceID.sidReserved + 1);
    public static final short idSayHelloHelloWorld = (short) (SPA.tagBaseRequestID.idReservedTwo + 1);
    public static final short idSleepHelloWorld = (short) (idSayHelloHelloWorld + 1);
    public static final short idEchoHelloWorld = (short) (idSleepHelloWorld + 1);
}
