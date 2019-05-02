using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SocketProAdapter;
using DefMyCallsInterface;

namespace PerfClient
{
    public partial class frmPerfClient : Form
    {
        DataTable m_dt;
        CUPerformanceQuery m_PerfQuery = new CUPerformanceQuery();
        IMyCalls m_MyCalls;

        public frmPerfClient()
        {
            InitializeComponent();
        }

        private void frmPerfClient_Load(object sender, EventArgs e)
        {

        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            btnSQL.Enabled = false;
            btnMyEcho.Enabled = false;
            string strHost = txtHost.Text;
            string strDestination = strHost + ":" + Int32.Parse(txtNetPort.Text);
            string strURL = "tcp://" + strDestination + "/MyCalls";
            try
            {
                m_MyCalls = (IMyCalls)Activator.GetObject(typeof(DefMyCallsInterface.CMyCallsImpl), strURL);
            }
            catch (Exception eEx)
            {
                MessageBox.Show(eEx.Message);
                m_MyCalls = null;
                return;
            }
            btnMyEcho.Enabled = true;
            btnSQL.Enabled = true;
        }

        private void btnSQL_Click(object sender, EventArgs e)
        {
            int n;
            long lStart = m_PerfQuery.Now();
            for (n = 0; n < 100; n++)
            {
                try
                {
                     m_dt = m_MyCalls.OpenRowset(txtSQL.Text);
                }
                catch (Exception myError)
                {
                    MessageBox.Show(myError.Message);
                    return;
                }
            }
            long lDiff = m_PerfQuery.Diff(lStart);
            txtTime.Text = lDiff.ToString();
        }

        private void btnMyEcho_Click(object sender, EventArgs e)
        {
            int n;
            string strEcho;
            long lStart = m_PerfQuery.Now();
            for (n = 0; n < 10000; n++)
            {
                strEcho = m_MyCalls.MyEcho("MyEcho");
            }
            long lDiff = m_PerfQuery.Diff(lStart);
            txtTime.Text = lDiff.ToString();
        }
    }
}