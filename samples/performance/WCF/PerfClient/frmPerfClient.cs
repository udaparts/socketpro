using System;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Windows.Forms;
using System.ServiceModel;

namespace PerfClient
{
    public partial class frmPerfClient : Form
    {
        System.Diagnostics.Stopwatch watch = new System.Diagnostics.Stopwatch();
        DataTable m_dt;

        public frmPerfClient()
        {
            InitializeComponent();
        }

        private void btnSQL_Click(object sender, EventArgs e)
        {
            int n;
            MyCallsClient m_myCalls = new MyCallsClient();
            watch.Reset();
            watch.Start();
            for (n = 0; n < 100; n++)
            {
                m_dt = m_myCalls.OpenRowset(txtSQL.Text);
            }
            watch.Stop();
            txtTime.Text = watch.ElapsedMilliseconds.ToString();
        }

        private void btnMyEcho_Click(object sender, EventArgs e)
        {
            int n;
            string str;
            MyCallsClient m_myCalls = new MyCallsClient();
            watch.Reset();
            watch.Start();
            for (n = 0; n < 10000; n++)
            {
                str = m_myCalls.MyEcho("MyEcho");
            }
            watch.Stop();
            txtTime.Text = watch.ElapsedMilliseconds.ToString();
        }
    }
}