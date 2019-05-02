using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using USOCKETLib;

namespace MySecureClient
{
    public partial class frmMySecure : Form
    {
        private CClientSocket m_ClientSocket = new CClientSocket();
        private CMySecure m_AsynHandler = new CMySecure();

        public frmMySecure()
        {
            InitializeComponent();
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Connect(txtAddress.Text, int.Parse(txtPort.Text));
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Disconnect();
        }

        private void OnClosed(int nSocketHandle, int nError)
        {
            btnToDB.Enabled = false;
            btnExecuteSQL.Enabled = false;
        }

        private void OnConnected(int nSocketHandle, int nError)
        {
            if (nError == 0)
            {
                btnToDB.Enabled = true;

                m_ClientSocket.EncryptionMethod = tagEncryptionMethod.BlowFish;

                //set user id and password
                m_ClientSocket.SetUID(txtUserID.Text);
                m_ClientSocket.SetPassword(txtPassword.Text);

                //switch for the service identified by sidSOneSvs
                m_ClientSocket.SwitchTo(m_AsynHandler);
            }
            else
            {
                MessageBox.Show(m_ClientSocket.GetErrorMsg());
            }
        }
       
        private void frmMySecure_Load(object sender, EventArgs e)
        {
            m_ClientSocket.m_OnSocketClosed += new DOnSocketClosed(OnClosed);
            m_ClientSocket.m_OnSocketConnected += new DOnSocketConnected(OnConnected);
            m_AsynHandler.Attach(m_ClientSocket);
        }

        private void btnToDB_Click(object sender, EventArgs e)
        {
            USocketClass usc = m_ClientSocket.GetUSocket();

            //we like to encrypt data before sending the request to server
            usc.EncryptionMethod = (short)USOCKETLib.tagEncryptionMethod.BlowFish;

            if (m_AsynHandler.Open(txtUserID.Text, txtPassword.Text) == null)
                MessageBox.Show(m_AsynHandler.m_strErrorMessage);
            else
                btnExecuteSQL.Enabled = true;
        }

        private void btnExecuteSQL_Click(object sender, EventArgs e)
        {
            USocketClass usc = m_ClientSocket.GetUSocket();

            //we do not want to encrypt data for the below requests
            usc.EncryptionMethod = (short)USOCKETLib.tagEncryptionMethod.NoEncryption;
            m_ClientSocket.BeginBatching();
            m_AsynHandler.BeginTransAsyn();
            m_AsynHandler.ExecuteNoQueryAsyn(txtSQL.Text);
            m_AsynHandler.CommitAsyn(true);
            m_ClientSocket.Commit(true);
            m_ClientSocket.WaitAll();
        }
    }
}