using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace SocketProAdapter
{
	namespace ClientSide
	{
        public delegate void DAsyncResultHandler(CAsyncResult AsyncResult);

        public class CAsyncResult
        {
            internal CAsyncResult(CAsyncServiceHandler ash, short sReqId, CUQueue q)
            {
                AsyncServiceHandler = ash;
                RequestId = sReqId;
                UQueue = q;
            }
            public CAsyncServiceHandler AsyncServiceHandler;
            public short RequestId;
            public CUQueue UQueue;
            public DAsyncResultHandler CurrentAsyncResultHandler;
        }

        public interface IAsyncResultsHandler
        {
            void OnExceptionFromServer(CAsyncServiceHandler AsyncServiceHandler, CSocketProServerException Exception);
            void Process(CAsyncResult AsyncResult);
        }

		public class CAsyncServiceHandler : IDisposable
		{
            class CIndexQueue
            {
                public ulong Index;
                public CUQueue UQueue;
            }

		    class CPair
		    {
		        public CPair(short sReqId, DAsyncResultHandler arh, ulong index)
			    {
                    RequestId = sReqId;
                    m_arh = arh;
                    m_nIndex = index;
			    }
			    public short				RequestId;
			    public DAsyncResultHandler	m_arh;
                public ulong m_nIndex;
		    }
            private ulong m_nSyncIndex = 0;
            private DAsyncResultHandler m_ash = new DAsyncResultHandler(Copy);
            //private Dictionary<ulong, CUQueue> m_mapSync = new Dictionary<ulong, CUQueue>();
            List<CIndexQueue> m_mapSync = new List<CIndexQueue>();

            private CUQueue Look(ulong index)
            {
                int n = 0;
                lock (m_cs)
                {
                    foreach (CIndexQueue iq in m_mapSync)
                    {
                        if (iq.Index == index)
                        {
                            m_mapSync.RemoveAt(n);
                            return iq.UQueue;
                        }
                        ++n;
                    }
                    return null;
                }
            }


            private bool PostProcessR5<R0, R1, R2, R3, R4>(short sReqId, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4, ulong index)
            {
                bool b = WaitAll();
                CUQueue q = Look(index);
                if (b && q.GetSize() > 0)
                {
                    q.Load(out r0);
                    q.Load(out r1);
                    q.Load(out r2);
                    q.Load(out r3);
                    q.Load(out r4);
                }
                else
                {
                    r0 = default(R0);
                    r1 = default(R1);
                    r2 = default(R2);
                    r3 = default(R3);
                    r4 = default(R4);
                }
                PutQueueIntoPool(sReqId, q);
                return b;
            }

            private bool PostProcessR4<R0, R1, R2, R3>(short sReqId, out R0 r0, out R1 r1, out R2 r2, out R3 r3, ulong index)
            {
                bool b = WaitAll();
                CUQueue q = Look(index);
                if (b && q.GetSize() > 0)
                {
                    q.Load(out r0);
                    q.Load(out r1);
                    q.Load(out r2);
                    q.Load(out r3);
                }
                else
                {
                    r0 = default(R0);
                    r1 = default(R1);
                    r2 = default(R2);
                    r3 = default(R3);
                }
                PutQueueIntoPool(sReqId, q);
                return b;
            }

            private bool PostProcessR3<R0, R1, R2>(short sReqId, out R0 r0, out R1 r1, out R2 r2, ulong index)
            {
                bool b = WaitAll();
                CUQueue q = Look(index);
                if (b && q.GetSize() > 0)
                {
                    q.Load(out r0);
                    q.Load(out r1);
                    q.Load(out r2);
                }
                else
                {
                    r0 = default(R0);
                    r1 = default(R1);
                    r2 = default(R2);
                }
                PutQueueIntoPool(sReqId, q);
                return b;
            }


            private bool PostProcessR2<R0, R1>(short sReqId, out R0 r0, out R1 r1, ulong index)
            {
                bool b = WaitAll();
                CUQueue q = Look(index);
                if (b && q.GetSize() > 0)
                {
                    q.Load(out r0);
                    q.Load(out r1);
                }
                else
                {
                    r0 = default(R0);
                    r1 = default(R1);
                }
                PutQueueIntoPool(sReqId, q);
                return b;
            }

            private bool PostProcessR1<R0>(short sReqId, out R0 r0, ulong index)
            {
                bool b = WaitAll();
                CUQueue q = Look(index);
                if (b && q.GetSize() > 0)
                    q.Load(out r0);
                else
                    r0 = default(R0);
                PutQueueIntoPool(sReqId, q);
                return b;
            }

            
            private bool PostProcessR0(short sReqId, ulong index)
            {
                bool b = WaitAll();
                CUQueue q = Look(index);
                PutQueueIntoPool(sReqId, q);
                return b;
            }

            private void PutQueueIntoPool(short sRequestID, CUQueue q)
            {
                if (m_ClientSocket.m_OnRequestProcessed != null)
                {
                    int len;
                    if (q == null)
                        len = 0;
                    else
                        len = q.GetSize();
                    m_ClientSocket.m_OnRequestProcessed.Invoke(m_ClientSocket.Socket, sRequestID, len, len, USOCKETLib.tagReturnFlag.rfCompleted);
                }
                CScopeUQueue.Unlock(q);
            }

            public bool ProcessR0(short sReqId)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<R0>(short sReqId, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<R0, R1>(short sReqId, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<R0, R1, R2>(short sReqId, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<R0, R1, R2, R3>(short sReqId, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<R0, R1, R2, R3, R4>(short sReqId, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }

            public bool ProcessR0<T0>(short sReqId, T0 t0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, R0>(short sReqId, T0 t0, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, R0, R1>(short sReqId, T0 t0, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, R0, R1, R2>(short sReqId, T0 t0, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, R0, R1, R2, R3>(short sReqId, T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, R0, R1, R2, R3, R4>(short sReqId, T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }

            public bool ProcessR0<T0, T1>(short sReqId, T0 t0, T1 t1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, R0>(short sReqId, T0 t0, T1 t1, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, R0, R1>(short sReqId, T0 t0, T1 t1, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, R0, R1, R2>(short sReqId, T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }

        //Process
            public bool ProcessR0<T0, T1, T2>(short sReqId, T0 t0, T1 t1, T2 t2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, R0>(short sReqId, T0 t0, T1 t1, T2 t2, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }
        //Process
            public bool ProcessR0<T0, T1, T2, T3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }

        //Process
            public bool ProcessR0<T0, T1, T2, T3, T4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, m_ash))
                     {
                         r0 = default(R0);
                         return false;
                     }
                     index = m_nSyncIndex;
                 }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, m_ash))
                     {
                         r0 = default(R0);
                         r1 = default(R1);
                         return false;
                     }
                     index = m_nSyncIndex;
                 }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, m_ash))
                     {
                         r0 = default(R0);
                         r1 = default(R1);
                         r2 = default(R2);
                         return false;
                     }
                     index = m_nSyncIndex;
                 }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, m_ash))
                     {
                         r0 = default(R0);
                         r1 = default(R1);
                         r2 = default(R2);
                         r3 = default(R3);
                         return false;
                     }
                     index = m_nSyncIndex;
                 }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, m_ash))
                     {
                         r0 = default(R0);
                         r1 = default(R1);
                         r2 = default(R2);
                         r3 = default(R3);
                         r4 = default(R4);
                         return false;
                     }
                     index = m_nSyncIndex;
                 }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }
        //Process
            public bool ProcessR0<T0, T1, T2, T3, T4, T5>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, m_ash))
                         return false;
                     index = m_nSyncIndex;
                 }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0)
            {
                 ulong index;
                 lock (m_cs)
                 {
                     if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, m_ash))
                     {
                         r0 = default(R0);
                         return false;
                     }
                     index = m_nSyncIndex;
                 }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                 }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2)
            {
                 ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                 }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                 }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }    
                    index = m_nSyncIndex;
                 }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }

        //Process
            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, m_ash))
                        return false;    
                    index = m_nSyncIndex;
                 }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }    
                    index = m_nSyncIndex;
                 }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }    
                    index = m_nSyncIndex;
                 }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }    
                    index = m_nSyncIndex;
                 }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }    
                    index = m_nSyncIndex;
                 }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }    
                    index = m_nSyncIndex;
                 }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }
        //Process
            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6, T7>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, m_ash))
                        return false;   
                    index = m_nSyncIndex;
                 }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, T7, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }    
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }    
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }    
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }    
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }
        //Process
            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6, T7, T8>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_ash))
                        return false;    
                    index = m_nSyncIndex;
                 }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                 }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                 }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }

        //Process
            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_ash))
                        return false;
                    index = m_nSyncIndex;
                }
                return PostProcessR0(sReqId, index);
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_ash))
                    {
                        r0 = default(R0);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR1(sReqId, out r0, index);
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR2(sReqId, out r0, out r1, index);
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR3(sReqId, out r0, out r1, out r2, index);
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR4(sReqId, out r0, out r1, out r2, out r3, index);
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3, R4>(short sReqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4)
            {
                ulong index;
                lock (m_cs)
                {
                    if (!SendRequest(sReqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, m_ash))
                    {
                        r0 = default(R0);
                        r1 = default(R1);
                        r2 = default(R2);
                        r3 = default(R3);
                        r4 = default(R4);
                        return false;
                    }
                    index = m_nSyncIndex;
                }
                return PostProcessR5(sReqId, out r0, out r1, out r2, out r3, out r4, index);
            }
        //process

            private object m_cs = new object();
            private List<CPair> m_lstFunc = new List<CPair>();
            private int m_nServiceId;
            private IAsyncResultsHandler m_IAsyncResultsHandler;
            public IAsyncResultsHandler AsyncResultsHandler
            {
                get { lock (m_cs) { return m_IAsyncResultsHandler; } }
                set { lock (m_cs) { m_IAsyncResultsHandler = value; } }
            }

            public int GetSvsID()
            {
                return m_nServiceId;
            }

            public bool BeginBatching()
            {
                return GetAttachedClientSocket().BeginBatching();
            }

            public bool CommitBatch(bool bBatchingAtServer)
            {
                return GetAttachedClientSocket().Commit(bBatchingAtServer);
            }

            public bool CommitBatch()
            {
                return CommitBatch(false);
            }

            public bool RollbackBatch()
            {
                return GetAttachedClientSocket().Rollback();
            }

            public bool WaitAll(int nTimeout)
            {
                return GetAttachedClientSocket().WaitAll(nTimeout);
            }

            public bool WaitAll()
            {
                return WaitAll(-1);
            }

            public bool Wait(short sRequestId, int nTimeout)
            {
                do
                {
                    if (sRequestId != 0)
                        break;
                    object objRequests = GetAttachedClientSocket().GetUSocket().GetRequestsInQueue();
                    short[] sReqIds = objRequests as short[];
                    if (sReqIds != null && sReqIds.Length > 0)
                    {
                        sRequestId = sReqIds[sReqIds.Length - 1];
                        break;
                    }
                    ushort[] usReqIds = objRequests as ushort[];
                    if (usReqIds != null && usReqIds.Length > 0)
                    {
                        sRequestId = (short)(usReqIds[usReqIds.Length - 1]);
                        break;
                    }
                    
                }while(false);

                if (sRequestId == 0)
                    return true;
                return GetAttachedClientSocket().Wait(sRequestId, nTimeout, GetSvsID());
            }

            public bool Wait(short sRequestId)
            {
                return Wait(sRequestId, -1);
            }

            public bool Wait()
            {
                return Wait(0, -1);
            }

            CPair GetAsyncResultsHandler(short sRequestId)
            {
                CPair p = null;
                do
                {
                    if (m_ClientSocket.ReturnRandom)
                    {
                        lock (m_lstFunc)
                        {
                            foreach (CPair pair in m_lstFunc)
                            {
                                if (pair.RequestId == sRequestId)
                                {
                                    p = pair;
                                    m_lstFunc.Remove(pair);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    lock (m_lstFunc)
                    {
                        if (m_lstFunc.Count > 0 && m_lstFunc[0].RequestId == sRequestId)
                        {
                            p = m_lstFunc[0];
                            m_lstFunc.RemoveAt(0);
                        }
                    }
                } while (false);
                return p;
            }

            private ulong OnE(short sRequestId, CSocketProServerException Exception)
            {
                IAsyncResultsHandler pIAsyncResultsHandler = AsyncResultsHandler;
                if(pIAsyncResultsHandler != null)
                    m_IAsyncResultsHandler.OnExceptionFromServer(this, Exception);
                else
                    OnExceptionFromServer(Exception);
                CPair p = GetAsyncResultsHandler(sRequestId);
                if (p == null)
                    return 0;
                return p.m_nIndex;
            }

            internal int RemovePairs(int nMax)
            {
                lock (m_lstFunc)
                {
                    if (nMax > m_lstFunc.Count)
                        nMax = m_lstFunc.Count;
                    m_lstFunc.RemoveRange(m_lstFunc.Count - nMax, nMax);
                }
                return nMax;
            }

            protected virtual void OnExceptionFromServer(CSocketProServerException Exception)
		    {

		    }

            protected virtual void OnResultReturned(short sRequestId, CUQueue UQueue)
            {

            }

            public CAsyncServiceHandler(int nServiceId)
            {
                m_nServiceId = nServiceId;
                m_ClientSocket = null;
                m_IAsyncResultsHandler = null;
            }

            public CAsyncServiceHandler(int nServiceId, CClientSocket cs)
            {
                m_nServiceId = nServiceId;
                if (cs != null)
                    Attach(cs);
                m_ClientSocket = cs;
                m_IAsyncResultsHandler = null;
            }

            static private void Copy(CAsyncResult ar)
            {

            }

            public CAsyncServiceHandler(int nServiceId, CClientSocket cs, IAsyncResultsHandler DefaultAsyncResultsHandler)
            {
                m_nServiceId = nServiceId;
                if (cs != null)
                    Attach(cs);
                m_ClientSocket = cs;
                m_IAsyncResultsHandler = DefaultAsyncResultsHandler;
            }

            internal void OnRR(int hSocket, short sRequestID, int nLen, short sFlag)
            {
                CSocketProServerException exception = null;
                CUQueue UQueue = CScopeUQueue.Lock();
                uint ulSize = (uint)nLen;
                UQueue.SetSize(nLen);
                if (ulSize > 0)
                {
                   
                    uint ulGet =  m_ClientSocket.GetUSocket().GetRtnBufferEx(ulSize, ref UQueue.GetBuffer()[0]);
                }
                if (m_ClientSocket.AutoTransferServerException)
                {
                    /* ###### ENHANCEMENT FROM SOCKETPRO VERSION 4.6.0.1 ###### */
                    //All request methods must return HRESULT at least!!!!!!
                    //If this assert fails here, it means that returned result does not contain HRESULT

                    //keep eyes on how to pop a CSocketProServerException!!!!
                    UQueue.Pop(out exception);
                }
                ulong index;
                if (exception != null && exception.HResult != 0)
                    index = OnE(sRequestID, exception);
                else
                    index = OnRR(sRequestID, UQueue);

                if (index == 0 && m_ClientSocket.m_OnRequestProcessed != null)
                {
                    m_ClientSocket.m_OnRequestProcessed.Invoke(hSocket, sRequestID, UQueue.GetSize(), UQueue.GetSize(), (USOCKETLib.tagReturnFlag)sFlag);
                }
                if (index > 0)
                {
                    lock (m_cs)
                    {
                        CIndexQueue iq = new CIndexQueue();
                        iq.Index = index;
                        iq.UQueue = UQueue;
                        m_mapSync.Add(iq);
                    }
                }
                else
                    CScopeUQueue.Unlock(UQueue);
            }

			private ulong OnRR(short sRequestId, CUQueue UQueue)
			{
                CPair p = GetAsyncResultsHandler(sRequestId);
                do
                {
                    if (p != null)
                    {
                        CAsyncResult ar = new CAsyncResult(this, sRequestId, UQueue);
                        ar.CurrentAsyncResultHandler = p.m_arh;
                        p.m_arh.Invoke(ar);
                        return p.m_nIndex;
                    }
                    IAsyncResultsHandler pIAsyncResultsHandler = AsyncResultsHandler;
                    if (pIAsyncResultsHandler != null)
                    {
                        CAsyncResult ar = new CAsyncResult(this, sRequestId, UQueue);
                        ar.CurrentAsyncResultHandler = pIAsyncResultsHandler.Process;
                        pIAsyncResultsHandler.Process(ar);
                        break;
                    }
                    OnResultReturned(sRequestId, UQueue);
                } while (false);
                return 0;
			}

			public bool Attach(CClientSocket ClientSocket)
			{
                Detach();

				//Your service id must be between 0x10000000 and 0xFFFFFFFF.
				//UDAParts reserves all of service ids from 0 through 0x10000000 - 1.
				if(GetSvsID() < (int)USOCKETLib.tagOtherDefine.odUserServiceIDMin)
				{
					throw new CClientSocketException(CSocketProServerException.E_FAIL, "Service id must be between 0x10000000 and 0xFFFFFFFF");
				}

                if (ClientSocket != null)
                {
                    lock (ClientSocket.m_lstAsyncServiceHandler)
                    {
                        //One ClientSocket can be attached with ONLY ONE CAsyncServiceHandler 
                        //for a given service id and a socket connection.
                        if (ClientSocket.m_lstAsyncServiceHandler.Contains(this))
                            return false;
                        ClientSocket.m_lstAsyncServiceHandler.Add(this);
                    }
                }
				m_ClientSocket = ClientSocket;
				return true;
			}

			public void Detach()
			{
                lock (m_lstFunc)
                {
                    m_lstFunc.Clear();
                }
				if(m_ClientSocket == null)
					return;
				lock(m_ClientSocket.m_lstAsyncServiceHandler)
				{
					m_ClientSocket.m_lstAsyncServiceHandler.Remove(this);
				}
				m_ClientSocket = null;
			}

			public CClientSocket GetAttachedClientSocket()
			{
				return m_ClientSocket;
			}

			public bool SendRequest(short sRequestId, byte []Buffer)
			{
				return SendRequest(sRequestId, Buffer, (DAsyncResultHandler)null);
			}

            public bool SendRequest(short sRequestId, byte[] Buffer, DAsyncResultHandler arh)
            {
                int nLen = 0;
                if (Buffer != null)
                    nLen = Buffer.Length;
                return SendRequest(sRequestId, Buffer, nLen, arh);
            }

            public bool SendRequest(short sRequestId, byte[] Buffer, int nLen)
            {
                return SendRequest(sRequestId, Buffer, nLen, (DAsyncResultHandler)null);
            }
		
			public virtual bool SendRequest(short sRequestId, byte []Buffer, int nLen, DAsyncResultHandler arh)
			{
				//SuppressSocketProServerException();
				if(nLen < 0)
				{
					throw new CClientSocketException(CSocketProServerException.E_FAIL, "Buffer length can't be less than 0!");
				}

				if(m_ClientSocket == null)
				{
					throw new CClientSocketException(CSocketProServerException.E_FAIL, "The handler is not attached with a socket!");
				}

				//Your request id must be between 0x2001 and 0xFFFF.
				//UDAParts reserves all request ids from 0 to 0x2000.
				if((ushort)sRequestId < (uint)USOCKETLib.tagOtherDefine.odUserRequestIDMin)
				{
					//Your request id must be between odUserRequestIDMin and odUserRequestIDMax.
					//UDAParts reserves all request ids from 0 to odUserRequestIDMin - 1.
					throw new CClientSocketException(CSocketProServerException.E_FAIL, "The request id can not be less than odUserRequestIDMin == 0x2000!");
				}

				if(Buffer != null)
				{
					if(nLen > Buffer.Length)
						nLen = Buffer.Length;
				}
				try
				{
					if(Buffer == null)
					{
						byte b = 0;
						m_ClientSocket.m_USocket.SendRequestEx((ushort)sRequestId, 0, ref b);
					}
					else
						m_ClientSocket.m_USocket.SendRequestEx((ushort)sRequestId, (uint)nLen, ref Buffer[0]);
				}
				catch(COMException)
				{
					return false;
				}
                if (arh != null)
                {
                    ulong index;
                    if (m_ash == arh)
                    {
                        lock (m_cs)
                        {
                            index = ++m_nSyncIndex;
                        }
                    }
                    else
                        index = 0;
                    CPair p = new CPair(sRequestId, arh, index);
                    lock (m_lstFunc)
                    {
                        m_lstFunc.Add(p);
                    }
                }
				return true;
			}

            public bool SendRequest(short sRequestId, CScopeUQueue UQueue)
            {
                return SendRequest(sRequestId, UQueue, (DAsyncResultHandler)null);
            }

            public bool SendRequest(short sRequestId, CScopeUQueue UQueue, DAsyncResultHandler arh)
            {
                if (UQueue == null)
                    return SendRequest(sRequestId, null, 0, arh);
                return SendRequest(sRequestId, UQueue.m_UQueue, arh);
            }

			public bool SendRequest(short sRequestId, CUQueue UQueue)
			{
                return SendRequest(sRequestId, UQueue, (DAsyncResultHandler)null);
			}

            public bool SendRequest(short sRequestId, CUQueue UQueue, DAsyncResultHandler arh)
            {
                if (UQueue == null)
                {
                    return SendRequest(sRequestId, null, 0, arh);
                }
                return SendRequest(sRequestId, UQueue.GetBuffer(), UQueue.GetSize(), arh);
            }

            public bool SendRequest(short sRequestId, DAsyncResultHandler arh)
            {
                return SendRequest(sRequestId, null, 0, arh);
            }

            public bool SendRequest(short sRequestId)
            {
                return SendRequest(sRequestId, null, 0, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0>(short sRequestId, T0 data0)
            {
                return SendRequest(sRequestId, data0, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0>(short sRequestId, T0 data0, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1>(short sRequestId, T0 data0, T1 data1)
            {
                return SendRequest(sRequestId, data0, data1, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1>(short sRequestId, T0 data0, T1 data1, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2>(short sRequestId, T0 data0, T1 data1, T2 data2)
            {
                return SendRequest(sRequestId, data0, data1, data2, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2>(short sRequestId, T0 data0, T1 data1, T2 data2, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3, T4>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, data4, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3, T4>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    uqueue.Save<T4>(data4);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, data4, data5, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    uqueue.Save<T4>(data4);
                    uqueue.Save<T5>(data5);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, data4, data5, data6, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    uqueue.Save<T4>(data4);
                    uqueue.Save<T5>(data5);
                    uqueue.Save<T6>(data6);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, data4, data5, data6, data7, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    uqueue.Save<T4>(data4);
                    uqueue.Save<T5>(data5);
                    uqueue.Save<T6>(data6);
                    uqueue.Save<T7>(data7);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, data4, data5, data6, data7, data8, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    uqueue.Save<T4>(data4);
                    uqueue.Save<T5>(data5);
                    uqueue.Save<T6>(data6);
                    uqueue.Save<T7>(data7);
                    uqueue.Save<T8>(data8);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8, T9 data9)
            {
                return SendRequest(sRequestId, data0, data1, data2, data3, data4, data5, data6, data7, data8, data9, (DAsyncResultHandler)null);
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(short sRequestId, T0 data0, T1 data1, T2 data2, T3 data3, T4 data4, T5 data5, T6 data6, T7 data7, T8 data8, T9 data9, DAsyncResultHandler arh)
            {
                using (CScopeUQueue uqueue = new CScopeUQueue())
                {
                    uqueue.Save<T0>(data0);
                    uqueue.Save<T1>(data1);
                    uqueue.Save<T2>(data2);
                    uqueue.Save<T3>(data3);
                    uqueue.Save<T4>(data4);
                    uqueue.Save<T5>(data5);
                    uqueue.Save<T6>(data6);
                    uqueue.Save<T7>(data7);
                    uqueue.Save<T8>(data8);
                    uqueue.Save<T9>(data9);
                    return SendRequest(sRequestId, uqueue.m_UQueue, arh);
                }
            }

			private CClientSocket m_ClientSocket = null;

            #region IDisposable Members

            public void Dispose()
            {
                Detach();
            }

            #endregion
        }
	}//namespace ClientSide
}//SocketProAdapter
