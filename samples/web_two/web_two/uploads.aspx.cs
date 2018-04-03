using System.Threading.Tasks;
using SocketProAdapter.UDB;
namespace web_two {
    public partial class CUploads : System.Web.UI.Page {
        protected void Page_Load(object sender, System.EventArgs e) {
            if (!IsPostBack) RegisterAsyncTask(new System.Web.UI.PageAsyncTask(ExecuteSql));
        }
        protected void btnDoit_Click(object sender, System.EventArgs e) {
            RegisterAsyncTask(new System.Web.UI.PageAsyncTask(ExecuteSql));
        }
        private async Task ExecuteSql() {
            CDBVariantArray v = new CDBVariantArray();
            v.Add(1);/*Google id*/ v.Add("Ted Cruz"); v.Add(System.DateTime.Now);
            v.Add(1);/*Google id*/ v.Add("Donald Trump"); v.Add(System.DateTime.Now);
            v.Add(2);/*Microsoft id*/ v.Add("Hillary Clinton"); v.Add(System.DateTime.Now);
            txtResult.Text = await DoInserts(v);
        }
        private Task<string> DoInserts(CDBVariantArray v) {
            TaskCompletionSource<string> tcs = new TaskCompletionSource<string>();
            var handler = Global.Master.LockByMyAlgorithm(6000); //6 seconds
            string s = ""; bool ok = handler.BeginTrans(); //start streaming multiple requests
            ok = handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId, Name, JoinDate)VALUES(?,?,?)");
            ok = handler.Execute(v, (h, res, errMsg, affected, fail_ok, vtId) => {
                if (res != 0) s = errMsg;
                else s = "Last employeeid=" + vtId.ToString();
            });
            ok = handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) => {
                if (res != 0) s = errMsg;
                tcs.SetResult(s);
            });
            if (!handler.AttachedClientSocket.Connected)
                tcs.SetResult("No connection to anyone of master databases but request queued for processing later");
            Global.Master.UnlockByMyAlgorithm(handler); //put handler back into pool for reuse
            return tcs.Task;
        }
    }
}
