
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Collections.Generic;
using System.Data;
using MySql.Data.MySqlClient;
using System.Net;

public class CMySqlCachePeer : CClientPeer
{
    //268435456 (SocketProAdapter.BaseServiceID.sidReserved) + 1024 = 268436480
    public const uint sidMySqlCache = BaseServiceID.sidReserved + 1024;

    private const ushort idQuerying = ((ushort)tagBaseRequestID.idReservedTwo + 2048);

    /// <summary>
    /// Predefined event types
    /// </summary>
    public enum EventType
    {
        /// <summary>
        /// Record added -- 1
        /// </summary>
        etAdd = 1,

        /// <summary>
        /// Record updated -- 2
        /// </summary>
        etUpdate = 2,

        /// <summary>
        /// Record deleted -- 3
        /// </summary>
        etDelete = 3
    }

    /// <summary>
    /// Cache source information structure
    /// </summary>
    public class CCacheSource
    {
        public string UserId;
        public string Password;
        public ushort Port;

        public CCacheSource(string uid = "", string pwd = "", ushort port = 3306)
        {
            UserId = uid;
            Password = pwd;
            Port = port;
        }

        /// <summary>
        /// A dictionary containing case-insensitive table name (key) and sql query filter which is usually an empty string (value)
        /// </summary>
        public Dictionary<string, string> TableFilter = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        public string GetConnectionString(string ipAddress)
        {
            return string.Format("server={0};port={1};uid={2};pwd={3}", ipAddress, Port, UserId, Password);
        }
    }

    /// <summary>
    /// Seek a datatable from cache according to given host name, table name, filter and sort
    /// </summary>
    /// <param name="host">A remote host where remote database server is located</param>
    /// <param name="tableName">A case-insensitive datatable name string</param>
    /// <param name="filter">A filter for creating a new datatable by use of DataView object</param>
    /// <param name="sort">A sort string for creating a new datatable by use of DataView object</param>
    /// <returns>A datatable object</returns>
    public static DataTable Get(string host, string tableName, string filter = "", string sort = "")
    {
        string ipAddr = GetIPAddress(host);
        return GetByIpAddr(ipAddr, tableName, filter, sort);
    }

    /// <summary>
    /// Seek a datatable instance from cache according to given host IP v4 address, table name, filter and sort
    /// </summary>
    /// <param name="host">A remote host IP v4 address where remote database server is located</param>
    /// <param name="tableName">A case-insensitive datatable name string</param>
    /// <param name="filter">A filter for creating a new datatable by use of DataView object</param>
    /// <param name="sort">A sort string for creating a new datatable by use of DataView object</param>
    /// <returns>A datatable object, and a null will be returned if not found</returns>
    public static DataTable GetByIpAddr(string ipAddr, string tableName, string filter = "", string sort = "")
    {
        DataTable dt = null;
        lock (m_cs)
        {
            if (!m_mapHostDS.ContainsKey(ipAddr))
                return null;
            DataSet ds = m_mapHostDS[ipAddr];
            if (!ds.Tables.Contains(tableName))
                return null;
            dt = ds.Tables[tableName];
            dt = new DataView(dt, filter, sort, DataViewRowState.CurrentRows).ToTable();
        }
        return dt;
    }

    /// <summary>
    /// Initialize cache with given database server name and cache source information
    /// </summary>
    /// <param name="host">A database server name</param>
    /// <param name="source">A structure for cache source information</param>
    /// <returns>Error message</returns>
    public static string InitializeCache(string host, CCacheSource source)
    {
        host = GetIPAddress(host);
        lock (m_cs)
        {
            m_mapHostTables[host] = source;
            return InitializeCache(host, source.GetConnectionString(host));
        }
    }

    protected override void OnSwitchFrom(uint oldServiceId)
    {
        //fake a request at server side and initialize cache within a worker thread
        MakeRequest(idQuerying);
    }

    protected override void OnPublish(object message, uint[] groups)
    {
        string host = GetPeerName();
        string str = message.ToString();
#if DEBUG
        Console.WriteLine("Going to update server real-time cache: <" + str + "> from " + host);
#endif
        string[] sep = str.Split('/', '@');
        EventType eventType = (EventType)int.Parse(sep[0]); //event type integer
        string filter = sep[1]; //valid sql filter
        string tableName = sep[2]; //valid datatable name

        //updated cache data according to event type, table name and filter
        HandleCache(host, tableName, filter, eventType);
    }

    [RequestAttr(idQuerying, true)] //true -- slow request
    private void InitializeCache()
    {
        lock (m_cs)
        {
            string host;
            string mysql_conn = GetMySqlConnectionString(out host);

            //set the latest version data into cache from one remote database server
            string errMsg = InitializeCache(host, mysql_conn);
            if (errMsg.Length > 0)
                Console.WriteLine("Cannot upgrade cache to the latest version of data from server at host {0} with error message '{1}'", host, errMsg);
#if DEBUG
            else
                Console.WriteLine("Successfully upgrade cache to the latest version of data from database server {0}", host);
#endif
        }
    }

