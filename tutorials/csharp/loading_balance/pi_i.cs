public static class piConst
{
	//defines for service Pi
	public const uint sidPi = (SocketProAdapter.BaseServiceID.sidReserved + 5);
    public const uint sidPiWorker = sidPi + 1;

	public const ushort idComputePi = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
}