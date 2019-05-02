using SocketProAdapter;

public static class TEchoDConst
{
    //defines for service CEchoSys
    public const uint sidCEchoSys = (SocketProAdapter.BaseServiceID.sidReserved + 3);
    public const uint sidRouteSvs0 = (SocketProAdapter.BaseServiceID.sidReserved + 120);
    public const uint sidRouteSvs1 = (sidRouteSvs0 + 1);

    public const ushort idEchoMyStructCEchoSys = ((ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo + 1);
    public const ushort idEchoUQueueCEchoSys = (idEchoMyStructCEchoSys + 1);
    public const ushort idEchoComplex0CEchoSys = (idEchoUQueueCEchoSys + 1);
    public const ushort idREcho0 = (idEchoComplex0CEchoSys + 1);
    public const ushort idREcho1 = (idREcho0 + 1);

    public const ushort idRoutorClientCount = (idREcho1 + 1);
    public const ushort idCheckRouteeServiceId = (idRoutorClientCount + 1);

    public const ushort idRouteComplex = (idCheckRouteeServiceId + 1);
    public const ushort idRouteStruct = (idRouteComplex + 1);

    public const ushort idRouteFake0 = (idRouteStruct + 1);
    public const ushort idRouteFake1 = (idRouteFake0 + 1);
}

public class MyStruct : SocketProAdapter.IUSerializer
{
    public byte[] AString;
    public bool ABool;
    public string WString;
    public uint AInt;

    #region IUSerializer Members

    public void LoadFrom(CUQueue UQueue)
    {
        UQueue.Load(out AString).Load(out ABool).Load(out WString).Load(out AInt);
    }

    public void SaveTo(CUQueue UQueue)
    {
        UQueue.Save(AString).Save(ABool).Save(WString).Save(AInt);
    }

    #endregion

    public override string ToString()
    {
        string astr = CUQueue.ToString(AString);
        return "Bool = " + ABool + ", WString = " + WString + ", AInt = " + AInt + ", AString = " + astr;
    }
};
