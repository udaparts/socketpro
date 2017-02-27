public static class radoConst
{
	//defines for service RAdo
	public const uint sidRAdo = (SocketProAdapter.BaseServiceID.sidReserved + 4);

	public const ushort idGetDataSetRAdo = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
	public const ushort idGetDataTableRAdo = (idGetDataSetRAdo + 1);
}