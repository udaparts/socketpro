using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Suntico
{
    namespace Client
    {
        /// <summary>
        /// A class for implementing SocketPro client load balancing/parallel computation through building multiple channels onto remote cloud servers.
        /// The key features are: requests batching, werver messages push in batch, asynchronous and synchronous computation, bi-directional transactions with confirmation,
        /// load balancing with paritial-session sticky and extreme performance. 
        /// The class also provides auto-reconnecting feature through an internal timer after disconnection.
        /// </summary>
        public class CClientPoint : IDisposable
        {
            internal CCloudMessage m_msgCloud;
            private CClientMessage m_msgCustomer;

            private List<CCloudConnectionContext> m_lstCloudServers = new List<CCloudConnectionContext>();

            #region IDisposable Members
            public void Dispose()
            {
                m_timer.Enabled = false;
                m_timer.AutoReset = false;
                m_ClientLoadBalancer.ShutdownPool();
            }
            #endregion

            /// <summary>
            /// An interface for receiving all messages in batch from cloud servers
            /// </summary>
            public ICloudMessage CloudMessage
            {
                get
                {
                    return m_msgCloud;
                }
            }
            /// <summary>
            /// An interface for sending all requests in batch from client to cloud servers.
            /// </summary>
            public ICustomerMessage CustomerMessage
            {
                get
                {
                    return m_msgCustomer;
                }
            }
            /// <summary>
            /// An event for tracking the event that all channels are closed. This event is always raised from SocketPro socket pool threads.
            /// </summary>
            public event DChannelsClosed ChannelsClosed;

            /// <summary>
            ///  An event for verifying server certficate right after secure channels are initially establised. This event is always raised from SocketPro socket pool threads.
            /// </summary>
            public event DDoSslAuthentication DoSslAuthentication;

            /// <summary>
            /// An event for tracking auto fail recovery. This event is always raised from SocketPro socket pool threads. 
            /// </summary>
            public event DFailover Failover;

            /// <summary>
            /// An event for tracking the event that one or more channels to cloud servers are opened. 
            /// The event is raised from calling thread if channels are built through calling the method Start.
            /// The event is raised from an internal timer thread if channels are reconnected through the timer thread.
            /// Note that internal timer is disabled as long as one channel is opened.
            /// </summary>
            public event DChannelsOpened ChannelsOpened;

            private List<CSunticoAsyncHandler> m_lstAsyncHandler = new List<CSunticoAsyncHandler>();

            /// <summary>
            /// A property for cloud servers
            /// </summary>
            public CCloudConnectionContext[] CloudServers
            {
                get
                {
                    return m_lstCloudServers.ToArray();
                }
            }

            private System.Timers.Timer m_timer;

            /// <summary>
            /// Create an instance of Suntico client point
            /// </summary>
            /// <param name="clouds">An array of connection contexts to one or more remote Suntico cloud servers</param>
            /// <param name="TimerInterval">Internal timer interval in ms for reconnecting in case all channels are closed </param>
            public CClientPoint(CCloudConnectionContext[] clouds, int TimerInterval = 3000)
            {
                if (TimerInterval < 500)
                    TimerInterval = 500;
                m_timer = new System.Timers.Timer(TimerInterval);
                m_timer.AutoReset = true;
                m_timer.Elapsed += new System.Timers.ElapsedEventHandler(m_timer_Elapsed);
                m_timer.Enabled = false;
                CSunticoAsyncHandler.m_lstAsyncHandlers = new List<CSunticoAsyncHandler>();
                if (clouds == null || clouds.Length == 0)
                    throw new InvalidOperationException("One cloud server required at least");
                foreach (CCloudConnectionContext s in clouds)
                {
                    m_lstCloudServers.Add(s);
                }
                m_ClientLoadBalancer.m_ClientPoint = this;
                m_msgCustomer = new CClientMessage(this);
                m_ClientLoadBalancer.m_Client = m_msgCustomer;
                m_msgCloud = new CCloudMessage(this);
                InitializeLoadBalancing();
            }

            void m_timer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
            {
                if(ConnectionStatus != ConnectionStatus.Opened)
                    DoConnect();
            }

            internal bool OnFailover(SocketProAdapter.IJobContext JobContext)
            {
                if (Failover != null)
                    Failover.Invoke();
                return true;
            }

            internal void OnAllSocketsDisconnected()
            {
                lock (m_timer)
                {
                    if (ChannelsClosed != null && m_timer.Enabled == false)
                        ChannelsClosed.Invoke();
                    m_timer.Enabled = true;
                }
            }

            private byte GetThreads(out byte ChannelsPerThread)
            {
                byte threads;
                int count = m_lstCloudServers.Count;
                int cores = System.Environment.ProcessorCount;
                if (cores >= m_lstCloudServers.Count)
                {
                    ChannelsPerThread = 1;
                    threads = (byte)m_lstCloudServers.Count;
                }
                else
                {
                    bool r = ((count % cores) > 0);
                    ChannelsPerThread = (byte)(count / cores + (r ? 1 : 0));
                    threads = 0; //use the number of cores
                }
                return threads;
            }

            private CSunticoAsyncHandler SeekHandler(int socket)
            {
                foreach (CSunticoAsyncHandler cs in m_lstAsyncHandler)
                {
                    if (cs.GetAttachedClientSocket().Socket == socket)
                        return cs;
                }
                return null;
            }

            private void InitializeLoadBalancing()
            {
                byte ChannelsPerThread;
                byte threads = GetThreads(out ChannelsPerThread);
                bool ok = m_ClientLoadBalancer.StartSocketPool(ChannelsPerThread, threads);
                List<CSunticoAsyncHandler> lst = CSunticoAsyncHandler.m_lstAsyncHandlers;
                int index = 0;
                foreach (CSunticoAsyncHandler sah in lst)
                {
                    SocketProAdapter.ClientSide.CClientSocket cs = sah.GetAttachedClientSocket();
                    if (cs != null && index < m_lstCloudServers.Count)
                    {
                        cs.m_OnOtherMessage += delegate(int hSocket, int nMsg, int wParam, int lParam)
                        {
                            do
                            {
                                if ((USOCKETLib.tagSSLEvent)wParam != USOCKETLib.tagSSLEvent.ssleHandshakeDone)
                                    break;
                                if (nMsg != (int)USOCKETLib.tagClientMessage.msgSSLEvent)
                                    break;
                                CSunticoAsyncHandler h = SeekHandler(hSocket);
                                if (h == null)
                                    break;
                                if (DoSslAuthentication == null)
                                    break;
                                USOCKETLib.IUCert UCert = (USOCKETLib.IUCert)h.GetAttachedClientSocket().GetUSocket().PeerCertificate;
                                ok = DoSslAuthentication(UCert);
                                if (!ok)
                                    cs.Shutdown();
                            }while(false);
                        };
                        sah.m_CustomerPoint = this;
                        sah.SetChat();
                        m_lstAsyncHandler.Add(sah);
                        m_lstCloudServers[index].AsyncHandler = sah;
                        ++index;
                    }
                }
                CSunticoAsyncHandler.m_lstAsyncHandlers.Clear();
            }

            private void DoConnect()
            {
                byte ChannelsPerThread;
                m_timer.Enabled = false;
                int n, count = m_lstCloudServers.Count;
                SocketProAdapter.CConnectionContext[] ccs = new SocketProAdapter.CConnectionContext[m_lstCloudServers.Count];
                for (n = 0; n < count; ++n)
                {
                    SocketProAdapter.CConnectionContext cc = new SocketProAdapter.CConnectionContext()
                    {
                        m_strUID = m_lstCloudServers[n].UserId,
                        m_strPassword = m_lstCloudServers[n].Password,
                        m_strHost = m_lstCloudServers[n].IpAddress,
                        m_nPort = m_lstCloudServers[n].Port,
                        m_EncrytionMethod = m_lstCloudServers[n].Secure ? USOCKETLib.tagEncryptionMethod.MSTLSv1 : USOCKETLib.tagEncryptionMethod.NoEncryption,
                        m_bZip = m_lstCloudServers[n].Zip
                    };
                    ccs[n] = cc;
                }
                byte threads = GetThreads(out ChannelsPerThread);
                bool ok = m_ClientLoadBalancer.StartSocketPool(ccs, ChannelsPerThread, threads);
                if (ok)
                {
                    byte b = 0;
                    foreach (CCloudConnectionContext ccc in m_lstCloudServers)
                    {
                        if (ccc.Connected)
                        {
                            //use raw USocket directly
                            ccc.AsyncHandler.GetAttachedClientSocket().GetUSocket().SendRequestEx((ushort)Const.idClientFakeSlow, 1, ref b);
                            //make sure that all of requests and server events all processed before using client load balancing!!!!
                            ok = ccc.AsyncHandler.WaitAll();
                        }
                        ccc.m_Backup = (b > 0);
                        ++b;
                    }
                    if (ChannelsOpened != null)
                        ChannelsOpened.Invoke();
                }
                else
                {
                    CSunticoAsyncHandler.m_lstAsyncHandlers.Clear();
                    m_timer.Enabled = true;
                }
            }

            /// <summary>
            /// Start building conenctions to remote cloud servers and client load balancing
            /// </summary>
            /// <returns>Connection status to Suntico cloud. If connection status is not opened, internal timer will be enabled to work for reconnecting.</returns>
            public ConnectionStatus Start()
            {
                if (ConnectionStatus == ConnectionStatus.Opened)
                    return ConnectionStatus.Opened;
                DoConnect();
                return ConnectionStatus;
            }

            /// <summary>
            /// Close channels to Suntico cloud servers. Also, internal timer is disabled to work.
            /// </summary>
            public void Close()
            {
                m_timer.Enabled = false;
                m_ClientLoadBalancer.ShutdownPool();
                InitializeLoadBalancing();
            }

            /// <summary>
            /// Connection status
            /// </summary>
            public ConnectionStatus ConnectionStatus
            {
                get
                {
                    return (m_ClientLoadBalancer.GetUSocketPool().ConnectedSocketsEx > 0) ? ConnectionStatus.Opened : ConnectionStatus.Closed;
                }
            }

            /// <summary>
            /// The number of channels to one or more Sunctico servers. The first one is master channel, and all of others are backups.
            /// </summary>
            public int Channels
            {
                get
                {
                    return m_ClientLoadBalancer.GetUSocketPool().ConnectedSocketsEx;
                }
            }

            /// <summary>
            /// The number of fails
            /// </summary>
            public int Fails
            {
                get
                {
                    return m_ClientLoadBalancer.Fails;
                }
            }

            internal CSunticoClientLoadBalancer m_ClientLoadBalancer = new CSunticoClientLoadBalancer();
        }
    }
}
