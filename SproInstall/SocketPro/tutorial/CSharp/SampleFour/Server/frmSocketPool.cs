using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using USOCKETLib;
using System.Runtime.InteropServices;

namespace SocketPool
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class frmSocketPool : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnStart;
		private System.Windows.Forms.TextBox txtPort;
		private System.Windows.Forms.ListBox lstInfo;
		private System.Windows.Forms.Label lblCount;
		private System.Windows.Forms.TextBox txtCount;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtLBStatus;

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.TextBox txtThreadCount;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox txtSocketsPerThread;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.TextBox txtConnectedSockets;
        private Button btnStop;
        private Button btnAdd;
		private CPoolSvr		m_PoolSvr;

		public frmSocketPool()
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
            this.label1 = new System.Windows.Forms.Label();
            this.btnStart = new System.Windows.Forms.Button();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.lstInfo = new System.Windows.Forms.ListBox();
            this.lblCount = new System.Windows.Forms.Label();
            this.txtCount = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.txtLBStatus = new System.Windows.Forms.TextBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtConnectedSockets = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.txtSocketsPerThread = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.txtThreadCount = new System.Windows.Forms.TextBox();
            this.btnStop = new System.Windows.Forms.Button();
            this.btnAdd = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(624, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(120, 16);
            this.label1.TabIndex = 0;
            this.label1.Text = "Listening socket port:";
            // 
            // btnStart
            // 
            this.btnStart.Enabled = false;
            this.btnStart.Location = new System.Drawing.Point(624, 40);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(136, 24);
            this.btnStart.TabIndex = 1;
            this.btnStart.Text = "Start server";
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // txtPort
            // 
            this.txtPort.Location = new System.Drawing.Point(624, 16);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(128, 20);
            this.txtPort.TabIndex = 3;
            this.txtPort.Text = "20910";
            // 
            // lstInfo
            // 
            this.lstInfo.Location = new System.Drawing.Point(8, 48);
            this.lstInfo.Name = "lstInfo";
            this.lstInfo.Size = new System.Drawing.Size(608, 264);
            this.lstInfo.TabIndex = 4;
            // 
            // lblCount
            // 
            this.lblCount.Location = new System.Drawing.Point(624, 104);
            this.lblCount.Name = "lblCount";
            this.lblCount.Size = new System.Drawing.Size(120, 16);
            this.lblCount.TabIndex = 5;
            this.lblCount.Text = "Socket Connections:";
            // 
            // txtCount
            // 
            this.txtCount.Location = new System.Drawing.Point(624, 128);
            this.txtCount.Name = "txtCount";
            this.txtCount.Size = new System.Drawing.Size(128, 20);
            this.txtCount.TabIndex = 6;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(8, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(200, 16);
            this.label2.TabIndex = 7;
            this.label2.Text = "RAdo Loading Balance Status:";
            // 
            // txtLBStatus
            // 
            this.txtLBStatus.Location = new System.Drawing.Point(8, 16);
            this.txtLBStatus.Name = "txtLBStatus";
            this.txtLBStatus.ReadOnly = true;
            this.txtLBStatus.Size = new System.Drawing.Size(466, 20);
            this.txtLBStatus.TabIndex = 8;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtConnectedSockets);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.txtSocketsPerThread);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.txtThreadCount);
            this.groupBox1.Location = new System.Drawing.Point(624, 160);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(136, 152);
            this.groupBox1.TabIndex = 9;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Pool Status:";
            // 
            // txtConnectedSockets
            // 
            this.txtConnectedSockets.Location = new System.Drawing.Point(8, 120);
            this.txtConnectedSockets.Name = "txtConnectedSockets";
            this.txtConnectedSockets.ReadOnly = true;
            this.txtConnectedSockets.Size = new System.Drawing.Size(88, 20);
            this.txtConnectedSockets.TabIndex = 5;
            this.txtConnectedSockets.Text = "0";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(8, 104);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(112, 16);
            this.label5.TabIndex = 4;
            this.label5.Text = "Connected Sockets";
            // 
            // txtSocketsPerThread
            // 
            this.txtSocketsPerThread.Location = new System.Drawing.Point(8, 80);
            this.txtSocketsPerThread.Name = "txtSocketsPerThread";
            this.txtSocketsPerThread.Size = new System.Drawing.Size(72, 20);
            this.txtSocketsPerThread.TabIndex = 3;
            this.txtSocketsPerThread.Text = "2";
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(8, 64);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(120, 16);
            this.label4.TabIndex = 2;
            this.label4.Text = "Sockets per thread:";
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 24);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(104, 16);
            this.label3.TabIndex = 1;
            this.label3.Text = "Threads count:";
            // 
            // txtThreadCount
            // 
            this.txtThreadCount.Location = new System.Drawing.Point(8, 40);
            this.txtThreadCount.Name = "txtThreadCount";
            this.txtThreadCount.Size = new System.Drawing.Size(56, 20);
            this.txtThreadCount.TabIndex = 0;
            this.txtThreadCount.Text = "3";
            // 
            // btnStop
            // 
            this.btnStop.Location = new System.Drawing.Point(624, 71);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(136, 23);
            this.btnStop.TabIndex = 10;
            this.btnStop.Text = "Stop";
            this.btnStop.UseVisualStyleBackColor = true;
            this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
            // 
            // btnAdd
            // 
            this.btnAdd.Location = new System.Drawing.Point(480, 16);
            this.btnAdd.Name = "btnAdd";
            this.btnAdd.Size = new System.Drawing.Size(136, 23);
            this.btnAdd.TabIndex = 11;
            this.btnAdd.Text = "Add a real server";
            this.btnAdd.UseVisualStyleBackColor = true;
            this.btnAdd.Click += new System.EventHandler(this.btnAdd_Click);
            // 
            // frmSocketPool
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(776, 333);
            this.Controls.Add(this.btnAdd);
            this.Controls.Add(this.btnStop);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.txtLBStatus);
            this.Controls.Add(this.txtCount);
            this.Controls.Add(this.txtPort);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.lblCount);
            this.Controls.Add(this.lstInfo);
            this.Controls.Add(this.btnStart);
            this.Controls.Add(this.label1);
            this.Name = "frmSocketPool";
            this.Text = "A server side application to demonstrate SocketPro pool";
            this.Load += new System.EventHandler(this.frmSocketPool_Load);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.frmSocketPool_Closed);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
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
			Application.Run(new frmSocketPool());
		}

        internal DPermmitEvent m_OnIsPermitted;
        internal DSocketEvent m_OnClose;
        internal DSocketEvent m_OnAccept;
        internal DUpdateLBStatus m_OnUpdateLBStatus;

        private void InitializeSocketPool()
        {
            m_PoolSvr = new CPoolSvr();
            m_PoolSvr.m_frmSocketPool = this;
            m_OnClose = new DSocketEvent(OnClose);
            m_OnAccept = new DSocketEvent(OnAccept);
            m_OnIsPermitted = new DPermmitEvent(OnIsPermitted);
            m_OnUpdateLBStatus = new DUpdateLBStatus(OnUpdateLBStatus);
        }

        private void OnUpdateLBStatus()
        {
            System.Threading.Thread.Sleep(0);
            string strMsg = ("Parallels = " + m_PoolSvr.m_RadoPoolSvs.SocketPool.SocketsInParallel.ToString());
            strMsg += (", Fails = " + m_PoolSvr.m_RadoPoolSvs.SocketPool.Fails.ToString());
            strMsg += (", Paused = " + m_PoolSvr.m_RadoPoolSvs.SocketPool.Paused.ToString());
            strMsg += (", Working = " + m_PoolSvr.m_RadoPoolSvs.SocketPool.Working.ToString());
            strMsg += (", Queue size = " + m_PoolSvr.m_RadoPoolSvs.SocketPool.JobManager.CountOfJobs.ToString());
            txtLBStatus.Text = strMsg;
            txtConnectedSockets.Text = m_PoolSvr.m_RadoPoolSvs.SocketPool.GetUSocketPool().ConnectedSocketsEx.ToString();
        }

		private void frmSocketPool_Load(object sender, System.EventArgs e)
		{
            InitializeSocketPool();
            btnStart.Enabled = true;
		}

        private void frmSocketPool_Closed(object sender, System.EventArgs e)
        {
            m_PoolSvr.Stop();
        }

		private void btnStart_Click(object sender, System.EventArgs e)
		{
			byte bThreads = (byte)Int32.Parse(txtThreadCount.Text);
			byte bSocketsPerThread = (byte)Int32.Parse(this.txtSocketsPerThread.Text);
			int nPort = Int32.Parse(txtPort.Text);
			string strOLEDBConnection = txtLBStatus.Text;
            m_PoolSvr.Run(nPort, bThreads, bSocketsPerThread);
            if(m_PoolSvr.IsRunning())
			{
				btnStart.Enabled = false;
				string str = null;
                str += m_PoolSvr.m_RadoPoolSvs.SocketPool.GetUSocketPool().ConnectedSocketsEx;
				txtConnectedSockets.Text = str;
			}
            OnUpdateLBStatus();
		}

		[DllImport("usktpror.dll")]
		private static extern bool GetPeerName(uint hSocket, out uint pnPeerPort, [MarshalAs(UnmanagedType.LPWStr)]string strPeerAddr, ushort usChars);
		private string GetPeerName(int hSocket, out int nPort)
		{
			uint uPort = 0;
			string str = new string((char)0, 256);
			bool ok = GetPeerName((uint)hSocket, out uPort, str, 256);
			int nLen = str.IndexOf((char)0, 0, 256);
			if(!ok || nLen == 0)
			{
				nPort = 0;
				return null;
			}
			nPort = (int)uPort;
			str = str.Remove(nLen, 256-nLen);
			return str;
		}

		bool OnIsPermitted(int hSocket, int lSvsID)
		{
			string strUID = CSocketProServer.GetUserID(hSocket);
			
			int nPort = 0;
			string str = GetPeerName(hSocket, out nPort);
			if(str != null && str.Length > 0)
			{
				string strMsg = strUID + " from ";
				strMsg += str;
				strMsg += "@";
				strMsg += nPort;
				lstInfo.Items.Add(strMsg);
				lstInfo.Show();
			}
			
			return true; 
		}

		void OnClose(int hSocket, int nError)
		{
			string str = null;
			System.Threading.Thread.Sleep(0);
			str += CSocketProServer.CountOfClients;
			txtCount.Text = str;

			if(nError != 0)
			{
				int nPort = 0;
				string strIPAddr = GetPeerName(hSocket, out nPort);
				if(str != null && str.Length > 0)
				{
					string strMsg = strIPAddr;
					strMsg += "@";
					strMsg += nPort;
					strMsg += " disconnected because of error code = ";
					strMsg += nError;
					lstInfo.Items.Add(strMsg);
					lstInfo.Show();
				}
			}
		}

		void OnAccept(int hSocket, int nError)
		{
			string str = null;
			str += CSocketProServer.CountOfClients;
			txtCount.Text = str;

			int nPort = 0;
			str = GetPeerName(hSocket, out nPort);
			if(str != null && str.Length > 0)
			{
				string strMsg = "Socket establised  from ";
				strMsg += str;
				strMsg += "@";
				strMsg += nPort;
				lstInfo.Items.Add(strMsg);
				lstInfo.Show();
			}
		}
		
		private void btnStop_Click(object sender, System.EventArgs e)
		{
			m_PoolSvr.Stop();
            m_PoolSvr.Dispose();
            m_PoolSvr = new CPoolSvr();
            m_PoolSvr.m_frmSocketPool = this;
			btnStart.Enabled = true;
            OnUpdateLBStatus();
		}

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (m_PoolSvr.IsRunning())
            {
                CConnectionContext cc = new CConnectionContext();
                cc.m_bZip = false;
                cc.m_nPort = 20901;
                cc.m_strPassword = "PassOne";
                cc.m_strUID = "SocketPro";
                cc.m_strHost = "127.0.0.1";
                cc.m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption;
                if (m_PoolSvr.m_RadoPoolSvs.SocketPool.MakeConnection(cc))
                {
                    OnUpdateLBStatus();
                }
            }
        }
	}
}
