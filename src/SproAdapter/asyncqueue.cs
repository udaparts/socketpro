
using System;
using System.Text;

namespace SocketProAdapter {
    namespace ClientSide {
        /// <summary>
        /// A client side class for easy accessing remote persistent message queues by use of SocketPro communication framework
        /// </summary>
        public class CAsyncQueue : CAsyncServiceHandler {
            public const uint sidQueue = BaseServiceID.sidChat;

            //queue-related request ids
            public const ushort idEnqueue = (ushort)tagBaseRequestID.idReservedTwo + 1;
            public const ushort idDequeue = (ushort)tagBaseRequestID.idReservedTwo + 2;
            public const ushort idStartTrans = (ushort)tagBaseRequestID.idReservedTwo + 3;
            public const ushort idEndTrans = (ushort)tagBaseRequestID.idReservedTwo + 4;
            public const ushort idFlush = (ushort)tagBaseRequestID.idReservedTwo + 5;
            public const ushort idClose = (ushort)tagBaseRequestID.idReservedTwo + 6;
            public const ushort idGetKeys = (ushort)tagBaseRequestID.idReservedTwo + 7;
            public const ushort idEnqueueBatch = (ushort)tagBaseRequestID.idReservedTwo + 8;

            //this id is designed for notifying dequeue batch size from server to client
            public const ushort idBatchSizeNotified = (ushort)tagBaseRequestID.idReservedTwo + 20;

            //error code
            public const int QUEUE_OK = 0;
            public const int QUEUE_TRANS_ALREADY_STARTED = 1;
            public const int QUEUE_TRANS_STARTING_FAILED = 2;
            public const int QUEUE_TRANS_NOT_STARTED_YET = 3;
            public const int QUEUE_TRANS_COMMITTING_FAILED = 4;
            public const int QUEUE_DEQUEUING = 5;
            public const int QUEUE_OTHER_WORKING_WITH_SAME_QUEUE = 6;
            public const int QUEUE_CLOSE_FAILED = 7;
            public const int QUEUE_ENQUEUING_FAILED = 8;

            //callback definitions
            public delegate void DQueueTrans(CAsyncQueue aq, int errCode);
            public delegate void DGetKeys(CAsyncQueue aq, string[] vKey);
            public delegate void DFlush(CAsyncQueue aq, ulong messageCount, ulong fileSize);
            public delegate void DEnqueue(CAsyncQueue aq, ulong indexMessage);
            public delegate void DClose(CAsyncQueue aq, int errCode);
            public delegate void DDequeue(CAsyncQueue aq, ulong messageCount, ulong fileSize, uint messagesDequeuedInBatch, uint bytesDequeuedInBatch);
            public delegate void DMessageQueued(CAsyncQueue aq);

            /// <summary>
            /// An event for tracking message queued notification from server side
            /// </summary>
            public event DMessageQueued MessageQueued;

            private uint m_nBatchSize = 0;
            private object m_csQ = new object();
            private byte[] m_keyDequeue = new byte[0]; //protected by m_csQ
            private DDequeue m_dDequeue = null; //protected by m_csQ

            public CAsyncQueue()
                : base(sidQueue) {

            }

            public CAsyncQueue(uint sid)
                : base(sid) {

            }

            /// <summary>
            /// Dequeue batch size in bytes
            /// </summary>
            public uint DequeueBatchSize {
                get {
                    return (m_nBatchSize & 0xffffff);
                }
            }

            /// <summary>
            /// Check if remote queue server is able to automatically notify a client when a message is enqueued at server side
            /// </summary>
            public bool EnqueueNotified {
                get {
                    return ((m_nBatchSize >> 24) == 0);
                }
            }

            /// <summary>
            /// Last dequeue callback
            /// </summary>
            public DDequeue LastDequeueCallback {
                get {
                    lock (m_csQ) {
                        return m_dDequeue;
                    }
                }
                set {
                    lock (m_csQ) {
                        m_dDequeue = value;
                    }
                }
            }

            private DAsyncResultHandler GetRH(DEnqueue e) {
                DAsyncResultHandler rh = null;
                if (e != null) {
                    rh = (ar) => {
                        ulong index;
                        ar.Load(out index);
                        e((CAsyncQueue)ar.AsyncServiceHandler, index);
                    };
                }
                return rh;
            }

            public static void BatchMessage(ushort idMessage, byte[] message, uint len, CUQueue q) {
                if (message == null) {
                    message = new byte[0];
                    len = 0;
                } else if (len > message.Length) {
                    len = (uint)message.Length;
                }

                if (q.GetSize() == 0) {
                    uint count = 1;
                    q.Save(count);
                } else {
                    unsafe {
                        fixed (byte* p = q.IntenalBuffer) {
                            uint* pN = (uint*)p;
                            *pN += 1;
                        }
                    }
                }
                q.Save(idMessage).Save(len);
                q.Push(message, len);
            }

