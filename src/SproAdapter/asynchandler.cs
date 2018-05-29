using System;
using System.Collections.Generic;
#if TASKS_ENABLED
using System.Threading.Tasks;
#endif

namespace SocketProAdapter {
    namespace ClientSide {
        public sealed class CAsyncResult : IDisposable {
            internal CAsyncResult(CAsyncServiceHandler ash, ushort sReqId, CUQueue q, CAsyncServiceHandler.DAsyncResultHandler arh) {
                m_AsyncServiceHandler = ash;
                m_RequestId = sReqId;
                m_UQueue = q;
                m_CurrentAsyncResultHandler = arh;
            }
            private CAsyncServiceHandler m_AsyncServiceHandler;
            private ushort m_RequestId;
            private CUQueue m_UQueue;
            private CAsyncServiceHandler.DAsyncResultHandler m_CurrentAsyncResultHandler;

            public CUQueue Load<T>(out T receiver) {
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

            public void Dispose() {
                Clean();
            }

            private void Clean() {
                if (m_UQueue != null) {
                    m_UQueue = null;
                }
            }

            ~CAsyncResult() {
                Clean();
            }
        }

        public class CAsyncServiceHandler : IDisposable {
            public delegate void DAsyncResultHandler(CAsyncResult AsyncResult);
            public delegate bool DOnResultReturned(CAsyncServiceHandler sender, ushort reqId, CUQueue qData);
            public delegate void DOnExceptionFromServer(CAsyncServiceHandler sender, ushort reqId, string errMessage, string errWhere, int errCode);
            public delegate void DOnBaseRequestProcessed(CAsyncServiceHandler sender, ushort reqId);
            public event DOnResultReturned ResultReturned;
            public event DOnExceptionFromServer ServerException;
            public event DOnBaseRequestProcessed BaseRequestProcessed;

            protected CAsyncServiceHandler(uint nServiceId) {
                m_nServiceId = nServiceId;
                m_ClientSocket = null;
            }

            internal void Detach() {
                if (m_ClientSocket == null)
                    return;
                CClientSocket cs = m_ClientSocket;
                m_ClientSocket = null;
                cs.Detach(this);
            }

