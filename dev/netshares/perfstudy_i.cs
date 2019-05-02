using System;

public static class PerfStudyConst
{
    public const uint sidPerfStudy = (SocketProAdapter.BaseServiceID.sidReserved + 15);
    public const ushort idDoMyEcho = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idSlowEcho = (idDoMyEcho + 1);
}
