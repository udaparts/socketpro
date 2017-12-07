
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.UDB;
using System.Data.SqlClient;

class CSqlPlugin : CSocketProServer
{
    public CSqlPlugin(int param = 0)
        : base(param)
    {
    }

    [ServiceAttr(CStreamSql.sidMsSql)]
    internal CSocketProService<CStreamSql> StreamSql = new CSocketProService<CStreamSql>();

#if PLUGIN_DEV

#else
    internal CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();
#endif

    public override bool Run(uint port, uint maxBacklog, bool v6Supported)
    {
#if PLUGIN_DEV

#else
        if (SQLConfig.HttpWebSocket)
            m_http.AddMe(BaseServiceID.sidHTTP);
        string[] vService = SQLConfig.Services.Split(';');
        foreach (string s in vService)
        {
            if (s.Length > 0)
                DllManager.AddALibrary(s);
        }
#endif
        PushManager.AddAChatGroup(DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for MS SQL SERVER Table events, DELETE, INSERT and UPDATE");
        return base.Run(port, maxBacklog, v6Supported);
    }

    private bool DoDBAuthentication(ulong hSocket, string userId, string password)
    {
#if PLUGIN_DEV
        string connection = string.Format("Server={0};User Id={1};Password={2}", "CYE-WIN8", userId, password);
#else
        string connection = string.Format("Server={0};User Id={1};Password={2}", SQLConfig.Server, userId, password);
#endif
        try
        {
            SqlConnection conn = new SqlConnection(connection);
            conn.Open();
            lock (CStreamSql.m_csPeer)
            {
                //Remember connection without re-connecting. The database session is closed when socket is closed
                CStreamSql.m_mapConnection.Add(hSocket, conn);
            }
            return true;
        }
        catch { }
        return false;
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
        switch (nSvsID)
        {
            case CStreamSql.sidMsSql:
                return DoDBAuthentication(hSocket, userId, password);
            case BaseServiceID.sidHTTP:
                return true; //do authentication inside the method CMyHttpPeer.DoAuthentication
            case BaseServiceID.sidChat: //SocketPro async persistent message queue
                break;
            default:
                break;
        }
        return true;
    }
}

