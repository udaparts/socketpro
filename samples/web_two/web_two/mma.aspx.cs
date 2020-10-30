namespace web_two {
    public partial class CMma : System.Web.UI.Page {
        protected void Page_Load(object sender, System.EventArgs e) {
            if (!IsPostBack) ExecuteSql();
        }
        protected void txtExecute_Click(object sender, System.EventArgs e) {
            ExecuteSql();
        }
        private void ExecuteSql() {
            string sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
            string filter = txtFilter.Text.Trim();
            if (filter.Length > 0) sql += (" WHERE " + filter);
            var handler = Global.Slave.SeekByQueue();
            if (!handler.Socket.Connected) {
                txtResults.Text = "All slaves are inaccessible at this time now";
                return;
            }
            var task = handler.execute(sql, (h, vData) => {
                txtResults.Text = string.Format("Max={0}, Min={1}, Avg={2}",
                    vData[0], vData[1], vData[2]);
            });
            if (!task.Wait(5000)) {
                txtResults.Text = "Querying max, min and avg timed out";
                return;
            }
            var res = task.Result;
            if (res.ec != 0)
                txtResults.Text = res.em; //error message
        }
    }
}