            public static void BatchMessage(ushort idMessage, byte[] message, CUQueue q) {
                uint len;
                if (message == null) {
                    message = new byte[0];
                    len = 0;
                } else {
                    len = (uint)message.Length;
                }
                BatchMessage(idMessage, message, len, q);
            }

            public static void BatchMessage(ushort idMessage, CUQueue q) {
                BatchMessage(idMessage, (byte[])null, (uint)0, q);
            }

            public static void BatchMessage<T0>(ushort idMessage, T0 t0, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1>(ushort idMessage, T0 t0, T1 t1, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2>(ushort idMessage, T0 t0, T1 t1, T2 t2, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3, T4>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3, T4, T5>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3, T4, T5, T6>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3, T4, T5, T6, T7>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public static void BatchMessage<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, CUQueue q) {
                CUQueue b = CScopeUQueue.Lock();
                b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                BatchMessage(idMessage, b.IntenalBuffer, b.Size, q);
                CScopeUQueue.Unlock(b);
            }

            public bool EnqueueBatch(byte[] key, CUQueue q) {
                return EnqueueBatch(key, q, null);
            }

            public bool EnqueueBatch(byte[] key, CUQueue q, DEnqueue e) {
                return EnqueueBatch(key, q, e, null);
            }

            public virtual bool EnqueueBatch(byte[] key, CUQueue q, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                if (q == null || q.GetSize() < 2 * sizeof(uint)) {
                    throw new InvalidOperationException("Bad operation");
                }
                CUQueue sb = CScopeUQueue.Lock();
                sb.Save(key).Push(q.IntenalBuffer, q.HeadPosition, q.Size);
                q.SetSize(0);
                bool ok = SendRequest(idEnqueueBatch, sb, GetRH(e), discarded, (DOnExceptionFromServer)null);
                CScopeUQueue.Unlock(sb);
                return ok;
            }

            public bool Enqueue(byte[] key, ushort idMessage) {
                return Enqueue(key, idMessage, (byte[])null, (DEnqueue)null, (DDiscarded)null);
            }

            public bool Enqueue(byte[] key, ushort idMessage, DEnqueue e) {
                return Enqueue(key, idMessage, (byte[])null, e, (DDiscarded)null);
            }

            public bool Enqueue(byte[] key, ushort idMessage, byte[] bytes) {
                return Enqueue(key, idMessage, bytes, (DEnqueue)null, (DDiscarded)null);
            }

            public bool Enqueue(byte[] key, ushort idMessage, byte[] bytes, DEnqueue e) {
                return Enqueue(key, idMessage, bytes, e, (DDiscarded)null);
            }

            public virtual bool Enqueue(byte[] key, ushort idMessage, byte[] bytes, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                CUQueue sb = CScopeUQueue.Lock();
                sb.Save(key).Save(idMessage).Push(bytes);
                bool ok = SendRequest(idEnqueue, sb, GetRH(e), discarded, (DOnExceptionFromServer)null);
                CScopeUQueue.Unlock(sb);
                return ok;
            }

            public bool Enqueue(byte[] key, ushort idMessage, CUQueue buffer) {
                return Enqueue(key, idMessage, buffer, (DEnqueue)null, (DDiscarded)null);
            }

            public bool Enqueue(byte[] key, ushort idMessage, CUQueue buffer, DEnqueue e) {
                return Enqueue(key, idMessage, buffer, e, (DDiscarded)null);
            }

            public virtual bool Enqueue(byte[] key, ushort idMessage, CUQueue buffer, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                CUQueue sb = CScopeUQueue.Lock();
                sb.Save(key).Save(idMessage);
                if (buffer != null)
                    sb.Push(buffer.IntenalBuffer, buffer.HeadPosition, buffer.Size);
                bool ok = SendRequest(idEnqueue, sb, GetRH(e), discarded, (DOnExceptionFromServer)null);
                CScopeUQueue.Unlock(sb);
                return ok;
            }

            public bool Enqueue<T0>(byte[] key, ushort idMessage, T0 t0) {
                return Enqueue(key, idMessage, t0, (DEnqueue)null, (DDiscarded)null);
            }

            public bool Enqueue<T0>(byte[] key, ushort idMessage, T0 t0, DEnqueue e) {
                return Enqueue<T0>(key, idMessage, t0, e, (DDiscarded)null);
            }
            public bool Enqueue<T0>(byte[] key, ushort idMessage, T0 t0, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1>(byte[] key, ushort idMessage, T0 t0, T1 t1) {
                return Enqueue(key, idMessage, t0, t1, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1>(byte[] key, ushort idMessage, T0 t0, T1 t1, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1>(byte[] key, ushort idMessage, T0 t0, T1 t1, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2) {
                return Enqueue(key, idMessage, t0, t1, t2, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3, T4>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3, T4, T5>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, t7, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, t7, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, t7, t8, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, t7, t8, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, (DEnqueue)null, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, DEnqueue e) {
                return Enqueue(key, idMessage, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, e, (DDiscarded)null);
            }
            public bool Enqueue<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(byte[] key, ushort idMessage, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, DEnqueue e, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                    return Enqueue(key, idMessage, sb.UQueue, e, discarded);
                }
            }

            /// <summary>
            /// Start enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="qt">A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool StartQueueTrans(byte[] key, DQueueTrans qt) {
                if (key == null)
                    key = new byte[0];
                IClientQueue cq = AttachedClientSocket.ClientQueue;
                if (cq.Available)
                    cq.StartJob();
                using (CScopeUQueue sq = new CScopeUQueue()) {
                    sq.UQueue.Save(key);
                    return SendRequest(idStartTrans, sq, (ar) => {
                        if (qt != null) {
                            int errCode;
                            ar.Load(out errCode);
                            qt((CAsyncQueue)ar.AsyncServiceHandler, errCode);
                        } else {
                            ar.UQueue.SetSize(0);
                        }
                    });
                }
            }

            /// <summary>
            /// Commit enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
            /// </summary>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool EndQueueTrans() {
                return EndQueueTrans(false, null, null);
            }

            /// <summary>
            /// End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
            /// </summary>
            /// <param name="rollback">true for rollback, and false for committing</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool EndQueueTrans(bool rollback) {
                return EndQueueTrans(rollback, null, null);
            }

            /// <summary>
            /// End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
            /// </summary>
            /// <param name="rollback">true for rollback, and false for committing</param>
            /// <param name="qt">A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool EndQueueTrans(bool rollback, DQueueTrans qt) {
                return EndQueueTrans(rollback, qt, null);
            }

            /// <summary>
            /// End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
            /// </summary>
            /// <param name="rollback">true for rollback, and false for committing</param>
            /// <param name="qt">A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public virtual bool EndQueueTrans(bool rollback, DQueueTrans qt, DDiscarded discarded) {
                bool ok = SendRequest(idEndTrans, rollback, (ar) => {
                    if (qt != null) {
                        int errCode;
                        ar.Load(out errCode);
                        qt((CAsyncQueue)ar.AsyncServiceHandler, errCode);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                }, discarded, (DOnExceptionFromServer)null);
                IClientQueue cq = AttachedClientSocket.ClientQueue;
                if (cq.Available) {
                    if (rollback)
                        cq.AbortJob();
                    else
                        cq.EndJob();
                }
                return ok;
            }

            /// <summary>
            /// Query queue keys opened at server side
            /// </summary>
            /// <param name="gk">A callback for tracking a list of key names</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool GetKeys(DGetKeys gk) {
                return GetKeys(gk, null);
            }

            /// <summary>
            /// Query queue keys opened at server side
            /// </summary>
            /// <param name="gk">A callback for tracking a list of key names</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public virtual bool GetKeys(DGetKeys gk, DDiscarded discarded) {
                return SendRequest(idGetKeys, (ar) => {
                    if (gk != null) {
                        uint size;
                        ar.Load(out size);
                        string[] v = new string[size];
                        for (uint n = 0; n < size; ++n) {
                            byte[] bytes;
                            ar.Load(out bytes);
                            if (bytes != null)
                                v[n] = Encoding.UTF8.GetString(bytes, 0, bytes.Length);
                        }
                        gk((CAsyncQueue)ar.AsyncServiceHandler, v);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                }, discarded, (DOnExceptionFromServer)null);
            }

            /// <summary>
            /// Try to close a persistent queue opened at server side
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool CloseQueue(byte[] key) {
                return CloseQueue(key, null, null, false);
            }

            /// <summary>
            /// Try to close a persistent queue opened at server side
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="c">A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool CloseQueue(byte[] key, DClose c) {
                return CloseQueue(key, c, null, false);
            }

            /// <summary>
            /// Try to close or delete a persistent queue opened at server side
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="c">A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool CloseQueue(byte[] key, DClose c, DDiscarded discarded) {
                return CloseQueue(key, c, discarded, false);
            }

            /// <summary>
            /// Try to close or delete a persistent queue opened at server side
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="c">A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <param name="permanent">true for deleting a queue file, and false for closing a queue file</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public virtual bool CloseQueue(byte[] key, DClose c, DDiscarded discarded, bool permanent) {
                if (key == null)
                    key = new byte[0];
                return SendRequest(idClose, key, permanent, (ar) => {
                    if (c != null) {
                        int errCode;
                        ar.Load(out errCode);
                        c((CAsyncQueue)ar.AsyncServiceHandler, errCode);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                }, discarded, (DOnExceptionFromServer)null);
            }

            /// <summary>
            /// Just get message count and queue file size in bytes only
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="f">A callback for tracking returning message count and queue file size in bytes</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool FlushQueue(byte[] key, DFlush f) {
                return FlushQueue(key, f, tagOptimistic.oMemoryCached, null);
            }

            /// <summary>
            /// May flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="f">A callback for tracking returning message count and queue file size in bytes</param>
            /// <param name="option">one of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool FlushQueue(byte[] key, DFlush f, tagOptimistic option) {
                return FlushQueue(key, f, option, null);
            }

            /// <summary>
            /// May flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="f">A callback for tracking returning message count and queue file size in bytes</param>
            /// <param name="option">one of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public virtual bool FlushQueue(byte[] key, DFlush f, tagOptimistic option, DDiscarded discarded) {
                if (key == null)
                    key = new byte[0];
                return SendRequest(idFlush, key, (int)option, (ar) => {
                    if (f != null) {
                        ulong messageCount, fileSize;
                        ar.Load(out messageCount).Load(out fileSize);
                        f((CAsyncQueue)ar.AsyncServiceHandler, messageCount, fileSize);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                }, discarded, (DOnExceptionFromServer)null);
            }

            /// <summary>
            /// Dequeue messages from a persistent message queue file at server side in batch without waiting
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="d">A callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool Dequeue(byte[] key, DDequeue d) {
                return Dequeue(key, d, 0, null);
            }

            /// <summary>
            /// Dequeue messages from a persistent message queue file at server side in batch
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="d">A callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch</param>
            /// <param name="timeout">A time-out number in milliseconds</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public bool Dequeue(byte[] key, DDequeue d, uint timeout) {
                return Dequeue(key, d, timeout, null);
            }

