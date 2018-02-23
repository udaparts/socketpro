using System; using System.Collections.Generic; using System.Web.UI.WebControls;
namespace web_two {
    public partial class Sessions : System.Web.UI.Page {
        protected void Page_Load(object sender, EventArgs e) {
            txtMasterConnections.Text = Global.Master.ConnectedSockets.ToString();
            txtSlaveConnections.Text = Global.Slave.ConnectedSockets.ToString();
            if (!IsPostBack) {
                List<KeyValuePair<string, string>> vP = Global.Cache.DBTablePair;
                List<string> list = new List<string>();
                foreach (KeyValuePair<string, string> p in vP) {
                    string s = p.Key + '.' + p.Value; list.Add(s);
                }
                lstTables.DataSource = list;
                lstTables.DataBind();
                if (list.Count > 0) {
                    lstTables.SelectedIndex = 0;
                    BindSelectedTable2GridView(lstTables.SelectedIndex);
                }
            }
        }
        protected void lstTables_SelectedIndexChanged(object sender, EventArgs e) {
            BindSelectedTable2GridView(lstTables.SelectedIndex);
        }
        private void BindSelectedTable2GridView(int index) {
            var item = lstTables.Items[index]; string[] v = item.Text.Split('.');
            //find source table data from real-time update cache instead of backend database
            System.Data.DataTable dt = Global.Cache.Find(v[0], v[1]);
            gvTable.Columns.Clear(); gvTable.DataSource = dt;
            foreach (System.Data.DataColumn dc in dt.Columns) {
                BoundField bf = new BoundField(); bf.DataField = dc.ColumnName;
                bf.HeaderText = dc.ColumnName; gvTable.Columns.Add(bf);
            }
            gvTable.DataBind();
        }
    }
}
