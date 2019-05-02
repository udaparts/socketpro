using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using USOCKETLib;
using UDBLib;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.RemoteDB;

namespace ParameterizedSQL
{
    public partial class frmPSQL : Form
    {
        CAsynDBLiteEx m_AsynDBLite = new CAsynDBLiteEx();
        CClientSocket m_ClientSocket = new CClientSocket();

        public frmPSQL()
        {
            InitializeComponent();
        }

        private void OnSocketConnected(int hSocket, int nError)
        {
            if (nError == 0)
            {
                m_ClientSocket.SetUID("SocketPro");
                m_ClientSocket.SetPassword("PassOne");
                m_ClientSocket.SwitchTo(m_AsynDBLite);
                m_AsynDBLite.ConnectDB("Provider=sqlncli;Data Source=localhost\\sqlexpress;Initial Catalog=northwind;Integrated Security=SSPI");
                m_ClientSocket.WaitAll();
                btnTestPSQL.Enabled = m_AsynDBLite.DBConnected;
            }
        }

        private void OnSocketClosed(int hSocket, int nError)
        {
            btnTestPSQL.Enabled = false;
        }

        private void OutputDataCome(object[] arrayOutput)
        {
            string strOut = "Output data 0 = ";
            strOut += arrayOutput[0].ToString();
            strOut += ", output data 1 = ";
            strOut += arrayOutput[1].ToString();
            MessageBox.Show(strOut);
        }

        private void btnTestPSQL_Click(object sender, EventArgs e)
        {
            string strCreateProcedure = "create Procedure OrderInfoEx @dtOrderDate datetime, @strCustomerID nchar(5), @strRegion nvarchar(15), @nSumEmployeeID int out, @strInfo nchar(255) out " +
                                        "as " +
                                        "select * from Orders where ShipRegion <> @strRegion and OrderDate <> @dtOrderDate and CustomerID<>@strCustomerID and EmployeeID<@nSumEmployeeID " +
                                        "select @nSumEmployeeID=sum(EmployeeID) from Orders " +
                                        "select @strInfo='This is a test from a procedure ' + @strCustomerID";

            btnTestPSQL.Enabled = false;
            m_AsynDBLite.DBErrors.Clear();
            
            m_AsynDBLite.GetAttachedClientSocket().BeginBatching();
            m_AsynDBLite.ExecuteNonQuery("Drop Procedure OrderInfoEx");
            m_AsynDBLite.ExecuteNonQuery(strCreateProcedure);
            
            List<CParamInfo> lstParamInfo = new List<CParamInfo>();
            lstParamInfo.Add(new CParamInfo());
            lstParamInfo.Add(new CParamInfo());
            lstParamInfo.Add(new CParamInfo());
            lstParamInfo.Add(new CParamInfo());
            lstParamInfo.Add(new CParamInfo());

            lstParamInfo[0].m_sDBType = tagSockDataType.sdVT_DATE;

            lstParamInfo[1].m_sDBType = tagSockDataType.sdVT_WSTR;
            lstParamInfo[1].m_nLen = (5 + 1) * 2; //in bytes !!!! 

            lstParamInfo[2].m_sDBType = tagSockDataType.sdVT_WSTR;
            lstParamInfo[2].m_nLen = (15 + 1) * 2;


            lstParamInfo[3].m_sDBType = tagSockDataType.sdVT_I4;
            lstParamInfo[3].m_nParamIO = tagSockDBParamType.sdParamInputOutput;


            lstParamInfo[4].m_sDBType = tagSockDataType.sdVT_WSTR;
            lstParamInfo[4].m_nLen = (1024 + 1) * 2;
            lstParamInfo[4].m_nParamIO = tagSockDBParamType.sdParamOutput;

            m_AsynDBLite.GetAttachedClientSocket().BeginBatching();
            m_AsynDBLite.OpenCommandWithParameters("{CALL OrderInfoEx(?, ?, ?, ?, ?)}", lstParamInfo);

            object[] aData = new object[5];
            aData[0] = DateTime.Now;
            aData[1] = "YZYZ";
            aData[2] = "RG";
            aData[3] = (int)3;
            aData[4] = null;

            m_AsynDBLite.OpenRowsetFromParameters(aData, "OrderInfo", tagCursorType.ctForwardOnly, CAsynDBLite.AsynFetch, 0, -1);

            m_AsynDBLite.GetAttachedClientSocket().Commit(true);
            m_AsynDBLite.GetAttachedClientSocket().WaitAll();
            btnTestPSQL.Enabled = true;
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Connect("localhost", 17001);
        }

        private void frmPSQL_Load(object sender, EventArgs e)
        {
            m_AsynDBLite.Attach(m_ClientSocket);
            m_ClientSocket.m_OnSocketClosed += new DOnSocketClosed(OnSocketClosed);
            m_ClientSocket.m_OnSocketConnected += new DOnSocketConnected(OnSocketConnected);
            m_AsynDBLite.m_OnOutputDataCome += new DOutputDataCome(OutputDataCome);
            m_AsynDBLite.AttachedDataGridView = dgvRowset;
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            m_ClientSocket.Disconnect();
        }
    }
}