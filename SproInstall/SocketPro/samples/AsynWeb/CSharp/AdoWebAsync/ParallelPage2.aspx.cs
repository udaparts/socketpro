using System;
using System.Data;
using System.Configuration;
using System.Collections;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Web.UI.HtmlControls;
using USOCKETLib;
using UDBLib;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.RemoteDB;

namespace AdoWebAsync
{
    public partial class ParallelPage : System.Web.UI.Page
    {
        CAsynDBLite m_dbCmd;
        CMyAdoHandler m_dbQuery;
        int m_nFirst;
        IAsyncResult BeginAsyncOperationCmd(object sender, EventArgs e, AsyncCallback cb, object state)
        {
            m_dbCmd = Global.m_AsyncRdb.Lock();
            if (m_dbCmd != null)
            {
                CClientSocket cs = m_dbCmd.GetAttachedClientSocket();
                cs.Callback = cb; //remember a callback
                if (!m_dbCmd.DBConnected)
                {
                    m_dbCmd.ConnectDB(null); //use global oledb connection string at server side
                }
                m_dbCmd.ExecuteNonQuery(txtCmd.Text);
                return cs;
            }
            return null;
        }

        void EndAsyncOperationCmd(IAsyncResult ar)
        {
            if (m_dbCmd != null)
            {
                if (m_nFirst == 0)
                {
                    m_nFirst = 1;
                    lblInfo.Text = "Command First";
                }
                Global.m_AsyncRdb.Unlock(m_dbCmd);
            }
        }

        void TimeoutAsyncOperationCmd(IAsyncResult ar)
        {
            lblInfo.Text = "Data temporarily unavailable";
        }

        IAsyncResult BeginAsyncOperationQuery(object sender, EventArgs e, AsyncCallback cb, object state)
        {
            m_dbQuery = Global.m_AsyncADO.Lock();
            if (m_dbQuery != null)
            {
                CClientSocket cs = m_dbQuery.GetAttachedClientSocket();
                cs.Callback = cb; //remember callback;
                m_dbQuery.GetDataTableAsync(txtQuery.Text);
                return cs;
            }
            return null;
        }

        void EndAsyncOperationQuery(IAsyncResult ar)
        {
            if (m_dbQuery != null)
            {
                DataTable dt = m_dbQuery.CurrentDataTable;
                gvQuery.DataSource = dt;
                gvQuery.DataBind();
                Global.m_AsyncADO.Unlock(m_dbQuery);
                if (m_nFirst == 0)
                {
                    m_nFirst = 2;
                    lblInfo.Text = "Query First";
                }
            }
        }

        void TimeoutAsyncOperationQuery(IAsyncResult ar)
        {
            lblInfo.Text = "Data temporarily unavailable";
        }

        protected void idDoMultiTask_Click(object sender, EventArgs e)
        {
            m_nFirst = 0;
            PageAsyncTask task = new PageAsyncTask(BeginAsyncOperationQuery, EndAsyncOperationQuery, TimeoutAsyncOperationQuery, null, true);
            RegisterAsyncTask(task);

            task = new PageAsyncTask(BeginAsyncOperationCmd, EndAsyncOperationCmd, TimeoutAsyncOperationCmd, null, true);
            RegisterAsyncTask(task);
        }
    }
}
