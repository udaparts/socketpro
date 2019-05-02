public static class SQueueConst
{
    //defines for service CServerQueue
    public const uint sidCServerQueue = (SocketProAdapter.BaseServiceID.sidReserved + 6);

    public const ushort idEnqueueCServerQueue = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idDequeueCServerQueue = (idEnqueueCServerQueue + 1);
    public const ushort idMyMessage = idDequeueCServerQueue + 1;
    public const ushort idQueryTimes = idMyMessage + 1;

    public const ushort idDoEnqueueNumbers = idQueryTimes + 1;
    public const ushort idMyNumbers = idDoEnqueueNumbers + 1;
    public const ushort idDoDequeueNumbers = idMyNumbers + 1;
}