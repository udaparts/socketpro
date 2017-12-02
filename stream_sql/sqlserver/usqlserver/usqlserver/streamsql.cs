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

class CStreamSql : CClientPeer
{
    public const uint sidMsSql = BaseServiceID.sidReserved + 0x6FFFFFF2;

    private bool m_global = true;
    private CUQueue m_Blob = new CUQueue();
    private SqlConnection m_conn = null;
    private ulong m_parameters = 0;
    private static object m_csPeer = new object();

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
        res = 0;
        m_vParam.Clear();
        m_global = true;
        if (m_conn != null && m_conn.State != ConnectionState.Closed)
        {
            m_conn.Close();
        }
        ResetMemories();
        if (m_EnableMessages)
        {
            Push.Unsubscribe();
            m_EnableMessages = false;
        }
        return "";
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