            internal bool Attach(CClientSocket cs) {
                bool ok = true;
                Detach();
                if (cs != null) {
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

            public bool CommitBatching() {
                return CommitBatching(false);
            }

            public virtual bool CommitBatching(bool bBatchingAtServerSide) {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                lock (m_cs) {
                    m_kvCallback.InsertRange(m_kvCallback.Count, m_kvBatching);
                    m_kvBatching.Clear();
                }
                return ClientCoreLoader.CommitBatching(h, (byte)(bBatchingAtServerSide ? 1 : 0)) != 0;
            }

            protected virtual void OnMergeTo(CAsyncServiceHandler to) {

            }

            internal void AppendTo(CAsyncServiceHandler to) {
                lock (to.m_cs) {
                    lock (m_cs) {
                        OnMergeTo(to);
                        to.m_kvCallback.InsertRange(to.m_kvCallback.Count, m_kvCallback);
                        m_kvCallback.Clear();
                    }
                }
            }

            public virtual bool StartBatching() {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                return ClientCoreLoader.StartBatching(h) != 0;
            }

            public virtual void AbortDequeuedMessage() {
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

            public virtual bool AbortBatching() {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                lock (m_cs) {
                    foreach (KeyValuePair<ushort, CResultCb> p in m_kvBatching) {
                        if (p.Value.Discarded != null) {
                            p.Value.Discarded.Invoke(this, true);
                        }
                    }
                    m_kvBatching.Clear();
                }
                return ClientCoreLoader.AbortBatching(h) != 0;
            }

            public bool WaitAll() {
                return WaitAll(uint.MaxValue);
            }

            public virtual bool WaitAll(uint timeOut) {
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

            public virtual uint CleanCallbacks() {
                uint size = 0;
                lock (m_cs) {
                    size = (uint)(m_kvBatching.Count + m_kvCallback.Count);
                    foreach (KeyValuePair<ushort, CResultCb> p in m_kvBatching) {
                        if (p.Value.Discarded != null) {
                            p.Value.Discarded.Invoke(this, AttachedClientSocket.CurrentRequestID == (ushort)tagBaseRequestID.idCancel);
                        }
                    }
                    m_kvBatching.Clear();
                    foreach (KeyValuePair<ushort, CResultCb> p in m_kvCallback) {
                        if (p.Value.Discarded != null) {
                            p.Value.Discarded.Invoke(this, AttachedClientSocket.CurrentRequestID == (ushort)tagBaseRequestID.idCancel);
                        }
                    }
                    m_kvCallback.Clear();
                }
                return size;
            }

            KeyValuePair<ushort, CResultCb> GetAsyncResultHandler(ushort reqId) {
                if (m_ClientSocket.Random) {
                    lock (m_cs) {
                        foreach (KeyValuePair<ushort, CResultCb> kv in m_kvCallback) {
                            if (kv.Key == reqId) {
                                m_kvCallback.Remove(kv);
                                return kv;
                            }
                        }
                    }
                } else {
                    lock (m_cs) {
                        if (m_kvCallback.Count > 0 && m_kvCallback[0].Key == reqId) {
                            KeyValuePair<ushort, CResultCb> kv = m_kvCallback.RemoveFromFront();
                            return kv;
                        }
                    }
                }
                return new KeyValuePair<ushort, CResultCb>(reqId, null);
            }

            protected virtual void OnExceptionFromServer(ushort reqId, string errMessage, string errWhere, int errCode) {

            }

            internal void OnSE(ushort reqId, string errMessage, string errWhere, int errCode) {
#if DEBUG
                Console.WriteLine("OnSE reqId = {0}, errMsge = {1}, errWhere = {2}, errCode = {3}", reqId, errMessage, errWhere, errCode);
#endif
                KeyValuePair<ushort, CResultCb> p = GetAsyncResultHandler(reqId);
                OnExceptionFromServer(reqId, errMessage, errWhere, errCode);
                CResultCb rcb = p.Value;
                if (rcb != null && rcb.ExceptionFromServer != null) {
                    rcb.ExceptionFromServer.Invoke(this, reqId, errMessage, errWhere, errCode);
                }
                if (ServerException != null)
                    ServerException.Invoke(this, reqId, errMessage, errWhere, errCode);
            }

            internal void onRR(ushort reqId, CUQueue mc) {
                KeyValuePair<ushort, CResultCb> p = GetAsyncResultHandler(reqId);
                if (p.Value != null && p.Value.AsyncResultHandler != null) {
                    CAsyncResult ar = new CAsyncResult(this, reqId, mc, p.Value.AsyncResultHandler);
                    p.Value.AsyncResultHandler.Invoke(ar);
                } else if (ResultReturned != null && ResultReturned.Invoke(this, reqId, mc)) {
                } else
                    OnResultReturned(reqId, mc);
            }

            protected virtual void OnResultReturned(ushort sRequestId, CUQueue UQueue) {

            }

            protected virtual void OnBaseRequestProcessed(ushort reqId) {

            }

            internal void OnBProcessed(ushort reqId) {
                if (BaseRequestProcessed != null)
                    BaseRequestProcessed(this, reqId);
                OnBaseRequestProcessed(reqId);
            }

            internal void OnAll() {
                OnAllProcessed();
            }

            protected virtual void OnAllProcessed() {

            }

            protected virtual bool SendRouteeResult(byte[] data, uint len, ushort reqId) {
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                if (reqId == 0)
                    reqId = m_ClientSocket.CurrentRequestID;
                unsafe {
                    if (data != null && len > (uint)data.Length)
                        len = (uint)data.Length;
                    fixed (byte* buffer = data) {
                        return ClientCoreLoader.SendRouteeResult(h, reqId, buffer, len) != 0;
                    }
                }
            }

            protected bool SendRouteeResult(byte[] data, uint len) {
                return SendRouteeResult(data, len, (ushort)0);
            }

            protected bool SendRouteeResult(ushort reqId) {
                return SendRouteeResult((byte[])null, (uint)0, reqId);
            }

            protected bool SendRouteeResult() {
                return SendRouteeResult((ushort)0);
            }

            protected virtual bool SendRouteeResult(CUQueue q, ushort reqId) {
                if (q == null || q.GetSize() == 0)
                    return SendRouteeResult(reqId);
                if (q.HeadPosition > 0)
                    return SendRouteeResult(q.GetBuffer(), q.GetSize(), reqId);
                return SendRouteeResult(q.m_bytes, q.GetSize(), reqId);
            }

            protected bool SendRouteeResult(CUQueue q) {
                return SendRouteeResult(q, (ushort)0);
            }

            protected bool SendRouteeResult(CScopeUQueue q) {
                return SendRouteeResult(q, (ushort)0);
            }

            protected virtual bool SendRouteeResult(CScopeUQueue q, ushort reqId) {
                if (q == null)
                    return SendRouteeResult(reqId);
                return SendRouteeResult(q.UQueue, reqId);
            }

            protected bool SendRouteeResult<T0>(T0 t0, ushort reqId) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0>(T0 t0) {
                return SendRouteeResult(t0, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1>(T0 t0, T1 t1, ushort reqId) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0, T1>(T0 t0, T1 t1) {
                return SendRouteeResult(t0, t1, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1, T2>(T0 t0, T1 t1, T2 t2, ushort reqId) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0, T1, T2>(T0 t0, T1 t1, T2 t2) {
                return SendRouteeResult(t0, t1, t2, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1, T2, T3>(T0 t0, T1 t1, T2 t2, T3 t3, ushort reqId) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }
            protected bool SendRouteeResult<T0, T1, T2, T3>(T0 t0, T1 t1, T2 t2, T3 t3) {
                return SendRouteeResult(t0, t1, t2, t3, (ushort)0);
            }

            protected bool SendRouteeResult<T0, T1, T2, T3, T4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, ushort reqId) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                bool ok = SendRouteeResult(su, reqId);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            protected bool SendRouteeResult<T0, T1, T2, T3, T4>(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) {
                return SendRouteeResult(t0, t1, t2, t3, t4, (ushort)0);
            }

            public virtual bool SendRequest(ushort reqId, byte[] data, uint len, DAsyncResultHandler ash) {
                return SendRequest(reqId, data, len, ash, null, null);
            }

            public virtual bool SendRequest(ushort reqId, CUQueue q, DAsyncResultHandler ash) {
                if (q == null)
                    return SendRequest(reqId, ash);
                if (q.HeadPosition > 0)
                    return SendRequest(reqId, q.GetBuffer(), q.GetSize(), ash);
                return SendRequest(reqId, q.m_bytes, q.GetSize(), ash);
            }

            public virtual bool SendRequest(ushort reqId, CUQueue q, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                if (q == null)
                    return SendRequest(reqId, ash, discarded, exception);
                if (q.HeadPosition > 0)
                    return SendRequest(reqId, q.GetBuffer(), q.GetSize(), ash, discarded, exception);
                return SendRequest(reqId, q.m_bytes, q.GetSize(), ash, discarded, exception);
            }

            public virtual bool SendRequest(ushort reqId, CScopeUQueue q, DAsyncResultHandler ash) {
                if (q == null)
                    return SendRequest(reqId, ash);
                return SendRequest(reqId, q.UQueue, ash);
            }

            public virtual bool SendRequest(ushort reqId, CScopeUQueue q, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                if (q == null)
                    return SendRequest(reqId, ash, discarded, exception);
                return SendRequest(reqId, q.UQueue, ash, discarded, exception);
            }

            public bool SendRequest(ushort reqId, DAsyncResultHandler ash) {
                return SendRequest(reqId, (byte[])null, (uint)0, ash);
            }

            public bool SendRequest(ushort reqId, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                return SendRequest(reqId, (byte[])null, (uint)0, ash, discarded, exception);
            }

            public bool SendRequest<T0>(ushort reqId, T0 t0, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1>(ushort reqId, T0 t0, T1 t1, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                bool ok = SendRequest(reqId, su, ash, discarded, exception);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0>(ushort reqId, T0 t0, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1>(ushort reqId, T0 t0, T1 t1, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool SendRequest<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, DAsyncResultHandler ash) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                bool ok = SendRequest(reqId, su, ash);
                CScopeUQueue.Unlock(su);
                return ok;
            }

            public bool ProcessR0(ushort reqId) {
                if (!SendRequest(reqId, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0>(ushort reqId, T0 t0) {
                if (!SendRequest(reqId, t0, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1>(ushort reqId, T0 t0, T1 t1) {
                if (!SendRequest(reqId, t0, t1, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2) {
                if (!SendRequest(reqId, t0, t1, t2, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3) {
                if (!SendRequest(reqId, t0, t1, t2, t3, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) {
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) {
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) {
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) {
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) {
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR0<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) {
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, delegate(CAsyncResult ar) { }))
                    return false;
                return WaitAll();
            }

            public bool ProcessR1<R0>(ushort reqId, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, R0>(ushort reqId, T0 t0, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, R0>(ushort reqId, T0 t0, T1 t1, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, T7, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR1<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0) {
                R0 res0 = default(R0);
                r0 = default(R0);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, delegate(CAsyncResult ar) {
                    ar.Load(out res0);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                return true;
            }

            public bool ProcessR2<R0, R1>(ushort reqId, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, R0, R1>(ushort reqId, T0 t0, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, R0, R1>(ushort reqId, T0 t0, T1 t1, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR2<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                return true;
            }

            public bool ProcessR3<R0, R1, R2>(ushort reqId, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, R0, R1, R2>(ushort reqId, T0 t0, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR3<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                return true;
            }

            public bool ProcessR4<R0, R1, R2, R3>(ushort reqId, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, R0, R1, R2, R3>(ushort reqId, T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR4<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                return true;
            }

            public bool ProcessR5<R0, R1, R2, R3, R4>(ushort reqId, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, T7, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, T7, T8, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public bool ProcessR5<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, R0, R1, R2, R3, R4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, out R0 r0, out R1 r1, out R2 r2, out R3 r3, out R4 r4) {
                R0 res0 = default(R0);
                r0 = default(R0);
                R1 res1 = default(R1);
                r1 = default(R1);
                R2 res2 = default(R2);
                r2 = default(R2);
                R3 res3 = default(R3);
                r3 = default(R3);
                R4 res4 = default(R4);
                r4 = default(R4);
                if (!SendRequest(reqId, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, delegate(CAsyncResult ar) {
                    ar.Load(out res0).Load(out res1).Load(out res2).Load(out res3).Load(out res4);
                }))
                    return false;
                if (!WaitAll())
                    return false;
                r0 = res0;
                r1 = res1;
                r2 = res2;
                r3 = res3;
                r4 = res4;
                return true;
            }

            public CClientSocket AttachedClientSocket {
                get {
                    return m_ClientSocket;
                }
            }

            internal void SetNull() {
                m_ClientSocket = null;
            }

            public virtual bool SendRequest(ushort reqId, byte[] data, uint len, DAsyncResultHandler ash, DDiscarded discarded, DOnExceptionFromServer exception) {
                bool sent = false;
                byte batching = 0;
                CResultCb rcb = null;
                if (m_ClientSocket == null)
                    return false;
                IntPtr h = m_ClientSocket.Handle;
                if (data != null && len > (uint)data.Length)
                    len = (uint)data.Length;
                lock (m_csSend) {
                    if (ash != null || discarded != null || exception != null) {
                        rcb = new CResultCb();
                        rcb.AsyncResultHandler = ash;
                        rcb.Discarded = discarded;
                        rcb.ExceptionFromServer = exception;
                        KeyValuePair<ushort, CResultCb> kv = new KeyValuePair<ushort, CResultCb>(reqId, rcb);
                        lock (m_cs) {
                            batching = ClientCoreLoader.IsBatching(h);
                            if (batching != 0) {
                                m_kvBatching.AddToBack(kv);
                            } else {
                                m_kvCallback.AddToBack(kv);
                            }
                        }
                        unsafe {
                            fixed (byte* buffer = data) {
                                sent = (ClientCoreLoader.SendRequest(h, reqId, buffer, len) != 0);
                            }
                        }
                    }
                }
                if (sent)
                    return true;
                if (rcb != null) {
                    lock (m_cs) {
                        if (batching > 0)
                            m_kvBatching.RemoveFromBack();
                        else
                            m_kvCallback.RemoveFromBack();
                    }
                }
                return false;
            }
            public delegate void DDiscarded(CAsyncServiceHandler h, bool canceled);
            internal class CResultCb {
                public DAsyncResultHandler AsyncResultHandler = null;
                public DDiscarded Discarded = null;
                public DOnExceptionFromServer ExceptionFromServer = null;
            }
#if TASKS_ENABLED
            public Task<R> Async<R>(ushort reqId, byte[] data, uint len) {
                //use threadless task only
                TaskCompletionSource<R> tcs = new TaskCompletionSource<R>();
                if (!SendRequest(reqId, data, len, (ar) => {
                    try {
                        R r;
                        ar.Load(out r);
                        tcs.SetResult(r);
                    } catch (Exception err) {
                        tcs.SetException(err);
                    }
                }, (h, canceled) => {
                    try {
                        //tcs.SetException(new Exception("Task canceled"));
                        tcs.SetCanceled();
                    } catch {
                    }
                }, (sender, rid, errMessage, errWhere, errCode) => {
                    try {
                        tcs.SetException(new Exception(errMessage));
                    } catch {
                    }
                })) {
                    tcs.SetException(new Exception(AttachedClientSocket.ErrorMsg));
                }
                return tcs.Task;
            }

            public Task Async(ushort reqId, byte[] data, uint len) {
                //use threadless task only
                TaskCompletionSource<bool> tcs = new TaskCompletionSource<bool>();
                if (!SendRequest(reqId, data, len, (ar) => {
                    try {
                        bool r = true;
                        tcs.SetResult(r);
                    } catch (Exception err) {
                        tcs.SetException(err);
                    }
                }, (h, canceled) => {
                    try {
                        //tcs.SetException(new Exception("Task canceled"));
                        tcs.SetCanceled();
                    } catch {
                    }
                }, (sender, rid, errMessage, errWhere, errCode) => {
                    try {
                        tcs.SetException(new Exception(errMessage));
                    } catch {
                    }
                })) {
                    tcs.SetException(new Exception(AttachedClientSocket.ErrorMsg));
                }
                return tcs.Task;
            }

            public Task<R> Async<R>(ushort reqId) {
                return Async<R>(reqId, (byte[])null, (uint)0);
            }

            public Task Async(ushort reqId) {
                return Async(reqId, (byte[])null, (uint)0);
            }

            public Task<R> Async<R, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8).Save(t9);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3, T4, T5, T6, T7, T8>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7).Save(t8);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3, T4, T5, T6, T7>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6).Save(t7);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3, T4, T5, T6>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5).Save(t6);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3, T4, T5>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4).Save(t5);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3, T4>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3).Save(t4);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2, T3>(ushort reqId, T0 t0, T1 t1, T2 t2, T3 t3) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2).Save(t3);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1, T2>(ushort reqId, T0 t0, T1 t1, T2 t2) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1).Save(t2);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0, T1>(ushort reqId, T0 t0, T1 t1) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0, T1>(ushort reqId, T0 t0, T1 t1) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0).Save(t1);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task<R> Async<R, T0>(ushort reqId, T0 t0) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                Task<R> r = Async<R>(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }

            public Task Async<T0>(ushort reqId, T0 t0) {
                CUQueue su = CScopeUQueue.Lock();
                su.Save(t0);
                Task r = Async(reqId, su.IntenalBuffer, su.GetSize());
                CScopeUQueue.Unlock(su);
                return r;
            }
#endif
            private CClientSocket m_ClientSocket;
            internal uint m_nServiceId;
            internal object m_cs = new object();
            private object m_csSend = new object();
            private Deque<KeyValuePair<ushort, CResultCb>> m_kvCallback = new Deque<KeyValuePair<ushort, CResultCb>>();
            private Deque<KeyValuePair<ushort, CResultCb>> m_kvBatching = new Deque<KeyValuePair<ushort, CResultCb>>();
            private static object m_csCallIndex = new object();
            private static ulong m_CallIndex = 0;

            /// <summary>
            /// Get an unique increment call index number
            /// </summary>
            public static ulong GetCallIndex() {
                lock (m_csCallIndex) {
                    return ++m_CallIndex;
                }
            }

            /// <summary>
            /// A property for the number of requests queued inside asynchronous handler
            /// </summary>
            public int RequestsQueued {
                get {
                    lock (m_cs) {
                        return m_kvCallback.Count;
                    }
                }
            }

            internal Deque<KeyValuePair<ushort, CResultCb>> GetCallbacks() {
                return m_kvCallback;
            }

            internal void EraseBack(int count) {
                int total = m_kvCallback.Count;
                if (count > total)
                    count = total;
                int start = (total - count);
                for (; start < total; ++start) {
                    CResultCb p = m_kvCallback[start].Value;
                    if (p.Discarded != null) {
                        p.Discarded(this, true);
                    }
                }
                m_kvCallback.RemoveRange(start, count);
            }

            #region IDisposable Members

            public void Dispose() {
                lock (m_cs) {
                    CleanCallbacks();
                    if (m_ClientSocket != null) {
                        m_ClientSocket.Detach(this);
                        m_ClientSocket = null;
                    }
                }
            }
            #endregion
        }
    }
}
