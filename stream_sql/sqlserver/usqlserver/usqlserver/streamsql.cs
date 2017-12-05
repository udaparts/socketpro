using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.UDB;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using Microsoft.SqlServer.Server;

class CStreamSql : CClientPeer
{
    public const uint sidMsSql = BaseServiceID.sidReserved + 0x6FFFFFF2;
    private string m_defaultDB = "";
    private bool m_global = true;
    private CUQueue m_Blob = new CUQueue();
    private SqlConnection m_conn = null;
    private ulong m_parameters = 0;
    internal static object m_csPeer = new object();
    internal static Dictionary<ulong, SqlConnection> m_mapConnection = new Dictionary<ulong, SqlConnection>(); //protected by m_csPeer

    protected CDBVariantArray m_vParam = new CDBVariantArray();
    protected bool m_EnableMessages = false;
    protected ulong m_oks = 0;
    protected ulong m_fails = 0;
    protected tagTransactionIsolation m_ti = tagTransactionIsolation.tiUnspecified;

    protected override void OnSwitchFrom(uint oldServiceId)
    {
        m_oks = 0;
        m_fails = 0;
        m_ti = tagTransactionIsolation.tiUnspecified;
        ulong socket = Handle;
        lock (m_csPeer)
        {
            if (m_mapConnection.ContainsKey(socket))
            {
                m_conn = m_mapConnection[socket];
                m_mapConnection.Remove(socket);
            }
        }
    }

    protected override void OnReleaseResource(bool bClosing, uint info)
    {
        m_EnableMessages = false;
        if (m_conn != null)
        {
            try
            {
                m_conn.Close();
                m_conn.Dispose();
            }
            finally
            {
                m_conn = null;
            }
        }
    }

    [RequestAttr(DB_CONSTS.idOpen, true)]
    int Open(string strConnection, uint flags, out int res, out string errMsg)
    {
        res = 0;
        errMsg = "";
        if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
            m_EnableMessages = Push.Subscribe(DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);
        SqlCommand cmd = new SqlCommand("select DB_NAME()", m_conn);
        try
        {
            m_defaultDB = (string)cmd.ExecuteScalar();
        }
        finally
        {
        }
        if (strConnection != null && strConnection.Length > 0)
        {
            string sql = "USE " + strConnection;
            try
            {
                cmd = new SqlCommand(sql, m_conn);
                int affected = cmd.ExecuteNonQuery();
            }
            catch (SqlException err)
            {
                res = err.ErrorCode;
                errMsg = err.Message;
            }
            catch (Exception err)
            {
                res = -1;
                errMsg = err.Message;
            }
        }
        return (int)tagManagementSystem.msMsSQL;
    }


    [RequestAttr(DB_CONSTS.idStartBLOB)]
    private void StartBLOB(uint lenExpected)
    {
        m_Blob.SetSize(0);
        if (lenExpected > m_Blob.MaxBufferSize)
            m_Blob.Realloc(lenExpected);
        m_Blob.Push(UQueue.IntenalBuffer, UQueue.GetSize());
        UQueue.SetSize(0);
    }

    [RequestAttr(DB_CONSTS.idChunk)]
    private void Chunk()
    {
        CUQueue q = UQueue;
        if (q.GetSize() > 0)
        {
            m_Blob.Push(q.IntenalBuffer, q.GetSize());
            q.SetSize(0);
        }
    }

    [RequestAttr(DB_CONSTS.idEndBLOB)]
    private void EndBLOB()
    {
        Chunk();
        object vt;
        m_Blob.Load(out vt);
        m_vParam.Add(vt);
    }

    [RequestAttr(DB_CONSTS.idTransferring)]
    private void Transferring()
    {
        CUQueue q = UQueue;
        while (q.GetSize() > 0)
        {
            object vt;
            q.Load(out vt);
            m_vParam.Add(vt);
        }
    }

    [RequestAttr(DB_CONSTS.idBeginRows)]
    private void BeginRows()
    {
        Transferring();
    }

    [RequestAttr(DB_CONSTS.idEndRows)]
    private void EndRows()
    {
        Transferring();
    }

    [RequestAttr(DB_CONSTS.idClose, true)]
    private string CloseDb(out int res)
    {
        string errMsg = "";
        res = 0;
        m_vParam.Clear();
        m_global = true;
        if (m_conn != null && m_conn.State != ConnectionState.Closed)
        {
            if (m_defaultDB != null && m_defaultDB.Length > 0)
            {
                try
                {
                    SqlCommand cmd = new SqlCommand("USE " + m_defaultDB, m_conn);
                    int ret = cmd.ExecuteNonQuery();
                }
                catch (SqlException err)
                {
                    errMsg = err.Message;
                    res = err.ErrorCode;
                }
                catch(Exception err)
                {
                    errMsg = err.Message;
                    res = -1;
                }
                finally { }
            }
        }
        ResetMemories();
        if (m_EnableMessages)
        {
            Push.Unsubscribe();
            m_EnableMessages = false;
        }
        return errMsg;
    }

    private void ResetMemories()
    {
        m_Blob.SetSize(0);
        if (m_Blob.MaxBufferSize > DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE)
        {
            m_Blob.Realloc(DB_CONSTS.DEFAULT_BIG_FIELD_CHUNK_SIZE);
        }
    }
}
