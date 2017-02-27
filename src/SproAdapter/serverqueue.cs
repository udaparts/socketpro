using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public interface IServerQueue : IMessageQueueBasic
        {
            uint Handle { get; }
            /// <summary>
            /// Replicate all messages within this queue onto one target queue
            /// </summary>
            /// <param name="serverQueue">A target queue for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, a target queue should be already opened and available</returns>
            bool AppendTo(IServerQueue serverQueue);

            /// <summary>
            /// Replicate all messages within this queue onto an array of queues
            /// </summary>
            /// <param name="serverQueues">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool AppendTo(IServerQueue[] serverQueues);

            /// <summary>
            /// Replicate all messages within this queue onto an array of queues
            /// </summary>
            /// <param name="queueHandles">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool AppendTo(uint[] queueHandles);


            /// <summary>
            /// Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
            /// </summary>
            /// <param name="serverQueue">A target queue for appending messages from this queue </param>
            /// <returns>True for success; and false for fail. To make the call success, a target queue should be already opened and available</returns>
            bool EnsureAppending(IServerQueue serverQueue);

            /// <summary>
            /// Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
            /// </summary>
            /// <param name="serverQueues">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool EnsureAppending(IServerQueue[] serverQueues);

            /// <summary>
            /// Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
            /// </summary>
            /// <param name="queueHandles">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool EnsureAppending(uint[] queueHandles);
        }

        public class CServerQueue : IServerQueue, IDisposable
        {
            private uint m_nHandle = 0;

            ~CServerQueue()
            {
                Clean();
            }

            public CServerQueue()
            {

            }

            public CServerQueue(uint handle)
            {
                m_nHandle = handle;
            }

            void Clean()
            {
                if (m_nHandle != 0)
                {
                    StopQueue(false);
                    m_nHandle = 0;
                }
            }

            public void Dispose()
            {
                Clean();
            }

            #region IServerQueue Members

            public uint Handle
            {
                get
                {
                    return m_nHandle;
                }

                set
                {
                    m_nHandle = value;
                }
            }

            public bool AppendTo(IServerQueue serverQueue)
            {
                if (serverQueue == null)
                    return true;
                uint[] queueHandles = { serverQueue.Handle };
                return AppendTo(queueHandles);
            }

            public bool AppendTo(uint[] queueHandles)
            {
                if (queueHandles == null || queueHandles.Length == 0)
                    return true;
                unsafe
                {
                    fixed (uint* p = queueHandles)
                    {
                        return ServerCoreLoader.PushQueueTo(Handle, p, (uint)queueHandles.Length);
                    }
                }
            }

            public bool AppendTo(IServerQueue[] serverQueues)
            {
                List<uint> targetQueues = new List<uint>();
                if (serverQueues == null || serverQueues.Length == 0)
                    return true;
                foreach (IServerQueue sq in serverQueues)
                {
                    targetQueues.Add(sq.Handle);
                }
                return AppendTo(targetQueues.ToArray());
            }

            public bool EnsureAppending(IServerQueue serverQueue)
            {
                if (serverQueue == null)
                    return true;
                uint[] queueHandles = { serverQueue.Handle };
                return EnsureAppending(queueHandles);
            }

            public bool EnsureAppending(IServerQueue[] serverQueues)
            {
                List<uint> targetQueues = new List<uint>();
                if (serverQueues == null || serverQueues.Length == 0)
                    return true;
                foreach (IServerQueue sq in serverQueues)
                {
                    targetQueues.Add(sq.Handle);
                }
                return EnsureAppending(targetQueues.ToArray());
            }

            public bool EnsureAppending(uint[] queueHandles)
            {
                if (!Available)
                    return false;
                if (QueueStatus != tagQueueStatus.qsMergePushing)
                    return true;
                if (queueHandles == null || queueHandles.Length == 0)
                    return true;
                List<uint> vHandles = new List<uint>();
                foreach (uint h in queueHandles)
                {
                    if (ServerCoreLoader.GetServerQueueStatus(h) != tagQueueStatus.qsMergeComplete)
                        vHandles.Add(h);
                }
                if (vHandles.Count > 0)
                {
                    return AppendTo(vHandles.ToArray());
                }
                Reset();
                return true;
            }

            #endregion

            #region IMessageQueueBasic Members
            public void StopQueue()
            {
                ServerCoreLoader.StopQueueByHandle(m_nHandle, false);
            }

            public void StopQueue(bool permanent)
            {
                ServerCoreLoader.StopQueueByHandle(m_nHandle, permanent);
            }

            public void Reset()
            {
                ServerCoreLoader.ResetQueue(m_nHandle);
            }

            public ulong CancelQueuedMessages(ulong startIndex, ulong endIndex)
            {
                return ServerCoreLoader.CancelQueuedRequestsByIndex(m_nHandle, startIndex, endIndex);
            }

            public uint MessagesInDequeuing
            {
                get
                {
                    return ServerCoreLoader.GetMessagesInDequeuing(m_nHandle);
                }
            }

            public ulong MessageCount
            {
                get
                {
                    return ServerCoreLoader.GetMessageCount(m_nHandle);
                }
            }

            public ulong QueueSize
            {
                get
                {
                    return ServerCoreLoader.GetQueueSize(m_nHandle);
                }
            }

            public bool Available
            {
                get
                {
                    return ServerCoreLoader.IsQueueStartedByHandle(m_nHandle);
                }
            }

            public bool Secure
            {
                get
                {
                    return ServerCoreLoader.IsQueueSecuredByHandle(m_nHandle);
                }
            }

            public string QueueFileName
            {
                get
                {
                    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetQueueFileName(m_nHandle));
                }
            }

            public string QueueName
            {
                get
                {
                    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetQueueName(m_nHandle));
                }
            }

            public ulong LastIndex
            {
                get
                {
                    return ServerCoreLoader.GetQueueLastIndex(m_nHandle);
                }
            }

            public bool AbortJob()
            {
                return ServerCoreLoader.AbortJob(m_nHandle);
            }

            public bool StartJob()
            {
                return ServerCoreLoader.StartJob(m_nHandle);
            }

            public bool EndJob()
            {
                return ServerCoreLoader.EndJob(m_nHandle);
            }

            public ulong JobSize
            {
                get { return ServerCoreLoader.GetJobSize(m_nHandle); }
            }

            public DateTime LastMessageTime
            {
                get
                {
                    ulong seconds = ServerCoreLoader.GetLastQueueMessageTime(m_nHandle);
                    DateTime dt = new DateTime(2013, 1, 1, 1, 0, 0, DateTimeKind.Utc);
                    if (DateTime.Now.IsDaylightSavingTime())
                        dt = dt.AddSeconds(3600);
                    dt = dt.AddSeconds(seconds);
                    return dt;
                }
            }

            public bool DequeueShared
            {
                get
                {
                    return ServerCoreLoader.IsDequeueShared(m_nHandle);
                }
            }

            public ulong RemoveByTTL()
            {
                return ServerCoreLoader.RemoveQueuedRequestsByTTL(m_nHandle);
            }

            public tagQueueStatus QueueStatus
            {
                get
                {
                    return ServerCoreLoader.GetServerQueueStatus(m_nHandle);
                }
            }

            public uint TTL
            {
                get
                {
                    return ServerCoreLoader.GetTTL(m_nHandle);
                }
            }

            public tagOptimistic Optimistic
            {
                get
                {
                    return ServerCoreLoader.GetOptimistic(m_nHandle);
                }
                set
                {
                    ServerCoreLoader.SetOptimistic(m_nHandle, value);
                }
            }

            #endregion
            public virtual ulong Enqueue(ushort reqId, byte[] data, uint size)
            {
                if (data != null && size > (uint)data.Length)
                    size = (uint)data.Length;
                unsafe
                {
                    fixed (byte* buffer = data)
                    {
                        return ServerCoreLoader.Enqueue(m_nHandle, reqId, buffer, size);
                    }
                }
            }

            public virtual ulong Enqueue(ushort reqId, CUQueue q)
            {
                if (q == null || q.GetSize() == 0)
                    return Enqueue(reqId);
                if (q.HeadPosition > 0)
                    return Enqueue(reqId, q.GetBuffer(), q.GetSize());
                return Enqueue(reqId, q.m_bytes, q.GetSize());
            }

            public ulong Enqueue(ushort reqId)
            {
                return Enqueue(reqId, (byte[])null, (uint)0);
            }

            public ulong Enqueue<T0>(ushort reqId, T0 t0)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1>(ushort reqId, T0 t0, T1 t1)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }

            public ulong Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                ulong index = Enqueue(reqId, su);
                CScopeUQueue.Unlock(su);
                return index;
            }
        }
    }
}
