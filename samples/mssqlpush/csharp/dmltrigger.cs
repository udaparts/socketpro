using System;
using System.Data;
using System.Data.SqlClient;
using Microsoft.SqlServer.Server;

public partial class sqlpush
{
    [Microsoft.SqlServer.Server.SqlTrigger(Name = "dmltrigger", Target = "reptable", Event = "FOR INSERT, UPDATE, DELETE")]
    public static void dmltrigger()
    {
        bool ok = m_hm.SendDmlTrigger("reptable", "dmltrigger");
    }
}
