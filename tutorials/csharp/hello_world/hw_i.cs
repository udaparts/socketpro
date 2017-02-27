
public static class hwConst
{
    //defines for service HelloWorld
    public const uint sidHelloWorld = (SocketProAdapter.BaseServiceID.sidReserved + 1);

    public const ushort idSayHelloHelloWorld = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idSleepHelloWorld = (idSayHelloHelloWorld + 1);
    public const ushort idEchoHelloWorld = (idSleepHelloWorld + 1);
}