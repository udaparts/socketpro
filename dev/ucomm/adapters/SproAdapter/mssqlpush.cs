using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Data;
using System.Data.SqlClient;
using Microsoft.SqlServer.Server;

namespace SocketProAdapter.ClientSide.MsSql
{
    public static class Utilities
    {
        private static char[] empty = { ' ', '\r', '\n', '\t', '[', ']', '"', '.' };

        /// <summary>
        /// Divide an object full name into parts without help from SQL server database. Use this method to fast construct database, instance and other full names without help from database. 
        /// </summary>
        /// <param name="fullName">An object full name</param>
        /// <returns>An array of strings</returns>
        public static string[] GetObjectParts(string fullName)
        {
            if (fullName == null)
                return new string[0];
            string[] sep = { "].[" };
            return fullName.Trim(empty).Split(sep, StringSplitOptions.RemoveEmptyEntries);
        }

        /// <summary>
        /// Get an object full name in the format [serverName].[dbInstance].[databaseName].[dbo].[objectName]
        /// </summary>
        /// <param name="objName">object name like table, view, trigger, stored procedure and so on within current database</param>
        /// <param name="objType">An object type referring to the column type of sys.objects</param>
        /// <returns>An object full name in the format [serverName].[dbInstance].[databaseName].[dbo].[objectName] if successful. Otherwise, an emty string if failed</returns>
        public static string GetObjectFullName(string objName, string objType)
        {
            string objFullName = null;
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                try
                {
                    conn.Open();
                    objFullName = GetObjectFullName(conn, objName, objType);
                }
                finally
                {
                    conn.Close();
                }
            }
            return objFullName;
        }

        /// <summary>
        /// Get an object full name in the format [serverName].[dbInstance].[databaseName].[dbo].[objectName]
        /// </summary>
        /// <param name="conn">An valid and opened conection</param>
        /// <param name="objName">object name like table, view, trigger, stored procedure and so on within current database</param>
        /// <param name="objType">An object type referring to the column type of sys.objects</param>
        /// <returns>An object full name in the format [serverName].[dbInstance].[databaseName].[dbo].[objectName] if successful. Otherwise, an emty string if failed</returns>
        public static string GetObjectFullName(SqlConnection conn, string objName, string objType)
        {
            if (objName == null)
                throw new ArgumentNullException("Object name can't be a null");
            if (objType == null)
                throw new ArgumentNullException("Object typecan't be a null");

            objName = objName.Trim(empty);
            if (objName.Length == 0)
                throw new ArithmeticException("Object name must not be an empty string");

            objType = objType.Trim(empty);
            if (objType.Length == 0 || objType.Length > 2)
                throw new ArithmeticException("The size of object type string must be 1 or 2");

            SqlDataReader dr = null;
            string fullName = "";
            string sqlCmd = "SELECT " + string.Format("@@servername, @@SERVICENAME, db_name(), OBJECT_SCHEMA_NAME(OBJECT_ID('{0}','{1}'))", objName, objType);
            try
            {
                SqlCommand cmd = new SqlCommand(sqlCmd, conn);
                dr = cmd.ExecuteReader();
                if (dr.Read())
                {
                    fullName = string.Format("[{0}].[{1}].[{2}].[{3}].[{4}]",
                        dr.GetString(0).Trim(empty),
                        dr.GetString(1).Trim(empty),
                        dr.GetString(2).Trim(empty),
                        dr.GetString(3).Trim(empty),
                        objName);
                }
            }
            finally
            {
                if (dr != null)
                    dr.Close();
            }
            return fullName;
        }

