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

namespace MyRAdoCe
{
    public partial class myado : Form
    {
        CRAdo m_RAdo = new CRAdo();
        CClientSocket m_cs = new CClientSocket();
        public myado()
        {
            InitializeComponent();
            m_RAdo.Attach(m_cs);

            m_cs.m_OnSocketClosed += OnSocketClosed;
            m_cs.m_OnSocketConnected += OnSocketConnected;
            m_cs.m_OnRequestProcessed += OnRequestProcessed;      
        }

        private void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib.tagReturnFlag ReturnFlag)
        {
            if (ReturnFlag != USOCKETLib.tagReturnFlag.rfCompleted)
                return;
            switch (sRequestID)
            {
                case (short)CAsyncAdoSerializationHelper.idDataReaderRecordsArrive:
                case (short)CAsyncAdoSerializationHelper.idDataTableRowsArrive:
                    if (!m_bUpdate)
                    {
                        dgTable.DataSource = m_RAdo.CurrentDataTable.Copy();
                        dgTable.Update(); //redraw data grid to increase UI response
                        m_bUpdate = true;
                    }
                    break;
                default:
                    break;
            }
        }

        private void OnSocketClosed(int hSocket, int nError)
        {
            btnDO.Enabled = false;
        }

        private void OnSocketConnected(int hSocket, int nError)
        {
            if (nError == 0)
            {
                btnDO.Enabled = true;
                m_cs.SetUID("SocketPro");
                m_cs.SetPassword("PassOne");
                m_cs.SwitchTo(m_RAdo);
                if (chkZip.Checked)
                {
                    m_cs.GetUSocket().TurnOnZipAtSvr(true);
                    m_cs.GetUSocket().ZipIsOn = true;
                }
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (chkSSL.Checked)
            {
                m_cs.EncryptionMethod = tagEncryptionMethod.MSTLSv1;
            }
            else
            {
                m_cs.EncryptionMethod = tagEncryptionMethod.NoEncryption;
            }
            m_cs.Connect(txtHost.Text, int.Parse(txtPort.Text));
        }

        bool m_bUpdate = false;
        private void btnDO_Click(object sender, EventArgs e)
        {
            m_bUpdate = false;
            DataTable dt = m_RAdo.GetDataReader(txtSQL.Text);

            //You can use the below code to display result if data record set size is not large.
            //You should consider latency and use delegate OnRequestProcessed instead 
            //if either record set is large or network bandwith is low.

			dgTable.DataSource = dt;
        }

        private void chkZip_CheckStateChanged(object sender, EventArgs e)
        {
            if (m_cs.IsConnected())
            {
                if (chkZip.Checked)
                {
                    m_cs.GetUSocket().TurnOnZipAtSvr(true);
                    m_cs.GetUSocket().ZipIsOn = true;
                }
                else
                {
                    m_cs.GetUSocket().TurnOnZipAtSvr(false);
                    m_cs.GetUSocket().ZipIsOn = false;
                }
            }
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            m_cs.Shutdown();
        }
    }
}