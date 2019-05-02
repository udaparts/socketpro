public static class TOneConst
{
    //defines for service CTOne
    public const uint sidCTOne = (SocketProAdapter.BaseServiceID.sidReserved + 30);

    public const ushort idQueryCountCTOne = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idQueryGlobalCountCTOne = (idQueryCountCTOne + 1);
    public const ushort idQueryGlobalFastCountCTOne = (idQueryGlobalCountCTOne + 1);
    public const ushort idSleepCTOne = (idQueryGlobalFastCountCTOne + 1);
    public const ushort idEchoCTOne = (idSleepCTOne + 1);
    public const ushort idEchoExCTOne = (idEchoCTOne + 1);
}