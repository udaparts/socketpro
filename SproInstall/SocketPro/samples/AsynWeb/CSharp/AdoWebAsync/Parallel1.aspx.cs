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
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.RemoteDB;

namespace AdoWebAsync
{
    public partial class Parallel : System.Web.UI.Page
    {
        
        protected void btnParallel_Click(object sender, EventArgs e)
        {
            //lock two instances of data accessing handlers, CAsynDBLite and CMyAdoHandler
            CAsynDBLite AsynDBLiteHandler = Global.m_AsyncRdb.Lock(100);
            CMyAdoHandler AsyncAdoHandler = Global.m_AsyncADO.Lock(100);

            if (AsynDBLiteHandler == null || AsyncAdoHandler == null)
            {
                //indicate error message like server is too busy
                return;
            }

            if (!AsynDBLiteHandler.DBConnected)
            {
                //connect to DB one time only
                AsynDBLiteHandler.ConnectDB(null); //using global oledb connection string at server side
                AsynDBLiteHandler.GetAttachedClientSocket().WaitAll();
            }

            //execute two SQLs in parallel with two different instances, AsynDBLiteHandler and AsyncAdoHandler
            AsynDBLiteHandler.OpenRowset(txtSQL1.Text, "SQL1");
            AsyncAdoHandler.GetDataTableAsync(txtSQL2.Text);

            //Block until two requests are processed
            AsyncAdoHandler.GetAttachedClientSocket().WaitAll();
            AsynDBLiteHandler.GetAttachedClientSocket().WaitAll();

            gvQueryOne.DataSource = AsynDBLiteHandler.CurrentDataTable;
            gvQueryTwo.DataSource = AsyncAdoHandler.CurrentDataTable;

            gvQueryOne.DataBind();
            gvQueryTwo.DataBind();

            //unlock sockets and their associated DB handlers, and return them back into pool for reuse
            Global.m_AsyncRdb.Unlock(AsynDBLiteHandler);
            Global.m_AsyncADO.Unlock(AsyncAdoHandler);
        }
    }
}
