using System;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using Microsoft.SqlServer.Server;

public partial class sqlpush
{
    [Microsoft.SqlServer.Server.SqlFunction(Name = "getConnections", DataAccess = DataAccessKind.Read)]
    public static SqlInt32 getConnections()
    {
        return (int)m_hm.Connections;
    }

    [Microsoft.SqlServer.Server.SqlFunction(Name = "getQueues", DataAccess = DataAccessKind.Read)]
    public static SqlInt32 getQueues()
    {
        return (int)m_hm.Queues;
    }

    [Microsoft.SqlServer.Server.SqlFunction(Name = "getHosts", DataAccess = DataAccessKind.Read)]
    public static SqlInt32 getHosts()
    {
        return (int)m_hm.Hosts;
    }
};

