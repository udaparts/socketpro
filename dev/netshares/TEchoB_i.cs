public static class TEchoBConst
{
    //defines for service CEchoBasic
    public const uint sidCEchoBasic = (SocketProAdapter.BaseServiceID.sidReserved + 1);

    public const ushort idEchoBoolCEchoBasic = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idEchoInt8CEchoBasic = (idEchoBoolCEchoBasic + 1);
    public const ushort idEchoUInt8CEchoBasic = (idEchoInt8CEchoBasic + 1);
    public const ushort idEchoInt16CEchoBasic = (idEchoUInt8CEchoBasic + 1);
    public const ushort idEchoUInt16CEchoBasic = (idEchoInt16CEchoBasic + 1);
    public const ushort idEchoInt32CEchoBasic = (idEchoUInt16CEchoBasic + 1);
    public const ushort idEchoUInt32CEchoBasic = (idEchoInt32CEchoBasic + 1);
    public const ushort idEchoInt64CEchoBasic = (idEchoUInt32CEchoBasic + 1);
    public const ushort idEchoUInt64CEchoBasic = (idEchoInt64CEchoBasic + 1);
    public const ushort idEchoFloatCEchoBasic = (idEchoUInt64CEchoBasic + 1);
    public const ushort idEchoDoubleCEchoBasic = (idEchoFloatCEchoBasic + 1);
    public const ushort idEchoStringCEchoBasic = (idEchoDoubleCEchoBasic + 1);
    public const ushort idEchoAStringCEchoBasic = (idEchoStringCEchoBasic + 1);
    public const ushort idEchoDecimalCEchoBasic = (idEchoAStringCEchoBasic + 1);
    public const ushort idEchoWCharCEchoBasic = (idEchoDecimalCEchoBasic + 1);
    public const ushort idEchoGuidCEchoBasic = (idEchoWCharCEchoBasic + 1);
    public const ushort idEchoCyCEchoBasic = (idEchoGuidCEchoBasic + 1);
    public const ushort idEchoDateTime = (idEchoCyCEchoBasic + 1);
}