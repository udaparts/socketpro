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

namespace PiLBClient
{
    public partial class frmMain : Form
    {
        private short[] m_requests;
        private double m_dPi;
        private int m_nPercent;
        private CClientSocket m_cs;
        private CPPi m_AsyncPi;
        private const int m_nDivision = 100;
        private const int m_nNum = 10000000;
        
        public frmMain()
        {
            InitializeComponent();
            m_cs = new CClientSocket();
            m_AsyncPi = new CPPi();
            m_AsyncPi.Attach(m_cs);
            m_cs.m_OnRequestProcessed += new DOnRequestProcessed(OnRequestProcessed);
            m_cs.m_OnSocketClosed += new DOnSocketClosed(OnSocketClosed);
            m_cs.m_OnSocketConnected += new DOnSocketConnected(OnSocketConnected);

        }

        private void OnSocketConnected(int hSocket, int nError)
        {
            if (nError == 0)
            {
                btnStart.Enabled = true;
                m_cs.SetUID("SocketPro");
                m_cs.SetPassword("PassOne");
                m_cs.SwitchTo(m_AsyncPi);
            }
        }

        private void OnSocketClosed(int hSocket, int nError)
        {
            string strErrorMsg = m_cs.GetErrorMsg();
            btnStart.Enabled = false;
        }

        private void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, tagReturnFlag ReturnFlag)
        {
            if (sRequestID == piConst.idComputeCPPi)
            {
                m_dPi += m_AsyncPi.GetCompute();
                m_nPercent++;
                txtPi.Text = "Pi = " + m_dPi + " with " + m_nPercent + "%";
                object obj = m_cs.GetUSocket().GetRequestsInQueue();
                m_requests = (short[])obj;
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            m_cs.Connect(txtHost.Text, int.Parse(txtPort.Text));
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            m_cs.Shutdown();
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            int n;
            m_dPi = 0.0;
            m_nPercent = 0;

            double dStep = 1.0 / m_nNum / m_nDivision;
            m_cs.BeginBatching();
            for (n = 0; n < m_nDivision; n++)
            {
                double dStart = (double)n / m_nDivision;
                m_AsyncPi.ComputeAsync(dStart, dStep, m_nNum);
            }
            m_cs.Commit();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            m_cs.Cancel();
        }
    }
}