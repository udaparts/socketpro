using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ServerSide.PLG;
using USOCKETLib;
using System.Runtime.InteropServices;
using System.Collections;
using System.Threading;

namespace SocketPool
{
    delegate void DSocketEvent(int hSocket, int nError);
    delegate bool DPermmitEvent(int hSocket, int lSvsID);
    delegate void DUpdateLBStatus(); 

   
    class CMyPLGPeer : CClientPeer, IPeerJobContext
    {
        protected override void OnFastRequestArrive(short sRequestID, int nLen)
        {
            //intercept data inside m_UQueue, and modify it here if neccessary
        }

        protected override int OnSlowRequestArrive(short sRequestID, int nLen)
        {
            //intercept data inside m_UQueue, and modify it here if neccessary
            return 0;
        }

        protected override void OnChatRequestComing(tagChatRequestID ChatRequestId, object Param0, object Param1)
        {
            switch (ChatRequestId)
            {
                case tagChatRequestID.idEnter:
                case tagChatRequestID.idXEnter:
                    {
                        long lConnected = m_JobManager.SocketPool.ConnectedSocketsEx;
                        string strUserId = UserID;
                        if (lConnected == 0)
                            Push.SendUserMessage(CMyPLGService.NoRealServerAvailable, strUserId);
                        else if (m_JobManager.CountOfJobs >= CMyPLGService.MAX_JOB_QUEUE_SIZE)
                            Push.SendUserMessage(CMyPLGService.ExceedingMaxJobQueueSize, strUserId);
                        else
                            Push.SendUserMessage(CMyPLGService.JobQueueNormal, strUserId);
                    }
                    break;
                default:
                    break;
            }
        }

        private IJobManager m_JobManager;

        #region IPeerJobContext Members

        IJobManager IPeerJobContext.JobManager
        {
            get
            {
                return m_JobManager;
            }
            set
            {
                m_JobManager = value;
            }
        }

        void IPeerJobContext.OnAddingTask(IJobContext JobContext, short sRequestId)
        {
            
        }

        void IPeerJobContext.OnEnqueuingJob(IJobContext JobContext, short sRequestId)
        {
            
        }

        void IPeerJobContext.OnJobEnqueued(IJobContext JobContext, short sRequestId)
        {
            
        }

        void IPeerJobContext.OnJobJustCreated(IJobContext JobContext, short sRequestId)
        {
            
        }

        void IPeerJobContext.OnPeerDataSent(IJobContext JobContext, short sRequestId)
        {
            
        }

        bool IPeerJobContext.OnSendingPeerData(IJobContext JobContext, short sRequestId, CUQueue UQueue)
        {
            //you can modify data inside UQueue here if neccessary

            return true; //true, will send result data in UQueue onto client peer; false, will not
        }

        void IPeerJobContext.OnTaskJustAdded(IJobContext JobContext, int nTaskId, short sRequestId)
        {
            
        }

        void IPeerJobContext.OnWaitable(IJobContext JobContext, int nTaskId, short sRequestId)
        {
            /*
            //you can put a barrier here
            bool b = JobContext.Wait();
            if (b)
            {
                //do your work here
            }*/
        }

        #endregion
    }

    class CMyPLGService : CPLGService<CMyPLGPeer>
    {
        public const int    sidCRAdo = ((int)USOCKETLib.tagOtherDefine.odUserServiceIDMin + 202);
        public const int    MAX_JOB_QUEUE_SIZE = 5;
        public const int	Failover = 11111;
        public const int    ExceedingMaxJobQueueSize = 11112;
        public const int    JobQueueNormal = 11113;
        public const int    NoRealServerAvailable = 11114;

        public frmSocketPool m_frmSocketPool = null;

        protected override void OnAllSocketsDisconnected()
        {
            //just get any one of peer sockets
            int nTotalClients = CSocketProServer.CountOfClients;
            while (nTotalClients > 0)
            {
                nTotalClients--;
                int hSocket = CSocketProServer.GetClient(nTotalClients);
                CClientPeer p = SeekClientPeer(hSocket);
                if (p != null)
                {
                    int[] groups = { 1 };
                    p.Push.Broadcast(CMyPLGService.NoRealServerAvailable, groups);
                    break;
                }
            }
            m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus);
        }

