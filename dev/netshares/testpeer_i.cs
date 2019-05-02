using System;

public static class TestMeConst
{
    public const uint sidTestService = (SocketProAdapter.BaseServiceID.sidReserved + 20);
    public const ushort idSleep = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idEcho = (idSleep + 1);
    public const ushort idOpenDb = (idEcho + 1);
    public const ushort idBadRequest = (idOpenDb + 1);
    public const ushort idDoRequest0 = (idBadRequest + 1);
    public const ushort idDoRequest1 = (idDoRequest0 + 1);
    public const ushort idDoRequest2 = (idDoRequest1 + 1);
    public const ushort idDoRequest3 = (idDoRequest2 + 1);
    public const ushort idDoRequest4 = (idDoRequest3 + 1);
    public const ushort idDequeue = (idDoRequest4 + 1);
    public const ushort idDoIdle = (idDequeue + 1);
}