        /// <summary>
        /// Query a database full name from the current sql server database
        /// </summary>
        public static string GetDbFullName()
        {
            string dbFullName = null;
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                try
                {
                    conn.Open();
                    dbFullName = GetDbFullName(conn);
                }
                finally
                {
                    conn.Close();
                }
            }
            return dbFullName;
        }

        /// <summary>
        /// Query current event data inside DDL or logon triggers
        /// </summary>
        /// <returns>An XML string for event data</returns>
        public static string GetEventData()
        {
            string eventData = "";
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                try
                {
                    conn.Open();
                    eventData = GetEventData(conn);
                }
                finally
                {
                    conn.Close();
                }
            }
            return eventData;
        }

        /// <summary>
        /// Query current event data inside DDL or logon triggers
        /// </summary>
        /// <param name="conn">An valid and opened conection</param>
        /// <returns>An XML string for event data</returns>
        public static string GetEventData(SqlConnection conn)
        {
            string eventData = "";
            if (conn == null || conn.State != ConnectionState.Open)
                throw new InvalidOperationException("An opened connection required");
            SqlDataReader dr = null;
            string sqlCmd = "SELECT EVENTDATA()";
            try
            {
                SqlCommand cmd = new SqlCommand(sqlCmd, conn);
                dr = cmd.ExecuteReader();
                if (dr.Read() && !dr.IsDBNull(0))
                {
                    eventData = dr.GetString(0);
                }
            }
            finally
            {
                if (dr != null)
                    dr.Close();
            }
            return eventData;
        }

        /// <summary>
        /// Query a database full name from the current sql server database
        /// </summary>
        /// <param name="conn">An valid and opened conection</param>
        /// <returns>A database full name in the format like [serverName].[dbInstance].[databaseName]</returns>
        public static string GetDbFullName(SqlConnection conn)
        {
            if (conn == null || conn.State != ConnectionState.Open)
                throw new InvalidOperationException("An opened connection required");
            string fullDbName = null;
            SqlDataReader dr = null;
            string sqlCmd = "SELECT @@servername, @@SERVICENAME, db_name()";
            try
            {
                SqlCommand cmd = new SqlCommand(sqlCmd, conn);
                dr = cmd.ExecuteReader();
                if (dr.Read())
                {
                    fullDbName = string.Format("[{0}].[{1}].[{2}]", dr.GetString(0).Trim(empty), dr.GetString(1).Trim(empty), dr.GetString(2).Trim(empty));
                }
            }
            finally
            {
                if (dr != null)
                    dr.Close();
            }
            return fullDbName;
        }

        /// <summary>
        /// Query an instance fully name from sql server database
        /// </summary>
        /// <returns>An instance full name in the format like [serverName].[dbInstance]</returns>
        public static string GetSqlInstanceFullName()
        {
            string fullName = null;
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                try
                {
                    conn.Open();
                    fullName = GetSqlInstanceFullName(conn);
                }
                finally
                {
                    conn.Close();
                }
            }
            return fullName;
        }

        /// <summary>
        /// Query an instance fully name from sql server database
        /// </summary>
        /// <param name="conn">An valid and opened conection</param>
        /// <returns>An instance full name in the format like [serverName].[dbInstance]</returns>
        public static string GetSqlInstanceFullName(SqlConnection conn)
        {
            if (conn == null || conn.State != ConnectionState.Open)
                throw new InvalidOperationException("An opened connection required");
            string fullDbName = null;
            SqlDataReader dr = null;
            string sqlCmd = "SELECT @@servername, @@SERVICENAME";
            try
            {
                SqlCommand cmd = new SqlCommand(sqlCmd, conn);
                dr = cmd.ExecuteReader();
                if (dr.Read())
                {
                    fullDbName = string.Format("[{0}].[{1}]", dr.GetString(0).Trim(empty), dr.GetString(1).Trim(empty));
                }
            }
            finally
            {
                if (dr != null)
                    dr.Close();
            }
            return fullDbName;
        }
    }

    public class CSqlReplication<THandler> : CReplication<THandler>
        where THandler : CAsyncAdohandler, new()
    {
        private object m_csSs = new object();
        /// <summary>
        /// Send a record set from a given query statement onto one or more remote SocketPro servers with auto replication if required
        /// </summary>
        /// <param name="sqlQuery">A statement creating a set of records</param>
        /// <param name="recordsetName">An string name for this record set</param>
        /// <returns>True for success; and false for failure</returns>
        public bool Send(string sqlQuery, string recordsetName)
        {
            return Send(sqlQuery, recordsetName, CAsyncAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        /// <summary>
        /// Send affected record set from a trigger for table update, delete or insert onto one or more remote SocketPro servers with auto replication if required.
        /// </summary>
        /// <param name="tableName">A valid table name</param>
        /// <param name="param">An extra info data. For example, a trigger name</param>
        /// <returns>True for success; and false for failure</returns>
        public bool SendDmlTrigger(string tableName, object param)
        {
            return SendDmlTrigger(tableName, param, CAsyncAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }


        /// <summary>
        /// Send a record set from a given query statement onto one or more remote SocketPro servers with auto replication if required
        /// </summary>
        /// <param name="sqlQuery">A statement creating a set of records</param>
        /// <param name="recordsetName">An string name for this record set</param>
        /// <param name="batchSize">The size of a set of records in byte. It defaults to CAsyncAdoSerializationHelper.DEFAULT_BATCH_SIZE</param>
        public virtual bool Send(string sqlQuery, string recordsetName, uint batchSize)
        {
            bool jobing = false;
            bool ok = false;
            if (sqlQuery == null)
                throw new ArgumentNullException("A valid sql select query required");
            if (sqlQuery.Length == 0)
                throw new ArgumentException("A valid sql select query required");
            IClientQueue srcQueue = SourceQueue;
            if (!srcQueue.Available)
                srcQueue = null;
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                CAsyncServiceHandler.DAsyncResultHandler arh = null;
                SqlDataReader dr = null;
                try
                {
                    conn.Open();
                    lock (m_csSs)
                    {
                        if (srcQueue != null)
                            jobing = (srcQueue.JobSize != 0);
                        THandler h = SourceHandler;
                        try
                        {
                            if (srcQueue != null && !jobing)
                                ok = srcQueue.StartJob();
                            ok = h.SendRequest(CAsyncAdoSerializationHelper.idRecordsetName, recordsetName, arh);
                            SqlCommand cmd = new SqlCommand(sqlQuery, conn);
                            dr = cmd.ExecuteReader();
                            do
                            {
                                ok = h.Send(dr, batchSize);
                            } while (dr.NextResult());
                            dr.Close();
                            if (srcQueue != null && !jobing)
                                ok = srcQueue.EndJob();
                            ok = true;
                        }
                        catch
                        {
                            if (dr != null)
                                dr.Close();
                            if (srcQueue != null && !jobing)
                                ok = srcQueue.AbortJob();
                            ok = false;
                        }
                    }
                }
                finally
                {
                    conn.Close();
                    if (!jobing && Replicable && ok)
                    {
                        ok = DoReplication();
                    }
                }
            }
            return ok;
        }

        /// <summary>
        /// Send affected record set from a trigger for table update, delete or insert onto one or more remote SocketPro servers with auto replication if required.
        /// </summary>
        /// <param name="tableName">A valid table name</param>
        /// <param name="param">An extra info data. For example, a trigger name</param>
        /// <param name="batchSize">The size of a set of records in byte. It defaults to CAsyncAdoSerializationHelper.DEFAULT_BATCH_SIZE</param>
        /// <returns>True for success; and false for failure</returns>
        public virtual bool SendDmlTrigger(string tableName, object param, uint batchSize)
        {
            bool jobing = false;
            bool ok = true;
            if (tableName == null)
                throw new ArgumentNullException("A valid table name required");
            if (tableName.Length == 0)
                throw new ArgumentException("A valid table name required");

            IClientQueue srcQueue = SourceQueue;
            if (!srcQueue.Available)
                srcQueue = null;

            TriggerAction ta = SqlContext.TriggerContext.TriggerAction;
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                CAsyncServiceHandler.DAsyncResultHandler arh = null;
                SqlDataReader dr = null;
                try
                {
                    conn.Open();
                    lock (m_csSs)
                    {
                        if (srcQueue != null)
                            jobing = (srcQueue.JobSize != 0);
                        THandler h = SourceHandler;
                        try
                        {
                            if (srcQueue != null && !jobing)
                                ok = srcQueue.StartJob();
                            switch (ta)
                            {
                                case TriggerAction.Update:
                                case TriggerAction.Delete:
                                case TriggerAction.Insert:
                                    ok = h.SendRequest(CAsyncAdoSerializationHelper.idDmlTriggerMessage, (int)ta, Utilities.GetObjectFullName(conn, tableName, "U"), param, arh);
                                    break;
                                default:
                                    throw new InvalidOperationException("SendDmlTrigger for table update, insert and delete events only");
                            }

                            if (ta == TriggerAction.Update || ta == TriggerAction.Delete)
                            {
                                SqlCommand cmd = new SqlCommand("select * from deleted", conn);
                                dr = cmd.ExecuteReader();
                                ok = h.Send(dr, batchSize);
                                dr.Close();
                            }

                            if (ta == TriggerAction.Update || ta == TriggerAction.Insert)
                            {
                                SqlCommand cmd = new SqlCommand("select * from inserted", conn);
                                dr = cmd.ExecuteReader();
                                ok = h.Send(dr, batchSize);
                                dr.Close();
                            }

                            if (srcQueue != null && !jobing)
                                ok = srcQueue.EndJob();
                            ok = true;
                        }
                        catch
                        {
                            if (dr != null)
                                dr.Close();
                            if (srcQueue != null && !jobing)
                                ok = srcQueue.AbortJob();
                            ok = false;
                        }
                    }
                }
                finally
                {
                    conn.Close();
                    if (Replicable && ok && !jobing)
                    {
                        ok = DoReplication();
                    }
                }
            }
            return ok;
        }

        /// <summary>
        /// Construct a CSqlReplication instance
        /// </summary>
        /// <param name="qms">A structure for setting its underlying socket pool and message queue directory as well as password for source queue</param>
        public CSqlReplication(ReplicationSetting qms)
            : base(qms)
        {
        }
    }
}
