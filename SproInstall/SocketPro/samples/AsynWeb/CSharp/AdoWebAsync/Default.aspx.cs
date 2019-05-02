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

namespace AdoWebAsync
{
    public partial class _Default : System.Web.UI.Page
    {
        CMyAdoHandler m_AsynAdoHandler;
        protected void btnExecute_Click(object sender, EventArgs e)
        {
            //Register two event handlers
            AddOnPreRenderCompleteAsync(new BeginEventHandler(BeginAsyncOperation), new EndEventHandler(EndAsyncOperation));
        }

        private IAsyncResult BeginAsyncOperation(object sender, EventArgs e, AsyncCallback cb, object state)
        {
            m_AsynAdoHandler = Global.m_AsyncADO.Lock(100); //timeout -- 0.1 second
            if (m_AsynAdoHandler != null)
            {
                //Start batch
                bool ok = m_AsynAdoHandler.GetAttachedClientSocket().BeginBatching();
                
                //delete some records
                m_AsynAdoHandler.ExecuteNoQueryAsync("Delete from Shippers Where ShipperID > 3");

                string strSQL = "Insert into Shippers (CompanyName, Phone) Values ('";
                strSQL += txtCompany.Text;
                strSQL += "', '";
                strSQL += txtPhoneNumber.Text;
                strSQL += "')";

                //Insert a record
                m_AsynAdoHandler.ExecuteNoQueryAsync(strSQL);

                //Query records and fetch all of them
                m_AsynAdoHandler.GetDataTableAsync("Select * from Shippers");

                //remember a callback
                m_AsynAdoHandler.GetAttachedClientSocket().Callback = cb;

                //Batch requests and remember a callback cb
                m_AsynAdoHandler.GetAttachedClientSocket().Commit(true);

                return m_AsynAdoHandler.GetAttachedClientSocket();
            }
            else //no socket is available
            {
                //report the error that web server is very busy here
            }
            return null;
        }

        private void EndAsyncOperation(IAsyncResult ar)
        {
            if (m_AsynAdoHandler != null)
            {
                //binding the last table onto a grid view
                DataTable dt = m_AsynAdoHandler.CurrentDataTable;
                gvRowset.DataSource = dt;
                gvRowset.DataBind();

                //unlock the attached socket, and return it back into socket pool for reuse
                Global.m_AsyncADO.Unlock(m_AsynAdoHandler);
            }
        }
    }
}
