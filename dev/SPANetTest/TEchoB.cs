

using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

public class CEchoBasic : CAsyncServiceHandler
{
    public CEchoBasic()
        : base(TEchoBConst.sidCEchoBasic)
    {
    }

    public bool EchoBool(bool b)
    {
        bool EchoBoolRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoBoolCEchoBasic, b, out EchoBoolRtn);
        return EchoBoolRtn;
    }

    public sbyte EchoInt8(sbyte c)
    {
        sbyte EchoInt8Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoInt8CEchoBasic, c, out EchoInt8Rtn);
        return EchoInt8Rtn;
    }

    public byte EchoUInt8(byte b)
    {
        byte EchoUInt8Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoUInt8CEchoBasic, b, out EchoUInt8Rtn);
        return EchoUInt8Rtn;
    }

    public short EchoInt16(short s)
    {
        short EchoInt16Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoInt16CEchoBasic, s, out EchoInt16Rtn);
        return EchoInt16Rtn;
    }

    public ushort EchoUInt16(ushort s)
    {
        ushort EchoUInt16Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoUInt16CEchoBasic, s, out EchoUInt16Rtn);
        return EchoUInt16Rtn;
    }

    public int EchoInt32(int data)
    {
        int EchoInt32Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoInt32CEchoBasic, data, out EchoInt32Rtn);
        return EchoInt32Rtn;
    }

    public uint EchoUInt32(uint data)
    {
        uint EchoUInt32Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoUInt32CEchoBasic, data, out EchoUInt32Rtn);
        return EchoUInt32Rtn;
    }

    public long EchoInt64(long data)
    {
        long EchoInt64Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoInt64CEchoBasic, data, out EchoInt64Rtn);
        return EchoInt64Rtn;
    }

    public ulong EchoUInt64(ulong data)
    {
        ulong EchoUInt64Rtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoUInt64CEchoBasic, data, out EchoUInt64Rtn);
        return EchoUInt64Rtn;
    }

    public float EchoFloat(float data)
    {
        float EchoFloatRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoFloatCEchoBasic, data, out EchoFloatRtn);
        return EchoFloatRtn;
    }

    public double EchoDouble(double data)
    {
        double EchoDoubleRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoDoubleCEchoBasic, data, out EchoDoubleRtn);
        return EchoDoubleRtn;
    }

    public string EchoString(string str)
    {
        string EchoStringRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoStringCEchoBasic, str, out EchoStringRtn);
        return EchoStringRtn;
    }

    public sbyte[] EchoAString(byte[] str)
    {
        sbyte[] EchoAStringRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoAStringCEchoBasic, str, out EchoAStringRtn);
        return EchoAStringRtn;
    }

    public decimal EchoDecimal(decimal dec)
    {
        decimal EchoDecimalRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoDecimalCEchoBasic, dec, out EchoDecimalRtn);
        return EchoDecimalRtn;
    }

    public char EchoWChar(char wc)
    {
        char EchoWCharRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoWCharCEchoBasic, wc, out EchoWCharRtn);
        return EchoWCharRtn;
    }

    public Guid EchoGuid(Guid guid)
    {
        Guid EchoGuidRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoGuidCEchoBasic, guid, out EchoGuidRtn);
        return EchoGuidRtn;
    }

    public DateTime EchoDateTime(DateTime dt)
    {
        DateTime dtRtn;
        bool bProcessRy = ProcessR1(TEchoBConst.idEchoDateTime, dt, out dtRtn);
        return dtRtn;
    }

    //public CY EchoCy(CY cy)
    //{
    //    CY EchoCyRtn;
    //    bool bProcessRy = ProcessR1(TEchoBConst.idEchoCyCEchoBasic, cy, out EchoCyRtn);
    //    return EchoCyRtn;
    //}
}