        protected override bool OnFailover(CAsyncServiceHandler Handler, IJobContext JobContext)
        {
            CClientPeer peer = (CClientPeer)JobContext.Identity;

            //send own a fail message
            peer.Push.SendUserMessage(CMyPLGService.Failover, peer.UserID);

            m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus);
            return true; //true, make disaster recovery; false, no disaster recovery
        }

        protected override void OnJobDone(CAsyncServiceHandler Handler, IJobContext JobContext)
        {
            if (JobContext.JobManager.CountOfJobs < (CMyPLGService.MAX_JOB_QUEUE_SIZE -1))
            {
                CClientPeer peer = (CClientPeer)JobContext.Identity;
                int[] groups ={ 1 };
                peer.Push.Broadcast(CMyPLGService.JobQueueNormal, groups);
            }
            m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus); 
        }

        protected override bool OnExecutingJob(CAsyncServiceHandler Handler, IJobContext JobContext)
        {
            if (JobContext.JobManager.CountOfJobs >= CMyPLGService.MAX_JOB_QUEUE_SIZE)
            {
                CClientPeer peer = (CClientPeer)JobContext.Identity;
                int[] groups ={ 1 };
                peer.Push.Broadcast(CMyPLGService.ExceedingMaxJobQueueSize, groups);
            }

            m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnUpdateLBStatus);
            return true;
        }
    }

	public class CPoolSvr : CSocketProServer
	{
		public CPoolSvr()
		{
            CMyPLGService.ServiceIdOnRealServer = CMyPLGService.sidCRAdo;
		}

        private bool IsAllowed(string strUserID, string strPassword)
        {
            if (strPassword != "PassOne")
                return false;
            return (strUserID.ToLower() == "socketpro");
        }

		protected override bool OnIsPermitted(int hSocket, int lSvsID)
		{
			if(m_frmSocketPool != null)
			{
                object[] args = new object[2];
                args[0] = hSocket;
                args[1] = lSvsID;

                string strUID = CSocketProServer.GetUserID(hSocket);

                //password is available ONLY IF authentication method to either amOwn or amMixed
                string strPassword = CSocketProServer.GetPassword(hSocket);

                IAsyncResult ar = m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnIsPermitted, args);
                return IsAllowed(strUID, strPassword);
			}
			return true; 
		}

		protected override void OnClose(int hSocket, int nError)
		{
			if(m_frmSocketPool != null)
			{
                object[] args = new object[2];
                args[0] = hSocket;
                args[1] = nError;
                m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnClose, args);
			}
		}

		protected override void OnAccept(int hSocket, int nError)
		{
			if(m_frmSocketPool != null)
			{
                object []args = new object[2];
                args[0] = hSocket;
                args[1] = nError;
                m_frmSocketPool.BeginInvoke(m_frmSocketPool.m_OnAccept, args);
			}
		}

        protected override bool OnSettingServer()
        {
            //turn off online compressing at the server side so that
            //SocketPro server will not compress return results unless a client turns on it 
            Config.DefaultZip = false;

            //amMixed
            Config.AuthenticationMethod = USOCKETLib.tagAuthenticationMethod.amMixed;

            PushManager.AddAChatGroup(1, "Loading Balance Job Queue Size");

            return AddService();
        }

		public bool IsRunning()
		{
            return m_bRunning;
		}

        bool AddService()
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
            pConnectionContext[3].m_strHost = "charliedev";
            pConnectionContext[4].m_strHost = "charliedev";
            for (n = 0; n < Count; n++)
            {
                pConnectionContext[n].m_nPort = 20901;
                pConnectionContext[n].m_strPassword = "SocketPro";
                pConnectionContext[n].m_strUID = "PassOne";
                pConnectionContext[n].m_EncrytionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption;
                pConnectionContext[n].m_bZip = false;
            }
            if (!m_RadoPoolSvs.SocketPool.StartSocketPool(pConnectionContext, m_bSocketsPerThread, m_bThreadCount))
                return false;
            if (!m_RadoPoolSvs.AddMe(CMyPLGService.sidCRAdo, tagThreadApartment.taFree))
                return false;
            if (!m_RadoPoolSvs.AddSlowRequest((short)USOCKETLib.tagBaseRequestID.idEndJob))
                return false;
            return true;
        }


        int m_nPort = 0;
        byte m_bThreadCount = 0;
        byte m_bSocketsPerThread = 0;
        
		public void Run(int nPort, byte bThreadCount, byte bSocketsPerThread)
		{
            Stop();
            m_RadoPoolSvs.m_frmSocketPool = m_frmSocketPool;
            m_nPort = nPort;
            m_bThreadCount = bThreadCount;
            m_bSocketsPerThread = bSocketsPerThread;
            m_bRunning = Run(nPort);
		}

        public void Stop()
        {
            if (m_bRunning)
            {
                StopSocketProServer();
                m_bRunning = false;
            }
        }

        internal CMyPLGService m_RadoPoolSvs = new CMyPLGService();
		public frmSocketPool m_frmSocketPool = null;
        private bool m_bRunning = false;
	}
}
