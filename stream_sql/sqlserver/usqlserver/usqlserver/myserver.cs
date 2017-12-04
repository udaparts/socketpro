using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.UDB;

class CSqlPlugin : CSocketProServer
{
    public CSqlPlugin(int param = 0)
        : base(param)
    {

    }

    [ServiceAttr(CStreamSql.sidMsSql)]
    internal CSocketProService<CStreamSql> StreamSql = new CSocketProService<CStreamSql>();

    internal CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();

    public override bool Run(uint port, uint maxBacklog, bool v6Supported)
    {
        if (SQLConfig.HttpWebSocket)
            m_http.AddMe(BaseServiceID.sidHTTP);
        string[] vService = SQLConfig.Services.Split(';');
        foreach (string s in vService)
        {
            if (s.Length > 0)
                DllManager.AddALibrary(s);
        }
        PushManager.AddAChatGroup(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for MS SQL SERVER Table events, DELETE, INSERT and UPDATE");
        return base.Run(port, maxBacklog, v6Supported);
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        Console.WriteLine("Ask for a service " + nSvsID + " from user " + userId + " with password = " + password);
        return true;
    }
}

