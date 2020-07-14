package hello_world;

public final class hwConst {

    public static final int sidHelloWorld = (SPA.BaseServiceID.sidReserved + 1);
    public static final short idSayHello = (short) (SPA.tagBaseRequestID.idReservedTwo + 1);
    public static final short idSleep = (short) (idSayHello + 1);
    public static final short idEcho = (short) (idSleep + 1);
}
