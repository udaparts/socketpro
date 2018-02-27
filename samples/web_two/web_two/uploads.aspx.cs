using System; using System.Threading.Tasks;
using System.Web.UI; using SocketProAdapter.UDB;
namespace web_two {
    public partial class CUploads : System.Web.UI.Page {
        protected void Page_Load(object sender, EventArgs e) {
            if (!IsPostBack) RegisterAsyncTask(new PageAsyncTask(ExecuteSql));
        }
        protected void btnDoit_Click(object sender, EventArgs e) {
            RegisterAsyncTask(new PageAsyncTask(ExecuteSql));
        }
        private async Task ExecuteSql() {
            try {
                CDBVariantArray vData = new CDBVariantArray();
                vData.Add(1);/*Google id*/ vData.Add("Ted Cruz"); vData.Add(DateTime.Now);
                vData.Add(1);/*Google id*/ vData.Add("Donald Trump"); vData.Add(DateTime.Now);
                vData.Add(2);/*Microsoft id*/ vData.Add("Hillary Clinton"); vData.Add(DateTime.Now);
                txtResult.Text = await DoInserts(vData);
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
            bool ok = handler.BeginTrans();
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
            /* you can asynchronously execute other SQL statements here and push results onto browsers
             * by ASP.NET SignalR to improve web response
             */
            return tcs.Task;
        }
    }
}
