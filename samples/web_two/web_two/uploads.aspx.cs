using System.Threading.Tasks;
using SocketProAdapter.UDB;
namespace web_two
{
    public partial class CUploads : System.Web.UI.Page
    {
        protected void Page_Load(object sender, System.EventArgs e)
        {
            if (!IsPostBack) RegisterAsyncTask(new System.Web.UI.PageAsyncTask(ExecuteSql));
        }

        protected void btnDoit_Click(object sender, System.EventArgs e)
        {
            RegisterAsyncTask(new System.Web.UI.PageAsyncTask(ExecuteSql));
        }

        private async Task ExecuteSql()
        {
            CDBVariantArray v = new CDBVariantArray();
            v.Add(1);/*Google id*/ v.Add("Ted Cruz"); v.Add(System.DateTime.Now);
            v.Add(1);/*Google id*/ v.Add("Donald Trump"); v.Add(System.DateTime.Now);
            v.Add(2);/*Microsoft id*/ v.Add("Hillary Clinton"); v.Add(System.DateTime.Now);
            var handler = Global.Master.SeekByQueue();
            if (handler == null)
            {
                txtResult.Text = "No session to master DB and processing failed";
                return;
            }
            var task = handler.executeBatch(tagTransactionIsolation.tiReadCommited, "INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)", v);
            if (!handler.AttachedClientSocket.Connected)
                txtResult.Text = "No session to master DB now but request is safely saved for processing later";
            else
            {
                var res = await task;
                if (res.ec == 0)
                    txtResult.Text = "Last employeeid: " + res.lastId;
                else
                    txtResult.Text = res.em;
            }
        }
    }
}
