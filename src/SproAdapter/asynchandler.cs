using System;
#if TASKS_ENABLED
using System.Threading.Tasks;
#endif

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public sealed class CAsyncResult : IDisposable
        {
            internal CAsyncResult(CAsyncServiceHandler ash, ushort sReqId, CUQueue q, CAsyncServiceHandler.DAsyncResultHandler arh)
            {
                m_AsyncServiceHandler = ash;
                m_RequestId = sReqId;
                m_UQueue = q;
                m_CurrentAsyncResultHandler = arh;
            }

            internal void Reset(ushort sReqId, CUQueue q, CAsyncServiceHandler.DAsyncResultHandler arh)
            {
                m_RequestId = sReqId;
                m_UQueue = q;
                m_CurrentAsyncResultHandler = arh;
            }

            private CAsyncServiceHandler m_AsyncServiceHandler;
            private ushort m_RequestId;
            private CUQueue m_UQueue;
            private CAsyncServiceHandler.DAsyncResultHandler m_CurrentAsyncResultHandler;

            public CUQueue Load<T>(out T receiver)
            {
                m_UQueue.Load(out receiver);
                return m_UQueue;
            }

            public CAsyncServiceHandler AsyncServiceHandler {
                get {
                    return m_AsyncServiceHandler;
                }
            }
            public ushort RequestId {
                get {
                    return m_RequestId;
                }
            }
            public CUQueue UQueue {
                get {
                    return m_UQueue;
                }
            }

            public CAsyncServiceHandler.DAsyncResultHandler CurrentAsyncResultHandler {
                get {
                    return m_CurrentAsyncResultHandler;
                }
            }

            public void Dispose()
            {
                Clean();
            }

            private void Clean()
            {
                if (m_UQueue != null)
                {
                    m_UQueue = null;
                }
            }

            ~CAsyncResult()
            {
                Clean();
            }
        }

        public class CAsyncServiceHandler : IDisposable
        {
            public const int SESSION_CLOSED_AFTER = -1000;
            public const int SESSION_CLOSED_BEFORE = -1001;
            public const int REQUEST_CANCELED = -1002;
            public const ulong DEFAULT_INTERRUPT_OPTION = 1;

            public delegate void DAsyncResultHandler(CAsyncResult AsyncResult);
            public delegate bool DOnResultReturned(CAsyncServiceHandler sender, ushort reqId, CUQueue qData);
            public delegate void DOnExceptionFromServer(CAsyncServiceHandler sender, ushort reqId, string errMessage, string errWhere, int errCode);
            public delegate void DOnBaseRequestProcessed(CAsyncServiceHandler sender, ushort reqId);

            private UDelegate<DOnResultReturned> m_lstRR;
            public event DOnResultReturned ResultReturned {
                add {
                    m_lstRR.add(value);
                }
                remove {
                    m_lstRR.remove(value);
                }
            }
            private UDelegate<DOnExceptionFromServer> m_lstEFS;
            public event DOnExceptionFromServer ServerException {
                add {
                    m_lstEFS.add(value);
                }
                remove {

                    m_lstEFS.remove(value);
                }
            }
            private UDelegate<DOnBaseRequestProcessed> m_lstBRP;
            public event DOnBaseRequestProcessed BaseRequestProcessed {
                add {
                    m_lstBRP.add(value);
                }
                remove {
                    m_lstBRP.Remove(value);
                }
            }

#if SP_MANAGER
            public CAsyncServiceHandler()
            {
                m_nServiceId = 0;
                m_ClientSocket = null;
                m_lstBRP = new UDelegate<DOnBaseRequestProcessed>(m_cs);
                m_lstEFS = new UDelegate<DOnExceptionFromServer>(m_cs);
                m_lstRR = new UDelegate<DOnResultReturned>(m_cs);
                m_ar = new CAsyncResult(this, 0, null, null);
            }
            public dynamic Pool { get; internal set; }
#endif
            protected CAsyncServiceHandler(uint nServiceId)
            {
                m_nServiceId = nServiceId;
                m_ClientSocket = null;
                m_lstBRP = new UDelegate<DOnBaseRequestProcessed>(m_cs);
                m_lstEFS = new UDelegate<DOnExceptionFromServer>(m_cs);
                m_lstRR = new UDelegate<DOnResultReturned>(m_cs);
                m_ar = new CAsyncResult(this, 0, null, null);
            }

            internal void Detach()
            {
                if (m_ClientSocket == null)
                    return;
                CClientSocket cs = m_ClientSocket;
                m_ClientSocket = null;
                cs.Detach(this);
            }

            internal void OnPP(uint hint, ulong data)
            {
                OnPostProcessing(hint, data);
            }

            protected virtual void OnPostProcessing(uint hint, ulong data)
            {

            }

            internal bool Attach(CClientSocket cs)
            {
                bool ok = true;
                Detach();
                if (cs != null)
                {
                    ok = cs.Attach(this);
                    m_ClientSocket = cs;
                }
                return ok;
            }

            public uint SvsID {
                get {
                    return m_nServiceId;
                }
            }

            public bool CommitBatching()
            {
                return CommitBatching(false);
            }

            public virtual bool CommitBatching(bool bBatchingAtServerSide)
            {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                lock (m_cs)
                {
                    m_kvCallback.InsertRange(m_kvCallback.Count, m_kvBatching);
                    m_kvBatching.Clear();
                }
                return ClientCoreLoader.CommitBatching(h, (byte)(bBatchingAtServerSide ? 1 : 0)) != 0;
            }

            protected virtual void OnMergeTo(CAsyncServiceHandler to)
            {

            }

            internal void AppendTo(CAsyncServiceHandler to)
            {
                lock (to.m_cs)
                {
                    lock (m_cs)
                    {
                        OnMergeTo(to);
                        to.m_kvCallback.InsertRange(to.m_kvCallback.Count, m_kvCallback);
                        m_kvCallback.Clear();
                    }
                }
            }

            public virtual bool StartBatching()
            {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                return ClientCoreLoader.StartBatching(h) != 0;
            }

            public virtual void AbortDequeuedMessage()
            {
                if (m_ClientSocket == null)
                    return;
                IntPtr h = m_ClientSocket.Handle;
                ClientCoreLoader.AbortDequeuedMessage(h);
            }

            public bool DequeuedResult {
                get {
                    if (m_ClientSocket == null)
                        return false;
                    IntPtr h = m_ClientSocket.Handle;
                    return ClientCoreLoader.DequeuedResult(h) != 0;
                }
            }

            public bool DequeuedMessageAborted {
                get {
                    if (m_ClientSocket == null)
                        return false;
                    IntPtr h = m_ClientSocket.Handle;
                    return ClientCoreLoader.IsDequeuedMessageAborted(h) != 0;
                }
            }

            public bool RouteeRequest {
                get {
                    if (m_ClientSocket == null)
                        return false;
                    IntPtr h = m_ClientSocket.Handle;
                    return ClientCoreLoader.IsRouteeRequest(h) != 0;
                }
            }

            public virtual bool AbortBatching()
            {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                lock (m_cs)
                {
                    foreach (MyKeyValue<ushort, CResultCb> p in m_kvBatching)
                    {
                        if (p.Value.Discarded != null)
                        {
                            p.Value.Discarded.Invoke(this, true);
                        }
                    }
                    m_kvBatching.Clear();
                }
                return ClientCoreLoader.AbortBatching(h) != 0;
            }

            public bool WaitAll()
            {
                return WaitAll(uint.MaxValue);
            }

            public virtual bool WaitAll(uint timeOut)
            {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                if (ClientCoreLoader.IsBatching(h) != 0)
                    throw new InvalidOperationException("Can't call the method WaitAll during batching requests");
                if (ClientCoreLoader.IsQueueStarted(h) != 0 && ClientCoreLoader.GetJobSize(h) > 0)
                    throw new InvalidOperationException("Can't call the method WaitAll during enqueuing transactional requests");
                return ClientCoreLoader.WaitAll(h, timeOut) != 0;
            }

            public bool Batching {
                get {
                    if (m_ClientSocket == null)
                        return false;
                    IntPtr h = m_ClientSocket.Handle;
                    return ClientCoreLoader.IsBatching(h) != 0;
                }
            }

            public virtual uint CleanCallbacks()
            {
                uint size = 0;
                Deque<MyKeyValue<ushort, CResultCb>> kvCallback, kvBatching;
                lock (m_cs)
                {
                    kvBatching = m_kvBatching;
                    kvCallback = m_kvCallback;
                    m_kvBatching = new Deque<MyKeyValue<ushort, CResultCb>>();
                    m_kvCallback = new Deque<MyKeyValue<ushort, CResultCb>>();
                }
                size = (uint)(kvBatching.Count + kvCallback.Count);
                foreach (MyKeyValue<ushort, CResultCb> p in kvBatching)
                {
                    if (p.Value.Discarded != null)
                    {
                        p.Value.Discarded.Invoke(this, AttachedClientSocket.CurrentRequestID == (ushort)tagBaseRequestID.idCancel);
                    }
                }
                kvBatching.Clear();
                foreach (MyKeyValue<ushort, CResultCb> p in kvCallback)
                {
                    if (p.Value.Discarded != null)
                    {
                        p.Value.Discarded.Invoke(this, AttachedClientSocket.CurrentRequestID == (ushort)tagBaseRequestID.idCancel);
                    }
                }
                kvCallback.Clear();
                return size;
            }

            internal bool m_bRandom = false;

            MyKeyValue<ushort, CResultCb> GetAsyncResultHandler(ushort reqId)
            {
                if (m_bRandom)
                {
                    lock (m_cs)
                    {
                        foreach (MyKeyValue<ushort, CResultCb> kv in m_kvCallback)
                        {
                            if (kv.Key == reqId)
                            {
                                m_kvCallback.Remove(kv);
                                return kv;
                            }
                        }
                    }
                }
                else
                {
                    lock (m_cs)
                    {
                        if (m_kvCallback.Count > 0 && m_kvCallback[0].Key == reqId)
                        {
                            return m_kvCallback.RemoveFromFront();
                        }
                    }
                }
                return null;
            }

            protected virtual void OnExceptionFromServer(ushort reqId, string errMessage, string errWhere, int errCode)
            {

            }

            internal void OnSE(ushort reqId, string errMessage, string errWhere, int errCode)
            {
                MyKeyValue<ushort, CResultCb> p = GetAsyncResultHandler(reqId);
                OnExceptionFromServer(reqId, errMessage, errWhere, errCode);
                if (p != null)
                {
                    CResultCb rcb = p.Value;
                    if (rcb != null && rcb.ExceptionFromServer != null)
                    {
                        rcb.ExceptionFromServer.Invoke(this, reqId, errMessage, errWhere, errCode);
                    }
                }
                lock (m_cs)
                {
                    foreach (var el in m_lstEFS)
                    {
                        el.Invoke(this, reqId, errMessage, errWhere, errCode);
                    }
                }
            }

            virtual protected void OnInterrupted(ulong options)
            {

            }

            private CAsyncResult m_ar = null;

            internal void onRR(ushort reqId, CUQueue mc)
            {
                if (tagBaseRequestID.idInterrupt == (tagBaseRequestID)reqId)
                {
                    ulong options;
                    mc.Load(out options);
                    OnInterrupted(options);
                    return;
                }

                MyKeyValue<ushort, CResultCb> p = GetAsyncResultHandler(reqId);
                do
                {
                    if (p != null && p.Value != null && p.Value.AsyncResultHandler != null)
                    {
                        m_ar.Reset(reqId, mc, p.Value.AsyncResultHandler);
                        p.Value.AsyncResultHandler.Invoke(m_ar);
                        break;
                    }
                    bool processed = false;
                    lock (m_cs)
                    {
                        foreach (DOnResultReturned r in m_lstRR)
                        {
                            if (r.Invoke(this, reqId, mc))
                            {
                                processed = true;
                                break;
                            }
                        }
                    }
                    if (processed) break;
                    OnResultReturned(reqId, mc);
                } while (false);
            }

            protected virtual void OnResultReturned(ushort sRequestId, CUQueue UQueue)
            {

            }

            protected virtual void OnBaseRequestProcessed(ushort reqId)
            {

            }

            internal void OnBProcessed(ushort reqId)
            {
                lock (m_cs)
                {
                    foreach (var el in m_lstBRP)
                    {
                        el.Invoke(this, reqId);
                    }
                }
                OnBaseRequestProcessed(reqId);
            }

            internal void OnAll()
            {
                OnAllProcessed();
            }

            protected virtual void OnAllProcessed()
            {

            }

            protected virtual bool SendRouteeResult(byte[] data, uint len, ushort reqId)
            {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                if (reqId == 0)
                    reqId = m_ClientSocket.CurrentRequestID;
                unsafe
                {
                    if (data != null && len > (uint)data.Length)
                        len = (uint)data.Length;
                    fixed (byte* buffer = data)
                    {
                        return ClientCoreLoader.SendRouteeResult(h, reqId, buffer, len) != 0;
                    }
                }
            }

            protected bool SendRouteeResult(byte[] data, uint len)
            {
                return SendRouteeResult(data, len, (ushort)0);
            }

            protected bool SendRouteeResult(ushort reqId)
            {
                return SendRouteeResult((byte[])null, (uint)0, reqId);
            }

            protected bool SendRouteeResult()
            {
                return SendRouteeResult((ushort)0);
            }

            protected virtual bool SendRouteeResult(CUQueue q, ushort reqId)
            {
                if (q == null || q.GetSize() == 0)
                    return SendRouteeResult(reqId);
                if (q.HeadPosition > 0)
                    return SendRouteeResult(q.GetBuffer(), q.GetSize(), reqId);
                return SendRouteeResult(q.m_bytes, q.GetSize(), reqId);
            }

            protected bool SendRouteeResult(CUQueue q)
            {
                return SendRouteeResult(q, (ushort)0);
            }

            protected bool SendRouteeResult(CScopeUQueue q)
            {
                return SendRouteeResult(q, (ushort)0);
            }

            protected virtual bool SendRouteeResult(CScopeUQueue q, ushort reqId)
            {
                if (q == null)
                    return SendRouteeResult(reqId);
                return SendRouteeResult(q.UQueue, reqId);
            }

            protected bool SendRouteeResult<T0>(T0 t0, ushort reqId)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0>(T0 t0)
            {
                return SendRouteeResult(t0, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1>(T0 t0, T1 t1, ushort reqId)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0, T1>(T0 t0, T1 t1)
            {
                return SendRouteeResult(t0, t1, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1, T2>(T0 t0, T1 t1, T2 t2, ushort reqId)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0, T1, T2>(T0 t0, T1 t1, T2 t2)
            {
                return SendRouteeResult(t0, t1, t2, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1, T2, T3>(T0 t0, T1 t1, T2 t2, T3 t3, ushort reqId)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0, T1, T2, T3>(T0 t0, T1 t1, T2 t2, T3 t3)
            {
                return SendRouteeResult(t0, t1, t2, t3, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1, T2, T3, T4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, ushort reqId)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            protected bool SendRouteeResult<T0, T1, T2, T3, T4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
            {
                return SendRouteeResult(t0, t1, t2, t3, t4, (ushort)0);
            }

            public virtual bool SendRequest(ushort reqId, byte[] data, uint len, DAsyncResultHandler ash)
            {
                return SendRequest(reqId, data, len, ash, null, null);
            }

            public virtual bool SendRequest(ushort reqId, CUQueue q, DAsyncResultHandler ash)
            {
                if (q == null)
                    return SendRequest(reqId, ash);
                if (q.HeadPosition > 0)
                    return SendRequest(reqId, q.GetBuffer(), q.GetSize(), ash);
                return SendRequest(reqId, q.m_bytes, q.GetSize(), ash);
            }

            public virtual bool SendRequest(ushort reqId, CUQueue q, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                if (q == null)
                    return SendRequest(reqId, ash, discarded, exception);
                if (q.HeadPosition > 0)
                    return SendRequest(reqId, q.GetBuffer(), q.GetSize(), ash, discarded, exception);
                return SendRequest(reqId, q.m_bytes, q.GetSize(), ash, discarded, exception);
            }

            public virtual bool SendRequest(ushort reqId, CScopeUQueue q, DAsyncResultHandler ash)
            {
                if (q == null)
                    return SendRequest(reqId, ash);
                return SendRequest(reqId, q.UQueue, ash);
            }

            public virtual bool SendRequest(ushort reqId, CScopeUQueue q, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                if (q == null)
                    return SendRequest(reqId, ash, discarded, exception);
                return SendRequest(reqId, q.UQueue, ash, discarded, exception);
            }

            public bool SendRequest(ushort reqId, DAsyncResultHandler ash)
            {
                return SendRequest(reqId, (byte[])null, (uint)0, ash);
            }

            public bool SendRequest(ushort reqId, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                return SendRequest(reqId, (byte[])null, (uint)0, ash, discarded, exception);
            }

            public bool SendRequest<T0>(ushort reqId, T0 t0, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1>(ushort reqId, T0 t0, T1 t1, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0>(ushort reqId, T0 t0, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1>(ushort reqId, T0 t0, T1 t1, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, DAsyncResultHandler ash)
            {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public CClientSocket Socket {
                get {
                    return m_ClientSocket;
                }
            }

            public CClientSocket AttachedClientSocket {
                get {
                    return m_ClientSocket;
                }
            }

            internal void SetNull()
            {
                m_ClientSocket = null;
            }

            public virtual bool Interrupt(ulong options)
            {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
#if WINCE
                return (ClientCoreLoader.SendInterruptRequest(h, (uint)options) != 0);
#else
                return (ClientCoreLoader.SendInterruptRequest(h, options) != 0);
#endif
            }

            public virtual bool SendRequest(ushort reqId, byte[] data, uint len, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception)
            {
                bool sent;
                byte batching;
                MyKeyValue<ushort, CResultCb> kv;
                if (null == m_ClientSocket)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                if (data != null && len > (uint)data.Length)
                    len = (uint)data.Length;
                if (ash != null || discarded != null || exception != null)
                {
                    CResultCb rcb = new CResultCb(ash, discarded, exception);
                    kv = new MyKeyValue<ushort, CResultCb>(reqId, rcb);
                    batching = ClientCoreLoader.IsBatching(h);
                    lock (m_csSend)
                    {
                        lock (m_cs)
                        {
                            if (batching != 0)
                            {
                                m_kvBatching.AddToBack(kv);
                            }
                            else
                            {
                                m_kvCallback.AddToBack(kv);
                            }
                        }
                        unsafe
                        {
                            fixed (byte* buffer = data)
                            {
                                sent = (ClientCoreLoader.SendRequest(h, reqId, buffer, len) != 0);
                            }
                        }
                    }
                }
                else
                {
                    kv = null;
                    batching = 0;
                    unsafe
                    {
                        fixed (byte* buffer = data)
                        {
                            sent = (ClientCoreLoader.SendRequest(h, reqId, buffer, len) != 0);
                        }
                    }
                }
                if (sent)
                    return true;
                if (kv != null)
                {
                    lock (m_cs)
                    {
                        if (batching > 0)
                            m_kvBatching.Clear();
                        else
                            m_kvCallback.Clear();
                    }
                }
                return false;
            }
            public delegate void DDiscarded(CAsyncServiceHandler h, bool canceled);
            internal class CResultCb
            {
                public CResultCb(DAsyncResultHandler rh, DDiscarded d, DOnExceptionFromServer ex)
                {
                    AsyncResultHandler = rh;
                    Discarded = d;
                    ExceptionFromServer = ex;
                }
                public DAsyncResultHandler AsyncResultHandler;
                public DDiscarded Discarded;
                public DOnExceptionFromServer ExceptionFromServer;
            }

            public void raise(string method_name, ushort req_id)
            {
                if (method_name == null || method_name.Length == 0)
                {
                    throw new ArgumentException("Method name cannot be empty");
                }
                if (req_id == 0)
                {
                    throw new ArgumentException("Request id cannot be zero");
                }
                CClientSocket cs = Socket;
                int ec = cs.ErrorCode;
                if (ec != 0)
                {
                    string em = cs.ErrorMsg;
                    throw new CSocketError(ec, em, req_id, true);
                }
                else
                {
                    throw new CSocketError(SESSION_CLOSED_BEFORE, "Session already closed before sending the request " + method_name, req_id, true);
                }
            }

#if TASKS_ENABLED

            public static DOnExceptionFromServer get_se<R>(TaskCompletionSource<R> tcs)
            {
                DOnExceptionFromServer se = (sender, rid, errMessage, errWhere, errCode) =>
                {
                    tcs.TrySetException(new CServerError(errCode, errMessage, errWhere, rid));
                };
                return se;
            }

            public static DDiscarded get_aborted<R>(TaskCompletionSource<R> tcs, string method_name, ushort req_id)
            {
                if (method_name == null || method_name.Length == 0)
                {
                    throw new ArgumentException("Method name cannot be empty");
                }
                if (req_id == 0)
                {
                    throw new ArgumentException("Request id cannot be zero");
                }
                DDiscarded aborted = (h, canceled) =>
                {
                    if (canceled)
                    {
                        tcs.TrySetException(new CSocketError(REQUEST_CANCELED, "Request " + method_name + " canceled", req_id, false));
                    }
                    else
                    {
                        CClientSocket cs = h.Socket;
                        int ec = cs.ErrorCode;
                        if (ec != 0)
                        {
                            string em = cs.ErrorMsg;
                            tcs.TrySetException(new CSocketError(ec, em, req_id, false));
                        }
                        else
                        {
                            tcs.TrySetException(new CSocketError(SESSION_CLOSED_AFTER, "Session closed after sending the request " + method_name, req_id, false));
                        }
                    }
                };
                return aborted;
            }

            public Task<CScopeUQueue> send(ushort reqId, byte[] data, uint len)
            {
                //use threadless task only
                TaskCompletionSource<CScopeUQueue> tcs = new TaskCompletionSource<CScopeUQueue>();
                if (!SendRequest(reqId, data, len, (ar) =>
                {
                    CScopeUQueue sb = new CScopeUQueue();
                    sb.UQueue.Swap(ar.UQueue);
                    tcs.TrySetResult(sb);
                }, get_aborted(tcs, "SendRequest", reqId), get_se(tcs)))
                {
                    raise("SendRequest", reqId);
                }
                return tcs.Task;
            }

            public Task<CScopeUQueue> send(ushort reqId)
            {
                return send(reqId, (byte[])null, (uint)0);
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2).Save(t3);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1).Save(t2);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0, T1>(ushort reqId, T0 t0, T1 t1)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0).Save(t1);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }

            public Task<CScopeUQueue> send<T0>(ushort reqId, T0 t0)
            {
                using (CScopeUQueue sb = new CScopeUQueue())
                {
                    CUQueue b = sb.UQueue;
                    b.Save(t0);
                    return send(reqId, b.IntenalBuffer, b.GetSize());
                }
            }
#endif
            private CClientSocket m_ClientSocket;
            internal uint m_nServiceId;
            internal object m_cs = new object();
            private object m_csSend = new object();

            internal class MyKeyValue<K, V>
            {
                public MyKeyValue(K key, V value)
                {
                    Key = key;
                    Value = value;
                }
                public K Key;
                public V Value;
            }

            private Deque<MyKeyValue<ushort, CResultCb>> m_kvCallback = new Deque<MyKeyValue<ushort, CResultCb>>();
            private Deque<MyKeyValue<ushort, CResultCb>> m_kvBatching = new Deque<MyKeyValue<ushort, CResultCb>>();
            private static object m_csCallIndex = new object();
            private static ulong m_CallIndex = 0;

            /// <summary>
            /// Get an unique increment call index number
            /// </summary>
            public static ulong GetCallIndex()
            {
                lock (m_csCallIndex)
                {
                    return ++m_CallIndex;
                }
            }

            /// <summary>
            /// A property for the number of requests queued inside asynchronous handler
            /// </summary>
            public int RequestsQueued {
                get {
                    lock (m_cs)
                    {
                        return m_kvCallback.Count;
                    }
                }
            }

            internal Deque<MyKeyValue<ushort, CResultCb>> GetCallbacks()
            {
                return m_kvCallback;
            }

            internal void EraseBack(int count)
            {
                int total = m_kvCallback.Count;
                if (count > total)
                    count = total;
                int start = (total - count);
                for (; start < total; ++start)
                {
                    CResultCb p = m_kvCallback[start].Value;
                    if (p.Discarded != null)
                    {
                        p.Discarded(this, true);
                    }
                }
                m_kvCallback.RemoveRange(start, count);
            }

            #region IDisposable Members

            public void Dispose()
            {
                lock (m_cs)
                {
                    CleanCallbacks();
                    if (m_ClientSocket != null)
                    {
                        m_ClientSocket.Detach(this);
                        m_ClientSocket = null;
                    }
                }
            }
            #endregion
        }
    }
}
