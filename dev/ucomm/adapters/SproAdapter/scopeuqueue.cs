using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    public class CScopeUQueue : IDisposable
    {
        public CScopeUQueue()
        {
            m_UQueue = Lock(Defines.OperationSystem);
        }

        public static void DestroyUQueuePool()
        {
            lock (m_cs)
            {
                while (m_sQueue.Count > 0)
                {
                    m_sQueue.RemoveAt(m_sQueue.Count - 1);
                }
            }
        }

        public static CUQueue Lock(tagOperationSystem os)
        {
            CUQueue UQueue = null;
            lock (m_cs)
            {
                if (m_sQueue.Count > 0)
                {
                    UQueue = m_sQueue[m_sQueue.Count - 1];
                    m_sQueue.RemoveAt(m_sQueue.Count - 1);
                }
            }
            if (UQueue == null)
                UQueue = new CUQueue();
            UQueue.OS = os;
            return UQueue;
        }

        private static uint m_cleanSize = 32 * 1024;

        public static uint SHARED_BUFFER_CLEAN_SIZE
        {
            get
            {
                lock (m_cs)
                {
                    return m_cleanSize;
                }
            }
            set
            {
                lock (m_cs)
                {
                    if (value < 512)
                        value = 512;
                    m_cleanSize = value;
                }
            }
        }

        public static ulong MemoryConsumed
        {
            get
            {
                ulong mem = 0;
                lock (m_cs)
                {
                    foreach (CUQueue q in m_sQueue)
                    {
                        mem += q.MaxBufferSize;
                    }
                }
                return mem;
            }
        }

        public static CUQueue Lock()
        {
            return Lock(Defines.OperationSystem);
        }

        public static void Unlock(CUQueue UQueue)
        {
            if (UQueue != null)
            {
                UQueue.SetSize(0);
                lock (m_cs)
                {
                    m_sQueue.Add(UQueue);
                }
            }
        }

        public CUQueue Detach()
        {
            CUQueue q = m_UQueue;
            m_UQueue = null;
            return q;
        }

        public void Attach(CUQueue q)
        {
            Unlock(m_UQueue);
            m_UQueue = q;
        }

        public CUQueue Load<T>(out T data)
        {
            return m_UQueue.Load(out data);
        }

        public CUQueue Save<T>(T data)
        {
            return m_UQueue.Save(data);
        }

        public CUQueue UQueue
        {
            get
            {
                return m_UQueue;
            }
        }

        private void CleanUp()
        {
            if (m_UQueue != null)
            {
                Unlock(m_UQueue);
                m_UQueue = null;
            }
        }

        ~CScopeUQueue()
        {
            CleanUp();
        }

        private CUQueue m_UQueue;
        private static List<CUQueue> m_sQueue = new List<CUQueue>();
        private static object m_cs = new object();
        #region IDisposable Members
        void IDisposable.Dispose()
        {
            CleanUp();
        }
        #endregion
    }
}
