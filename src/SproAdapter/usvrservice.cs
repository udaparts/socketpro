using System;
using System.Collections.Generic;
//using System.Text;
using System.Reflection;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public abstract class CBaseService : IDisposable
        {
            internal static bool m_bRegEvent = false;
            internal static uint m_nMainThreads = uint.MaxValue;
            internal static object m_csService = new object();
            private static List<CBaseService> m_lstService = new List<CBaseService>();
            private uint m_svsId;

            internal object m_cs = new object();
            private CSvsContext m_sc = new CSvsContext();
            private List<CSocketPeer> m_lstPeer = new List<CSocketPeer>();
            private List<CSocketPeer> m_lstDeadPeer = new List<CSocketPeer>();

            protected abstract CSocketPeer GetPeerSocket();

            internal Dictionary<ushort, MethodInfo> m_dicMethod = new Dictionary<ushort, MethodInfo>();

            private void ReleasePeer(ulong hSocket, bool bClosing, uint info)
            {
                lock (m_cs)
                {
                    CSocketPeer found = m_lstPeer.Find(delegate(CSocketPeer sp) { return (sp.m_sh == hSocket); });
                    if (found != null)
                    {
                        found.m_qBuffer.SetSize(0);
                        found.OnRelease(bClosing, info);
                        found.m_sh = 0;
                        m_lstPeer.Remove(found);
                        m_lstDeadPeer.Add(found);
                    }
                }
            }

            private CSocketPeer CreatePeer(ulong hSocket, uint newServiceId)
            {
                CSocketPeer sp = null;
                lock (m_cs)
                {
                    if (m_lstDeadPeer.Count > 0)
                    {
                        sp = m_lstDeadPeer[0];
                        m_lstDeadPeer.RemoveAt(0);
                    }
                }
                if (sp == null)
                {
                    sp = GetPeerSocket();
                }
                sp.m_Service = this;
                sp.m_sh = hSocket;
                lock (m_cs)
                {
                    m_lstPeer.Add(sp);
                }
                return sp;
            }

            public void RemoveMe()
            {
                if (m_svsId > 0)
                {
                    ServerCoreLoader.RemoveASvsContext(m_svsId);
                    lock (m_csService)
                    {
                        m_lstService.Remove(this);
                    }
                    m_svsId = 0;
                }
            }

            /// <summary>
            /// Register a service
            /// </summary>
            /// <param name="svsId">A service id</param>
            /// <returns>True if successful. Otherwise false if failed</returns>
            public bool AddMe(uint svsId)
            {
                return AddMe(svsId, tagThreadApartment.taNone);
            }

            /// <summary>
            /// Register a service
            /// </summary>
            /// <param name="svsId">A service id</param>
            /// <param name="ta">Thread apartment for windows default to tagThreadApartment.taNone. It is ignored on non-windows platforms</param>
            /// <returns>True if successful. Otherwise false if failed</returns>
            public virtual bool AddMe(uint svsId, tagThreadApartment ta)
            {
                m_sc.m_OnBaseRequestCame = new DOnBaseRequestCame(OnBaseCame);
                m_sc.m_OnChatRequestCame = new DOnChatRequestCame(OnChatCame);
                m_sc.m_OnChatRequestComing = new DOnChatRequestComing(OnChatComing);
                m_sc.m_OnClose = new DOnClose(OnClose);
                m_sc.m_OnFastRequestArrive = new DOnFastRequestArrive(OnFast);
                m_sc.m_OnHttpAuthentication = new DOnHttpAuthentication(OnHttpAuthentication);
                m_sc.m_OnRequestArrive = new DOnRequestArrive(OnReqArrive);
                m_sc.m_OnRequestProcessed = new DOnRequestProcessed(OnSRProcessed);
                m_sc.m_OnSwitchTo = new DOnSwitchTo(OnSwitch);
                m_sc.m_SlowProcess = new DSLOW_PROCESS(OnSlow);
                m_sc.m_OnResultsSent = new DOnResultsSent(OnResultsSent);
                m_sc.m_ta = ta;
                if (svsId > 0 && ServerCoreLoader.AddSvsContext(svsId, m_sc))
                {
                    m_svsId = svsId;
                    lock (m_csService)
                    {
                        if (!m_bRegEvent)
                        {
                            ServerCoreLoader.SetThreadEvent(CSocketProServer.te);
                            m_bRegEvent = true;
                        }
                        m_lstService.Add(this);
                    }
                    return true;
                }
                return false;
            }

            public uint SvsID
            {
                get
                {
                    return m_svsId;
                }
            }

            public uint CountOfSlowRequests
            {
                get
                {
                    return ServerCoreLoader.GetCountOfSlowRequests(m_svsId);
                }
            }

            public List<ushort> AllSlowRequestIds
            {
                get
                {
                    uint res;
                    ushort[] sr = new ushort[4097];
                    unsafe
                    {
                        fixed (ushort* p = sr)
                        {
                            res = ServerCoreLoader.GetAllSlowRequestIds(m_svsId, p, 4097);
                        }
                    }
                    List<ushort> s = new List<ushort>();
                    for (uint n = 0; n < res; ++n)
                    {
                        s.Add(sr[n]);
                    }
                    return s;
                }
            }

            public bool ReturnRandom
            {
                get
                {
                    return ServerCoreLoader.GetReturnRandom(m_svsId);
                }
                set
                {
                    ServerCoreLoader.SetReturnRandom(m_svsId, value);
                }
            }

            /// <summary>
            /// Register a slow request
            /// </summary>
            /// <param name="reqId">A request id</param>
            /// <returns>True if successful. Otherwise false if failed</returns>
            public bool AddSlowRequest(ushort reqId)
            {
                return ServerCoreLoader.AddSlowRequest(m_svsId, reqId);
            }

            public void RemoveSlowRequest(ushort reqId)
            {
                ServerCoreLoader.RemoveSlowRequest(m_svsId, reqId);
            }

            public void RemoveAllSlowRequests()
            {
                ServerCoreLoader.RemoveAllSlowRequests(m_svsId);
            }

            /// <summary>
            /// Make a request processed at router for a routee
            /// </summary>
            /// <param name="reqId">A request id</param>
            /// <returns>True if successful. Otherwise, false if failed</returns>
            public bool AddAlphaRequest(ushort reqId)
            {
                return ServerCoreLoader.AddAlphaRequest(m_svsId, reqId);
            }

            public List<ushort> AlphaRequestIds
            {
                get
                {
                    uint res;
                    ushort[] sr = new ushort[4097];
                    unsafe
                    {
                        fixed (ushort* p = sr)
                        {
                            res = ServerCoreLoader.GetAlphaRequestIds(m_svsId, p, 4097);
                        }
                    }
                    List<ushort> s = new List<ushort>();
                    for (uint n = 0; n < res; ++n)
                    {
                        s.Add(sr[n]);
                    }
                    return s;
                }
            }

            public CSocketPeer Seek(ulong hSocket)
            {
                lock (m_cs)
                {
                    foreach (CSocketPeer sp in m_lstPeer)
                    {
                        if (sp.m_sh == hSocket)
                            return sp;
                    }
                }
                return null;
            }

            private void OnReqArrive(ulong hSocket, ushort usRequestID, uint len)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                sp.m_CurrReqID = usRequestID;
                CUQueue q = sp.m_qBuffer;
                q.SetSize(0);
                if (len > q.MaxBufferSize)
                    q.Realloc(len);
                if (len > 0)
                {
                    uint res;
                    unsafe
                    {
                        fixed (byte* buffer = q.m_bytes)
                        {
                            if (m_nMainThreads <= 1)
                            {
                                CUQueue.CopyMemory(buffer, (void*)ServerCoreLoader.GetRequestBuffer(hSocket), len);
                                res = len;
                            }
                            else
                            {
                                res = ServerCoreLoader.RetrieveBuffer(hSocket, len, buffer, false);
                            }
                        }
                    }
                    System.Diagnostics.Debug.Assert(res == len);
                    q.SetSize(res);
                }
                if (m_svsId != BaseServiceID.sidHTTP)
                {
                    q.OS = sp.m_os;
                    q.Endian = sp.m_endian;
                }
                else
                {
                    CHttpPeerBase hp = (CHttpPeerBase)sp;
                    hp.m_WebRequestName = null;
                    hp.m_vArg.Clear();
                    if (usRequestID == (ushort)tagHttpRequestID.idUserRequest)
                    {
                        uint count;
                        sbyte[] reqName;
                        q.Load(out reqName);
                        hp.m_WebRequestName = CUQueue.ToString(reqName);
                        q.Load(out count);
                        for (uint n = 0; n < count; ++n)
                        {
                            object arg;
                            q.Load(out arg);
                            hp.m_vArg.Add(arg);
                        }
                    }
                }
                sp.OnRArrive(usRequestID, len);
            }

            private void OnFast(ulong hSocket, ushort usRequestID, uint len)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                try
                {
                    if (sp.SvsID == BaseServiceID.sidHTTP)
                        ((CHttpPeerBase)sp).OnHttpRequestArrive(usRequestID, (uint)sp.m_qBuffer.GetSize());
                    else
                    {
                        CClientPeer cp = (CClientPeer)sp;
                        if (!cp.m_dicDel.ContainsKey(usRequestID))
                        {
                            if (sp is CClientPeer)
                                ((CClientPeer)sp).OnFast(usRequestID, len);
                            else
                                ServerCoreLoader.SendExceptionResult(hSocket, "Request not registered at server side", Environment.StackTrace, usRequestID, 0);
                        }
                        else
                        {
                            object[] args = new object[] { cp.m_dicDel[usRequestID] };
                            object res = m_dicMethod[usRequestID].Invoke(cp, args);
                            res = null;
                        }
                    }
                }
                catch (Exception ex)
                {
                    ServerCoreLoader.SendExceptionResult(hSocket, ex.Message, ex.StackTrace, usRequestID, 0);
                }
            }

            private void OnSwitch(ulong hSocket, uint oldServiceId, uint newServiceId)
            {
                if (m_nMainThreads == uint.MaxValue)
                    m_nMainThreads = ServerCoreLoader.GetMainThreads();
                CBaseService bsOld;
                if (oldServiceId != BaseServiceID.sidStartup && (bsOld = SeekService(oldServiceId)) != null)
                {
                    bsOld.ReleasePeer(hSocket, false, newServiceId);
                }
                CBaseService bsNew = SeekService(newServiceId);
                if (bsNew != null)
                {
                    CSocketPeer sp = bsNew.CreatePeer(hSocket, newServiceId);
                    sp.m_bRandom = bsNew.ReturnRandom;
                    if (newServiceId == BaseServiceID.sidHTTP)
                    {
                        CHttpPeerBase hp = (CHttpPeerBase)sp;
                        hp.m_bHttpOk = false;
                    }
                    else
                    {
                        sp.m_os = ServerCoreLoader.GetPeerOs(hSocket, ref sp.m_endian);
                        sp.m_qBuffer.OS = sp.m_os;
                        sp.m_qBuffer.Endian = sp.m_endian;
                    }
                    sp.OnSwitch(oldServiceId);
                }
            }

            private int OnSlow(ushort usRequestID, uint len, ulong hSocket)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return 0;
                try
                {
                    if (sp.SvsID == BaseServiceID.sidHTTP)
                        return ((CHttpPeerBase)sp).OnHttpRequestArrive(usRequestID, (uint)sp.m_qBuffer.GetSize());
                    else
                    {
                        CClientPeer cp = (CClientPeer)sp;
                        if (!cp.m_dicDel.ContainsKey(usRequestID))
                        {
                            if (sp is CClientPeer)
                                return ((CClientPeer)sp).OnSlow(usRequestID, len);
                            ServerCoreLoader.SendExceptionResult(hSocket, "Request not registered at server side", Environment.StackTrace, usRequestID, 0);
                            return 0;
                        }
                        else
                        {
                            object[] args = new object[] { cp.m_dicDel[usRequestID] };
                            object res = m_dicMethod[usRequestID].Invoke(cp, args);
                            res = null;
                        }
                    }
                }
                catch (Exception ex)
                {
                    ServerCoreLoader.SendExceptionResult(hSocket, ex.Message, ex.StackTrace, usRequestID, 0);
                }
                return 0;
            }

            private void OnClose(ulong hSocket, int nError)
            {
                ReleasePeer(hSocket, true, (uint)nError);
            }

            private void OnResultsSent(ulong hSocket)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                sp.OnRSent();
            }

            private void OnBaseCame(ulong hSocket, ushort reqId)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                if (m_svsId == BaseServiceID.sidHTTP && reqId == (ushort)tagBaseRequestID.idPing)
                {
                    CHttpPeerBase hp = (CHttpPeerBase)sp;
                    hp.m_vArg.Clear();
                    hp.m_WebRequestName = null;
                }
                sp.OnBRCame(reqId);
            }

            private void OnSRProcessed(ulong hSocket, ushort usRequestID)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                sp.OnSRProcessed(usRequestID);
            }

            private void OnChatComing(ulong hSocket, tagChatRequestID chatRequestID, uint len)
            {
                uint res;
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                sp.m_CurrReqID = (ushort)chatRequestID;
                CUQueue q = sp.m_qBuffer;
                q.SetSize(0);
                if (len > q.MaxBufferSize)
                    q.Realloc(len);
                unsafe
                {
                    fixed (byte* buffer = q.m_bytes)
                    {
                        res = ServerCoreLoader.RetrieveBuffer(hSocket, len, buffer, true);
                    }
                }
                System.Diagnostics.Debug.Assert(res == len);
                q.SetSize(res);
                sp.OnChatComing(chatRequestID);
            }

            private void OnChatCame(ulong hSocket, tagChatRequestID chatRequestId)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return;
                sp.OnCRCame(chatRequestId);
            }

            private bool OnHttpAuthentication(ulong hSocket, IntPtr userId, IntPtr password)
            {
                CSocketPeer sp = Seek(hSocket);
                if (sp == null)
                    return false;
                CHttpPeerBase hp = (CHttpPeerBase)sp;
                unsafe
                {
                    string uid = new string((char*)userId);
                    string pwd = new string((char*)password);
                    hp.m_bHttpOk = hp.DoHttpAuth(uid, pwd);
                }
                return hp.m_bHttpOk;
            }

            public static CBaseService SeekService(ulong hSocket)
            {
                uint nServiceId = ServerCoreLoader.GetSvsID(hSocket);
                return SeekService(nServiceId);
            }

            public static CBaseService SeekService(uint serviceId)
            {
                lock (m_csService)
                {
                    foreach (CBaseService bs in m_lstService)
                    {
                        if (bs.SvsID == serviceId)
                            return bs;
                    }
                }
                return null;
            }

            #region IDisposable Members

            public void Dispose()
            {
                if (m_sc.m_OnClose != null)
                {
                    lock (m_csService)
                    {
                        m_lstService.Remove(this);
                    }
                    m_sc = new CSvsContext();
                }
            }

            #endregion
        }
    }
}
