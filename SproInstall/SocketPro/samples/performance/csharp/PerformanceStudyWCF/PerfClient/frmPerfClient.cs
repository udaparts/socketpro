using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.ServiceModel;
using SocketProAdapter;

namespace PerfClient
{
    public partial class frmPerfClient : Form
    {
        CUPerformanceQuery m_PerfQuery = new CUPerformanceQuery();
        DataTable m_dt;

        public frmPerfClient()
        {
            InitializeComponent();
        }

        private void frmPerfClient_Load(object sender, EventArgs e)
        {
            
        }

        private void btnSQL_Click(object sender, EventArgs e)
        {
            int n;
            MyCallsClient m_myCalls = new MyCallsClient();
            long lStart = m_PerfQuery.Now();
            for (n = 0; n < 100; n++)
            {
                m_dt = m_myCalls.OpenRowset(txtSQL.Text);
            }
            long lDiff = m_PerfQuery.Diff(lStart);
            txtTime.Text = lDiff.ToString();
        }

        private void btnMyEcho_Click(object sender, EventArgs e)
        {
            int n;
            string str;
            MyCallsClient m_myCalls = new MyCallsClient();
            long lStart = m_PerfQuery.Now();
            for (n = 0; n < 10000; n++)
            {
                str = m_myCalls.MyEcho("MyEcho");
            }
            long lDiff = m_PerfQuery.Diff(lStart);
            txtTime.Text = lDiff.ToString();
        }
    }
}