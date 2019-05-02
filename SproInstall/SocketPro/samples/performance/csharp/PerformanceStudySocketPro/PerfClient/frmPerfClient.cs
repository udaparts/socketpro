using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.RemoteDB;
using USOCKETLib;


namespace PerfClient
{
    public partial class frmPerfClient : Form
    {
        CSocketPool<CPerf> m_SocketPool = new CSocketPool<CPerf>();
        CPerf m_Perf;
        CUPerformanceQuery m_PerfQuery = new CUPerformanceQuery();
        long m_lStart;
        int m_nIndex;
                            
        public frmPerfClient()
        {
            InitializeComponent();
        }

        delegate void DOnClosed();
        delegate void DOnEchoProcessed();

        private void OnEchoProcessed()
        {
            long lDiff = m_PerfQuery.Diff(m_lStart);
            txtTime.Text = lDiff.ToString();
        }

        private void OnRequestProcessed(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib.tagReturnFlag ReturnFlag)
        {
            switch (sRequestID)
            {
                case PerfConst.idMyEchoCPerf:
                    m_nIndex++;
                    if (m_nIndex < 10000)
                    {
                        m_Perf.MyEchoAsync("TestData");
                    }
                    else
                    {
                        BeginInvoke(new DOnEchoProcessed(OnEchoProcessed));
                    }
                    break;
                default:
                    break;
            }
        }

        private void OnClosed()
        {
            btnEcho.Enabled = false;
            btnSQL.Enabled = false;
        }

        private void OnSocketClosed(int hSocket, int nError)
        {
            BeginInvoke(new DOnClosed(OnClosed));
        }

        private void frmPerfClient_Closing(object sender, System.Windows.Forms.FormClosingEventArgs e)
        {
            m_SocketPool.ShutdownPool();
        }

        private void frmPerfClient_Load(object sender, EventArgs e)
        {
           
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (m_SocketPool.StartSocketPool(txtHost.Text, int.Parse(txtPort.Text), "SocketPro", "PassOne", 1, 1, tagEncryptionMethod.NoEncryption, chkZip.Checked))
            {
                m_Perf = m_SocketPool.Lock();
                m_Perf.GetAttachedClientSocket().m_OnSocketClosed += new DOnSocketClosed(OnSocketClosed);
                if (!chkBatch.Checked)
                    m_Perf.GetAttachedClientSocket().m_OnRequestProcessed += new DOnRequestProcessed(OnRequestProcessed);
                m_SocketPool.Unlock(m_Perf);
                m_Perf.GetAttachedClientSocket().GetUSocket().TurnOnZipAtSvr(chkZip.Checked);
                m_Perf.GetAttachedClientSocket().GetUSocket().ZipIsOn = chkZip.Checked;
                if (radioDefault.Checked)
                {
                    m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(tagZipLevel.zlDefault);
                    m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = tagZipLevel.zlDefault;
                }
                else
                {
                    m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(tagZipLevel.zlBestSpeed);
                    m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = tagZipLevel.zlBestSpeed;
                }
                btnEcho.Enabled = true;
                btnSQL.Enabled = true;
            }
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            m_Perf.Detach();
            m_SocketPool.ShutdownPool();
        }

        private void chkBatch_CheckedChanged(object sender, EventArgs e)
        {
            if (m_Perf == null || m_Perf.GetAttachedClientSocket() == null)
                return;
            if (!m_Perf.GetAttachedClientSocket().IsConnected())
                return;
            if (chkBatch.Checked)
                m_Perf.GetAttachedClientSocket().m_OnRequestProcessed -= new SocketProAdapter.ClientSide.DOnRequestProcessed(OnRequestProcessed);
            else
                m_Perf.GetAttachedClientSocket().m_OnRequestProcessed += new SocketProAdapter.ClientSide.DOnRequestProcessed(OnRequestProcessed);
        }

        private void btnEcho_Click(object sender, EventArgs e)
        {
            m_lStart = m_PerfQuery.Now();
            if (chkBatch.Checked)
            {
                int n;
                int j;
                for (n = 0; n < 40; n++)
                {
                    m_Perf.BeginBatching();
                    for (j = 0; j < 250; j++)
                    {
                        m_Perf.MyEchoAsync("TestData");
                    }
                    m_Perf.CommitBatch(true);
                    m_Perf.WaitAll();
                }
                long lDiff = m_PerfQuery.Diff(m_lStart);
                txtTime.Text = lDiff.ToString();
            }
            else
            {
                m_nIndex = 0;
                m_Perf.MyEcho("TestData");
            }
        }

        private void btnSQL_Click(object sender, EventArgs e)
        {
            int n;
            DataTable dt;
            m_lStart = m_PerfQuery.Now();
            if (chkBatch.Checked)
            {
                for (n = 0; n < 10; n++)
                {
                    int j;
                    m_Perf.BeginBatching();
                    for (j = 0; j < 10; j++)
                    {
                        m_Perf.OpenRecordsAsync(txtSQL.Text);
                    }
                    m_Perf.CommitBatch(true);
                    m_Perf.WaitAll();
                    dt = m_Perf.CurrentDataTable;
                }
            }
            else
            {
                for (n = 0; n < 100; n++)
                {
                    dt = m_Perf.OpenRecords(txtSQL.Text);
                }
            }
            long lDiff = m_PerfQuery.Diff(m_lStart);
            txtTime.Text = lDiff.ToString();
        }

        private void radioRealtime_CheckedChanged(object sender, EventArgs e)
        {
            if (!m_SocketPool.IsStarted() || !m_Perf.GetAttachedClientSocket().IsConnected())
                return;
            m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(tagZipLevel.zlBestSpeed);
            m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = tagZipLevel.zlBestSpeed;
        }

        private void radioDefault_CheckedChanged(object sender, EventArgs e)
        {
            if (!m_SocketPool.IsStarted() || !m_Perf.GetAttachedClientSocket().IsConnected())
                return;
            m_Perf.GetAttachedClientSocket().GetUSocket().SetZipLevelAtSvr(tagZipLevel.zlDefault);
            m_Perf.GetAttachedClientSocket().GetUSocket().zipLevel = tagZipLevel.zlDefault;
        }

        private void chkZip_CheckedChanged(object sender, EventArgs e)
        {
            if (!m_SocketPool.IsStarted() || !m_Perf.GetAttachedClientSocket().IsConnected())
                return;
            m_Perf.GetAttachedClientSocket().GetUSocket().TurnOnZipAtSvr(chkZip.Checked);
            m_Perf.GetAttachedClientSocket().GetUSocket().ZipIsOn = chkZip.Checked;
        }
    }
}