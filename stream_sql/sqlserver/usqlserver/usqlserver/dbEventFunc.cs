using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using Microsoft.SqlServer.Server;
using SocketProAdapter;
using SocketProAdapter.UDB;

public static class DBEvents
{
    [Microsoft.SqlServer.Server.SqlFunction(DataAccess=DataAccessKind.Read)]
    public static SqlString PublishDBEvent(SqlInt32 eventType, SqlString host, SqlString dbUser, SqlString dbName, SqlString tblName)
    {
        tagUpdateEvent et = (tagUpdateEvent)eventType.Value;
        string sHost = host.Value;
        string sdbuser = host.Value;
        string sdbName = host.Value;
        string stblName = host.Value;


        // Put your code here
        return new SqlString("Hello World " + MsSql.Utilities.GetDbFullName());
    }
}

