﻿using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Runtime.InteropServices;

public class CSqlPlugin : CSocketProServer
{
    public CSqlPlugin(int param = 0)
        : base(param)
    {
    }

    [DllImport("sodbc")]
    private static extern int DoSPluginAuthentication(ulong hSocket, [MarshalAs(UnmanagedType.LPWStr)] string userId, [MarshalAs(UnmanagedType.LPWStr)] string password, uint nSvsId, [MarshalAs(UnmanagedType.LPWStr)] string dsn);

#if PLUGIN_DEV

#else
    internal CSocketProService<CMyHttpPeer> m_http = new CSocketProService<CMyHttpPeer>();
#endif

    public override bool Run(uint port, uint maxBacklog, bool v6Supported)
    {
        IntPtr p = CSocketProServer.DllManager.AddALibrary("sodbc");
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
        PushManager.AddAChatGroup(SocketProAdapter.UDB.DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID, "Subscribe/publish for MS SQL SERVER Table events, DELETE, INSERT and UPDATE");
        return base.Run(port, maxBacklog, v6Supported);
    }

    protected override bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
    {
#if PLUGIN_DEV
        return (DoSPluginAuthentication(hSocket, userId, password, nSvsID, "DRIVER={SQL Server Native Client 11.0};Server=(local)") > 0);
#else
        if (nSvsID == BaseServiceID.sidHTTP)
            return true; //do authentication inside the method CMyHttpPeer.DoAuthentication
        string driver = "DRIVER=" + SQLConfig.ODBCDriver + ";Server=(local)";
        return (DoSPluginAuthentication(hSocket, userId, password, nSvsID, driver) > 0);
#endif
    }
}
