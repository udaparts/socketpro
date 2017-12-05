
using System;
using System.Data;
using System.Data.SqlClient;

namespace MsSql
{
    public static class Utilities
    {
        private static readonly char[] empty = { ' ', '\r', '\n', '\t', '[', ']', '"', '.' };

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

        public static string[] GetUSqlServerKeys()
        {
            string []v = null;
            using (SqlConnection conn = new SqlConnection("context connection=true"))
            {
                try
                {
                    conn.Open();
                    v = GetUSqlServerKeys(conn);
                }
                finally
                {
                    conn.Close();
                }
            }
            return v;
        }

        public static string[] GetUSqlServerKeys(SqlConnection conn)
        {
            Exception ex = null;
            if (conn == null || conn.State != ConnectionState.Open)
                throw new InvalidOperationException("An opened connection required");
            string[] v = null;
            SqlDataReader dr = null;
            string sqlCmd = "select @@SERVICENAME + '@' + @@servername, SUSER_NAME(), DB_NAME()";
            try
            {
                SqlCommand cmd = new SqlCommand(sqlCmd, conn);
                dr = cmd.ExecuteReader();
                if (dr.Read())
                {
                    v = new string[3];
                    v[0] = dr.GetString(0);
                    v[1] = dr.GetString(1);
                    v[2] = dr.GetString(2);
                }
            }
            catch (Exception err)
            {
                ex = err;
            }
            finally
            {
                if (dr != null)
                    dr.Close();
            }
            if (ex != null)
                throw ex;
            return v;
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

        /// <summary>
        /// Query a server name from sql server database for connection string
        /// </summary>
        /// <param name="conn">An valid and opened conection</param>
        /// <returns>An instance full name in the format like [serverName].[dbInstance]</returns>
        public static string GetServerName(SqlConnection conn)
        {
            if (conn == null || conn.State != ConnectionState.Open)
                throw new InvalidOperationException("An opened connection required");
            string serverName = Environment.MachineName;
            SqlDataReader dr = null;
            string sqlCmd = "SELECT @@servername";
            try
            {
                SqlCommand cmd = new SqlCommand(sqlCmd, conn);
                dr = cmd.ExecuteReader();
                if (dr.Read())
                {
                    if (dr.IsDBNull(0))
                    {
                        dr.Close();
                        sqlCmd = "SELECT @@SERVICENAME";
                        cmd.CommandText = sqlCmd;
                        dr = cmd.ExecuteReader();
                        if (dr.Read())
                            serverName += ("\\" + dr.GetString(0));
                    }
                    else
                        serverName = dr.GetString(0);
                }
            }
            finally
            {
                if (dr != null)
                    dr.Close();
            }
            return serverName;
        }
    }
}
