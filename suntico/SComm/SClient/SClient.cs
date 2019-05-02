using System;
using System.Data;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Suntico
{
    namespace Client
    {

        public delegate void DGenericObject(long Clue, SocketProAdapter.CUQueue Queue);

        /// <summary>
        /// A simple structure for building a bi-direction channel between a client and a Suntico cloud server.
        /// </summary>
        public sealed class CCloudConnectionContext
        {
            internal bool m_Backup = false;
            public string IpAddress { get; set; }

            public ushort Port { get; set; }

            /// <summary>
            /// A case-insensitive string for user id
            /// </summary>
            public string UserId { get; set; }

            public string Password { get; set; }

            /// <summary>
            /// True for TLSv1, and false for non-secure transaction between client and Suntico cloud server
            /// </summary>
            public bool Secure { get; set; }

            public bool Zip { get; set; }
            public bool Backup { get { return m_Backup; } }
            public bool Connected { get { return (AsyncHandler.GetAttachedClientSocket().Socket > 0); } }
            internal CSunticoAsyncHandler AsyncHandler;
        }

        /// <summary>
        /// An interface exposing methods for sending different objects from client to cloud servers
        /// Note that all callbacks of all methods are always raised from one of SocketPro socket pool threads.
        /// </summary>
        public interface ICustomerMessage
        {
            CClientPoint ClientPoint { get; }

            /// <summary>
            /// Send a customer defined object which is implemented with the interface IUSerializer
            /// </summary>
            /// <typeparam name="T">A generics object type with constraints IUSerializer and new()</typeparam>
            /// <param name="Clue">A required object type indicator so that Suntico cloud server knows how to reconstruct its object from memory bytes</param>
            /// <param name="obj">A generics object</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            void Send<T>(long Clue, T obj, DRequestCompletedAtCloud p = null) where T : SocketProAdapter.IUSerializer, new();

            // <summary>
            /// Start a transaction with setiing an expected receiving timeout.
            /// </summary>
            /// <param name="Clue">A clue from client to server which will be used by Suntico cloud servers to do special treatment.</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            /// <param name="ReceivingTimeout">An expected timeout in ms which should be not less than 1 second</param>
            /// <returns>Current connection status</returns>
            ConnectionStatus BeginTrans(long Clue = 0, DBeginTransAtCloud p = null, int ReceivingTimeout = 30000);

            /// <summary>
            /// Send a dataset object to a Suntico cloud server with an optional callback
            /// </summary>
            /// <param name="ds">A reference to a dataset object. The reference can be null.</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            void Send(DataSet ds, DRequestCompletedAtCloud p = null);

            /// <summary>
            /// Send a datatable object to a Suntico cloud server with an optional callback
            /// </summary>
            /// <param name="dt">A reference to a datatable object. The reference can be null.</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            void Send(DataTable dt, DRequestCompletedAtCloud p = null);

            /// <summary>
            /// Send a datareader object to a Suntico cloud server with an optional callback
            /// </summary>
            /// <param name="dt">A reference to a datareader object. The reference can be null.</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            void Send(IDataReader dr, DRequestCompletedAtCloud p = null);

            /// <summary>
            /// Commit sending one or an array of objects with tracking transaction event at server side
            /// </summary>
            /// <param name="Clue">An optional hint for special treatment</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            /// <returns>Current connection status</returns>
            ConnectionStatus Commit(long Clue = 0, DCommitAtCloud p = null);

            /// <summary>
            /// Send a string object with object type
            /// </summary>
            /// <param name="str">A string containing JSON, XML or other types of objects</param>
            /// <param name="sot">A required object type indicator used by Suntico cloud servers</param>
            /// <param name="p">An optional callback to track completion event at cloud side</param>
            void Send(string str, StringObjectType sot, DRequestCompletedAtCloud p = null);

            /// <summary>
            /// Rollback or discard batched objects inside memory pool at client side.
            /// </summary>
            void Rollback();

            /// <summary>
            /// Wait until all of batched objects are processed at cloud server side with an expected timeout
            /// </summary>
            /// <param name="timeout">A timeout default to 30 seconds</param>
            /// <returns>True for successful transaction during expected time period, and false for failed</returns>
            bool Wait(int timeout = 30000);
        }

        /// <summary>
        /// A thread-safe class for sending objects from a client to Suntico cloud servers with support of client side load balancing and auto failover recovery.
        /// Note that all of events are originated from cloud server and worker threads different from a calling thread.
        /// </summary>
        class CClientMessage : ICustomerMessage
        {
            public CClientPoint ClientPoint
            {
                get
                {
                    return m_ClientPoint;
                }
            }

            internal void OnJobDone(SocketProAdapter.IJobContext JobContext)
            {
                lock(this)
                {
                    m_sc = null;
                }
            }

            public CClientMessage(CClientPoint p)
            {
                m_ClientPoint = p;
                m_ClientLoadBalancer = p.m_ClientLoadBalancer;
            }

            public ConnectionStatus BeginTrans(long Clue = 0, DBeginTransAtCloud p = null, int ReceivingTimeout = 3000)
            {
                ConnectionStatus cs;
                lock (this)
                {
                    cs = m_ClientPoint.ConnectionStatus;
                    if (cs == ConnectionStatus.Closed)
                        throw new InvalidOperationException("No channels to Suntico cloud servers available yet");
                    if (m_sc != null)
                        throw new InvalidOperationException("Can not start a new transaction without closing pervious one");
                }
                m_ClientLoadBalancer.SetRecvTimeout(ReceivingTimeout);
                CSunticoAsyncHandler sc = (CSunticoAsyncHandler)m_ClientLoadBalancer.JobManager.LockIdentity(ReceivingTimeout);
                if (sc == null || !sc.GetAttachedClientSocket().StartJob())
                {
                    if(sc != null)
                        m_ClientLoadBalancer.JobManager.UnlockIdentity(sc);
                    throw new Exception("All channels are busy!");
                }
                else
                {
                    sc.SendRequest(Const.idClientBeginTrans, Clue,
                        delegate(SocketProAdapter.ClientSide.CAsyncResult ar)
                        {
                            long result;
                            ar.UQueue.Pop(out result);
                            if (p != null)
                                p.Invoke(result);
                        }
                    );
                }
                lock (this)
                {
                    m_sc = sc;
                }
                return cs;
            }

            public void Send<T>(long Clue, T obj, DRequestCompletedAtCloud p = null) where T : SocketProAdapter.IUSerializer, new()
            {
                bool b;
                if (obj == null)
                    throw new InvalidOperationException("Can not send an invalid object");
                lock (this)
                {
                    if (m_sc == null)
                        throw new InvalidOperationException("No transaction is started yet");
                }
                b = m_sc.SendRequest(Const.idClientSendObject, Clue, obj, delegate(SocketProAdapter.ClientSide.CAsyncResult ar)
                    {
                        if (p != null)
                        {
                            p.Invoke();
                        }
                    }
                );
            }

            public void Send(DataSet ds, DRequestCompletedAtCloud p = null)
            {
                lock (this)
                {
                    if (m_sc == null)
                        throw new InvalidOperationException("No transaction is started yet");
                }
                m_sc.Send(p, ds);
            }


            public void Send(DataTable dt, DRequestCompletedAtCloud p = null)
            {
                lock (this)
                {
                    if (m_sc == null)
                        throw new InvalidOperationException("No transaction is started yet");
                }
                m_sc.Send(p, dt);
            }

            public void Send(IDataReader dr, DRequestCompletedAtCloud p = null)
            {
                lock (this)
                {
                    if (m_sc == null)
                        throw new InvalidOperationException("No transaction is started yet");
                }
                m_sc.Send(p, dr);
            }

            public ConnectionStatus Commit(long Clue = 0, DCommitAtCloud p = null)
            {
                lock (this)
                {
                    if (m_sc == null)
                        throw new InvalidOperationException("No transaction is started yet");
                    ConnectionStatus cs = (m_ClientLoadBalancer.GetUSocketPool().ConnectedSocketsEx > 0) ? ConnectionStatus.Opened : ConnectionStatus.Closed;
                    if (cs != ConnectionStatus.Opened)
                    {
                        m_sc = null;
                        return cs;
                    }

                    m_sc.SendRequest(Const.idClientCommit, Clue, delegate(SocketProAdapter.ClientSide.CAsyncResult ar)
                        {
                            long result;
                            ar.UQueue.Pop(out result);
                            if (p != null)
                                p.Invoke(result);
                        }
                    );
                    m_sc.GetAttachedClientSocket().EndJob();
                    m_ClientLoadBalancer.JobManager.UnlockIdentity(m_sc);
                    return cs;
                }
            }

            public void Rollback()
            {
                lock (this)
                {
                    if (m_sc != null)
                    {
                        m_ClientLoadBalancer.JobManager.CancelJobs(m_sc);
                        m_ClientLoadBalancer.JobManager.UnlockIdentity(m_sc);
                        m_sc = null;
                    }
                }
            }

            public void Send(string str, StringObjectType sot, DRequestCompletedAtCloud p = null)
            {
                lock (this)
                {
                    if (m_sc == null)
                        throw new InvalidOperationException("No transaction is started yet");
                }
                m_sc.Send(str, sot, p);
            }

            public bool Wait(int timeout = 30000)
            {
                lock (this)
                {
                    if (m_sc == null)
                        return true;
                }
                return m_ClientLoadBalancer.JobManager.WaitAll(timeout);
            }
            private CSunticoClientLoadBalancer m_ClientLoadBalancer;
            private CSunticoAsyncHandler m_sc = null;
            private CClientPoint m_ClientPoint;
        }
    }
}
