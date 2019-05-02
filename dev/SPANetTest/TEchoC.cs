

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class CEchoObject : CAsyncServiceHandler
{
    public CEchoObject()
        : base(TEchoCConst.sidCEchoObject)
    {
    }

    public object EchoDateTime(object datetime)
    {
        object EchoDateTimeRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoDateTimeCEchoObject, datetime, out EchoDateTimeRtn);
        return EchoDateTimeRtn;
    }

    public object EchoDateTimeArray(object datetimeArray)
    {
        object EchoDateTimeArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoDateTimeArrayCEchoObject, datetimeArray, out EchoDateTimeArrayRtn);
        return EchoDateTimeArrayRtn;
    }

    public object EchoEmpty(object empty)
    {
        object EchoEmptyRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoEmptyCEchoObject, empty, out EchoEmptyRtn);
        return EchoEmptyRtn;
    }

    public object EchoBool(object b)
    {
        object EchoBoolRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoBoolCEchoObject, b, out EchoBoolRtn);
        return EchoBoolRtn;
    }

    public object EchoInt8(object c)
    {
        object EchoInt8Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt8CEchoObject, c, out EchoInt8Rtn);
        return EchoInt8Rtn;
    }

    public object EchoUInt8(object b)
    {
        object EchoUInt8Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt8CEchoObject, b, out EchoUInt8Rtn);
        return EchoUInt8Rtn;
    }

    public object EchoInt16(object s)
    {
        object EchoInt16Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt16CEchoObject, s, out EchoInt16Rtn);
        return EchoInt16Rtn;
    }

    public object EchoUInt16(object s)
    {
        object EchoUInt16Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt16CEchoObject, s, out EchoUInt16Rtn);
        return EchoUInt16Rtn;
    }

    public object EchoInt32(object data)
    {
        object EchoInt32Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt32CEchoObject, data, out EchoInt32Rtn);
        return EchoInt32Rtn;
    }

    public object EchoUInt32(object data)
    {
        object EchoUInt32Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt32CEchoObject, data, out EchoUInt32Rtn);
        return EchoUInt32Rtn;
    }

    public object EchoInt64(object data)
    {
        object EchoInt64Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt64CEchoObject, data, out EchoInt64Rtn);
        return EchoInt64Rtn;
    }

    public object EchoUInt64(object data)
    {
        object EchoUInt64Rtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt64CEchoObject, data, out EchoUInt64Rtn);
        return EchoUInt64Rtn;
    }

    public object EchoFloat(object data)
    {
        object EchoFloatRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoFloatCEchoObject, data, out EchoFloatRtn);
        return EchoFloatRtn;
    }

    public object EchoDouble(object data)
    {
        object EchoDoubleRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoDoubleCEchoObject, data, out EchoDoubleRtn);
        return EchoDoubleRtn;
    }

    public object EchoString(object str)
    {
        object EchoStringRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoStringCEchoObject, str, out EchoStringRtn);
        return EchoStringRtn;
    }

    public object EchoAString(object str)
    {
        object EchoAStringRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoAStringCEchoObject, str, out EchoAStringRtn);
        return EchoAStringRtn;
    }

    public object EchoDecimal(object dec)
    {
        object EchoDecimalRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoDecimalCEchoObject, dec, out EchoDecimalRtn);
        return EchoDecimalRtn;
    }

    public object EchoBoolArray(object bArr)
    {
        object EchoBoolArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoBoolArrayCEchoObject, bArr, out EchoBoolArrayRtn);
        return EchoBoolArrayRtn;
    }

    public object EchoUInt8Array(object bArr)
    {
        object EchoUInt8ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt8ArrayCEchoObject, bArr, out EchoUInt8ArrayRtn);
        return EchoUInt8ArrayRtn;
    }

    public object EchoInt16Array(object shortArr)
    {
        object EchoInt16ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt16ArrayCEchoObject, shortArr, out EchoInt16ArrayRtn);
        return EchoInt16ArrayRtn;
    }

    public object EchoUInt16Array(object ushortArr)
    {
        object EchoUInt16ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt16ArrayCEchoObject, ushortArr, out EchoUInt16ArrayRtn);
        return EchoUInt16ArrayRtn;
    }

    public object EchoInt32Array(object intArr)
    {
        object EchoInt32ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt32ArrayCEchoObject, intArr, out EchoInt32ArrayRtn);
        return EchoInt32ArrayRtn;
    }

    public object EchoUInt32Array(object uintArr)
    {
        object EchoUInt32ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt32ArrayCEchoObject, uintArr, out EchoUInt32ArrayRtn);
        return EchoUInt32ArrayRtn;
    }

    public object EchoInt64Array(object longArr)
    {
        object EchoInt64ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoInt64ArrayCEchoObject, longArr, out EchoInt64ArrayRtn);
        return EchoInt64ArrayRtn;
    }

    public object EchoUInt64Array(object ulongArr)
    {
        object EchoUInt64ArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUInt64ArrayCEchoObject, ulongArr, out EchoUInt64ArrayRtn);
        return EchoUInt64ArrayRtn;
    }

    public object EchoFloatArray(object fArr)
    {
        object EchoFloatArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoFloatArrayCEchoObject, fArr, out EchoFloatArrayRtn);
        return EchoFloatArrayRtn;
    }

    public object EchoDoubleArray(object dArr)
    {
        object EchoDoubleArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoDoubleArrayCEchoObject, dArr, out EchoDoubleArrayRtn);
        return EchoDoubleArrayRtn;
    }

    public object EchoStringArray(object strArr)
    {
        object EchoStringArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoStringArrayCEchoObject, strArr, out EchoStringArrayRtn);
        return EchoStringArrayRtn;
    }

    public object EchoDecimalArray(object decArr)
    {
        object EchoDecimalArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoDecimalArrayCEchoObject, decArr, out EchoDecimalArrayRtn);
        return EchoDecimalArrayRtn;
    }

    public object EchoUUID(object clsid)
    {
        object EchoUUIDRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUUIDCEchoObject, clsid, out EchoUUIDRtn);
        return EchoUUIDRtn;
    }

    public object EchoUUIDArray(object clsidArr)
    {
        object EchoUUIDArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoUUIDArrayCEchoObject, clsidArr, out EchoUUIDArrayRtn);
        return EchoUUIDArrayRtn;
    }

    public object EchoCY(object cy)
    {
        object EchoCYRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoCYCEchoObject, cy, out EchoCYRtn);
        return EchoCYRtn;
    }

    public object EchoCYArray(object cyArray)
    {
        object EchoCYArrayRtn;
        bool bProcessRy = ProcessR1(TEchoCConst.idEchoCYArrayCEchoObject, cyArray, out EchoCYArrayRtn);
        return EchoCYArrayRtn;
    }
}
