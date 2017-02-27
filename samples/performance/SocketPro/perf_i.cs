public static class perfConst
{
	//defines for service CPerf
	public const uint sidCPerf = (SocketProAdapter.BaseServiceID.sidReserved + 1111);

	public const ushort idMyEchoCPerf = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
	public const ushort idOpenRecordsCPerf = (idMyEchoCPerf + 1);
}