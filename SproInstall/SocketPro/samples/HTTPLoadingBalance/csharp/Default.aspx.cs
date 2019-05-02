using System;
using System.Data;
using System.Configuration;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using System.Collections;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

public partial class _Default : System.Web.UI.Page 
{
    private static CMyRAdoLB m_lbRAdo = new CMyRAdoLB();
    private static bool StartLB()
    {
        if (m_lbRAdo.IsStarted())
            return true;
        int n;
        const int Count = 5;
        CConnectionContext[] pConnectionContext = new CConnectionContext[Count];
        for (n = 0; n < Count; n++)
            pConnectionContext[n] = new CConnectionContext();

        //set connection contexts
        pConnectionContext[0].m_strHost = "127.0.0.1";
        pConnectionContext[1].m_strHost = "localhost";
        pConnectionContext[2].m_strHost = "127.0.0.1";
        pConnectionContext[3].m_strHost = "localhost";
        pConnectionContext[4].m_strHost = "localhost";
        for (n = 0; n < Count; n++)
        {
            pConnectionContext[n].m_nPort = 20901;
            pConnectionContext[n].m_strPassword = "SocketPro";
            pConnectionContext[n].m_strUID = "PassOne";
            pConnectionContext[n].m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption;
            pConnectionContext[n].m_bZip = false;
        }
        return m_lbRAdo.StartSocketPool(pConnectionContext, 3, 2);
    }

    protected void btnExecute_Click(object sender, EventArgs e)
    {
        gvSQL1.DataSource = null;
        gvSQL2.DataSource = null;
        gvSQL3.DataSource = null;
        if (!StartLB())
            return;
        List<string> lstSQL = new List<string>();
        lstSQL.Add(txtSQL1.Text);
        lstSQL.Add(txtSQL2.Text);
        lstSQL.Add(txtSQL3.Text);
        IList<DataTable> tables = m_lbRAdo.PrepareAndExecuteJobs(lstSQL);
        gvSQL1.DataSource = tables[0];
        gvSQL2.DataSource = tables[1];
        gvSQL3.DataSource = tables[2];
        gvSQL1.DataBind();
        gvSQL2.DataBind();
        gvSQL3.DataBind();
    }
}

class CRAdo : CAsyncAdohandler 
{
    const int sidCRAdo = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 202);
    const short idGetDataSetCRAdo = ((short)USOCKETLib.tagOtherDefine.odUserRequestIDMin + 0);
    const short idGetDataReaderCRAdo = (idGetDataSetCRAdo + 1);
    public CRAdo()
        : base(sidCRAdo)
    {
    }
    public CRAdo(CClientSocket cs)
        : base(sidCRAdo, cs)
    {
    }
    public CRAdo(CClientSocket cs, IAsyncResultsHandler arh)
        : base(sidCRAdo, cs, arh)
    {
    }

    public DataTable ReturnTable
    {
        get
        {
            return m_AdoSerialier.CurrentDataTable;
        }
    }

    public void GetDataReaderAsyn(string strSQL)
    {
        SendRequest(idGetDataReaderCRAdo, strSQL);
    }

    public SortedList<long, DataTable> m_JobTable;

    //When a result comes from a remote SocketPro server, the below virtual function will be called.
    //We always process returning results inside the function.
    protected override void OnResultReturned(short sRequestID, CUQueue UQueue)
    {
        switch (sRequestID)
        {
            case CAsyncAdoSerializationHelper.idDataSetHeaderArrive:
            case CAsyncAdoSerializationHelper.idDataTableHeaderArrive:
            case CAsyncAdoSerializationHelper.idDataReaderHeaderArrive:
            case CAsyncAdoSerializationHelper.idDataReaderRecordsArrive:
            case CAsyncAdoSerializationHelper.idEndDataTable:
            case CAsyncAdoSerializationHelper.idEndDataReader:
            case CAsyncAdoSerializationHelper.idDataTableRowsArrive:
            case CAsyncAdoSerializationHelper.idEndDataSet:
                base.OnResultReturned(sRequestID, UQueue); //chain down to CAsyncAdohandler for processing
                break;
            case idGetDataSetCRAdo:
                break;
            case idGetDataReaderCRAdo:
                break;
            default:
                break;
        }
    }
}

class CMyRAdoLB : CSocketPoolEx<CRAdo>
{
    private object m_cs = new object();
    protected override void OnJobDone(CRAdo Handler, IJobContext JobContext)
    {
        CRAdo RAdo = (CRAdo)JobContext.Identity;
        lock (m_cs)
        {
            RAdo.m_JobTable[JobContext.JobId] = Handler.ReturnTable;
        }
    }

    public IList<DataTable> PrepareAndExecuteJobs(List<string> lstSQL)
    {
        if(lstSQL == null || lstSQL.Count == 0)
            throw new InvalidOperationException("Must pass in a list of SQL statements");
        
        CRAdo handler = (CRAdo)JobManager.LockIdentity();
        if(handler == null)
            throw new InvalidOperationException("ADO loading balance is down");

        handler.m_JobTable = new SortedList<long, DataTable>();
        
        foreach (string strSQL in lstSQL)
            handler.GetDataReaderAsyn(strSQL);
           
        //wait until all of jobs with this identity are completed.
        JobManager.Wait(handler);

        IList<DataTable> tables = handler.m_JobTable.Values;
        
        //release identity
        JobManager.UnlockIdentity(handler); 
        return tables;
    }
}