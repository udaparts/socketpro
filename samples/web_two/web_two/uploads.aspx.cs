using System;
using System.Threading.Tasks;
using System.Web.UI;
using SocketProAdapter.UDB;
namespace web_two {
    public partial class CUploads : System.Web.UI.Page {
        protected void Page_Load(object sender, System.EventArgs e) {
            if (!IsPostBack) RegisterAsyncTask(new PageAsyncTask(ExecuteSql));
        }
        protected void btnDoit_Click(object sender, EventArgs e) {
            RegisterAsyncTask(new PageAsyncTask(ExecuteSql));
        }
        private async Task ExecuteSql() {
            try {
                CDBVariantArray v = new CDBVariantArray();
                v.Add(1);/*Google id*/ v.Add("Ted Cruz"); v.Add(DateTime.Now);
                v.Add(1);/*Google id*/ v.Add("Donald Trump"); v.Add(DateTime.Now);
                v.Add(2);/*Microsoft id*/ v.Add("Hillary Clinton"); v.Add(DateTime.Now);
                txtResult.Text = await DoInserts(v);
            } catch (Exception err) {
                txtResult.Text = err.Message;
            }
        }
        private Task<string> DoInserts(CDBVariantArray v) {
            TaskCompletionSource<string> tcs = new TaskCompletionSource<string>();
            var handler = Global.Master.LockByMyAlgorithm(60000);
            if (handler == null) {
                tcs.SetResult("No connection to anyone of master databases");
                return tcs.Task;
            }
            string s = "";
            bool ok = handler.BeginTrans(); //start streaming multiple requests
            ok = handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId, Name, JoinDate)VALUES(?,?,?)");
            ok = handler.Execute(v, (h, r, err, affected, fail_ok, vtId) => {
                if (r != 0) s = err;
                else s = "Last employeeid=" + vtId.ToString();
            });
            ok = handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) => {
                if (res != 0) s = errMsg;
                tcs.SetResult(s);
            });
            Global.Master.UnlockByMyAlgorithm(handler); //put handler back into pool for reuse
            return tcs.Task;
        }
    }
}
