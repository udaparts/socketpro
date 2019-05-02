using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Diagnostics;
using System.Runtime.InteropServices;

using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using SampleThreeShared;

namespace SOneClient
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	/// 
	public class frmSampleOne : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TextBox txtProPort;
		private System.Windows.Forms.TextBox txtHost;
		private System.Windows.Forms.CheckBox chkZip;
		private System.Windows.Forms.Button btnDisconnect;
		private System.Windows.Forms.Button btnConnect;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private long						m_lPrev;
		private CTOne				        m_MySvsHandler;
		private CClientSocket				m_ClientSocket;
		private CTThree					    m_S3Handler;
		private CUPerformanceQuery			m_PerfQuery;
		private Stack						m_Stack;

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
		private System.Windows.Forms.CheckBox chkUseSSL;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox txtMsg;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TextBox txtLatency;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.TextBox txtTimeRequired;
        private System.Windows.Forms.Button btnGetOneItemFromServer;
        private System.Windows.Forms.Button btnGetALotItemsFromServer;
		private System.Windows.Forms.TextBox txtGetALot;
        private System.Windows.Forms.Button btnSendOneItemToServer;
		private System.Windows.Forms.Button btnSendALotItemsToServer;
        private System.Windows.Forms.TextBox txtSendALot;
		private System.Windows.Forms.TextBox txtBytesRecv;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.TextBox txtBytesSent;
		private System.Windows.Forms.TextBox txtPassword;
		
		public frmSampleOne()
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
            this.chkUseSSL = new System.Windows.Forms.CheckBox();
            this.label4 = new System.Windows.Forms.Label();
            this.txtMsg = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.txtLatency = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.txtTimeRequired = new System.Windows.Forms.TextBox();
            this.btnGetOneItemFromServer = new System.Windows.Forms.Button();
            this.btnGetALotItemsFromServer = new System.Windows.Forms.Button();
            this.txtGetALot = new System.Windows.Forms.TextBox();
            this.btnSendOneItemToServer = new System.Windows.Forms.Button();
            this.btnSendALotItemsToServer = new System.Windows.Forms.Button();
            this.txtSendALot = new System.Windows.Forms.TextBox();
            this.txtBytesRecv = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.txtBytesSent = new System.Windows.Forms.TextBox();
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
            this.chkZip.Location = new System.Drawing.Point(456, 32);
            this.chkZip.Name = "chkZip";
            this.chkZip.Size = new System.Drawing.Size(48, 16);
            this.chkZip.TabIndex = 15;
            this.chkZip.Text = "Zip ?";
            this.chkZip.CheckedChanged += new System.EventHandler(this.chkZip_CheckedChanged);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(456, 56);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(160, 24);
            this.btnDisconnect.TabIndex = 14;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(456, 0);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(160, 24);
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
            // chkUseSSL
            // 
            this.chkUseSSL.Location = new System.Drawing.Point(536, 32);
            this.chkUseSSL.Name = "chkUseSSL";
            this.chkUseSSL.Size = new System.Drawing.Size(72, 16);
            this.chkUseSSL.TabIndex = 38;
            this.chkUseSSL.Text = "Use SSL";
            this.chkUseSSL.CheckedChanged += new System.EventHandler(this.chkUseSSL_CheckedChanged);
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(216, 112);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(360, 16);
            this.label4.TabIndex = 39;
            this.label4.Text = "Notification Message:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // txtMsg
            // 
            this.txtMsg.Location = new System.Drawing.Point(152, 128);
            this.txtMsg.Name = "txtMsg";
            this.txtMsg.Size = new System.Drawing.Size(472, 20);
            this.txtMsg.TabIndex = 40;
            this.txtMsg.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(344, 8);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(80, 16);
            this.label6.TabIndex = 41;
            this.label6.Text = "Latency (us):";
            // 
            // txtLatency
            // 
            this.txtLatency.Location = new System.Drawing.Point(344, 24);
            this.txtLatency.Name = "txtLatency";
            this.txtLatency.Size = new System.Drawing.Size(104, 20);
            this.txtLatency.TabIndex = 42;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(344, 48);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(104, 16);
            this.label7.TabIndex = 43;
            this.label7.Text = "TimeRequired (us):";
            // 
            // txtTimeRequired
            // 
            this.txtTimeRequired.Location = new System.Drawing.Point(344, 72);
            this.txtTimeRequired.Name = "txtTimeRequired";
            this.txtTimeRequired.Size = new System.Drawing.Size(104, 20);
            this.txtTimeRequired.TabIndex = 44;
            // 
            // btnGetOneItemFromServer
            // 
            this.btnGetOneItemFromServer.Enabled = false;
            this.btnGetOneItemFromServer.Location = new System.Drawing.Point(320, 157);
            this.btnGetOneItemFromServer.Name = "btnGetOneItemFromServer";
            this.btnGetOneItemFromServer.Size = new System.Drawing.Size(240, 24);
            this.btnGetOneItemFromServer.TabIndex = 45;
            this.btnGetOneItemFromServer.Text = "GetOneItemFromServer";
            this.btnGetOneItemFromServer.Click += new System.EventHandler(this.btnGetOneItemFromServer_Click);
            // 
            // btnGetALotItemsFromServer
            // 
            this.btnGetALotItemsFromServer.Enabled = false;
            this.btnGetALotItemsFromServer.Location = new System.Drawing.Point(320, 184);
            this.btnGetALotItemsFromServer.Name = "btnGetALotItemsFromServer";
            this.btnGetALotItemsFromServer.Size = new System.Drawing.Size(240, 24);
            this.btnGetALotItemsFromServer.TabIndex = 48;
            this.btnGetALotItemsFromServer.Text = "GetALotItemsFromServer";
            this.btnGetALotItemsFromServer.Click += new System.EventHandler(this.btnGetALotItemsFromServer_Click);
            // 
            // txtGetALot
            // 
            this.txtGetALot.Location = new System.Drawing.Point(568, 184);
            this.txtGetALot.Name = "txtGetALot";
            this.txtGetALot.Size = new System.Drawing.Size(56, 20);
            this.txtGetALot.TabIndex = 49;
            this.txtGetALot.Text = "50000";
            // 
            // btnSendOneItemToServer
            // 
            this.btnSendOneItemToServer.Enabled = false;
            this.btnSendOneItemToServer.Location = new System.Drawing.Point(320, 266);
            this.btnSendOneItemToServer.Name = "btnSendOneItemToServer";
            this.btnSendOneItemToServer.Size = new System.Drawing.Size(240, 24);
            this.btnSendOneItemToServer.TabIndex = 52;
            this.btnSendOneItemToServer.Text = "SendOneItemToServer";
            this.btnSendOneItemToServer.Click += new System.EventHandler(this.btnSendOneItemToServer_Click);
            // 
            // btnSendALotItemsToServer
            // 
            this.btnSendALotItemsToServer.Enabled = false;
            this.btnSendALotItemsToServer.Location = new System.Drawing.Point(320, 296);
            this.btnSendALotItemsToServer.Name = "btnSendALotItemsToServer";
            this.btnSendALotItemsToServer.Size = new System.Drawing.Size(240, 24);
            this.btnSendALotItemsToServer.TabIndex = 55;
            this.btnSendALotItemsToServer.Text = "SendALotItemsToServer";
            this.btnSendALotItemsToServer.Click += new System.EventHandler(this.btnSendALotItemsToServer_Click);
            // 
            // txtSendALot
            // 
            this.txtSendALot.Location = new System.Drawing.Point(568, 296);
            this.txtSendALot.Name = "txtSendALot";
            this.txtSendALot.Size = new System.Drawing.Size(56, 20);
            this.txtSendALot.TabIndex = 56;
            this.txtSendALot.Text = "50000";
            // 
            // txtBytesRecv
            // 
            this.txtBytesRecv.Location = new System.Drawing.Point(208, 336);
            this.txtBytesRecv.Name = "txtBytesRecv";
            this.txtBytesRecv.Size = new System.Drawing.Size(96, 20);
            this.txtBytesRecv.TabIndex = 59;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(208, 312);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(96, 16);
            this.label8.TabIndex = 60;
            this.label8.Text = "Bytes received";
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(64, 312);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(80, 16);
            this.label9.TabIndex = 61;
            this.label9.Text = "Bytes sent:";
            // 
            // txtBytesSent
            // 
            this.txtBytesSent.Location = new System.Drawing.Point(64, 336);
            this.txtBytesSent.Name = "txtBytesSent";
            this.txtBytesSent.Size = new System.Drawing.Size(120, 20);
            this.txtBytesSent.TabIndex = 62;
            // 
            // frmSampleOne
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(640, 373);
            this.Controls.Add(this.txtBytesSent);
            this.Controls.Add(this.txtBytesRecv);
            this.Controls.Add(this.txtSendALot);
            this.Controls.Add(this.txtGetALot);
            this.Controls.Add(this.txtTimeRequired);
            this.Controls.Add(this.txtLatency);
            this.Controls.Add(this.txtMsg);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.txtUserID);
            this.Controls.Add(this.txtQueryGlobalFastCount);
            this.Controls.Add(this.txtQueryGlobalCount);
            this.Controls.Add(this.txtCount);
            this.Controls.Add(this.txtSleep);
            this.Controls.Add(this.txtProPort);
            this.Controls.Add(this.txtHost);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.btnSendALotItemsToServer);
            this.Controls.Add(this.btnSendOneItemToServer);
            this.Controls.Add(this.btnGetALotItemsFromServer);
            this.Controls.Add(this.btnGetOneItemFromServer);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.chkUseSSL);
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

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new frmSampleOne());
		}

		private void frmSampleOne_Load(object sender, System.EventArgs e)
		{
			m_S3Handler = new CTThree();
			m_MySvsHandler = new CTOne();
            m_ClientSocket = new CClientSocket();

            m_ClientSocket.m_OnBaseRequestProcessed += new DOnBaseRequestProcessed(OnBaseRequestProcessed);
            m_ClientSocket.m_OnSocketClosed += new DOnSocketClosed(OnClosed);
            m_ClientSocket.m_OnSocketConnected += new DOnSocketConnected(OnConnected);
            m_ClientSocket.m_OnRequestProcessed += new DOnRequestProcessed(OnRequestProcessed);
			
			//two services share one socket connection
			m_MySvsHandler.Attach(m_ClientSocket);
			m_S3Handler.Attach(m_ClientSocket);

			m_PerfQuery = new CUPerformanceQuery();
			m_Stack = new Stack();
		}

		private void frmSampleOne_Closed(object sender, System.EventArgs e)
		{
			//abort socket connection
			m_ClientSocket.Disconnect();
		}

		public void UpdateBytes()
		{
			int nHigh = 0;
			int nSent = m_ClientSocket.GetUSocket().GetBytesSent(out nHigh);
			int nRecv = m_ClientSocket.GetUSocket().GetBytesReceived(out nHigh);
			string str = null;
			str += nSent;
			txtBytesSent.Text = str;
			str = null;
			str += nRecv;
			txtBytesRecv.Text = str;
		}

		private void EnableButtons(bool bEnable)
		{
			btnSleep.Enabled = bEnable;
			btnQueryCount.Enabled = bEnable;
			btnEchoData.Enabled = bEnable;
			btnGetAllCounts.Enabled = bEnable;
			btnQueryGlobalFastCount.Enabled = bEnable;
			btnQueryGlobalCount.Enabled = bEnable;
			btnGetOneItemFromServer.Enabled = bEnable;
			btnGetALotItemsFromServer.Enabled = bEnable;
			btnSendOneItemToServer.Enabled = bEnable;
			btnSendALotItemsToServer.Enabled = bEnable;
		}

		private void PrepareStack(int nSize)
		{
			int n;
			m_Stack.Clear();
			for(n=0; n<nSize; n++)
			{
				CTestItem item = new CTestItem();
				item.m_dt = System.DateTime.Now;
				item.m_lData = n;
				item.m_strUID = m_ClientSocket.GetUSocket().UserID;
				m_Stack.Push(item);
			}
		}

        void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, tagReturnFlag sFlag)
        {
            if (sFlag == tagReturnFlag.rfCompleted)
            {
                UpdateBytes();
            }
        }

		public void OnClosed(int nSocketHandle, int nError)
		{
			EnableButtons(false);
			if (nError != 0)
			{
				MessageBox.Show(m_ClientSocket.GetErrorMsg());
			}
		}

		public void OnBaseRequestProcessed ( System.Int16 sRequestID )
		{
			switch(sRequestID)
			{
				case (short)USOCKETLib.tagBaseRequestID.idSwitchTo:
				{
					string str = null;
					str += m_PerfQuery.Diff(m_lPrev);
					txtTimeRequired.Text = str;
				}
					break;
				case (short)USOCKETLib.tagChatRequestID.idEnter:
                case (short)USOCKETLib.tagChatRequestID.idXEnter:
				{
					string strMsg;
					int nGroup = 0;
					int nPort = 0;
					int nSvsID = 0;
					string strUID = null;
					string strIPAddr = m_ClientSocket.GetUSocket().GetInfo(0, out nGroup, out strUID, out nSvsID, out nPort);
					strMsg = strUID;
					strMsg += "@";
					strMsg += strIPAddr;
					strMsg += ":";
					strMsg += nPort;
					strMsg += " has just joined the group";
					txtMsg.Text = strMsg;
				}
					break;
				case (short)USOCKETLib.tagChatRequestID.idExit:
				{
					string strMsg;
					int nGroup = 0;
					int nPort = 0;
					int nSvsID = 0;
					string strUID = null;
					string strIPAddr = m_ClientSocket.GetUSocket().GetInfo(0, out nGroup, out strUID, out nSvsID, out nPort);
					strMsg = strUID;
					strMsg += "@";
					strMsg += strIPAddr;
					strMsg += ":";
					strMsg += nPort;
					strMsg += " has just exited from the group";
					txtMsg.Text = strMsg;
				}
					break;
                case (short)USOCKETLib.tagChatRequestID.idXSpeak:
				case (short)USOCKETLib.tagChatRequestID.idSpeak:
				{
					int nGroup = 0;
					int nPort = 0;
					int nSvsID = 0;
					string strUID = null;
					string strMsg = (string)m_ClientSocket.GetUSocket().Message;
					string strIPAddr = m_ClientSocket.GetUSocket().GetInfo(0, out nGroup, out strUID, out nSvsID, out nPort);
					strMsg += " from ";
					strMsg += strUID;
					strMsg += "@";
					strMsg += strIPAddr;
					strMsg += ":";
					strMsg += nPort;
					txtMsg.Text = strMsg;
				}
					break;
				default:
					break;
			}
		}

		public void OnConnected(int nSocketHandle, int nError)
		{
			string str = null;
			str += m_PerfQuery.Diff(m_lPrev);
			txtLatency.Text = str;
			if(nError == 0)
			{
				if(m_ClientSocket.GetUSocket().EncryptionMethod != (short)USOCKETLib.tagEncryptionMethod.NoEncryption && m_ClientSocket.GetUSocket().EncryptionMethod != (short)USOCKETLib.tagEncryptionMethod.BlowFish)
				{
                    int nErrorCode;
                    USOCKETLib.IUCert UCert = (USOCKETLib.IUCert)m_ClientSocket.GetUSocket().PeerCertificate;

                    str = UCert.Subject.ToLower();

                    //check certificate subject here

                    //verify certificate chain
                    str = UCert.Verify(out nErrorCode);
                    str = null;

                    //authenticate a remote server before sending password by verifing a certificate
				}
				EnableButtons(true);

				//set user id and password
				m_ClientSocket.SetUID(txtUserID.Text);
				m_ClientSocket.SetPassword(txtPassword.Text);

				//Turn on/off online compressing at client side
				m_ClientSocket.GetUSocket().ZipIsOn = chkZip.Checked;
				
				//Incerasing TCP sending and receiving buffer sizes will help performance 
				//when there is a lot of data transferred if your network bandwidth is over 10 mbps
				m_ClientSocket.GetUSocket().SetSockOpt((int)tagSocketOption.soSndBuf, 116800, (int)tagSocketLevel.slSocket);
				m_ClientSocket.GetUSocket().SetSockOpt((int)tagSocketOption.soRcvBuf, 116800, (int)tagSocketLevel.slSocket);
				
				//can not batch requests when the property Syn is set to true
				m_ClientSocket.BeginBatching();
				
				//switch for the service identified by sidSOneSvs
				m_ClientSocket.SwitchTo(m_MySvsHandler);
				
				//Incerasing TCP sending and receiving buffer sizes will help performance 
				//when there is a lot of data transferred if your network bandwidth is over 10 mbps
				m_ClientSocket.GetUSocket().SetSockOptAtSvr((int)tagSocketOption.soSndBuf, 116800, (int)tagSocketLevel.slSocket);
				m_ClientSocket.GetUSocket().SetSockOptAtSvr((int)tagSocketOption.soRcvBuf, 116800, (int)tagSocketLevel.slSocket);

                int[] groups = { 1 };
				//join the group 1 -- SOne Group
                m_ClientSocket.Push.Enter(groups);

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

			m_lPrev = m_PerfQuery.Now();

//			bSync = true;
			
			//As shown in the following, when calling the function Connect, 
			//set all the last four input parameters with their default data

			//connect to a remote server asynchronously
			m_ClientSocket.Connect(txtHost.Text, Int32.Parse(txtProPort.Text), bSync);
			
			//you are also encouraged to build a connection synchronously
			//put debug points here and inside the function OnConnected
			//to see code execution sequence.
/*			if(bSync)
			{
				Debug.WriteLine("Existing btnConnect_Click");
			}*/
		}

		private void btnDisconnect_Click(object sender, System.EventArgs e)
		{
			//notify the server to shut down a connection gracefully
			m_ClientSocket.Disconnect();
		}

		private void btnSleep_Click(object sender, System.EventArgs e)
		{
            btnSleep.Enabled = false;
			int nSleep = Int32.Parse(txtSleep.Text);
			if(m_ClientSocket.GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
				m_ClientSocket.SwitchTo(m_MySvsHandler);
			string str = null;
			m_lPrev = m_PerfQuery.Now();
            m_MySvsHandler.Sleep(nSleep);
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
            btnSleep.Enabled = m_ClientSocket.IsConnected();
		}

		private void btnQueryCount_Click(object sender, System.EventArgs e)
		{
			if(m_ClientSocket.GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
				m_ClientSocket.SwitchTo(m_MySvsHandler);
			string str = null;
			m_lPrev = m_PerfQuery.Now();
			txtCount.Text = m_MySvsHandler.QueryCount().ToString();
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}

		private void chkFrozen_CheckedChanged(object sender, System.EventArgs e)
		{
			m_ClientSocket.DisableUI(chkFrozen.Checked);
		}

		private void btnQueryGlobalCount_Click(object sender, System.EventArgs e)
		{
			txtQueryGlobalCount.Text = "";
			if(m_ClientSocket.GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
				m_ClientSocket.SwitchTo(m_MySvsHandler);
			string str = null;
			m_lPrev = m_PerfQuery.Now();
			txtQueryGlobalCount.Text = m_MySvsHandler.QueryGlobalCount().ToString();
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}

		private void btnQueryGlobalFastCount_Click(object sender, System.EventArgs e)
		{
			if(m_ClientSocket.GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
				m_ClientSocket.SwitchTo(m_MySvsHandler);
			txtQueryGlobalFastCount.Text = "";

			string str = null;
			m_lPrev = m_PerfQuery.Now();
			txtQueryGlobalFastCount.Text = m_MySvsHandler.QueryGlobalFastCount().ToString();
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}
		
		private void btnEchoData_Click(object sender, System.EventArgs e)
		{
			int nData = 123456;
			string str = null;
			if(m_ClientSocket.GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
				m_ClientSocket.SwitchTo(m_MySvsHandler);
			m_lPrev = m_PerfQuery.Now();
			
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
			
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;

			//put debug breaking point here, and see results
			nData = 0;
		}

		private void btnGetAllCounts_Click(object sender, System.EventArgs e)
		{
			int nCount;
			int nGlobalCount;
			int nGlobalFastCount;

			if(m_ClientSocket.GetCurrentServiceID() != m_MySvsHandler.GetSvsID())
				m_ClientSocket.SwitchTo(m_MySvsHandler);
			
			string str = null;
			m_lPrev = m_PerfQuery.Now();

			m_MySvsHandler.GetAllCounts(out nCount, out nGlobalCount, out nGlobalFastCount);
			
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;

			txtQueryGlobalFastCount.Text = nGlobalFastCount.ToString();
			txtQueryGlobalCount.Text = nGlobalCount.ToString();
			txtCount.Text = nCount.ToString();
		}
		
		private void chkZip_CheckedChanged(object sender, System.EventArgs e)
		{
			m_ClientSocket.GetUSocket().ZipIsOn = chkZip.Checked;
			m_ClientSocket.GetUSocket().TurnOnZipAtSvr(chkZip.Checked);
		}

		private void chkUseSSL_CheckedChanged(object sender, System.EventArgs e)
		{
			m_ClientSocket.GetUSocket().EncryptionMethod = (short)(chkUseSSL.Checked ? USOCKETLib.tagEncryptionMethod.TLSv1 : USOCKETLib.tagEncryptionMethod.NoEncryption);
		}

		private void btnGetOneItemFromServer_Click(object sender, System.EventArgs e)
		{
			if(m_ClientSocket.GetCurrentServiceID() != m_S3Handler.GetSvsID())
			{
				m_ClientSocket.SwitchTo(m_S3Handler);
			}
			string str = null;
			m_lPrev = m_PerfQuery.Now();

            CTestItem Item = m_S3Handler.GetOneItem();

			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}

		private void btnSendOneItemToServer_Click(object sender, System.EventArgs e)
		{
			if(m_ClientSocket.GetCurrentServiceID() != m_S3Handler.GetSvsID())
			{
				m_ClientSocket.SwitchTo((int)USOCKETLib.tagServiceID.sidStartup);
				m_ClientSocket.SetPassword(txtPassword.Text);
				m_ClientSocket.SwitchTo(m_S3Handler);
			}
			string str = null;
			CTestItem item = new CTestItem();
			item.m_lData = 12345678901234;
			item.m_dt = System.DateTime.Now;
			item.m_strUID = m_ClientSocket.GetUSocket().UserID;
			m_lPrev = m_PerfQuery.Now();

			m_S3Handler.SendOneItem(item);
		
			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}

		private void btnGetALotItemsFromServer_Click(object sender, System.EventArgs e)
		{
			if(m_ClientSocket.GetCurrentServiceID() != m_S3Handler.GetSvsID())
			{
				m_ClientSocket.SwitchTo(m_S3Handler);
			}
			string str = null;
			m_lPrev = m_PerfQuery.Now();
			
			m_Stack = m_S3Handler.GetManyItems(Int32.Parse(txtGetALot.Text));

			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}

		private void btnSendALotItemsToServer_Click(object sender, System.EventArgs e)
		{
			PrepareStack(Int32.Parse(txtSendALot.Text));
			if(m_ClientSocket.GetCurrentServiceID() != m_S3Handler.GetSvsID())
			{
				m_ClientSocket.SwitchTo(m_S3Handler);
			}
			string str = null;
			m_lPrev = m_PerfQuery.Now();
			
			m_S3Handler.SendManyItems(m_Stack);

			str += m_PerfQuery.Diff(m_lPrev);
			txtTimeRequired.Text = str;
		}
	}
}
