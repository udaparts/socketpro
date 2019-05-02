using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using UDBLib;
using USOCKETLib;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.RemoteDB;

namespace MyTest
{
    public partial class frmMyTest : Form
    {
        CAsynDBLiteEx m_AsynDBLite;
        CClientSocket m_ClientSocket;
        public frmMyTest()
        {
            InitializeComponent();
        }

        private void btnDoSQL_Click(object sender, EventArgs e)
        {
            //diable this button temporarily so that one SQL query is executed only
            btnDoSQL.Enabled = false;

            //SubBatchSize reduces latency
            m_AsynDBLite.SubBatchSize = 20;
            
            //Open one rowset and set generated DataTable with the name "Table1"
            m_AsynDBLite.OpenRowset(txtSQL.Text, "Table1", tagCursorType.ctStatic, CAsynDBLite.Scrollable, 100, -1);
            
            //wait until all of requests are executed
            m_AsynDBLite.GetAttachedClientSocket().WaitAll();
            btnDoSQL.Enabled = true;
            
            //enable or disable buttons according to rowset properties (readonly and scrollable)
            if (m_AsynDBLite.IsRowsetOpened)
            {
                btnNextBatch.Enabled = btnFirst.Enabled = true;
                btnAdd.Enabled = (!m_AsynDBLite.IsRowsetReadonly);
            }
            else
            {
                btnFirst.Enabled = btnNextBatch.Enabled = btnAdd.Enabled = false;
            }
            btnPrev.Enabled = btnLast.Enabled = m_AsynDBLite.IsRowsetScrollable;
        }

        private void OnSocketClosed(int hSocket, int nError)
        {
            btnDoSQL.Enabled = false;
            btnFirst.Enabled = false;
            btnNextBatch.Enabled = false;
            btnPrev.Enabled = false;
            btnLast.Enabled = false;
            btnAdd.Enabled = false;
            Text = "AsynDBLite Demo";
        }

        private void OnSocketConnected(int hSocket, int nError)
        {
            if (nError == 0)
            {
                int lPeerPort = 0;
                int lHandle;
                string strAlias;
                btnDoSQL.Enabled = true;
                
                m_ClientSocket.SetUID("SocketPro");
                m_ClientSocket.SetPassword("PassOne");
                
                //ask for remote DB service
                m_ClientSocket.SwitchTo(m_AsynDBLite);

                //get remote host ip adress and host name
                string strIPAddr = m_ClientSocket.GetUSocket().GetPeerName(out lPeerPort);
                string strHostName = m_ClientSocket.GetUSocket().GetHostByAddr(strIPAddr, (int)USOCKETLib.tagAddressFamily.afINet, true, out lHandle, out strAlias);
                Text += " (";
                Text += strHostName;
                Text += ":";
                Text += lPeerPort.ToString();
                Text += ")";

//                m_AsynDBLite.ConnectDB("Provider=sqlncli;Data Source=localhost\\sqlexpress;Initial Catalog=northwind;Integrated Security=SSPI");
                
                //connect to database
                m_AsynDBLite.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb");
            }
        }

        private void frmMyTest_FormClosed(object sender, FormClosedEventArgs args)
        {
            m_ClientSocket.m_OnSocketClosed -= new DOnSocketClosed(OnSocketClosed);
            m_ClientSocket.m_OnSocketConnected -= new DOnSocketConnected(OnSocketConnected);
            m_ClientSocket.DestroyUSocket();
        }

        private void frmMyTest_Load(object sender, EventArgs e)
        {
            m_ClientSocket = new CClientSocket();
            m_AsynDBLite = new CAsynDBLiteEx();
            m_AsynDBLite.Attach(m_ClientSocket);

            //use += new DOnSocketClosed(OnSocketClosed) instead = new DOnSocketClosed(OnSocketClosed)
            m_ClientSocket.m_OnSocketClosed += new DOnSocketClosed(OnSocketClosed);

            //use += new DOnSocketConnected(OnSocketConnected) instead = new DOnSocketConnected(OnSocketConnected)
            m_ClientSocket.m_OnSocketConnected += new DOnSocketConnected(OnSocketConnected);

            //Bind a DataGridView control with an instance of CAsynDBLite
            m_AsynDBLite.AttachedDataGridView = dgvTable;
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            //Connect to a remote SocketPro server
            m_ClientSocket.Connect("localhost", 17001);
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Disconnect();
        }

        private void btnNextBatch_Click(object sender, EventArgs e)
        {
            int nSkip = 0;
            try
            {
                nSkip = int.Parse(txtSkip.Text);
            }
            catch (Exception myError)
            {
                myError = null;
            }
            m_AsynDBLite.NextBatch(nSkip);
        }

        private void btnPrev_Click(object sender, EventArgs e)
        {
            m_AsynDBLite.PrevBatch();
        }

        private void btnLast_Click(object sender, EventArgs e)
        {
            m_AsynDBLite.LastBatch();
        }

        private void btnFirst_Click(object sender, EventArgs e)
        {
            m_AsynDBLite.FirstBatch();
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            //There are many ways to insert rows onto a database within remote database service.
            //Here we add rows in batch using cursor.
            m_AsynDBLite.DBErrors.Clear();
            m_AsynDBLite.GetAttachedClientSocket().BeginBatching();
            foreach (DataGridViewRow row in dgvTable.Rows)
            {
                int nDataGridViewRowIndex = row.Cells[0].RowIndex;
                int nRecordRowIndex = m_AsynDBLite.GetRecordRowIndex(nDataGridViewRowIndex);
                if (nRecordRowIndex >= m_AsynDBLite.RowsFetched)
                    m_AsynDBLite.AddRecord(row);
            }
            m_AsynDBLite.GetAttachedClientSocket().Commit(true);
        }
    }
}