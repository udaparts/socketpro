using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SocketProAdapter;
using SocketProAdapter.ClientSide;


namespace PerfClient
{
    public partial class frmPerfClient : Form
    {
        CSocketPool<CPerf> m_SocketPool = new CSocketPool<CPerf>();
        System.Diagnostics.Stopwatch m_watch = new System.Diagnostics.Stopwatch();

        public frmPerfClient()
        {
            InitializeComponent();
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (m_SocketPool.ConnectedSockets > 0)
                return;
            m_SocketPool.ShutdownPool();
            CConnectionContext cc = new CConnectionContext(txtHost.Text, uint.Parse(txtPort.Text), "SocketPro", "PassOne");
            if (m_SocketPool.StartSocketPool(cc, 1, 1))
            {
                btnEcho.Enabled = true;
                btnSQL.Enabled = true;
            }
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            m_SocketPool.ShutdownPool();
            btnEcho.Enabled = false;
            btnSQL.Enabled = false;
        }

        private void btnEcho_Click(object sender, EventArgs e)
        {
            string str;
            CPerf perf = m_SocketPool.Seek();
            m_watch.Reset();
            m_watch.Start();
            if (chkAsync.Checked)
            {
                for (int n = 0; n < 10000; n++)
                {
                    perf.SendRequest(perfConst.idMyEchoCPerf, "TestEcho", (ar) =>
                    {
                        ar.Load(out str);
                    });
                }
                perf.WaitAll();
            }
            else
            {
                for (int n = 0; n < 10000; ++n)
                {
                    str = perf.MyEcho("TestEcho");
                }
            }
            m_watch.Stop();
            txtTime.Text = m_watch.ElapsedMilliseconds.ToString();
        }

        private void btnSQL_Click(object sender, EventArgs e)
        {
            DataTable dt;
            CPerf perf = m_SocketPool.Seek();
            m_watch.Reset();
            m_watch.Start();
            if (chkAsync.Checked)
            {
                for (int n = 0; n < 100; n++)
                {
                    perf.SendRequest(perfConst.idOpenRecordsCPerf, txtSQL.Text, (ar) =>
                    {
                        dt = perf.CurrentDataTable;
                    });
                }
                perf.WaitAll();
            }
            else
            {
                for (int n = 0; n < 100; n++)
                {
                    dt = perf.OpenRecords(txtSQL.Text);
                }
            }
            m_watch.Stop();
            txtTime.Text = m_watch.ElapsedMilliseconds.ToString();
        }
    }
}