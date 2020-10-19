﻿using System.Threading.Tasks;
using System.Web.UI;
namespace web_two
{
    public partial class CRdt : System.Web.UI.Page
    {
        protected void Page_Load(object sender, System.EventArgs e)
        {
            if (!IsPostBack) RegisterAsyncTask(new PageAsyncTask(ExecuteSql));
        }
        protected void btnExecute_Click(object sender, System.EventArgs e)
        {
            RegisterAsyncTask(new PageAsyncTask(ExecuteSql));
        }
        private async Task ExecuteSql()
        {
            try
            {
                txtResult.Text = await DoSql();
            }
            catch (System.Exception err)
            {
                txtResult.Text = err.Message;
            }
        }
        private Task<string> DoSql()
        {
            string s = "", sql = "SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=" + txtRentalId.Text;
            TaskCompletionSource<string> tcs = new TaskCompletionSource<string>();
            var handler = Global.Slave.SeekByQueue();
            if (handler == null)
            {
                tcs.SetResult("No connection to anyone of slave databases");
                return tcs.Task;
            }
            bool ok = handler.Execute(sql, (h, res, errMsg, affected, fail_ok, vtId) =>
            {
                tcs.SetResult((res != 0) ? errMsg : s);
            }, (h, vData) =>
            {
                s = string.Format("rental_id={0}, rental={1}, return={2}, lastupdate={3}", vData[0], vData[1], vData[2], vData[3]);
            });
            return tcs.Task;
        }
    }
}