    private static string InitializeCache(string host, string mysqlConn, CMySqlCachePeer cachePeer = null)
    {
        Dictionary<string, string> mapTables = GetDataTableMap(host);
        if (mapTables.Count == 0)
            return "Host not set for quering cache data";
        string errMsg = "";
        MySqlConnection conn = null;
        try
        {
            DataSet ds = new DataSet(host);
            conn = new MySqlConnection(mysqlConn);
            conn.Open();
            foreach (var p in mapTables)
            {
                string tableName = p.Key;
                string sql = "select * from " + p.Key;
                if (p.Value != null && p.Value.Length > 0)
                {
                    sql += (" where " + p.Value);
                }
                MySqlDataAdapter adapter = new MySqlDataAdapter(new MySqlCommand(sql, conn));
                adapter.Fill(ds, tableName);
                if (cachePeer != null)
                {
                    uint res = cachePeer.SendResult(idQuerying);
                    if (res == CSocketPeer.REQUEST_CANCELED || res == CSocketPeer.SOCKET_NOT_FOUND)
                    {
                        throw new Exception("Connection disconnected or message canceled");
                    }
                }
            }
            m_mapHostDS[host] = ds;
        }
        catch (Exception ex)
        {
            errMsg = ex.Message;
        }
        finally
        {
            if (conn != null && conn.State != ConnectionState.Closed)
                conn.Close();
        }
        return errMsg;
    }

    private static void HandleCache(string ipAddr, string tableName, string filter, EventType et)
    {
        lock (m_cs)
        {
            if (!m_mapHostDS.ContainsKey(ipAddr) || !m_mapHostTables.ContainsKey(ipAddr))
            {
#if DEBUG
                Console.WriteLine("Host <{0}> not registerred at real-time cache server!!!", ipAddr);
#endif
                return;
            }
            DataSet ds = m_mapHostDS[ipAddr];
            if (!ds.Tables.Contains(tableName))
            {
#if DEBUG
                Console.WriteLine("Table <{0}> not registerred at real-time cache server!!!", tableName);
#endif
                return;
            }
            DataTable dt = ds.Tables[tableName];
            switch (et)
            {
                case EventType.etAdd:
                case EventType.etUpdate:
                    do
                    {
                        MySqlConnection conn = null;
                        IDataReader dr = null;
                        try
                        {
                            CCacheSource source = m_mapHostTables[ipAddr];
                            string s = source.GetConnectionString(ipAddr);
                            conn = new MySqlConnection(s);
                            conn.Open();
                            string sql = "select * from " + tableName + " where " + filter;
                            IDbCommand cmd = new MySqlCommand(sql, conn);
                            dr = cmd.ExecuteReader();
                            var rows = dt.Select(filter);
                            foreach (var row in rows)
                            {
                                row.Delete();
                            }
                            dt.AcceptChanges();
                            dt.Load(dr);
                            dt.AcceptChanges();
                            dr.Close();
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine(ex.Message);
                        }
                        finally
                        {
                            if (dr != null)
                                dr.Close();
                            if (conn != null)
                                conn.Close();
                        }
                    } while (false);
                    break;
                case EventType.etDelete:
                    {
                        var rows = dt.Select(filter);
                        foreach (var row in rows)
                        {
                            row.Delete();
                        }
                        dt.AcceptChanges();
                    }
                    break;
                default:
                    Console.WriteLine("Unsupported event type " + et);
                    break;
            }
        }
    }

    private static object m_cs = new object();
    private static Dictionary<string, DataSet> m_mapHostDS = new Dictionary<string, DataSet>(); //protected by m_cs
    private static Dictionary<string, CCacheSource> m_mapHostTables = new Dictionary<string, CCacheSource>(); //protected by m_cs

    private static string GetIPAddress(string hostname)
    {
        IPHostEntry host = Dns.GetHostEntry(hostname);
        foreach (IPAddress ip in host.AddressList)
        {
            if (ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
            {
                return ip.ToString();
            }
        }
        return "";
    }

    private string GetMySqlConnectionString(out string host)
    {
        host = GetPeerName();
        if (!m_mapHostTables.ContainsKey(host))
            return string.Format("server={0};port={1};uid={2};pwd={3}", host, 3306, "", "");
        CCacheSource source = m_mapHostTables[host];
        return source.GetConnectionString(host);
    }

    private static Dictionary<string, string> GetDataTableMap(string host)
    {
        if (!m_mapHostTables.ContainsKey(host))
            return new Dictionary<string, string>();
        return m_mapHostTables[host].TableFilter;
    }
}

