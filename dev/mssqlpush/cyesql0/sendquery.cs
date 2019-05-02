using System;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using Microsoft.SqlServer.Server;

public partial class sqlpush
{
    [Microsoft.SqlServer.Server.SqlFunction(Name = "sendquery", DataAccess=DataAccessKind.Read)]
    public static SqlBoolean sendquery(SqlString sqlQuery, SqlString name)
    {
        return m_hm.Send(sqlQuery.ToString(), name.ToString());
    }
};

