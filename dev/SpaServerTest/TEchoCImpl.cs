/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

//server implementation for service CEchoObject
public class CEchoObjectPeer : CClientPeer
{
    [RequestAttr(TEchoCConst.idEchoEmptyCEchoObject)]
    private void EchoEmpty(object empty, out object EchoEmptyRtn)
    {
        EchoEmptyRtn = empty;
    }

    [RequestAttr(TEchoCConst.idEchoBoolCEchoObject)]
    private void EchoBool(object b, out object EchoBoolRtn)
    {
        EchoBoolRtn = b;
    }

    [RequestAttr(TEchoCConst.idEchoInt8CEchoObject)]
    private void EchoInt8(object c, out object EchoInt8Rtn)
    {
        EchoInt8Rtn = c;
    }

    [RequestAttr(TEchoCConst.idEchoUInt8CEchoObject)]
    private void EchoUInt8(object b, out object EchoUInt8Rtn)
    {
        EchoUInt8Rtn = b;
    }

    [RequestAttr(TEchoCConst.idEchoInt16CEchoObject)]
    private void EchoInt16(object s, out object EchoInt16Rtn)
    {
        EchoInt16Rtn = s;
    }

    [RequestAttr(TEchoCConst.idEchoUInt16CEchoObject)]
    private void EchoUInt16(object s, out object EchoUInt16Rtn)
    {
        EchoUInt16Rtn = s;
    }

    [RequestAttr(TEchoCConst.idEchoInt32CEchoObject)]
    private void EchoInt32(object data, out object EchoInt32Rtn)
    {
        EchoInt32Rtn = data;
    }

    [RequestAttr(TEchoCConst.idEchoUInt32CEchoObject)]
    private void EchoUInt32(object data, out object EchoUInt32Rtn)
    {
        EchoUInt32Rtn = data;
    }

    [RequestAttr(TEchoCConst.idEchoInt64CEchoObject)]
    private void EchoInt64(object data, out object EchoInt64Rtn)
    {
        EchoInt64Rtn = data;
    }

    [RequestAttr(TEchoCConst.idEchoUInt64CEchoObject)]
    private void EchoUInt64(object data, out object EchoUInt64Rtn)
    {
        EchoUInt64Rtn = data;
    }

    [RequestAttr(TEchoCConst.idEchoFloatCEchoObject)]
    private void EchoFloat(object data, out object EchoFloatRtn)
    {
        EchoFloatRtn = data;
    }

    [RequestAttr(TEchoCConst.idEchoDoubleCEchoObject)]
    private void EchoDouble(object data, out object EchoDoubleRtn)
    {
        EchoDoubleRtn = data;
    }

    [RequestAttr(TEchoCConst.idEchoStringCEchoObject)]
    private void EchoString(object str, out object EchoStringRtn)
    {
        EchoStringRtn = str;
    }

    [RequestAttr(TEchoCConst.idEchoAStringCEchoObject)]
    private void EchoAString(object str, out object EchoAStringRtn)
    {
        EchoAStringRtn = str;
    }

    [RequestAttr(TEchoCConst.idEchoDecimalCEchoObject)]
    private void EchoDecimal(object dec, out object EchoDecimalRtn)
    {
        EchoDecimalRtn = dec;
    }

    [RequestAttr(TEchoCConst.idEchoDateTimeCEchoObject)]
    private void EchoDateTime(object datetime, out object EchoDateTimeRtn)
    {
        EchoDateTimeRtn = datetime;
    }

    [RequestAttr(TEchoCConst.idEchoDateTimeArrayCEchoObject)]
    private void EchoDateTimeArray(object datetimeArray, out object EchoDateTimeArrayRtn)
    {
        EchoDateTimeArrayRtn = datetimeArray;
    }

    [RequestAttr(TEchoCConst.idEchoBoolArrayCEchoObject)]
    private void EchoBoolArray(object bArr, out object EchoBoolArrayRtn)
    {
        EchoBoolArrayRtn = bArr;
    }

    [RequestAttr(TEchoCConst.idEchoUInt8ArrayCEchoObject)]
    private void EchoUInt8Array(object bArr, out object EchoUInt8ArrayRtn)
    {
        EchoUInt8ArrayRtn = bArr;
    }

    [RequestAttr(TEchoCConst.idEchoInt16ArrayCEchoObject)]
    private void EchoInt16Array(object shortArr, out object EchoInt16ArrayRtn)
    {
        EchoInt16ArrayRtn = shortArr;
    }

    [RequestAttr(TEchoCConst.idEchoUInt16ArrayCEchoObject)]
    private void EchoUInt16Array(object ushortArr, out object EchoUInt16ArrayRtn)
    {
        EchoUInt16ArrayRtn = ushortArr;
    }

    [RequestAttr(TEchoCConst.idEchoInt32ArrayCEchoObject)]
    private void EchoInt32Array(object intArr, out object EchoInt32ArrayRtn)
    {
        EchoInt32ArrayRtn = intArr;
    }

    [RequestAttr(TEchoCConst.idEchoUInt32ArrayCEchoObject)]
    private void EchoUInt32Array(object uintArr, out object EchoUInt32ArrayRtn)
    {
        EchoUInt32ArrayRtn = uintArr;
    }

    [RequestAttr(TEchoCConst.idEchoInt64ArrayCEchoObject)]
    private void EchoInt64Array(object longArr, out object EchoInt64ArrayRtn)
    {
        EchoInt64ArrayRtn = longArr;
    }

    [RequestAttr(TEchoCConst.idEchoUInt64ArrayCEchoObject)]
    private void EchoUInt64Array(object ulongArr, out object EchoUInt64ArrayRtn)
    {
        EchoUInt64ArrayRtn = ulongArr;
    }

    [RequestAttr(TEchoCConst.idEchoFloatArrayCEchoObject)]
    private void EchoFloatArray(object fArr, out object EchoFloatArrayRtn)
    {
        EchoFloatArrayRtn = fArr;
    }

    [RequestAttr(TEchoCConst.idEchoDoubleArrayCEchoObject)]
    private void EchoDoubleArray(object dArr, out object EchoDoubleArrayRtn)
    {
        EchoDoubleArrayRtn = dArr;
    }

    [RequestAttr(TEchoCConst.idEchoStringArrayCEchoObject)]
    private void EchoStringArray(object strArr, out object EchoStringArrayRtn)
    {
        EchoStringArrayRtn = strArr;
    }

    [RequestAttr(TEchoCConst.idEchoDecimalArrayCEchoObject)]
    private void EchoDecimalArray(object decArr, out object EchoDecimalArrayRtn)
    {
        EchoDecimalArrayRtn = decArr;
    }

    [RequestAttr(TEchoCConst.idEchoUUIDCEchoObject)]
    private void EchoUUID(object clsid, out object EchoUUIDRtn)
    {
        EchoUUIDRtn = clsid;
    }

    [RequestAttr(TEchoCConst.idEchoUUIDArrayCEchoObject)]
    private void EchoUUIDArray(object clsidArr, out object EchoUUIDArrayRtn)
    {
        EchoUUIDArrayRtn = clsidArr;
    }

    [RequestAttr(TEchoCConst.idEchoCYCEchoObject)]
    private void EchoCY(object cy, out object EchoCYRtn)
    {
        EchoCYRtn = cy;
    }

    [RequestAttr(TEchoCConst.idEchoCYArrayCEchoObject)]
    private void EchoCYArray(object cyArray, out object EchoCYArrayRtn)
    {
        EchoCYArrayRtn = cyArray;
    }
}


