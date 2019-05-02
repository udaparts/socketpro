/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Text;

//server implementation for service CEchoBasic
public class CEchoBasicPeer : CClientPeer
{
    [RequestAttr(TEchoBConst.idEchoBoolCEchoBasic)]
    private void EchoBool(bool b, out bool EchoBoolRtn)
    {
        EchoBoolRtn = b;
    }

    [RequestAttr(TEchoBConst.idEchoInt8CEchoBasic)]
    private void EchoInt8(sbyte c, out sbyte EchoInt8Rtn)
    {
        EchoInt8Rtn = c;
    }

    [RequestAttr(TEchoBConst.idEchoUInt8CEchoBasic)]
    private void EchoUInt8(byte b, out byte EchoUInt8Rtn)
    {
        EchoUInt8Rtn = b;
    }

    [RequestAttr(TEchoBConst.idEchoInt16CEchoBasic)]
    private void EchoInt16(short s, out short EchoInt16Rtn)
    {
        EchoInt16Rtn = s;
    }

    [RequestAttr(TEchoBConst.idEchoUInt16CEchoBasic)]
    private void EchoUInt16(ushort s, out ushort EchoUInt16Rtn)
    {
        EchoUInt16Rtn = s;
    }

    [RequestAttr(TEchoBConst.idEchoInt32CEchoBasic)]
    private void EchoInt32(int data, out int EchoInt32Rtn)
    {
        EchoInt32Rtn = data;
    }

    [RequestAttr(TEchoBConst.idEchoUInt32CEchoBasic)]
    private void EchoUInt32(uint data, out uint EchoUInt32Rtn)
    {
        EchoUInt32Rtn = data;
    }

    [RequestAttr(TEchoBConst.idEchoInt64CEchoBasic)]
    private void EchoInt64(long data, out long EchoInt64Rtn)
    {
        EchoInt64Rtn = data;
    }

    [RequestAttr(TEchoBConst.idEchoUInt64CEchoBasic)]
    private void EchoUInt64(ulong data, out ulong EchoUInt64Rtn)
    {
        EchoUInt64Rtn = data;
    }

    [RequestAttr(TEchoBConst.idEchoFloatCEchoBasic)]
    private void EchoFloat(float data, out float EchoFloatRtn)
    {
        EchoFloatRtn = data;
    }

    [RequestAttr(TEchoBConst.idEchoDoubleCEchoBasic)]
    private void EchoDouble(double data, out double EchoDoubleRtn)
    {
        EchoDoubleRtn = data;
    }

    [RequestAttr(TEchoBConst.idEchoStringCEchoBasic)]
    private void EchoString(string str, out string EchoStringRtn)
    {
        EchoStringRtn = str;
    }

    [RequestAttr(TEchoBConst.idEchoAStringCEchoBasic)]
    private void EchoAString(sbyte[] str, out sbyte[] EchoStringRtn)
    {
        EchoStringRtn = str;
    }

    [RequestAttr(TEchoBConst.idEchoDecimalCEchoBasic)]
    private void EchoDecimal(decimal dec, out decimal EchoDecimalRtn)
    {
        EchoDecimalRtn = dec;
    }

    [RequestAttr(TEchoBConst.idEchoWCharCEchoBasic)]
    private void EchoWChar(char wc, out char EchoWCharRtn)
    {
        EchoWCharRtn = wc;
    }

    [RequestAttr(TEchoBConst.idEchoGuidCEchoBasic)]
    private void EchoGuid(Guid guid, out Guid EchoGuidRtn)
    {
        EchoGuidRtn = guid;
    }

    [RequestAttr(TEchoBConst.idEchoDateTime)]
    private void EchoDateTime(DateTime dt, out DateTime dtRtn)
    {
        dtRtn = dt;
    }
}

public class CMySocketProServer : CSocketProServer
{
    public CMySocketProServer(int nParam = 0) : base(nParam)
    {
        
    }
    protected override void OnIdle(ulong milliseconds)
    {
		Console.WriteLine("OnIdle called with " + milliseconds);
        bool ok;
        if ((milliseconds % 7) == 4)
        {
            int count = (new System.Random().Next() % 6);
            if (count == 0)
                return;
            ok = CMyPeer.m_mq.StartJob();
            for (int n = 0; n < count; ++n)
            {
                ulong index = CMyPeer.m_mq.Enqueue(TestMeConst.idDoIdle, milliseconds, n, Encoding.ASCII.GetBytes("MyTestMessage"));
            }
            ok = CMyPeer.m_mq.EndJob();
        }
    }

    protected override void OnAccept(ulong hSocket, int nError)
    {
        Console.WriteLine("Socket accepted, connections = " + CSocketProServer.CountOfClients.ToString());
    }

    protected override void OnClose(ulong hSocket, int nError)
    {
        Console.WriteLine("Socket closed with error = " + nError + ", connections = " + CSocketProServer.CountOfClients.ToString());
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        Console.WriteLine("userId = " + userId + ", password = " + password);
        return true;
    }

    [ServiceAttr(TEchoBConst.sidCEchoBasic)]
    private CSocketProService<CEchoBasicPeer> m_CEchoBasic = new CSocketProService<CEchoBasicPeer>();

    [ServiceAttr(TEchoCConst.sidCEchoObject)]
    private CSocketProService<CEchoObjectPeer> m_CEchoObject = new CSocketProService<CEchoObjectPeer>();

    [ServiceAttr(TEchoDConst.sidCEchoSys)]
    private CSocketProService<CEchoSysPeer> m_CEchoSys = new CSocketProService<CEchoSysPeer>();

    [ServiceAttr(BaseServiceID.sidChat)]
    private CSocketProService<CClientPeer> m_Push = new CSocketProService<CClientPeer>();

    [ServiceAttr(TestMeConst.sidTestService)]
    private CSocketProService<CMyPeer> m_MyPeer = new CSocketProService<CMyPeer>();

    [ServiceAttr(HwConst.sidHelloWorld)]
    private CSocketProService<CHelloWorld> m_hw = new CSocketProService<CHelloWorld>();

    [ServiceAttr(BaseServiceID.sidHTTP)]
    private CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();

    [ServiceAttr(PerfStudyConst.sidPerfStudy)]
    private CSocketProService<CPerfStudyPeer> m_perfStudy = new CSocketProService<CPerfStudyPeer>();

    [ServiceAttr(TOneConst.sidCTOne)]
    private CSocketProService<CTOnePeer> m_CTOne = new CSocketProService<CTOnePeer>();

    [ServiceAttr(RAdoConst.sidRAdo)]
    private CSocketProService<RAdoPeer> m_RAdo = new CSocketProService<RAdoPeer>();

    [ServiceAttr(SQueueConst.sidCServerQueue)]
    private CSocketProService<CServerQueuePeer> m_svrQueue = new CSocketProService<CServerQueuePeer>();

    [ServiceAttr(TEchoDConst.sidRouteSvs0)]
    private CSocketProService<CR0Peer> m_svsR0 = new CSocketProService<CR0Peer>();
    public bool SetR0AlphaRequest()
    {
        return m_svsR0.AddAlphaRequest(TEchoDConst.idRoutorClientCount);
    }

    [ServiceAttr(TEchoDConst.sidRouteSvs1)]
    private CSocketProService<CR1Peer> m_svsR1 = new CSocketProService<CR1Peer>();
    public bool SetR1AlphaRequest()
    {
        return m_svsR1.AddAlphaRequest(TEchoDConst.idCheckRouteeServiceId);
    }
}

