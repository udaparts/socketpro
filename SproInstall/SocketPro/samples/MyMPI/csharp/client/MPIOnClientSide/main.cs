using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using USOCKETLib;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Diagnostics;

namespace PPi
{
    public delegate void DUpdateProgress();
    public partial class main : Form
    {
        USocketPoolClass m_spc;
        private CPiParallel m_NPPi;
        public DUpdateProgress UpdateProgress;
        public main()
        {
            InitializeComponent();
            m_NPPi = new CPiParallel();
            m_NPPi.m_dlg = this;
            UpdateProgress = new DUpdateProgress(OnUpdateControl);

            //use m_spc for your debug
            m_spc = m_NPPi.GetUSocketPool();
        }

        private void OnUpdateControl()
        {
            System.Threading.Thread.Sleep(0);
            string strMsg = "Progress = " + m_NPPi.Progress.ToString() + "%";
            strMsg += (", Parallels = " + m_NPPi.SocketsInParallel.ToString());
            strMsg += (", Fails = " + m_NPPi.Fails.ToString());
            strMsg += (", Paused = " + m_NPPi.Paused.ToString());
            strMsg += (", Working = " + m_NPPi.Working.ToString());
            strMsg += (", Pi = " + m_NPPi.GetPi().ToString());

            txtStatus.Text = strMsg;

            if (m_NPPi.SocketsInParallel == 0)
            {
                btnStart.Text = "Start";
                btnPause.Text = "Pause";
            }
        }

        private void main_FormClosed(object sender, System.Windows.Forms.FormClosedEventArgs e)
        {
            m_NPPi.ShutdownPool();
        }

        private void main_Load(object sender, EventArgs e)
        {
            btnStart.Enabled = m_NPPi.BuildConnections();
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (btnStart.Text == "Start")
            {
                m_NPPi.PrepareAndExecuteJobs();
                if (m_NPPi.Process())
                {
                    btnStart.Text = "Stop";
                    btnPause.Enabled = true;
                }
            }
            else
            {
                m_NPPi.Stop();
                btnStart.Text = "Start";
                btnPause.Text = "Pause";
                btnPause.Enabled = false;
            }
        }

        private void btnPause_Click(object sender, EventArgs e)
        {
            if (btnPause.Text == "Pause")
            {
                m_NPPi.Pause();
                btnPause.Text = "Resume";
            }
            else
            {
                m_NPPi.Resume();
                btnPause.Text = "Pause";
            }
        }
    }

    class CPiParallel : CSocketPoolEx<CPPi>
    {
        private const int m_nDivision = 100;
        public double GetPi()
        {
            lock (m_cs)
            {
                return m_dPi;
            }
        }
        public int Progress
        {
            get
            {
                lock (m_cs)
                {
                    return m_nDivision - JobManager.CountOfJobs;
                }
            }
        }

        protected override bool OnFailover(CPPi pHandler, IJobContext JobContext)
        {
            string str = "JobFail, JobId = " + JobContext.JobId;
            str += ", Progress = " + Progress;
            System.Diagnostics.Trace.WriteLine(str);

            //this is called within a worker thread
            m_dlg.BeginInvoke(m_dlg.UpdateProgress);
            return true;
        }

        protected override void OnJobDone(CPPi Handler, IJobContext JobContext)
        {
            string str = "JobDone, JobId = " + JobContext.JobId;
            str += ", Progress = " + Progress;
            System.Diagnostics.Trace.WriteLine(str);

            m_dlg.BeginInvoke(m_dlg.UpdateProgress);
        }

        protected override void OnReturnedResultProcessed(CPPi pHandler, IJobContext JobContext, short sRequestId)
        {
            if (sRequestId == piConst.idComputeCPPi)
            {
                lock (m_cs)
                {
                    m_dPi += pHandler.m_ComputeRtn;
                }
            }
        }

        public void PrepareAndExecuteJobs()
        {
            int		n;
		    double	dStart;
		    int		nNum = 10000000;
		    double	dStep = 1.0/nNum/m_nDivision;
    		
		    lock (m_cs)
            {
			    //initialize member
			    m_dPi = 0.0;
		    }

            //get an async handler
            CPPi pi = (CPPi)JobManager.LockIdentity();

            if (pi == null)
                return;

            //a job containing one task only
            for (n = 0; n < m_nDivision; n++)
            {
                dStart = (double)n / m_nDivision;
                pi.ComputeAsync(dStart, dStep, nNum);
            }
        
            //a job containing two tasks
            //for (n = 0; n < m_nDivision; n++)
            //{
            //    pi.GetAttachedClientSocket().StartJob();
            //    dStart = (double)n / m_nDivision;
            //    pi.ComputeAsyn(dStart, dStep, nNum);
            //    n += 1;
            //    dStart = (double)n / m_nDivision;
            //    pi.ComputeAsyn(dStart, dStep, nNum);
            //    pi.GetAttachedClientSocket().EndJob();
            //}

            JobManager.UnlockIdentity(pi);
            
		    //manually divide a large task into nDivision sub-tasks
            //int		nTaskId;
            //bool	ok;
            //short sRequestId = piConst.idComputeCPPi;
            //CUQueue	UQueue = new CUQueue();
            //for(n=0; n<m_nDivision; n++)
            //{
            //    dStart = (double)n/m_nDivision;
    
            //    UQueue.Push(dStart);
            //    UQueue.Push(dStep);
            //    UQueue.Push(nNum);

            //    IJobContext jc = JobManager.CreateJob(this);
            //    nTaskId = jc.AddTask(sRequestId, UQueue.GetBuffer(), UQueue.GetSize());
            //    ok = JobManager.EnqueueJob(jc);
            //    Process();

            //    UQueue.SetSize(0);
            //}
        }

        public bool BuildConnections()
        {
            int n;
            const int Count = 5;
            CConnectionContext[] pConnectionContext = new CConnectionContext[Count];
            for (n = 0; n < Count; n++)
                pConnectionContext[n] = new CConnectionContext();

            //set connection contexts
            pConnectionContext[0].m_strHost = "127.0.0.1";
            pConnectionContext[1].m_strHost = "localhost";
            pConnectionContext[2].m_strHost = "127.0.0.1";
            pConnectionContext[3].m_strHost = "localhost";
            pConnectionContext[4].m_strHost = "127.0.0.1";
            for (n = 0; n < Count; n++)
            {
                pConnectionContext[n].m_nPort = 20901;
                pConnectionContext[n].m_strPassword = "SocketPro";
                pConnectionContext[n].m_strUID = "PassOne";
                pConnectionContext[n].m_EncrytionMethod = tagEncryptionMethod.NoEncryption;
                pConnectionContext[n].m_bZip = false;
            }

            //start socket pool with 2*3 USocket objects 
            return StartSocketPool(pConnectionContext, 2, 3);
        }

        private object m_cs = new object();
        public PPi.main m_dlg;

        //protect the following member by monitor
        private double m_dPi = 0.0;
    }
}

