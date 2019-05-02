public static class RAdoConst
{
    //defines for service RAdo
    public const uint sidRAdo = (SocketProAdapter.BaseServiceID.sidReserved + 45);

    public const ushort idGetDataSetRAdo = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idGetDataReaderRAdo = (idGetDataSetRAdo + 1);
    public const ushort idSendDataSetRAdo = (idGetDataReaderRAdo + 1);
    public const ushort idSendDataReaderRAdo = (idSendDataSetRAdo + 1);
    public const ushort idSendDataTableRAdo = (idSendDataReaderRAdo + 1);

    public const ushort idRep0 = idSendDataTableRAdo + 1;
    public const ushort idRep1 = idRep0 + 1;
}