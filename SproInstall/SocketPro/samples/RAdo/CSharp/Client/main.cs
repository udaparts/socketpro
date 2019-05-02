//#define USE_SQLCLIENT

#if USE_SQLCLIENT
using System.Data.SqlClient;
#else
using System.Data.OleDb;
#endif

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

namespace RAdo
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class frmRemotingAdoNet : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button btnConnect;
		private System.Windows.Forms.Button btnDisconnect;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtHost;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtPort;
		private System.Windows.Forms.CheckBox chkSSL;
		private System.Windows.Forms.CheckBox chkZip;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

        CClientSocket m_ClientSocket = new CClientSocket();
		private System.Windows.Forms.Button btnGetDR;
		private System.Windows.Forms.Button btnGetDS;
		private System.Windows.Forms.Button btnSendDR;
		private System.Windows.Forms.Button btnSendDS;
        private DataGridView dgTable;
		CRAdo m_RAdo = new CRAdo();

		public frmRemotingAdoNet()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.chkSSL = new System.Windows.Forms.CheckBox();
            this.chkZip = new System.Windows.Forms.CheckBox();
            this.btnGetDR = new System.Windows.Forms.Button();
            this.btnGetDS = new System.Windows.Forms.Button();
            this.btnSendDR = new System.Windows.Forms.Button();
            this.btnSendDS = new System.Windows.Forms.Button();
            this.dgTable = new System.Windows.Forms.DataGridView();
            ((System.ComponentModel.ISupportInitialize)(this.dgTable)).BeginInit();
            this.SuspendLayout();
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(352, 0);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(88, 24);
            this.btnConnect.TabIndex = 0;
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(352, 32);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(88, 23);
            this.btnDisconnect.TabIndex = 1;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(100, 24);
            this.label1.TabIndex = 2;
            this.label1.Text = "Remote Host:";
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(16, 32);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(160, 20);
            this.txtHost.TabIndex = 3;
            this.txtHost.Text = "localhost";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(192, 8);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(56, 24);
            this.label2.TabIndex = 4;
            this.label2.Text = "Port #:";
            // 
            // txtPort
            // 
            this.txtPort.Location = new System.Drawing.Point(192, 32);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(56, 20);
            this.txtPort.TabIndex = 5;
            this.txtPort.Text = "20901";
            // 
            // chkSSL
            // 
            this.chkSSL.Location = new System.Drawing.Point(272, 8);
            this.chkSSL.Name = "chkSSL";
            this.chkSSL.Size = new System.Drawing.Size(56, 24);
            this.chkSSL.TabIndex = 6;
            this.chkSSL.Text = "SSL";
            // 
            // chkZip
            // 
            this.chkZip.Location = new System.Drawing.Point(272, 32);
            this.chkZip.Name = "chkZip";
            this.chkZip.Size = new System.Drawing.Size(56, 24);
            this.chkZip.TabIndex = 7;
            this.chkZip.Text = "Zip";
            // 
            // btnGetDR
            // 
            this.btnGetDR.Enabled = false;
            this.btnGetDR.Location = new System.Drawing.Point(16, 64);
            this.btnGetDR.Name = "btnGetDR";
            this.btnGetDR.Size = new System.Drawing.Size(96, 23);
            this.btnGetDR.TabIndex = 9;
            this.btnGetDR.Text = "Get DataReader";
            this.btnGetDR.Click += new System.EventHandler(this.btnGetDR_Click);
            // 
            // btnGetDS
            // 
            this.btnGetDS.Enabled = false;
            this.btnGetDS.Location = new System.Drawing.Point(128, 64);
            this.btnGetDS.Name = "btnGetDS";
            this.btnGetDS.Size = new System.Drawing.Size(88, 23);
            this.btnGetDS.TabIndex = 10;
            this.btnGetDS.Text = "Get DataSet";
            this.btnGetDS.Click += new System.EventHandler(this.btnGetDS_Click);
            // 
            // btnSendDR
            // 
            this.btnSendDR.Enabled = false;
            this.btnSendDR.Location = new System.Drawing.Point(232, 64);
            this.btnSendDR.Name = "btnSendDR";
            this.btnSendDR.Size = new System.Drawing.Size(104, 23);
            this.btnSendDR.TabIndex = 11;
            this.btnSendDR.Text = "Send DataReader";
            this.btnSendDR.Click += new System.EventHandler(this.btnSendDR_Click);
            // 
            // btnSendDS
            // 
            this.btnSendDS.Enabled = false;
            this.btnSendDS.Location = new System.Drawing.Point(352, 64);
            this.btnSendDS.Name = "btnSendDS";
            this.btnSendDS.Size = new System.Drawing.Size(88, 23);
            this.btnSendDS.TabIndex = 12;
            this.btnSendDS.Text = "Send DataSet";
            this.btnSendDS.Click += new System.EventHandler(this.btnSendDS_Click);
            // 
            // dgTable
            // 
            this.dgTable.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgTable.Location = new System.Drawing.Point(19, 94);
            this.dgTable.Name = "dgTable";
            this.dgTable.Size = new System.Drawing.Size(421, 242);
            this.dgTable.TabIndex = 13;
            // 
            // frmRemotingAdoNet
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(456, 348);
            this.Controls.Add(this.dgTable);
            this.Controls.Add(this.btnSendDS);
            this.Controls.Add(this.btnSendDR);
            this.Controls.Add(this.btnGetDS);
            this.Controls.Add(this.btnGetDR);
            this.Controls.Add(this.chkZip);
            this.Controls.Add(this.chkSSL);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Name = "frmRemotingAdoNet";
            this.Text = "Bi-directional Remoting ADO.NET Objects";
            this.Load += new System.EventHandler(this.frmRemotingAdoNet_Load);
            ((System.ComponentModel.ISupportInitialize)(this.dgTable)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new frmRemotingAdoNet());
		}
		
		private void OnSocketClosed(int hSocket, int nError)
		{
			btnGetDS.Enabled = false;
			btnGetDR.Enabled = false;
			btnSendDS.Enabled = false;
			btnSendDR.Enabled = false;
			if(nError != 0)
			{
				MessageBox.Show(m_ClientSocket.GetErrorMsg());
			}
		}

		private void OnSocketConnected(int hSokcet, int nError)
		{
			if(nError == 0)
			{
				btnGetDS.Enabled = true;
				btnGetDR.Enabled = true;
				btnSendDS.Enabled = true;
				btnSendDR.Enabled = true;

				m_ClientSocket.SetUID("SocketPro");
				m_ClientSocket.SetPassword("PassOne");
				m_ClientSocket.BeginBatching();
				m_ClientSocket.SwitchTo(m_RAdo, true);
				if(chkZip.Checked)
				{
					m_ClientSocket.GetUSocket().TurnOnZipAtSvr(true);
				}
				
				//Incerasing TCP sending and receiving buffer sizes will help performance
				//when there is a lot of data transferred if your network bandwidth is over 10 mbps
				m_ClientSocket.GetUSocket().SetSockOpt((int)USOCKETLib.tagSocketOption.soSndBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);
				m_ClientSocket.GetUSocket().SetSockOpt((int) USOCKETLib.tagSocketOption.soRcvBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);              
				m_ClientSocket.GetUSocket().SetSockOptAtSvr((int) USOCKETLib.tagSocketOption.soSndBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);
				m_ClientSocket.GetUSocket().SetSockOptAtSvr((int) USOCKETLib.tagSocketOption.soRcvBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);

				//turn off Nagel algorithm for both sides
				m_ClientSocket.GetUSocket().SetSockOpt((int)USOCKETLib.tagSocketOption.soTcpNoDelay, 1, (int)USOCKETLib.tagSocketLevel.slTcp);
				m_ClientSocket.GetUSocket().SetSockOptAtSvr((int)USOCKETLib.tagSocketOption.soTcpNoDelay, 1, (int)USOCKETLib.tagSocketLevel.slTcp);
				m_ClientSocket.Commit(false); //must be false for the first switch
			}
		}
		private void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib.tagReturnFlag ReturnFlag)
		{
			if(ReturnFlag != USOCKETLib.tagReturnFlag.rfCompleted)
				return;
            switch (sRequestID)
            {
                case (short)CAsyncAdoSerializationHelper.idDataTableRowsArrive:
                case (short)CAsyncAdoSerializationHelper.idDataReaderRecordsArrive:
                    if (!m_bUpdate)
                    {
                        //show the first batch of records only
                        dgTable.DataSource = m_RAdo.CurrentDataTable.Copy();
                        dgTable.Update();
                        m_bUpdate = true;
                    }
                    break;
                default:
                    break;
            }
		}
		private void frmRemotingAdoNet_Load(object sender, System.EventArgs e)
		{
			m_RAdo.Attach(m_ClientSocket);
			m_ClientSocket.m_OnSocketClosed += OnSocketClosed;
			m_ClientSocket.m_OnSocketConnected += OnSocketConnected;
			m_ClientSocket.m_OnRequestProcessed += OnRequestProcessed;
		}

		private void btnConnect_Click(object sender, System.EventArgs e)
		{
			if(chkSSL.Checked)
				m_ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.MSTLSv1;
			else
				m_ClientSocket.EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption;
			m_ClientSocket.Connect(txtHost.Text, int.Parse(txtPort.Text));
		}

		private void btnDisconnect_Click(object sender, System.EventArgs e)
		{
			m_ClientSocket.Disconnect();
		}
        bool m_bUpdate = false;
		private void btnGetDR_Click(object sender, System.EventArgs e)
		{
            m_bUpdate = false;
			DataTable dt = m_RAdo.GetDataReader("Select * from products, orders");

/*			You can use the below code to display result if data record set size is not large.
 *			You should consider latency and use delegate OnRequestProcessed instead if either record set is large or network bandwith is small.
 */
            dgTable.DataSource = dt;
		}

		private void btnGetDS_Click(object sender, System.EventArgs e)
		{
            m_bUpdate = false;
            DataSet ds = m_RAdo.GetDataSet("Select * from customers", "Select * from employees");
/*			You can use the below code to display result if data record set size is not large.
 *			You should consider latency and use delegate OnRequestProcessed instead if either record set is large or network bandwith is small.
 */
            if(ds != null && ds.Tables.Count > 1)
                dgTable.DataSource = ds.Tables[1];
		}

		private void btnSendDR_Click(object sender, System.EventArgs e)
		{
			string strSQL = "Select * from Customers";
			IDataReader dr = null;
#if USE_SQLCLIENT
		SqlConnection conn = new SqlConnection("server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind");
#else
            OleDbConnection conn = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\\Program Files\\udaparts\\SocketPro\\bin\\nwind3.mdb");
#endif
			try
			{
				conn.Open();
			}
			catch (Exception err)
			{
				Console.WriteLine(err.Message);
				return;
			}

#if USE_SQLCLIENT
		SqlCommand cmd = new SqlCommand(strSQL, conn);
#else
			OleDbCommand cmd = new OleDbCommand(strSQL, conn);
#endif
			try
			{
				dr = cmd.ExecuteReader();
			}
			catch (Exception err)
			{
				Console.WriteLine(err.Message);
				conn.Close();
				return;
			}
			bool  ok = m_RAdo.SendDataReader(dr);
			conn.Close();
		}

		private void btnSendDS_Click(object sender, System.EventArgs e)
		{
			string strSQL0 = "Select * from Shippers";
			string strSQL1 = "Select * from Products";

			DataSet ds = new DataSet("MyDataSet");
#if USE_SQLCLIENT
		SqlConnection conn = new SqlConnection("server=localhost\\sqlexpress;Integrated Security=SSPI;database=northwind");
#else
            OleDbConnection conn = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=c:\\Program Files\\udaparts\\SocketPro\\bin\\nwind3.mdb");
#endif
			try
			{
				conn.Open();
			}
			catch (Exception err)
			{
				Console.WriteLine(err.Message);
				return;
			}
#if USE_SQLCLIENT
		SqlCommand cmd = new SqlCommand(strSQL0, conn);
		SqlDataAdapter adapter = new SqlDataAdapter(cmd);
		SqlCommand cmd1 = new SqlCommand(strSQL1, conn);
		SqlDataAdapter adapter1 = new SqlDataAdapter(cmd1);
#else
			OleDbCommand cmd = new OleDbCommand(strSQL0, conn);
			OleDbDataAdapter adapter = new OleDbDataAdapter(cmd);
			OleDbCommand cmd1 = new OleDbCommand(strSQL1, conn);
			OleDbDataAdapter adapter1 = new OleDbDataAdapter(cmd1);
#endif
			try
			{
				adapter.Fill(ds, "Table1");
				adapter1.Fill(ds, "Table2");
			}
			catch (Exception err)
			{
				Console.WriteLine(err.Message);
				conn.Close();
				return;
			}
			bool ok = m_RAdo.SendDataSet(ds);
			conn.Close();
		}
	}
}
