using System.Collections.Generic;
namespace web_two {
    public partial class Sessions : System.Web.UI.Page {
        protected void Page_Load(object sender, System.EventArgs e) {
            txtMasterConnections.Text = Global.Master.ConnectedSockets.ToString();
            txtSlaveConnections.Text = Global.Slave.ConnectedSockets.ToString();
            if (!IsPostBack) {
                List<KeyValuePair<string, string>> vP = Global.Cache.DBTablePair;
                List<string> list = new List<string>();
                foreach (KeyValuePair<string, string> p in vP)
                    list.Add(p.Key + '.' + p.Value);
                lstTables.DataSource = list; lstTables.DataBind();
                if (list.Count > 0) BindSelectedTable2GridView((lstTables.SelectedIndex = 0));
            }
        }
        protected void lstTables_SelectedIndexChanged(object sender, System.EventArgs e) {
            BindSelectedTable2GridView(lstTables.SelectedIndex);
        }
        private void BindSelectedTable2GridView(int index) {
            string[] v = lstTables.Items[index].Text.Split('.');
            //find source table data from real-time update cache instead of backend database
            System.Data.DataTable dt = Global.Cache.Find(v[0], v[1]);
            gvTable.Columns.Clear(); gvTable.DataSource = dt;
            foreach (System.Data.DataColumn dc in dt.Columns) {
                var bf = new System.Web.UI.WebControls.BoundField();
                bf.DataField = dc.ColumnName; bf.HeaderText = dc.ColumnName;
                gvTable.Columns.Add(bf);
            }
            gvTable.DataBind();
        }
    }
}
