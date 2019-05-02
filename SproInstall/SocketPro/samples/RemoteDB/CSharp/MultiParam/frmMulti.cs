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

namespace MultiParam
{
    public partial class frmMulti : Form
    {
        CAsynDBLite m_AsyDBLite = new CAsynDBLite();
        CClientSocket m_ClientSocket = new CClientSocket();

        public frmMulti()
        {
            InitializeComponent();
        }

        private void OnSocketConnected(int hSocket, int nError)
        {
            if (nError == 0)
            {
                m_ClientSocket.SetUID("SocketPro");
                m_ClientSocket.SetPassword("PassOne");
                m_ClientSocket.SwitchTo(m_AsyDBLite);
                m_AsyDBLite.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb");
                m_ClientSocket.WaitAll();
                btnInsert.Enabled = m_AsyDBLite.DBConnected;
            }
        }

        private void OnSocketClosed(int hSocket, int nError)
        {
            btnInsert.Enabled = false;
        }

        private void frmMulti_Load(object sender, EventArgs e)
        {
            m_AsyDBLite.Attach(m_ClientSocket);

            m_ClientSocket.m_OnSocketConnected += new DOnSocketConnected(OnSocketConnected);
            m_ClientSocket.m_OnSocketClosed += new DOnSocketClosed(OnSocketClosed);
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Connect("127.0.0.1", 17001);
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Disconnect();
        }

        private void btnInsert_Click(object sender, EventArgs e)
        {
            List<CParamInfo> lstParamInfo = new List<CParamInfo>();
            lstParamInfo.Add(new CParamInfo());
            lstParamInfo.Add(new CParamInfo());

            lstParamInfo[0].m_sDBType = tagSockDataType.sdVT_WSTR;
            lstParamInfo[0].m_nLen = 255 * 2; //in bytes

            lstParamInfo[1].m_sDBType = tagSockDataType.sdVT_WSTR;
            lstParamInfo[1].m_nLen = 255 * 2; //in bytes

            m_AsyDBLite.DBErrors.Clear();

            //ShipperID ignored because it is an auto-number.
            m_AsyDBLite.OpenCommandWithParameters("Insert into Shippers (CompanyName, Phone) Values (?, ?)", lstParamInfo);

            object[] aData = new object[8];
            aData[0] = "Oracle";
            aData[1] = "(111) 111-1111";

            aData[2] = "Microsoft";
            aData[3] = "(222) 121-1221";

            aData[4] = "Google";
            aData[5] = "(333) 111-1111";

            aData[6] = "Yahoo!";
            aData[7] = "(444) 444-4444";

            //4 sets of parameter data in one batch
            m_AsyDBLite.DoBatch(aData);

            m_AsyDBLite.GetAttachedClientSocket().WaitAll();

            if (m_AsyDBLite.DBErrors.Count != 0)
                MessageBox.Show(m_AsyDBLite.DBErrors[0].m_strErrorMsg);
        }
    }
}