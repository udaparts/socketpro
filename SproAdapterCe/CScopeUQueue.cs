using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace SocketProAdapter
{
    public class CScopeUQueue : IDisposable
    {
        public CScopeUQueue()
        {
            m_UQueue = Lock();
        }

        public static void DestroyUQueuePool()
        {
            lock (m_cs)
            {
                while (m_sQueue.Count > 0)
                {
                    m_sQueue.Pop().Empty();
                }
            }
        }
        public static CUQueue Lock()
        {
            CUQueue UQueue = null;
            lock (m_cs)
            {
                if(m_sQueue.Count > 0)
                    UQueue = m_sQueue.Pop();
            }
            if (UQueue == null)
                UQueue = new CUQueue();
            return UQueue;
        }
        public static void Unlock(CUQueue UQueue)
        {
            if (UQueue != null)
            {
                UQueue.SetSize(0);
                lock (m_cs)
                {
                    m_sQueue.Push(UQueue);
                }
            }
        }

        public virtual int Load<T>(out T data)
        {
            return m_UQueue.Load(out data);
        }

        public virtual void Save<T>(T data)
        {
            m_UQueue.Save(data);
            //if(data is string)
            //{
            //    string str = data as string;
            //    m_UQueue.Save(str);
            //}
            //else if (data is int)
            //{
            //    object obj = data;
            //    m_UQueue.Push((int)obj);
            //}
            //else if (data is bool)
            //{
            //    object obj = data;
            //    m_UQueue.Push((bool)obj);
            //}
            //else if (data is short)
            //{
            //    object obj = data;
            //    m_UQueue.Push((short)obj);
            //}
            //else if (data is DateTime)
            //{
            //    object obj = data;
            //    m_UQueue.Push((DateTime)obj);
            //}
            //else if (data is float)
            //{
            //    object obj = data;
            //    m_UQueue.Push((float)obj);
            //}
            //else if (data is double)
            //{
            //    object obj = data;
            //    m_UQueue.Push((double)obj);
            //}
            //else if (data is decimal)
            //{
            //    object obj = data;
            //    m_UQueue.Push((decimal)obj);
            //}
            //else if (data is byte[])
            //{
            //    object obj = data;
            //    m_UQueue.Push((byte[])obj);
            //}
            //else if (data is long)
            //{
            //    object obj = data;
            //    m_UQueue.Push((long)obj);
            //}
            //else if (data is char)
            //{
            //    object obj = data;
            //    m_UQueue.Push((char)obj);
            //}
            //else if (data is FILETIME)
            //{
            //    object obj = data;
            //    m_UQueue.Push((FILETIME)obj);
            //}
            //else if (data is Guid)
            //{
            //    object obj = data;
            //    m_UQueue.Push((Guid)obj);
            //}
            //else if (data is IUSerializer)
            //{
            //    IUSerializer serializer = data as IUSerializer;
            //    m_UQueue.Push(serializer);
            //}
            //else if (data is object)
            //{
            //    m_UQueue.Push((object)data);
            //}
            //else if (data is sbyte)
            //{
            //    object obj = data;
            //    m_UQueue.Push((sbyte)obj);
            //}
            //else if (data is ulong)
            //{
            //    object obj = data;
            //    m_UQueue.Push((ulong)obj);
            //}
            //else if (data is uint)
            //{
            //    object obj = data;
            //    m_UQueue.Push((uint)obj);
            //}
            //else if (data is ushort)
            //{
            //    object obj = data;
            //    m_UQueue.Push((ushort)obj);
            //}
            //else if (data is byte)
            //{
            //    object obj = data;
            //    m_UQueue.Push((byte)obj);
            //}
            //else
            //{
            //    throw new InvalidOperationException("Unsupported data type");
            //}
        }

        public CUQueue UQueue
        {
            get
            {
                return m_UQueue;
            }
        }

        internal CUQueue m_UQueue;
        private static Stack<CUQueue> m_sQueue = new Stack<CUQueue>();
        private static object m_cs = new object();
        #region IDisposable Members
        void IDisposable.Dispose()
        {
            if (m_UQueue != null)
                Unlock(m_UQueue);
            m_UQueue = null;
        }
        #endregion
    }
}
