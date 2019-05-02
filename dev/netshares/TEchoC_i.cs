public static class TEchoCConst
{
    //defines for service CEchoObject
    public const uint sidCEchoObject = (SocketProAdapter.BaseServiceID.sidReserved + 2);

    public const ushort idEchoEmptyCEchoObject = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idEchoBoolCEchoObject = (idEchoEmptyCEchoObject + 1);
    public const ushort idEchoInt8CEchoObject = (idEchoBoolCEchoObject + 1);
    public const ushort idEchoUInt8CEchoObject = (idEchoInt8CEchoObject + 1);
    public const ushort idEchoInt16CEchoObject = (idEchoUInt8CEchoObject + 1);
    public const ushort idEchoUInt16CEchoObject = (idEchoInt16CEchoObject + 1);
    public const ushort idEchoInt32CEchoObject = (idEchoUInt16CEchoObject + 1);
    public const ushort idEchoUInt32CEchoObject = (idEchoInt32CEchoObject + 1);
    public const ushort idEchoInt64CEchoObject = (idEchoUInt32CEchoObject + 1);
    public const ushort idEchoUInt64CEchoObject = (idEchoInt64CEchoObject + 1);
    public const ushort idEchoFloatCEchoObject = (idEchoUInt64CEchoObject + 1);
    public const ushort idEchoDoubleCEchoObject = (idEchoFloatCEchoObject + 1);
    public const ushort idEchoStringCEchoObject = (idEchoDoubleCEchoObject + 1);
    public const ushort idEchoAStringCEchoObject = (idEchoStringCEchoObject + 1);
    public const ushort idEchoDecimalCEchoObject = (idEchoAStringCEchoObject + 1);
    public const ushort idEchoBoolArrayCEchoObject = (idEchoDecimalCEchoObject + 1);
    public const ushort idEchoUInt8ArrayCEchoObject = (idEchoBoolArrayCEchoObject + 1);
    public const ushort idEchoInt16ArrayCEchoObject = (idEchoUInt8ArrayCEchoObject + 1);
    public const ushort idEchoUInt16ArrayCEchoObject = (idEchoInt16ArrayCEchoObject + 1);
    public const ushort idEchoInt32ArrayCEchoObject = (idEchoUInt16ArrayCEchoObject + 1);
    public const ushort idEchoUInt32ArrayCEchoObject = (idEchoInt32ArrayCEchoObject + 1);
    public const ushort idEchoInt64ArrayCEchoObject = (idEchoUInt32ArrayCEchoObject + 1);
    public const ushort idEchoUInt64ArrayCEchoObject = (idEchoInt64ArrayCEchoObject + 1);
    public const ushort idEchoFloatArrayCEchoObject = (idEchoUInt64ArrayCEchoObject + 1);
    public const ushort idEchoDoubleArrayCEchoObject = (idEchoFloatArrayCEchoObject + 1);
    public const ushort idEchoStringArrayCEchoObject = (idEchoDoubleArrayCEchoObject + 1);
    public const ushort idEchoDecimalArrayCEchoObject = (idEchoStringArrayCEchoObject + 1);
    public const ushort idEchoUUIDCEchoObject = (idEchoDecimalArrayCEchoObject + 1);
    public const ushort idEchoUUIDArrayCEchoObject = (idEchoUUIDCEchoObject + 1);
    public const ushort idEchoCYCEchoObject = (idEchoUUIDArrayCEchoObject + 1);
    public const ushort idEchoCYArrayCEchoObject = (idEchoCYCEchoObject + 1);
    public const ushort idEchoDateTimeCEchoObject = (idEchoCYArrayCEchoObject + 1);
    public const ushort idEchoDateTimeArrayCEchoObject = (idEchoDateTimeCEchoObject + 1);
}