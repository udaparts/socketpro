using System;
using System.Data;
using System.Data.SqlClient;
using Microsoft.SqlServer.Server;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.MsSql;
using System.Collections.Generic;

public partial class sqlpush
{
    [Microsoft.SqlServer.Server.SqlTrigger(Name = "logontrigger", Target = "ALL SERVER", Event = "FOR LOGON")]
    public static void logontrigger()
    {
        m_hm.Send(SocketProAdapter.CAsyncAdoSerializationHelper.idDbEventTriggerMessage,
                    (int)TriggerAction.AlterLogin,
                    Utilities.GetSqlInstanceFullName(),
                    Utilities.GetEventData());
    }
}
