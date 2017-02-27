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
#if TASKS_ENABLED
            CUQueue UQueue;
            if (m_sQueue.TryPop(out UQueue))
            {
                UQueue.Empty();
            }
#else
            lock (m_cs)
            {
                while (m_sQueue.Count > 0)
                {
                    m_sQueue.Pop().Empty();
                }
            }
#endif
        }

        public static CUQueue Lock(tagOperationSystem os)
        {
#if TASKS_ENABLED
            CUQueue UQueue;
            if (!m_sQueue.TryPop(out UQueue))
                UQueue = new CUQueue();
#else
            CUQueue UQueue = null;
            lock (m_cs)
            {
                if (m_sQueue.Count > 0)
                {
                    UQueue = m_sQueue.Pop();
                }
            }
            if (UQueue == null)
                UQueue = new CUQueue();
#endif
            UQueue.OS = os;
            return UQueue;
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
#if TASKS_ENABLED
                m_sQueue.Push(UQueue);
#else
                lock (m_cs)
                {
                    m_sQueue.Push(UQueue);
                }
#endif
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
#if TASKS_ENABLED
        private static System.Collections.Concurrent.ConcurrentStack<CUQueue> m_sQueue = new System.Collections.Concurrent.ConcurrentStack<CUQueue>();
#else
        private static Stack<CUQueue> m_sQueue = new Stack<CUQueue>();
        private static object m_cs = new object();
#endif
        #region IDisposable Members
        void IDisposable.Dispose()
        {
            CleanUp();
        }
        #endregion
    }
}
