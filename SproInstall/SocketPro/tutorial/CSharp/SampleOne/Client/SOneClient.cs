using System;
using System.Drawing;
using System.ComponentModel;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

namespace SOneClient
{
	public class frmSampleOne : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TextBox txtProPort;
		private System.Windows.Forms.TextBox txtHost;
		private System.Windows.Forms.CheckBox chkZip;
		private System.Windows.Forms.Button btnDisconnect;
		private System.Windows.Forms.Button btnConnect;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label1;
		private System.ComponentModel.Container components = null;
        private CTOne                       m_MySvsHandler;
		private CClientSocket				m_ClientSocket;
		private System.Windows.Forms.TextBox txtSleep;
		private System.Windows.Forms.Button btnSleep;
		private System.Windows.Forms.CheckBox chkFrozen;
		private System.Windows.Forms.Button btnQueryCount;
		private System.Windows.Forms.TextBox txtCount;
		private System.Windows.Forms.Button btnQueryGlobalCount;
		private System.Windows.Forms.TextBox txtQueryGlobalCount;
		private System.Windows.Forms.Button btnQueryGlobalFastCount;
		private System.Windows.Forms.TextBox txtQueryGlobalFastCount;
		private System.Windows.Forms.Button btnEchoData;
		private System.Windows.Forms.Button btnGetAllCounts;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox txtUserID;
		private System.Windows.Forms.TextBox txtPassword;
		public frmSampleOne(){
			InitializeComponent();
		}
		protected override void Dispose( bool disposing ){
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
		private void InitializeComponent()
		{
            this.txtProPort = new System.Windows.Forms.TextBox();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.chkZip = new System.Windows.Forms.CheckBox();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.txtSleep = new System.Windows.Forms.TextBox();
            this.btnSleep = new System.Windows.Forms.Button();
            this.chkFrozen = new System.Windows.Forms.CheckBox();
            this.btnQueryCount = new System.Windows.Forms.Button();
            this.txtCount = new System.Windows.Forms.TextBox();
            this.btnQueryGlobalCount = new System.Windows.Forms.Button();
            this.txtQueryGlobalCount = new System.Windows.Forms.TextBox();
            this.btnQueryGlobalFastCount = new System.Windows.Forms.Button();
            this.txtQueryGlobalFastCount = new System.Windows.Forms.TextBox();
            this.btnEchoData = new System.Windows.Forms.Button();
            this.btnGetAllCounts = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.txtUserID = new System.Windows.Forms.TextBox();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // txtProPort
            // 
            this.txtProPort.Location = new System.Drawing.Point(144, 32);
            this.txtProPort.Name = "txtProPort";
            this.txtProPort.Size = new System.Drawing.Size(48, 20);
            this.txtProPort.TabIndex = 18;
            this.txtProPort.Text = "20901";
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(16, 32);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(120, 20);
            this.txtHost.TabIndex = 16;
            this.txtHost.Text = "localhost";
            // 
            // chkZip
            // 
            this.chkZip.Location = new System.Drawing.Point(344, 32);
            this.chkZip.Name = "chkZip";
            this.chkZip.Size = new System.Drawing.Size(48, 20);
            this.chkZip.TabIndex = 15;
            this.chkZip.Text = "Zip ?";
            this.chkZip.CheckedChanged += new System.EventHandler(this.chkZip_CheckedChanged);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(344, 56);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(80, 24);
            this.btnDisconnect.TabIndex = 14;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(344, 0);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(80, 24);
            this.btnConnect.TabIndex = 13;
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(144, 8);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(40, 16);
            this.label3.TabIndex = 12;
            this.label3.Text = "Port:";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(132, 16);
            this.label1.TabIndex = 10;
            this.label1.Text = "Host Address:";
            // 
            // txtSleep
            // 
            this.txtSleep.Location = new System.Drawing.Point(96, 80);
            this.txtSleep.Name = "txtSleep";
            this.txtSleep.Size = new System.Drawing.Size(48, 20);
            this.txtSleep.TabIndex = 21;
            this.txtSleep.Text = "5000";
            // 
            // btnSleep
            // 
            this.btnSleep.Enabled = false;
            this.btnSleep.Location = new System.Drawing.Point(16, 72);
            this.btnSleep.Name = "btnSleep";
            this.btnSleep.Size = new System.Drawing.Size(72, 32);
            this.btnSleep.TabIndex = 22;
            this.btnSleep.Text = "Sleep";
            this.btnSleep.Click += new System.EventHandler(this.btnSleep_Click);
            // 
            // chkFrozen
            // 
            this.chkFrozen.Location = new System.Drawing.Point(160, 80);
            this.chkFrozen.Name = "chkFrozen";
            this.chkFrozen.Size = new System.Drawing.Size(104, 24);
            this.chkFrozen.TabIndex = 24;
            this.chkFrozen.Text = "GUI Frozen";
            this.chkFrozen.CheckedChanged += new System.EventHandler(this.chkFrozen_CheckedChanged);
            // 
            // btnQueryCount
            // 
            this.btnQueryCount.Enabled = false;
            this.btnQueryCount.Location = new System.Drawing.Point(16, 152);
            this.btnQueryCount.Name = "btnQueryCount";
            this.btnQueryCount.Size = new System.Drawing.Size(128, 32);
            this.btnQueryCount.TabIndex = 25;
            this.btnQueryCount.Text = "QueryCount";
            this.btnQueryCount.Click += new System.EventHandler(this.btnQueryCount_Click);
            // 
            // txtCount
            // 
            this.txtCount.Location = new System.Drawing.Point(152, 160);
            this.txtCount.Name = "txtCount";
            this.txtCount.Size = new System.Drawing.Size(64, 20);
            this.txtCount.TabIndex = 26;
            this.txtCount.Text = "0";
            // 
            // btnQueryGlobalCount
            // 
            this.btnQueryGlobalCount.Enabled = false;
            this.btnQueryGlobalCount.Location = new System.Drawing.Point(16, 192);
            this.btnQueryGlobalCount.Name = "btnQueryGlobalCount";
            this.btnQueryGlobalCount.Size = new System.Drawing.Size(160, 32);
            this.btnQueryGlobalCount.TabIndex = 27;
            this.btnQueryGlobalCount.Text = "QueryGlobalCount";
            this.btnQueryGlobalCount.Click += new System.EventHandler(this.btnQueryGlobalCount_Click);
            // 
            // txtQueryGlobalCount
            // 
            this.txtQueryGlobalCount.Location = new System.Drawing.Point(184, 200);
            this.txtQueryGlobalCount.Name = "txtQueryGlobalCount";
            this.txtQueryGlobalCount.Size = new System.Drawing.Size(64, 20);
            this.txtQueryGlobalCount.TabIndex = 28;
            this.txtQueryGlobalCount.Text = "0";
            // 
            // btnQueryGlobalFastCount
            // 
            this.btnQueryGlobalFastCount.Enabled = false;
            this.btnQueryGlobalFastCount.Location = new System.Drawing.Point(16, 232);
            this.btnQueryGlobalFastCount.Name = "btnQueryGlobalFastCount";
            this.btnQueryGlobalFastCount.Size = new System.Drawing.Size(216, 32);
            this.btnQueryGlobalFastCount.TabIndex = 29;
            this.btnQueryGlobalFastCount.Text = "QueryGlobalFastCount";
            this.btnQueryGlobalFastCount.Click += new System.EventHandler(this.btnQueryGlobalFastCount_Click);
            // 
            // txtQueryGlobalFastCount
            // 
            this.txtQueryGlobalFastCount.Location = new System.Drawing.Point(240, 240);
            this.txtQueryGlobalFastCount.Name = "txtQueryGlobalFastCount";
            this.txtQueryGlobalFastCount.Size = new System.Drawing.Size(64, 20);
            this.txtQueryGlobalFastCount.TabIndex = 30;
            this.txtQueryGlobalFastCount.Text = "0";
            // 
            // btnEchoData
            // 
            this.btnEchoData.Enabled = false;
            this.btnEchoData.Location = new System.Drawing.Point(16, 272);
            this.btnEchoData.Name = "btnEchoData";
            this.btnEchoData.Size = new System.Drawing.Size(112, 32);
            this.btnEchoData.TabIndex = 31;
            this.btnEchoData.Text = "Echo Data";
            this.btnEchoData.Click += new System.EventHandler(this.btnEchoData_Click);
            // 
            // btnGetAllCounts
            // 
            this.btnGetAllCounts.Enabled = false;
            this.btnGetAllCounts.Location = new System.Drawing.Point(16, 112);
            this.btnGetAllCounts.Name = "btnGetAllCounts";
            this.btnGetAllCounts.Size = new System.Drawing.Size(120, 32);
            this.btnGetAllCounts.TabIndex = 32;
            this.btnGetAllCounts.Text = "Get All Counts";
            this.btnGetAllCounts.Click += new System.EventHandler(this.btnGetAllCounts_Click);
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(216, 8);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(48, 16);
            this.label2.TabIndex = 34;
            this.label2.Text = "User ID:";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(200, 32);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(64, 24);
            this.label5.TabIndex = 35;
            this.label5.Text = "Password:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txtUserID
            // 
            this.txtUserID.Location = new System.Drawing.Point(272, 8);
            this.txtUserID.Name = "txtUserID";
            this.txtUserID.Size = new System.Drawing.Size(64, 20);
            this.txtUserID.TabIndex = 36;
            this.txtUserID.Text = "SocketPro";
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(272, 32);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(64, 20);
            this.txtPassword.TabIndex = 37;
            this.txtPassword.Text = "PassOne";
            // 
            // frmSampleOne
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(448, 326);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.txtUserID);
            this.Controls.Add(this.txtQueryGlobalFastCount);
            this.Controls.Add(this.txtQueryGlobalCount);
            this.Controls.Add(this.txtCount);
            this.Controls.Add(this.txtSleep);
            this.Controls.Add(this.txtProPort);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnGetAllCounts);
            this.Controls.Add(this.btnEchoData);
            this.Controls.Add(this.btnQueryGlobalFastCount);
            this.Controls.Add(this.btnQueryGlobalCount);
            this.Controls.Add(this.btnQueryCount);
            this.Controls.Add(this.chkFrozen);
            this.Controls.Add(this.btnSleep);
            this.Controls.Add(this.chkZip);
            this.Controls.Add(this.btnDisconnect);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label1);
            this.Name = "frmSampleOne";
            this.Text = "Sample SocketPro Client Application One";
            this.Closed += new System.EventHandler(this.frmSampleOne_Closed);
            this.Load += new System.EventHandler(this.frmSampleOne_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion
		[STAThread]
		static void Main() 
		{
			Application.Run(new frmSampleOne());
		}
		private void frmSampleOne_Load(object sender, System.EventArgs e)
		{
            m_MySvsHandler = new CTOne();
            m_ClientSocket = new CClientSocket();
            m_ClientSocket.m_OnSocketClosed += OnClosed;
            m_ClientSocket.m_OnSocketConnected += OnConnected;
            m_ClientSocket.m_OnRequestProcessed += delegate(int hSocket, short sRequestID, int nLen, int nLenInBuffer, tagReturnFlag ReturnFlag)
            {
                sRequestID = 0;
            };
			m_MySvsHandler.Attach(m_ClientSocket);
		}
		private void frmSampleOne_Closed(object sender, System.EventArgs e)
		{
			m_ClientSocket.Disconnect();
			m_MySvsHandler.Detach();
		}
		private void OnClosed(int nSocketHandle, int nError)
		{
			btnSleep.Enabled = false;
			btnQueryCount.Enabled = false;
			btnEchoData.Enabled = false;
			btnGetAllCounts.Enabled = false;
			btnQueryGlobalFastCount.Enabled = false;
			btnQueryGlobalCount.Enabled = false;
		}
		private void OnConnected(int nSocketHandle, int nError)
		{
			if(nError == 0)
			{
				btnSleep.Enabled = true;
				btnQueryCount.Enabled = true;
				btnEchoData.Enabled = true;
				btnGetAllCounts.Enabled = true;
				btnQueryGlobalFastCount.Enabled = true;
				btnQueryGlobalCount.Enabled = true;
				//set user id and password
				m_ClientSocket.SetUID(txtUserID.Text);
				m_ClientSocket.SetPassword(txtPassword.Text);
				//Turn on/off online compressing at client side
				m_ClientSocket.GetUSocket().ZipIsOn = chkZip.Checked;
				//can not batch requests when the property Syn is set to true
				m_ClientSocket.BeginBatching();
				//switch for the service identified by sidSOneSvs
				m_ClientSocket.SwitchTo(m_MySvsHandler);
				//Turn on/off online compressing at server side
				m_ClientSocket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked);
				//batch the three requests and send them to a remote server for processing in one batch only
				m_ClientSocket.Commit(false); //when switch from sidStartup to another, false is a must.
			}
			else
			{
				MessageBox.Show(m_ClientSocket.GetErrorMsg());
			}
		}
		private void btnConnect_Click(object sender, System.EventArgs e)
		{
			bool bSync = false;
			//connect to a remote server asynchronously
			m_ClientSocket.Connect(txtHost.Text, Int32.Parse(txtProPort.Text), bSync);
		}
		private void btnDisconnect_Click(object sender, System.EventArgs e)
		{
			//notify the server to shut down a connection gracefully
			m_ClientSocket.Disconnect();
		}
		private void btnSleep_Click(object sender, System.EventArgs e)
		{
			int nSleep = Int32.Parse(txtSleep.Text);
            btnSleep.Enabled = false;
            m_MySvsHandler.Sleep(nSleep);
            btnSleep.Enabled = m_ClientSocket.IsConnected();
		}
		private void btnQueryCount_Click(object sender, System.EventArgs e)
		{
			txtCount.Text = m_MySvsHandler.QueryCount().ToString();
		}
		private void chkFrozen_CheckedChanged(object sender, System.EventArgs e)
		{
			m_ClientSocket.DisableUI(chkFrozen.Checked);
		}
		private void btnQueryGlobalCount_Click(object sender, System.EventArgs e)
		{
			txtQueryGlobalCount.Text = m_MySvsHandler.QueryGlobalCount().ToString();
		}
		private void btnQueryGlobalFastCount_Click(object sender, System.EventArgs e)
		{
			txtQueryGlobalFastCount.Text = m_MySvsHandler.QueryGlobalFastCount().ToString();
		}
		private void btnEchoData_Click(object sender, System.EventArgs e)
		{
			int nData = 12345678;
			//set a rather complicate object data
			object []objA = new object[3];
			object []objChildren = new object[4];
			objA[0] = "This is a test string";
			objA[1] = 123456789;
			objChildren[0] = System.DateTime.Now;
			objChildren[1] = "This is a sub-string";
			objChildren[2] = nData;
			objA[2] = objChildren;
			object objOut = m_MySvsHandler.Echo(objA);
            nData = 0; //put debug break here
		}

		private void btnGetAllCounts_Click(object sender, System.EventArgs e)
		{
			int nCount;
			int nGlobalCount;
			int nGlobalFastCount;
			m_MySvsHandler.GetAllCounts(out nCount, out nGlobalCount, out nGlobalFastCount);
			txtQueryGlobalFastCount.Text = nGlobalFastCount.ToString();
			txtQueryGlobalCount.Text = nGlobalCount.ToString();
			txtCount.Text = nCount.ToString();
		}
		
		private void chkZip_CheckedChanged(object sender, System.EventArgs e)
		{
			m_ClientSocket.GetUSocket().ZipIsOn = chkZip.Checked;
			m_ClientSocket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked);
		}
	}
}
