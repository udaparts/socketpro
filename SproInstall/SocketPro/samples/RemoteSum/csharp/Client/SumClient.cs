using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using SocketProAdapter.ClientSide;

namespace Client
{
    public partial class SumClient : Form, IAsyncResultsHandler
    {
        public SumClient()
        {
            InitializeComponent();
        }

        private CClientSocket m_cs = new CClientSocket();
        private CAsyncServiceHandler m_ash;
        private void SumClient_Load(object sender, EventArgs e)
        {
            m_ash = new CAsyncServiceHandler(RemoteSumConst.sidRemSum, m_cs, this);
            
            m_cs.m_OnSocketConnected = delegate(int hSocket, int nErrorCode)
            {
                if (nErrorCode == 0)
                {
                    btnDoSum.Enabled = true;
                    btnPause.Enabled = true;
                    btnRedoSum.Enabled = true;
                    m_cs.SwitchTo(m_ash);
                }
                else
                    MessageBox.Show(m_cs.GetErrorMsg());
            };

            m_cs.m_OnSocketClosed = delegate(int hSocket, int nErrorCode)
            {
                btnDoSum.Enabled = false;
                btnPause.Enabled = false;
                btnRedoSum.Enabled = false;
            };

            m_cs.Connect("localhost", 20901);
        }

        #region IAsyncResultsHandler Members

        public void OnExceptionFromServer(CAsyncServiceHandler AsyncServiceHandler, SocketProAdapter.CSocketProServerException Exception)
        {
            throw new Exception("The method or operation is not implemented.");
        }

        public void Process(CAsyncResult AsyncResult)
        {
            switch (AsyncResult.RequestId)
            {
                case RemoteSumConst.idPauseRemSum:
                case RemoteSumConst.idRedoSumRemSum:
                case RemoteSumConst.idDoSumRemSum:
                    {
                        int rtn = 0;
                        AsyncResult.UQueue.Pop(out rtn);
                        txtSum.Text = rtn.ToString();
                    }
                    break;
                case RemoteSumConst.idReportProgress:
                    {
                        int nWhere = 0;
                        int nSum = 0;
                        AsyncResult.UQueue.Pop(out nWhere);
                        AsyncResult.UQueue.Pop(out nSum);
                        txtSum.Text = "Where = " + nWhere.ToString() + ", Sum = " + nSum.ToString();
                    }
                    break;
                default:
                    break;
            }
        }

        #endregion

        private void btnPause_Click(object sender, EventArgs e)
        {
            m_cs.BeginBatching();

            //send a cancel request to a remote server. Here is a big secret from SocketPro!
            m_cs.Cancel(); 

            m_ash.SendRequest(RemoteSumConst.idPauseRemSum);
            m_cs.Commit(true); //make two requests in one shot
        }

        private void btnDoSum_Click(object sender, EventArgs e)
        {
            int start = 100;
            int end = 400;
            m_ash.SendRequest(RemoteSumConst.idDoSumRemSum, start, end);
        }

        private void btnRedoSum_Click(object sender, EventArgs e)
        {
            m_ash.SendRequest(RemoteSumConst.idRedoSumRemSum);
        }
    }
}