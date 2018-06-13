
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Windows.Forms;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;

namespace test_cache
{
    public partial class frmCache : Form
    {
        delegate void DSessionClose(string message);
        delegate void DMessage(string message, DataTable dt);
        public frmCache()
        {
            InitializeComponent();
            txtHost.Text = "localhost";
            txtUser.Text = "root";
            txtPassword.Text = "Smash123";
        }

        private CSocketPool<CSqlite> m_spSqlite;

        private DataSet m_ds = null;
        private DMessage m_thread_message;
        private DSessionClose m_closed;
        private object m_cs = new object();
        private DSQLMessage m_sql_message;

        private void frmCache_Load(object sender, EventArgs e)
        {
            m_thread_message = DbMessage;
            m_closed = Connection_Closed;
            m_sql_message = SQLMessage;
        }

        private void Connection_Closed(string s)
        {
            if (m_spSqlite != null)
            {
                m_spSqlite.Dispose();
                m_spSqlite = null;
            }
            btnDisconnect.Enabled = false;
            btnConnect.Enabled = true;
            txtMessage.Text = s;
        }

        private void m_spSqlite_SocketPoolEvent(CSocketPool<CSqlite> sender, tagSocketPoolEvent spe, CSqlite AsyncServiceHandler)
        {
            switch (spe)
            {
                case tagSocketPoolEvent.speSocketClosed:
                    //this event is fired from worker thread from socket pool thread
                    BeginInvoke(m_closed, "Database server or network shut down");
                    break;
                default:
                    break;
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            CConnectionContext cc = new CConnectionContext(txtHost.Text, 20901, txtUser.Text, txtPassword.Text);
            m_spSqlite = new CSocketPool<CSqlite>(false);

            //set event for MySQL/Mariadb database shutdown
            m_spSqlite.SocketPoolEvent += new CSocketPool<CSqlite>.DOnSocketPoolEvent(m_spSqlite_SocketPoolEvent);

            if (!m_spSqlite.StartSocketPool(cc, 1, 1))
            {
                txtMessage.Text = "No connection to " + txtHost.Text;
                return;
            }
            CSqlite sqlite = m_spSqlite.AsyncHandlers[0];

            //set event for tracking all database table update events, delete, update and insert
            m_spSqlite.Sockets[0].Push.OnPublish += new DOnPublish(Push_OnPublish);

            //create a DB session with default to sample database sakil
            bool ok = sqlite.Open("sakila.db", null, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);

            m_ds = new DataSet("real-time cache");
            DataTable dt = null;
            string errMsg = "";
            //query all cached tables into client side for intial cache data
            ok = sqlite.Execute("", (h, ret, err_msg, affected, fail_ok, id) =>
            {
                //this callback is fired from worker thread from socket pool thread
                ok = (ret == 0);
                errMsg = err_msg;
            }, (h, data) =>
            {
                //this callback is fired from worker thread from socket pool thread
                CSqlite.AppendRowDataIntoDataTable(data, dt);
            }, (h) =>
            {
                //this callback is fired from worker thread from socket pool thread
                dt = CSqlite.MakeDataTable(h.ColumnInfo);
                string name = h.ColumnInfo[0].DBPath + "." + h.ColumnInfo[0].TablePath;
                dt.TableName = name;
                m_ds.Tables.Add(dt);
            });
            ok = sqlite.WaitAll();
            txtMessage.Text = errMsg;
            lstTables.Items.Clear();
            foreach (DataTable table in m_ds.Tables)
            {
                lstTables.Items.Add(table.TableName);
            }
            if (m_ds.Tables.Count > 0)
            {
                lstTables.SelectedIndex = 0;
            }
            btnDisconnect.Enabled = ok;
            btnConnect.Enabled = !ok;
        }

        static DataRow FindRowByKeys(DataTable dt, List<KeyValuePair<DataColumn, object>> vKey, tagUpdateEvent ue, out string filter)
        {
            DataRow[] rows = null;
            filter = "";
            foreach (var kv in vKey)
            {
                if (filter.Length > 0)
                    filter += " AND ";
                filter += ("`" + kv.Key.ColumnName + "`=");
                if (kv.Value is long || kv.Value is decimal || kv.Value is double)
                    filter += kv.Value.ToString();
                else if (kv.Value is string)
                    filter += ("'" + kv.Value.ToString() + "'");
                else
                    throw new Exception("Other key column not supported");
            }
            if (ue != tagUpdateEvent.ueInsert)
                rows = dt.Select(filter);

            if (rows != null && rows.Length == 1)
                return rows[0];
            else if (rows != null && rows.Length > 1)
                throw new Exception("Multiple rows found beyond our expectation");
            return null;
        }

        private void DbMessage(string message, DataTable dt)
        {
            txtMessage.Text = message;
            lock (m_cs)
            {
                dgvShow.Update();
                dgvShow.Refresh();
            }
        }

        private void Push_OnPublish(CClientSocket sender, CMessageSender messageSender, uint[] group, object msg)
        {
            //this event is fired from worker thread from socket pool thread

            string message = "", filter = "";
            List<KeyValuePair<DataColumn, object>> vKeyVal = new List<KeyValuePair<DataColumn, object>>();
            object[] vData = (object[])msg;

            //vData[0] == event type; vData[1] == host; vData[2] = user; vData[3] == db name; vData[4] == table name
            tagUpdateEvent ue = (tagUpdateEvent)(int)vData[0];

            string table_name = vData[3].ToString() + "." + vData[4].ToString();
            lock (m_cs)
            {
                DataTable dt = m_ds.Tables[table_name];
                if (dt == null)
                    return;
                DataColumn[] keys = dt.PrimaryKey;
                int cols = dt.Columns.Count;
                switch (ue)
                {
                    case tagUpdateEvent.ueUpdate:
                        {
                            foreach (DataColumn dc in keys)
                            {
                                KeyValuePair<DataColumn, object> kv = new KeyValuePair<DataColumn, object>(dc, vData[5 + 2 * dc.Ordinal]);
                                vKeyVal.Add(kv);
                            }
                            DataRow dr = FindRowByKeys(dt, vKeyVal, ue, out filter);
                            for (int n = 0; n < cols; ++n)
                            {
                                if (dt.Columns[n].ReadOnly)
                                    continue;
                                object d = vData[5 + 2 * n + 1];
                                dr[n] = d;
                            }
                            message = "Table " + table_name + " updated for row (" + filter + ")";
                        }
                        break;
                    case tagUpdateEvent.ueDelete:
                        {
                            int index = 0;
                            foreach (DataColumn dc in keys)
                            {
                                KeyValuePair<DataColumn, object> kv = new KeyValuePair<DataColumn, object>(dc, vData[5 + index]);
                                vKeyVal.Add(kv);
                                ++index;
                            }
                            dt.Rows.Remove(FindRowByKeys(dt, vKeyVal, ue, out filter));
                            message = "Table " + table_name + " deleted for row (" + filter + ")";
                        }
                        break;
                    case tagUpdateEvent.ueInsert:
                        {
                            foreach (DataColumn dc in keys)
                            {
                                KeyValuePair<DataColumn, object> kv = new KeyValuePair<DataColumn, object>(dc, vData[5 + dc.Ordinal]);
                                vKeyVal.Add(kv);
                            }
                            DataRow dr = FindRowByKeys(dt, vKeyVal, ue, out filter); //generate filter only
                            dr = dt.NewRow();
                            for (int n = 0; n < cols; ++n)
                            {
                                dr[n] = vData[5 + n];
                            }
                            dt.Rows.Add(dr);
                            message = "Table " + table_name + " inserted for row (" + filter + ")";
                        }
                        break;
                    default:
                        message = "Unknown DB message found"; //shouldn't come here
                        break;
                }
                BeginInvoke(m_thread_message, message, dt);
            }
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            Connection_Closed("Connection closed by myself");
        }

        private void lstTables_SelectedIndexChanged(object sender, EventArgs e)
        {
            int index = lstTables.SelectedIndex;
            lock (m_cs)
            {
                dgvShow.DataSource = m_ds.Tables[index];
                dgvShow.Update();
            }
        }

        delegate void DSQLMessage(int res, string message);
        private void SQLMessage(int res, string message)
        {
            if (res == 0)
                txtMessage.Text = "";
            else
                txtMessage.Text = message;
        }
        private void btnDoSQL_Click(object sender, EventArgs e)
        {
            CSqlite sqlite = null;
            if (m_spSqlite != null)
                sqlite = m_spSqlite.Seek();
            if (sqlite == null)
            {
                txtMessage.Text = "No connection to a SocketPro SQLite server";
                return;
            }
            sqlite.Execute(txtSQL.Text, (h, res, errMsg, affected, fail_ok, lastId) => {
                BeginInvoke(m_sql_message,res, errMsg);
            });
        }
    }
}