            /// <summary>
            /// Dequeue messages from a persistent message queue file at server side in batch
            /// </summary>
            /// <param name="key">An ASCII string for identifying a queue at server side</param>
            /// <param name="d">A callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch</param>
            /// <param name="timeout">A time-out number in milliseconds</param>
            /// <param name="discarded">a callback for tracking cancel or socket closed event</param>
            /// <returns>true for sending the request successfully, and false for failure</returns>
            public virtual bool Dequeue(byte[] key, DDequeue d, uint timeout, DDiscarded discarded) {
                DAsyncResultHandler rh = null;
                if (key == null)
                    key = new byte[0];
                lock (m_csQ) {
                    m_keyDequeue = key;
                    if (d != null) {
                        rh = (ar) => {
                            ulong messageCount, fileSize, ret;
                            ar.Load(out messageCount).Load(out fileSize).Load(out ret);
                            uint messages = (uint)ret;
                            uint bytes = (uint)(ret >> 32);
                            d((CAsyncQueue)ar.AsyncServiceHandler, messageCount, fileSize, messages, bytes);
                        };
                        m_dDequeue = d;
                    } else {
                        m_dDequeue = null;
                    }
                }
                using (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(key).Save(timeout);
                    return SendRequest(idDequeue, sb, rh, discarded, (DOnExceptionFromServer)null);
                }
            }

            protected override void OnBaseRequestProcessed(ushort reqId) {
                switch (reqId) {
                    case (ushort)tagBaseRequestID.idMessageQueued:
                        byte[] key;
                        DDequeue deq;
                        lock (m_csQ) {
                            key = m_keyDequeue;
                            deq = m_dDequeue;
                        }
                        if (deq != null) {
                            //we send a request to dequeue messages after a notification message arrives that a new message is enqueued at server side
                            Dequeue(key, deq, 0);
                        }
                        if (MessageQueued != null) {
                            MessageQueued(this);
                        }
                        break;
                    default:
                        break;
                }
            }

            protected override void OnResultReturned(ushort reqId, CUQueue mc) {
                switch (reqId) {
                    case idClose:
                    case idEnqueue:
                        mc.SetSize(0);
                        break;
                    case idBatchSizeNotified:
                        mc.Load(out m_nBatchSize);
                        break;
                    default:
                        break;
                }
            }
        }
    } //namespace ClientSide
} //namespace SocketProAdapter
