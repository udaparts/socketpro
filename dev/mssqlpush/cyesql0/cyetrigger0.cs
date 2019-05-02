using System;
using System.Data;
using System.Collections.Generic;
using System.Data.SqlClient;
using Microsoft.SqlServer.Server;
using System.Runtime.InteropServices;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.MsSql;

public partial class sqlpush
{
    static CSqlReplication<MyAdo> m_hm;
    static sqlpush()
    {
        CClientSocket.WorkDirectory = "c:\\cyetest";
        ReplicationSetting rs = new ReplicationSetting();
        rs.Password = "MyPassword";
        rs.QueueDir = @"c:\cyetest";
        Dictionary<CConnectionContext, string> ConnQueue = new Dictionary<CConnectionContext, string>();

        CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "SocketPro", "PassOne");
        ConnQueue[cc] = "ToLocal";

        cc = new CConnectionContext("10.3.10.128", 20901, "SocketPro", "PassOne");
        ConnQueue[cc] = "Tolinux";

        m_hm = new CSqlReplication<MyAdo>(ConnQueue, rs);
    }

    [Microsoft.SqlServer.Server.SqlTrigger(Name = "cyetrigger0", Target = "cyetable0", Event = "FOR INSERT, UPDATE, DELETE")]
    public static void cyetrigger0()
    {
        bool ok = m_hm.SendDmlTrigger("cyetable0", "cyetrigger0");
    }
}
