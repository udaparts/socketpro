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
            bool ok = handler.ExecuteBatch(tagTransactionIsolation.tiReadCommited,
                "INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)",
                v, (h, res, errMsg, affected, fail_ok, vtId) => {
                    try {
                        tcs.SetResult((res != 0) ? errMsg : "Last employeeid=" + vtId.ToString());
                    } finally { }
                });
            if (!handler.AttachedClientSocket.Connected)
                tcs.SetResult("No session to master DB now but request is safely saved for processing later");
            Global.Master.UnlockByMyAlgorithm(handler); //put handler back into pool for reuse
            return tcs.Task;
        }
    }
}
