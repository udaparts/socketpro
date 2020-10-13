using System;
using System.Collections.Concurrent;

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
            while (m_sQueue.Count > 0)
            {
                CUQueue q;
                m_sQueue.TryDequeue(out q);
            }
        }

        public static CUQueue Lock(tagOperationSystem os)
        {
            CUQueue UQueue;
            if (!m_sQueue.TryDequeue(out UQueue))
            {
                UQueue = new CUQueue();
            }
            UQueue.OS = os;
            return UQueue;
        }

        private static uint m_cleanSize = 32 * 1024;

        public static uint SHARED_BUFFER_CLEAN_SIZE {
            get {
                return m_cleanSize;
            }
            set {
                if (value < 512)
                    value = 512;
                m_cleanSize = value;
            }
        }

        public static ulong MemoryConsumed {
            get {
                ulong mem = 0;
                foreach (CUQueue q in m_sQueue)
                {
                    mem += q.MaxBufferSize;
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
                m_sQueue.Enqueue(UQueue);
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
            return m_UQueue.Load<T>(out data);
        }

        public CUQueue Save<T>(T data)
        {
            return m_UQueue.Save<T>(data);
        }

        public T Load<T>()
        {
            return m_UQueue.Load<T>();
        }

        public CUQueue UQueue {
            get {
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
        private static ConcurrentQueue<CUQueue> m_sQueue = new ConcurrentQueue<CUQueue>();
        #region IDisposable Members
        public void Dispose()
        {
            CleanUp();
        }
        #endregion
    }
}
