using System.Threading.Tasks;
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
            string sql = "SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=" + txtRentalId.Text;
            var handler = Global.Slave.SeekByQueue();
            if (!handler.Socket.Connected)
            {
                txtResult.Text = "No connection to anyone of slave databases";
                return;
            }
            var res = await handler.execute(sql, (h, v) =>
            {
                txtResult.Text = string.Format("rental_id={0}, rental={1}, return={2}, lastupdate={3}", v[0], v[1], v[2], v[3]);
            });
            if (res.ec != 0)
                txtResult.Text = res.em; //error message
        }
    }
}
