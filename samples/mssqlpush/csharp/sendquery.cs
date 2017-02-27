using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using Microsoft.SqlServer.Server;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Collections.Generic;
using MsSql;

public partial class sqlpush
{
    static readonly CSqlReplication<MyAdo> m_hm;
    static sqlpush()
    {
        CClientSocket.QueueConfigure.MessageQueuePassword = "MyPassword";
        CClientSocket.QueueConfigure.WorkDirectory = "c:\\cyetest\\sqllog";
        ReplicationSetting rs = new ReplicationSetting();
        Dictionary<string, CConnectionContext> QNameConn = new Dictionary<string, CConnectionContext>();

        CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "SocketPro", "PassOne");

        string []parts = Utilities.GetObjectParts(Utilities.GetDbFullName());

        string qName = parts[2] + "_ToLocal";
        QNameConn[qName] = cc;

        cc = new CConnectionContext("10.3.10.53", 20901, "SocketPro", "PassOne");
        qName = parts[2] + "_ToLinux";
        QNameConn[qName] = cc;

        m_hm = new CSqlReplication<MyAdo>(rs, QNameConn);
    }

    [Microsoft.SqlServer.Server.SqlFunction(Name = "sendQuery", DataAccess=DataAccessKind.Read)]
    public static SqlBoolean sendQuery(SqlString sqlQuery, SqlString name)
    {
        return m_hm.Send(sqlQuery.ToString(), name.ToString());
    }
};

