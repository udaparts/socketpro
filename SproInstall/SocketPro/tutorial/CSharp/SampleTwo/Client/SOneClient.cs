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

namespace SOneClient
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	/// 
	public class frmSampleOne : System.Windows.Forms.Form, IAsyncResultsHandler
    {
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		private CAsyncServiceHandler		m_MySvsHandler;
        internal TextBox txtMsg;
        internal Label label4;
        internal CheckBox chkUseSSL;
        internal TextBox txtPassword;
        internal TextBox txtUserID;
        internal TextBox txtQueryGlobalFastCount;
        internal TextBox txtQueryGlobalCount;
        internal TextBox txtCount;
        internal TextBox txtSleep;
        internal TextBox txtPort;
        internal TextBox txtHost;
        internal Label label5;
        internal Label label2;
        internal Button btnGetAllCounts;
        internal Button btnEchoData;
        internal Button btnQueryGlobalFastCount;
        internal Button btnQueryGlobalCount;
        internal Button btnQueryCount;
        internal CheckBox chkFrozen;
        internal Button btnSleep;
        internal CheckBox chkZip;
        internal Button btnDisconnect;
        internal Button btnConnect;
        internal Label label3;
        internal Label label1;
        private CClientSocket m_ClientSocket;
		
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
            this.txtMsg = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.chkUseSSL = new System.Windows.Forms.CheckBox();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.txtUserID = new System.Windows.Forms.TextBox();
            this.txtQueryGlobalFastCount = new System.Windows.Forms.TextBox();
            this.txtQueryGlobalCount = new System.Windows.Forms.TextBox();
            this.txtCount = new System.Windows.Forms.TextBox();
            this.txtSleep = new System.Windows.Forms.TextBox();
            this.txtPort = new System.Windows.Forms.TextBox();
            this.txtHost = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.btnGetAllCounts = new System.Windows.Forms.Button();
            this.btnEchoData = new System.Windows.Forms.Button();
            this.btnQueryGlobalFastCount = new System.Windows.Forms.Button();
            this.btnQueryGlobalCount = new System.Windows.Forms.Button();
            this.btnQueryCount = new System.Windows.Forms.Button();
            this.chkFrozen = new System.Windows.Forms.CheckBox();
            this.btnSleep = new System.Windows.Forms.Button();
            this.chkZip = new System.Windows.Forms.CheckBox();
            this.btnDisconnect = new System.Windows.Forms.Button();
            this.btnConnect = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // txtMsg
            // 
            this.txtMsg.Location = new System.Drawing.Point(148, 135);
            this.txtMsg.Name = "txtMsg";
            this.txtMsg.Size = new System.Drawing.Size(368, 20);
            this.txtMsg.TabIndex = 87;
            this.txtMsg.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(156, 114);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(360, 18);
            this.label4.TabIndex = 86;
            this.label4.Text = "Notification Message:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // chkUseSSL
            // 
            this.chkUseSSL.Checked = true;
            this.chkUseSSL.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkUseSSL.Location = new System.Drawing.Point(428, 47);
            this.chkUseSSL.Name = "chkUseSSL";
            this.chkUseSSL.Size = new System.Drawing.Size(72, 16);
            this.chkUseSSL.TabIndex = 85;
            this.chkUseSSL.Text = "Use SSL";
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(276, 39);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.PasswordChar = '*';
            this.txtPassword.Size = new System.Drawing.Size(64, 20);
            this.txtPassword.TabIndex = 84;
            this.txtPassword.Text = "PassOne";
            // 
            // txtUserID
            // 
            this.txtUserID.Location = new System.Drawing.Point(276, 15);
            this.txtUserID.Name = "txtUserID";
            this.txtUserID.Size = new System.Drawing.Size(64, 20);
            this.txtUserID.TabIndex = 83;
            this.txtUserID.Text = "SocketPro";
            // 
            // txtQueryGlobalFastCount
            // 
            this.txtQueryGlobalFastCount.Location = new System.Drawing.Point(244, 247);
            this.txtQueryGlobalFastCount.Name = "txtQueryGlobalFastCount";
            this.txtQueryGlobalFastCount.Size = new System.Drawing.Size(64, 20);
            this.txtQueryGlobalFastCount.TabIndex = 78;
            this.txtQueryGlobalFastCount.Text = "0";
            // 
            // txtQueryGlobalCount
            // 
            this.txtQueryGlobalCount.Location = new System.Drawing.Point(188, 207);
            this.txtQueryGlobalCount.Name = "txtQueryGlobalCount";
            this.txtQueryGlobalCount.Size = new System.Drawing.Size(64, 20);
            this.txtQueryGlobalCount.TabIndex = 76;
            this.txtQueryGlobalCount.Text = "0";
            // 
            // txtCount
            // 
            this.txtCount.Location = new System.Drawing.Point(156, 167);
            this.txtCount.Name = "txtCount";
            this.txtCount.Size = new System.Drawing.Size(64, 20);
            this.txtCount.TabIndex = 74;
            this.txtCount.Text = "0";
            // 
            // txtSleep
            // 
            this.txtSleep.Location = new System.Drawing.Point(100, 87);
            this.txtSleep.Name = "txtSleep";
            this.txtSleep.Size = new System.Drawing.Size(48, 20);
            this.txtSleep.TabIndex = 70;
            this.txtSleep.Text = "5000";
            // 
            // txtPort
            // 
            this.txtPort.Location = new System.Drawing.Point(148, 39);
            this.txtPort.Name = "txtPort";
            this.txtPort.Size = new System.Drawing.Size(48, 20);
            this.txtPort.TabIndex = 69;
            this.txtPort.Text = "20901";
            // 
            // txtHost
            // 
            this.txtHost.Location = new System.Drawing.Point(20, 39);
            this.txtHost.Name = "txtHost";
            this.txtHost.Size = new System.Drawing.Size(120, 20);
            this.txtHost.TabIndex = 68;
            this.txtHost.Text = "localhost";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(204, 39);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(64, 24);
            this.label5.TabIndex = 82;
            this.label5.Text = "Password:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(220, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(48, 16);
            this.label2.TabIndex = 81;
            this.label2.Text = "User ID:";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // btnGetAllCounts
            // 
            this.btnGetAllCounts.Enabled = false;
            this.btnGetAllCounts.Location = new System.Drawing.Point(20, 119);
            this.btnGetAllCounts.Name = "btnGetAllCounts";
            this.btnGetAllCounts.Size = new System.Drawing.Size(120, 32);
            this.btnGetAllCounts.TabIndex = 80;
            this.btnGetAllCounts.Text = "Get All Counts";
            this.btnGetAllCounts.Click += new System.EventHandler(this.btnGetAllCounts_Click);
            // 
            // btnEchoData
            // 
            this.btnEchoData.Enabled = false;
            this.btnEchoData.Location = new System.Drawing.Point(20, 279);
            this.btnEchoData.Name = "btnEchoData";
            this.btnEchoData.Size = new System.Drawing.Size(112, 32);
            this.btnEchoData.TabIndex = 79;
            this.btnEchoData.Text = "Echo Data";
            this.btnEchoData.Click += new System.EventHandler(this.btnEchoData_Click);
            // 
            // btnQueryGlobalFastCount
            // 
            this.btnQueryGlobalFastCount.Enabled = false;
            this.btnQueryGlobalFastCount.Location = new System.Drawing.Point(20, 239);
            this.btnQueryGlobalFastCount.Name = "btnQueryGlobalFastCount";
            this.btnQueryGlobalFastCount.Size = new System.Drawing.Size(216, 32);
            this.btnQueryGlobalFastCount.TabIndex = 77;
            this.btnQueryGlobalFastCount.Text = "QueryGlobalFastCount";
            this.btnQueryGlobalFastCount.Click += new System.EventHandler(this.btnQueryGlobalFastCount_Click);
            // 
            // btnQueryGlobalCount
            // 
            this.btnQueryGlobalCount.Enabled = false;
            this.btnQueryGlobalCount.Location = new System.Drawing.Point(20, 199);
            this.btnQueryGlobalCount.Name = "btnQueryGlobalCount";
            this.btnQueryGlobalCount.Size = new System.Drawing.Size(160, 32);
            this.btnQueryGlobalCount.TabIndex = 75;
            this.btnQueryGlobalCount.Text = "QueryGlobalCount";
            this.btnQueryGlobalCount.Click += new System.EventHandler(this.btnQueryGlobalCount_Click);
            // 
            // btnQueryCount
            // 
            this.btnQueryCount.Enabled = false;
            this.btnQueryCount.Location = new System.Drawing.Point(20, 159);
            this.btnQueryCount.Name = "btnQueryCount";
            this.btnQueryCount.Size = new System.Drawing.Size(128, 32);
            this.btnQueryCount.TabIndex = 73;
            this.btnQueryCount.Text = "QueryCount";
            this.btnQueryCount.Click += new System.EventHandler(this.btnQueryCount_Click);
            // 
            // chkFrozen
            // 
            this.chkFrozen.Location = new System.Drawing.Point(164, 87);
            this.chkFrozen.Name = "chkFrozen";
            this.chkFrozen.Size = new System.Drawing.Size(104, 24);
            this.chkFrozen.TabIndex = 72;
            this.chkFrozen.Text = "GUI Frozen";
            // 
            // btnSleep
            // 
            this.btnSleep.Enabled = false;
            this.btnSleep.Location = new System.Drawing.Point(20, 79);
            this.btnSleep.Name = "btnSleep";
            this.btnSleep.Size = new System.Drawing.Size(72, 32);
            this.btnSleep.TabIndex = 71;
            this.btnSleep.Text = "Sleep";
            this.btnSleep.Click += new System.EventHandler(this.btnSleep_Click);
            // 
            // chkZip
            // 
            this.chkZip.Location = new System.Drawing.Point(348, 47);
            this.chkZip.Name = "chkZip";
            this.chkZip.Size = new System.Drawing.Size(72, 20);
            this.chkZip.TabIndex = 67;
            this.chkZip.Text = "Zip ?";
            this.chkZip.CheckedChanged += new System.EventHandler(this.chkZip_CheckedChanged);
            // 
            // btnDisconnect
            // 
            this.btnDisconnect.Location = new System.Drawing.Point(348, 71);
            this.btnDisconnect.Name = "btnDisconnect";
            this.btnDisconnect.Size = new System.Drawing.Size(168, 24);
            this.btnDisconnect.TabIndex = 66;
            this.btnDisconnect.Text = "Disconnect";
            this.btnDisconnect.Click += new System.EventHandler(this.btnDisconnect_Click);
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(348, 15);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(168, 24);
            this.btnConnect.TabIndex = 65;
            this.btnConnect.Text = "Connect";
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(148, 15);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(40, 16);
            this.label3.TabIndex = 64;
            this.label3.Text = "Port:";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(20, 15);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(132, 16);
            this.label1.TabIndex = 63;
            this.label1.Text = "Host Address:";
            // 
            // frmSampleOne
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(536, 326);
            this.Controls.Add(this.txtMsg);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.chkUseSSL);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.txtUserID);
            this.Controls.Add(this.txtQueryGlobalFastCount);
            this.Controls.Add(this.txtQueryGlobalCount);
            this.Controls.Add(this.txtCount);
            this.Controls.Add(this.txtSleep);
            this.Controls.Add(this.txtPort);
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
            this.Load += new System.EventHandler(this.frmSampleOne_Load);
            this.Closed += new System.EventHandler(this.frmSampleOne_Closed);
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
            m_ClientSocket = new CClientSocket();
            m_MySvsHandler = new CAsyncServiceHandler(TOneConst.sidCTOne, m_ClientSocket, this);
            m_ClientSocket.m_OnSocketClosed += OnClosed;
            m_ClientSocket.m_OnSocketConnected += OnConnected;
            m_ClientSocket.m_OnBaseRequestProcessed += OnBaseRequestProcessed;
		}

		private void frmSampleOne_Closed(object sender, System.EventArgs e)
		{
			//abort socket connection
			m_ClientSocket.Disconnect();
		}

		public void OnClosed(int nSocketHandle, int nError)
		{
			btnSleep.Enabled = false;
			btnQueryCount.Enabled = false;
			btnEchoData.Enabled = false;
			btnGetAllCounts.Enabled = false;
			btnQueryGlobalFastCount.Enabled = false;
			btnQueryGlobalCount.Enabled = false;
			if (nError != 0)
			{
				MessageBox.Show(m_ClientSocket.GetErrorMsg());
			}
		}

		public void OnBaseRequestProcessed ( System.Int16 sRequestID )
		{
			switch(sRequestID)
			{
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
				case (short)USOCKETLib.tagChatRequestID.idSpeak:
                case (short)USOCKETLib.tagChatRequestID.idXSpeak:
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
                case (short)USOCKETLib.tagChatRequestID.idSpeakEx:
                case (short)USOCKETLib.tagChatRequestID.idXSpeakEx:
                    {
                        int nGroup = 0;
                        int nPort = 0;
                        int nSvsID = 0;
                        string strUID = null;
                        System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();
                        string strMsg = utf8.GetString((byte[])m_ClientSocket.GetUSocket().Message);
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
			if(nError == 0)
			{
                short em = m_ClientSocket.GetUSocket().EncryptionMethod;
                if (em != (short)USOCKETLib.tagEncryptionMethod.NoEncryption && em != (short)USOCKETLib.tagEncryptionMethod.BlowFish)
				{
                    int nErrorCode;
					USOCKETLib.IUCert UCert = (USOCKETLib.IUCert)m_ClientSocket.GetUSocket().PeerCertificate;
                    
                    string str = UCert.Subject.ToLower();
                    //check certificate subject here

                    //verify certificate chain
                    str = UCert.Verify(out nErrorCode);
                    str = null;

                    //authenticate a remote server before sending password by verifing a certificate
				}

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
				m_ClientSocket.SwitchTo(m_MySvsHandler, true);
				
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
            m_ClientSocket.GetUSocket().RecvTimeout = -1;
            if (chkUseSSL.Checked)
            {
                m_ClientSocket.GetUSocket().EncryptionMethod = (short)USOCKETLib.tagEncryptionMethod.MSTLSv1;
            }
            else
            {
                m_ClientSocket.GetUSocket().EncryptionMethod = (short)USOCKETLib.tagEncryptionMethod.NoEncryption;
            }
			//connect to a remote server asynchronously
			m_ClientSocket.Connect(txtHost.Text, Int32.Parse(txtPort.Text));
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
            m_MySvsHandler.SendRequest(TOneConst.idSleepCTOne, nSleep);
		}

		private void btnQueryCount_Click(object sender, System.EventArgs e)
		{
            m_MySvsHandler.SendRequest(TOneConst.idQueryCountCTOne, delegate(CAsyncResult ar)
            {
                int nData = 0;
                ar.UQueue.Pop(out nData);
                txtCount.Text = nData.ToString();
            });
		}

		private void chkFrozen_CheckedChanged(object sender, System.EventArgs e)
		{
			m_ClientSocket.DisableUI(chkFrozen.Checked);
		}

		private void btnQueryGlobalCount_Click(object sender, System.EventArgs e)
		{
            m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalCountCTOne, delegate(CAsyncResult ar) {
                int nData = 0;
                ar.UQueue.Pop(out nData);
                txtQueryGlobalCount.Text = nData.ToString();
            });
		}

		private void btnQueryGlobalFastCount_Click(object sender, System.EventArgs e)
		{
            m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalFastCountCTOne, delegate(CAsyncResult ar)
            {
                int nData = 0;
                ar.UQueue.Pop(out nData);
                txtQueryGlobalFastCount.Text = nData.ToString();
            });
		}
		
		private void btnEchoData_Click(object sender, System.EventArgs e)
		{
			int nData = 123456;
			
			//set a rather complicate object data
			object []objA = new object[3];
			object []objChildren = new object[4];
			objA[0] = "This is a test string";
			objA[1] = 123456789;
			objChildren[0] = System.DateTime.Now;
			objChildren[1] = "This is a sub-string";
			objChildren[2] = nData;
			objA[2] = objChildren;

            int []nGroups = {1, 2, 3, 4, 5};

            m_ClientSocket.BeginBatching();
            m_MySvsHandler.SendRequest(TOneConst.idEchoCTOne, (object)objA);
            m_ClientSocket.Push.Broadcast("This is a test from method call Echo ", nGroups);
            m_ClientSocket.Commit(true);
		}

		private void btnGetAllCounts_Click(object sender, System.EventArgs e)
		{
            m_ClientSocket.BeginBatching();
            m_MySvsHandler.SendRequest(TOneConst.idQueryCountCTOne);
            m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalCountCTOne);
            m_MySvsHandler.SendRequest(TOneConst.idQueryGlobalFastCountCTOne);
            m_ClientSocket.Commit(true);
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

        #region IAsyncResultsHandler Members

        public void OnExceptionFromServer(CAsyncServiceHandler AsyncServiceHandler, CSocketProServerException Exception)
        {
            switch (Exception.m_sRequestID)
            {
                case TOneConst.idSleepCTOne:
                    btnSleep.Enabled = true;
                    break;
                default:
                    break;
            }
            MessageBox.Show(Exception.Message);
        }

        public void Process(CAsyncResult AsyncResult)
        {
            object objOut = null;
            int nData = 0;
            switch (AsyncResult.RequestId)
            {
                case TOneConst.idQueryCountCTOne:
                    AsyncResult.UQueue.Load(out nData);
                    txtCount.Text = nData.ToString();
                    break;
                case TOneConst.idQueryGlobalCountCTOne:
                    AsyncResult.UQueue.Load(out nData);
                    txtQueryGlobalCount.Text = nData.ToString();
                    break;
                case TOneConst.idQueryGlobalFastCountCTOne:
                    AsyncResult.UQueue.Load(out nData);
                    txtQueryGlobalFastCount.Text = nData.ToString();
                    break;
                case TOneConst.idEchoCTOne:
                    AsyncResult.UQueue.Load(out objOut);
                    break;
                case TOneConst.idSleepCTOne:
                    btnSleep.Enabled = true;
                    break;
                default:
                    break;
            }
        }

        #endregion
    }
}
