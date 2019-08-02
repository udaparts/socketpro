using System;
using System.Collections.Generic;


namespace SocketProAdapter
{
    namespace ClientSide
    {
        public sealed class CClientSocket : IDisposable
        {
            public delegate void DOnSocketClosed(CClientSocket sender, int errorCode);
            public delegate void DOnHandShakeCompleted(CClientSocket sender, int errorCode);
            public delegate void DOnSocketConnected(CClientSocket sender, int errorCode);
            public delegate void DOnServerException(CClientSocket sender, ushort requestId, string errMessage, string errWhere, int errCode);


            internal object m_cs = new object(); //used for protecting events

            private UDelegate<DOnSocketClosed> m_lstClosed;
            public event DOnSocketClosed SocketClosed {
                add {
                    m_lstClosed.add(value);
                }
                remove {
                    m_lstClosed.remove(value);
                }
            }

            private UDelegate<DOnHandShakeCompleted> m_lstShake;
            public event DOnHandShakeCompleted HandShakeCompleted {
                add {
                    m_lstShake.add(value);
                }
                remove {
                    m_lstShake.remove(value);
                }
            }

            private UDelegate<DOnSocketConnected> m_lstConnected;
            public event DOnSocketConnected SocketConnected {
                add {
                    m_lstConnected.add(value);
                }
                remove {
                    m_lstConnected.remove(value);
                }
            }

            private UDelegate<DOnServerException> m_lstSE;
            public event DOnServerException SeverException {
                add {
                    m_lstSE.add(value);
                }
                remove {
                    m_lstSE.remove(value);
                }
            }
#if ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
            public delegate void DOnRequestProcessed(CClientSocket sender, ushort requestId, uint len);
            public delegate void DOnBaseRequestProcessed(CClientSocket sender, tagBaseRequestID requestId);
            public delegate void DOnAllRequestsProcessed(CClientSocket sender, ushort lastRequestId);

            private UDelegate<DOnAllRequestsProcessed> m_lstAll;
            public event DOnAllRequestsProcessed AllRequestsProcessed {
                add {
                    m_lstAll.add(value);
                }
                remove {
                    m_lstAll.remove(value);
                }
            }

            private UDelegate<DOnRequestProcessed> m_lstRP;
            public event DOnRequestProcessed RequestProcessed {
                add {
                    m_lstRP.add(value);
                }
                remove {
                    m_lstRP.remove(value);
                }
            }

            private UDelegate<DOnBaseRequestProcessed> m_lstBRP;
            public event DOnBaseRequestProcessed BaseRequestProcessed {
                add {
                    m_lstBRP.add(value);
                }
                remove {
                    m_lstBRP.remove(value);
                }
            }
#endif
            private CAsyncServiceHandler m_ash = null;

            public const uint DEFAULT_RECV_TIMEOUT = 30000;
            public const uint DEFAULT_CONN_TIMEOUT = 30000;

            internal bool Attach(CAsyncServiceHandler ash)
            {
                if (ash == null || ash == m_ash)
                    return false;
                m_ash = ash;
                return true;
            }

            internal void Detach(CAsyncServiceHandler ash)
            {
                if (ash == null || ash != m_ash)
                    return;
                m_ash = null;
                ash.SetNull();
            }

            #region IDisposable Members

            public void Dispose()
            {
                if (m_ash != null)
                {
                    m_ash.SetNull();
                    m_ash = null;
                }
            }

            #endregion

            internal CClientSocket()
            {
                m_h = IntPtr.Zero;
                m_PushImpl = new CPushImpl(this);
                m_qm = new CClientQueueImpl(this);

                m_hsc += new POnHandShakeCompleted(OnHSCompleted);
                m_rp += new POnRequestProcessed(OnRProcessed);
                m_ss += new POnSocketClosed(OnSClosed);
                m_sc += new POnSocketConnected(OnSConnected);
                m_brp += new POnBaseRequestProcessed(OnBRProcessed);
                m_arp += new POnAllRequestsProcessed(OnARProcessed);
                m_se += new POnServerException(OnSException);
                m_sx += new POnSpeakEx(OnBEx);
                m_enter += new POnEnter(OnEnter);
                m_exit += new POnExit(OnExit);
                m_s += new POnSpeak(OnB);
                m_sume += new POnSendUserMessageEx(OnPUMessageEx);
                m_sum += new POnSendUserMessage(OnPUMessage);
                m_pp += new POnPostProcessing(OnPostProcessing);
#if ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
                m_lstBRP = new UDelegate<DOnBaseRequestProcessed>(m_cs);
                m_lstAll = new UDelegate<DOnAllRequestsProcessed>(m_cs);
                m_lstRP = new UDelegate<DOnRequestProcessed>(m_cs);
#endif
                m_lstClosed = new UDelegate<DOnSocketClosed>(m_cs);
                m_lstConnected = new UDelegate<DOnSocketConnected>(m_cs);
                m_lstSE = new UDelegate<DOnServerException>(m_cs);
                m_lstShake = new UDelegate<DOnHandShakeCompleted>(m_cs);
            }

            public CAsyncServiceHandler CurrentHandler {
                get {
                    return m_ash;
                }
            }

            CAsyncServiceHandler Seek(uint svsId)
            {
                return m_ash;
            }

            public bool WaitAll()
            {
                return WaitAll(uint.MaxValue);
            }

            public bool WaitAll(uint timeOut)
            {
                if (ClientCoreLoader.IsBatching(m_h) != 0)
                    throw new InvalidOperationException("Can't call the method WaitAll during batching requests");
                if (ClientCoreLoader.IsQueueStarted(m_h) != 0 && ClientCoreLoader.GetJobSize(m_h) > 0)
                    throw new InvalidOperationException("Can't call the method WaitAll during enqueuing transactional requests");
                return ClientCoreLoader.WaitAll(m_h, timeOut) != 0;
            }

            public bool Cancel()
            {
                if (ClientCoreLoader.IsBatching(m_h) != 0)
                    throw new InvalidOperationException("Can't call the method Cancel during batching requests");
                return ClientCoreLoader.Cancel(m_h, uint.MaxValue) != 0;
            }

            public bool DoEcho()
            {
                return ClientCoreLoader.DoEcho(m_h) != 0;
            }

            public void AbortDequeuedMessage()
            {
                ClientCoreLoader.AbortDequeuedMessage(m_h);
            }

            public IUPushClient Push {
                get {
                    return m_PushImpl;
                }
            }

            public bool AutoConn {
                get {
                    return ClientCoreLoader.GetAutoConn(m_h) != 0;
                }
                set {
                    ClientCoreLoader.SetAutoConn(m_h, (byte)(value ? 1 : 0));
                }
            }

            public uint BytesBatched {
                get {
                    return ClientCoreLoader.GetBytesBatched(m_h);
                }
            }

            public uint BytesInReceivingBuffer {
                get {
                    return ClientCoreLoader.GetBytesInReceivingBuffer(m_h);
                }
            }

            public uint BytesInSendingBuffer {
                get {
                    return ClientCoreLoader.GetBytesInSendingBuffer(m_h);
                }
            }

            public ulong BytesReceived {
                get {
                    return ClientCoreLoader.GetBytesReceived(m_h);
                }
            }

            public ulong BytesSent {
                get {
                    return ClientCoreLoader.GetBytesSent(m_h);
                }
            }

            public uint ConnectingTimeout {
                get {
                    return ClientCoreLoader.GetConnTimeout(m_h);
                }
                set {
                    ClientCoreLoader.SetConnTimeout(m_h, value);
                }
            }

            public uint CountOfRequestsInQueue {
                get {
                    return ClientCoreLoader.GetCountOfRequestsQueued(m_h);
                }
            }

            public ushort CurrentRequestID {
                get {
                    return ClientCoreLoader.GetCurrentRequestID(m_h);
                }
            }

            private CConnectionContext m_cc = null;
            public CConnectionContext ConnectionContext {
                get {
                    return m_cc;
                }
                set {
                    m_cc = value;
                }
            }

            public static string Version {
                get {
                    unsafe
                    {

                        return new string((sbyte*)ClientCoreLoader.GetUClientSocketVersion());
                    }
                }
            }

            /// <summary>
            /// Use the method for debugging crash within cross development environments.
            /// </summary>
            /// <param name="str">A string will be sent to client core library to be output into a crash text file</param>
            public static void SetLastCallInfo(string str)
            {
                if (str == null)
                    str = "";
                unsafe
                {
                    fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(str))
                    {
                        IntPtr p = new IntPtr(data);
                        ClientCoreLoader.SetLastCallInfo(p);
                    }
                }
            }

            public static class SSL
            {
                private static PCertificateVerifyCallback m_cvCallback;

                public delegate bool DOnCertificateVerify(bool preverified, int depth, int errorCode, string errMessage, CertInfo ci);
                public static event DOnCertificateVerify CertificateVerify;

                static SSL()
                {
                    m_cvCallback += (preverified, depth, errCode, errMessage, ptr) =>
                    {
                        if (CertificateVerify != null && ptr != IntPtr.Zero)
                        {
                            string errMsg = null;
                            CertInfo ci = new CertInfo();
                            CertInfoIntenal cii = new CertInfoIntenal();
                            System.Runtime.InteropServices.Marshal.PtrToStructure(ptr, cii);
                            ci.Set(cii);
                            unsafe
                            {
                                if (errMessage != IntPtr.Zero)
                                    errMsg = new string((sbyte*)errMessage);
                            }
                            return (byte)(CertificateVerify.Invoke(preverified != 0, depth, errCode, errMsg, ci) ? 1 : 0);
                        }
                        return 1;
                    };
                    ClientCoreLoader.SetCertificateVerifyCallback(m_cvCallback);
                }

                public static bool SetVerifyLocation(string certFile)
                {
                    if (certFile == null || certFile.Length == 0)
                        throw new ArgumentException("Invalid queue file name");
                    unsafe
                    {
                        fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(certFile))
                        {
                            IntPtr file = new IntPtr(data);
                            return ClientCoreLoader.SetVerifyLocation(file) != 0;
                        }
                    }
                }
            }

            public static class QueueConfigure
            {
                public static bool IsClientQueueIndexPossiblyCrashed {
                    get {
                        return ClientCoreLoader.IsClientQueueIndexPossiblyCrashed() != 0;
                    }
                }

                public unsafe static string WorkDirectory {
                    get {
                        return new string((sbyte*)ClientCoreLoader.GetClientWorkDirectory());
                    }

                    set {
                        string s = value;
                        if (s == null)
                            s = "";
                        fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(s))
                        {
                            IntPtr dir = new IntPtr(data);
                            ClientCoreLoader.SetClientWorkDirectory(dir);
                        }
                    }
                }

                public static string MessageQueuePassword {
                    set {
                        string s = value;
                        if (s == null)
                            s = "";
                        unsafe
                        {
                            fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(s))
                            {
                                IntPtr pwd = new IntPtr(data);
                                ClientCoreLoader.SetMessageQueuePassword(pwd);
                            }
                        }
                    }
                }
            }

            public uint CurrentResultSize {
                get {
                    return ClientCoreLoader.GetCurrentResultSize(m_h);
                }
            }

            private uint m_nCurrentSvsId = BaseServiceID.sidStartup;
            public uint CurrentServiceID {
                get {
                    return m_nCurrentSvsId;
                }
            }

            public tagEncryptionMethod EncryptionMethod {
                get {
                    return ClientCoreLoader.GetEncryptionMethod(m_h);
                }
                set {
                    ClientCoreLoader.SetEncryptionMethod(m_h, value);
                }
            }

            public tagConnectionState ConnectionState {
                get {
                    return ClientCoreLoader.GetConnectionState(m_h);
                }
            }

            public int ErrorCode {
                get {
                    return ClientCoreLoader.GetErrorCode(m_h);
                }
            }

            public uint ReceivingTimeout {
                get {
                    return ClientCoreLoader.GetRecvTimeout(m_h);
                }
                set {
                    ClientCoreLoader.SetRecvTimeout(m_h, value);
                }
            }

            public string GetPeerName()
            {
                uint port = 0;
                sbyte[] addr = new sbyte[256];
                unsafe
                {
                    fixed (sbyte* p = addr)
                    {
                        ClientCoreLoader.GetPeerName(m_h, ref port, p, (ushort)256);
                        return new string(p);
                    }
                }
            }

            public string GetPeerName(out uint port)
            {
                port = 0;
                sbyte[] addr = new sbyte[256];
                unsafe
                {
                    fixed (sbyte* p = addr)
                    {
                        ClientCoreLoader.GetPeerName(m_h, ref port, p, (ushort)256);
                        return new string(p);
                    }
                }
            }

            public string ErrorMsg {
                get {
                    sbyte[] errMsg = new sbyte[1024];
                    unsafe
                    {
                        fixed (sbyte* p = errMsg)
                        {
                            ClientCoreLoader.GetErrorMessage(m_h, p, 1024);
                            return new string(p);
                        }
                    }
                }
            }

            public uint PoolId {
                get {
                    return ClientCoreLoader.GetSocketPoolId(m_h);
                }
            }

            private tagOperationSystem m_os = Defines.OperationSystem;
            private bool m_endian = false;
            public tagOperationSystem GetPeerOs()
            {
                return m_os;
            }

            public tagOperationSystem GetPeerOs(ref bool bigEndian)
            {
                bigEndian = m_endian;
                return m_os;
            }

            public void Close()
            {
                ClientCoreLoader.Close(m_h);
            }

            public void Shutdown()
            {
                ClientCoreLoader.Shutdown(m_h, tagShutdownType.stBoth);
            }

            public void Shutdown(tagShutdownType st)
            {
                ClientCoreLoader.Shutdown(m_h, st);
            }

            private bool m_bRandom = false;
            public bool Random {
                get {
                    return m_bRandom;
                }
            }

            public uint RouteeCount {
                get {
                    return ClientCoreLoader.GetRouteeCount(m_h);
                }
            }
            private bool m_routing = false;
            public bool Routing {
                get {
                    return m_routing;
                }
            }

            public bool DequeuedMessageAborted {
                get {
                    return ClientCoreLoader.IsDequeuedMessageAborted(m_h) != 0;
                }
            }

            public ushort ServerPingTime {
                get {
                    return ClientCoreLoader.GetServerPingTime(m_h);
                }
            }

            public IntPtr Handle {
                get {
                    return m_h;
                }
            }

            public ulong SocketNativeHandle {
                get {
                    return ClientCoreLoader.GetSocketNativeHandle(m_h);
                }
            }


            public bool Connected {
                get {
                    return ClientCoreLoader.IsOpened(m_h) != 0;
                }
            }

            public IntPtr SslHandle {
                get {
                    return ClientCoreLoader.GetSSL(m_h);
                }
            }

            public string UID {
                get {
                    uint res;
                    char[] id = new char[256];
                    unsafe
                    {
                        fixed (char* p = id)
                        {
                            res = ClientCoreLoader.GetUID(m_h, p, 256);
                        }
                    }
                    return new string(id, 0, (int)res);
                }
                set {
                    ClientCoreLoader.SetUserID(m_h, value);
                }
            }

            public bool Zip {
                get {
                    return ClientCoreLoader.GetZip(m_h) != 0;
                }

                set {
                    ClientCoreLoader.SetZip(m_h, (byte)(value ? 1 : 0));
                }
            }

            public tagZipLevel ZipLevel {
                get {
                    return ClientCoreLoader.GetZipLevel(m_h);
                }
                set {
                    ClientCoreLoader.SetZipLevel(m_h, value);
                }
            }

            public bool Batching {
                get {
                    return ClientCoreLoader.IsBatching(m_h) != 0;
                }
            }

            public string Password {
                set {
                    ClientCoreLoader.SetPassword(m_h, value);
                }
            }

            public bool TurnOnZipAtSvr(bool enableZip)
            {
                return ClientCoreLoader.TurnOnZipAtSvr(m_h, (byte)(enableZip ? 1 : 0)) != 0;
            }

            public bool SetZipLevelAtSvr(tagZipLevel zipLevel)
            {
                return ClientCoreLoader.SetZipLevelAtSvr(m_h, zipLevel) != 0;
            }

            private void OnSClosed(IntPtr handler, int nError)
            {
                CAsyncServiceHandler ash = Seek(CurrentServiceID);
                if (ash != null && !Sendable)
                    ash.CleanCallbacks();
                lock (m_cs)
                {
                    foreach (var el in m_lstClosed)
                    {
                        el.Invoke(this, nError);
                    }
                }
            }

            private void OnHSCompleted(IntPtr handler, int nError)
            {
                lock (m_cs)
                {
                    foreach (var el in m_lstShake)
                    {
                        el.Invoke(this, nError);
                    }
                }
            }

            private void OnSConnected(IntPtr handler, int nError)
            {
                if (nError == 0 && ClientCoreLoader.GetSSL(handler).ToInt64() != 0)
                {
                    m_cert = new CUCertImpl(this);
                }
                else
                {
                    m_cert = null;
                }
                lock (m_cs)
                {
                    foreach (var el in m_lstConnected)
                    {
                        el.Invoke(this, nError);
                    }
                }
            }

            private CUQueue m_qRecv = new CUQueue();

            private void OnRProcessed(IntPtr handler, ushort requestId, uint len)
            {
                CAsyncServiceHandler ash = Seek(CurrentServiceID);
                if (ash != null)
                {
                    if (m_routing)
                        m_os = ClientCoreLoader.GetPeerOs(handler, ref m_endian);
                    CUQueue q = m_qRecv;
                    q.OS = m_os;
                    q.Endian = m_endian;
                    if (q.MaxBufferSize < len)
                        q.Realloc(len);
                    if (len > 0)
                    {
                        IntPtr source = ClientCoreLoader.GetResultBuffer(handler);
#if WINCE
                        System.Runtime.InteropServices.Marshal.Copy(source, q.m_bytes, 0, (int)len);
#else
                        unsafe
                        {
                            fixed (byte* des = q.m_bytes)
                            {
                                CUQueue.CopyMemory(des, (void*)source, len);
                            }
                        }
#endif
                    }
                    q.SetSize(len);
                    ash.onRR(requestId, q);
                }
#if ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
                lock (m_cs)
                {
                    foreach (var el in m_lstRP)
                    {
                        el.Invoke(this, requestId, len);
                    }
                }
#endif
            }

            private CMessageSender ToMessageSender(CMessageSenderCe senderCe)
            {
                CMessageSender sender = new CMessageSender();
                sender.UserId = senderCe.UserId;
                unsafe
                {
                    sender.IpAddress = new string((sbyte*)senderCe.IpAddress);
                }
                sender.Port = senderCe.Port;
                sender.SelfMessage = (senderCe.SelfMessage != 0);
                sender.SvsID = senderCe.SvsID;
                return sender;
            }

            private void OnB(IntPtr handler, IntPtr senderCe, IntPtr groups, uint count, IntPtr message, uint size)
            {
                CMessageSenderCe msc = new CMessageSenderCe();
                System.Runtime.InteropServices.Marshal.PtrToStructure(senderCe, msc);
                using (CScopeUQueue su = new CScopeUQueue())
                {
                    ushort vt = (ushort)(tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI4);
                    su.UQueue.Save(vt);
                    su.UQueue.Save(count);
                    su.UQueue.Push(groups, count * sizeof(uint));
                    object obj;
                    su.UQueue.Load(out obj);
                    object msg;
                    su.UQueue.SetSize(0);
                    su.UQueue.Push(message, size);
                    su.UQueue.Load(out msg);
                    m_PushImpl.OnB(handler, ToMessageSender(msc), (uint[])obj, count, msg);
                }
            }

            private void OnPUMessage(IntPtr handler, IntPtr senderCe, IntPtr message, uint size)
            {
                CMessageSenderCe msc = new CMessageSenderCe();
                System.Runtime.InteropServices.Marshal.PtrToStructure(senderCe, msc);
                using (CScopeUQueue su = new CScopeUQueue())
                {
                    object msg;
                    su.UQueue.Push(message, size).Load(out msg);
                    m_PushImpl.OnPUMessage(handler, ToMessageSender(msc), msg);
                }
            }

            private void OnEnter(IntPtr handler, IntPtr senderCe, IntPtr groups, uint count)
            {
                CMessageSenderCe msc = new CMessageSenderCe();
                System.Runtime.InteropServices.Marshal.PtrToStructure(senderCe, msc);
                using (CScopeUQueue su = new CScopeUQueue())
                {
                    ushort vt = (ushort)(tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI4);
                    su.UQueue.Save(vt);
                    su.UQueue.Save(count);
                    su.UQueue.Push(groups, count * sizeof(uint));
                    object obj;
                    su.UQueue.Load(out obj);
                    m_PushImpl.OnEnter(handler, ToMessageSender(msc), (uint[])obj, count);
                }
            }

            private void OnExit(IntPtr handler, IntPtr senderCe, IntPtr groups, uint count)
            {
                CMessageSenderCe msc = new CMessageSenderCe();
                System.Runtime.InteropServices.Marshal.PtrToStructure(senderCe, msc);
                using (CScopeUQueue su = new CScopeUQueue())
                {
                    ushort vt = (ushort)(tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI4);
                    su.UQueue.Save(vt);
                    su.UQueue.Save(count);
                    su.UQueue.Push(groups, count * sizeof(uint));
                    object obj;
                    su.UQueue.Load(out obj);
                    m_PushImpl.OnExit(handler, ToMessageSender(msc), (uint[])obj, count);
                }
            }

            private void OnBEx(IntPtr handler, IntPtr senderCe, IntPtr groups, uint count, IntPtr message, uint size)
            {
                CMessageSenderCe msc = new CMessageSenderCe();
                System.Runtime.InteropServices.Marshal.PtrToStructure(senderCe, msc);
                using (CScopeUQueue su = new CScopeUQueue())
                {
                    ushort vt = (ushort)(tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_UI4);
                    su.UQueue.Save(vt);
                    su.UQueue.Save(count);
                    su.UQueue.Push(groups, count * sizeof(uint));
                    object obj;
                    su.UQueue.Load(out obj);
                    byte[] msg = new byte[size];
                    if (size > 0)
                    {
                        System.Runtime.InteropServices.Marshal.Copy(message, msg, 0, (int)size);
                    }
                    m_PushImpl.OnBEx(handler, ToMessageSender(msc), (uint[])obj, count, msg, size);
                }
            }

            private void OnPUMessageEx(IntPtr handler, IntPtr senderCe, IntPtr message, uint size)
            {
                CMessageSenderCe msc = new CMessageSenderCe();
                System.Runtime.InteropServices.Marshal.PtrToStructure(senderCe, msc);
                byte[] msg = new byte[size];
                System.Runtime.InteropServices.Marshal.Copy(message, msg, 0, (int)size);
                m_PushImpl.OnPUMessageEx(handler, ToMessageSender(msc), msg);
            }

            private void OnSException(IntPtr handler, ushort requestId, string errMessage, IntPtr errWhere, int errCode)
            {
                string location = "";
                if (errWhere != IntPtr.Zero)
                {
                    unsafe
                    {
                        location = new string((sbyte*)errWhere);
                    }
                }
                CAsyncServiceHandler ash = Seek(CurrentServiceID);
                if (ash != null)
                {
                    ash.OnSE(requestId, errMessage, location, errCode);
                }
                lock (m_cs)
                {
                    foreach (var el in m_lstSE)
                    {
                        el.Invoke(this, requestId, errMessage, location, errCode);
                    }
                }
            }

            private void OnBRProcessed(IntPtr handler, ushort requestId)
            {
                if (requestId == (uint)tagBaseRequestID.idSwitchTo)
                {
                    m_nCurrentSvsId = ClientCoreLoader.GetCurrentServiceId(handler);
                    m_bRandom = (ClientCoreLoader.IsRandom(m_h) != 0);
                    m_routing = (ClientCoreLoader.IsRouting(m_h) != 0);
                    m_os = ClientCoreLoader.GetPeerOs(m_h, ref m_endian);
                }
                CAsyncServiceHandler ash = Seek(CurrentServiceID);
                if (ash != null)
                {
                    ash.m_bRandom = m_bRandom;
                    ash.OnBProcessed(requestId);
                    if ((tagBaseRequestID)requestId == tagBaseRequestID.idCancel)
                        ash.CleanCallbacks();
                }
#if ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
                lock (m_cs)
                {
                    foreach (var el in m_lstBRP)
                    {
                        el.Invoke(this, (tagBaseRequestID)requestId);
                    }
                }
#endif
            }

            private void OnARProcessed(IntPtr handler, ushort lastRequestId)
            {
                CAsyncServiceHandler ash = Seek(CurrentServiceID);
                if (ash != null)
                    ash.OnAll();
#if ENABLE_SOCKET_REQUEST_AND_ALL_EVENTS
                lock (m_cs)
                {
                    foreach (var el in m_lstAll)
                    {
                        el.Invoke(this, lastRequestId);
                    }
                }
#endif
            }

            private void OnPostProcessing(IntPtr handler, uint hint, ulong data)
            {
                CAsyncServiceHandler ash = Seek(CurrentServiceID);
                if (ash != null)
                    ash.OnPP(hint, data);
            }

            private POnHandShakeCompleted m_hsc;
            private POnSocketClosed m_ss;
            private POnRequestProcessed m_rp;
            private POnSocketConnected m_sc;
            private POnBaseRequestProcessed m_brp;
            private POnAllRequestsProcessed m_arp;
            private POnServerException m_se;
            private POnSpeak m_s;
            private POnSpeakEx m_sx;
            private POnEnter m_enter;
            private POnExit m_exit;
            private POnSendUserMessage m_sum;
            private POnSendUserMessageEx m_sume;
            private POnPostProcessing m_pp;

            internal void Set(IntPtr h)
            {
                ClientCoreLoader.SetOnHandShakeCompleted(h, m_hsc);
                ClientCoreLoader.SetOnRequestProcessed(h, m_rp);
                ClientCoreLoader.SetOnSocketClosed(h, m_ss);
                ClientCoreLoader.SetOnSocketConnected(h, m_sc);
                ClientCoreLoader.SetOnBaseRequestProcessed(h, m_brp);
                ClientCoreLoader.SetOnAllRequestsProcessed(h, m_arp);
                ClientCoreLoader.SetOnServerException(h, m_se);
                ClientCoreLoader.SetOnSpeakEx2(h, m_sx);
                ClientCoreLoader.SetOnEnter2(h, m_enter);
                ClientCoreLoader.SetOnExit2(h, m_exit);
                ClientCoreLoader.SetOnSpeak2(h, m_s);
                ClientCoreLoader.SetOnSendUserMessageEx2(h, m_sume);
                ClientCoreLoader.SetOnSendUserMessage2(h, m_sum);
                ClientCoreLoader.SetOnPostProcessing(h, m_pp);
                m_h = h;
            }

            class CClientQueueImpl : IClientQueue
            {
                private int m_nQIndex = 0;
                internal CClientQueueImpl(CClientSocket cs)
                {
                    m_cs = cs;
                }

                private CClientSocket m_cs;

                #region IClientQueue Members

                public bool StartQueue(string qName, uint ttl)
                {
                    return StartQueue(qName, ttl, true, false);
                }

                public bool StartQueue(string qName, uint ttl, bool secure)
                {
                    return StartQueue(qName, ttl, secure, false);
                }

                public bool StartQueue(string qName, uint ttl, bool secure, bool dequeueShared)
                {
                    if (qName == null || qName.Length == 0)
                        throw new ArgumentException("Invalid queue file name");
                    unsafe
                    {
                        fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(qName))
                        {
                            IntPtr name = new IntPtr(data);
                            return ClientCoreLoader.StartQueue(m_cs.Handle, name, (byte)(secure ? 1 : 0), (byte)(dequeueShared ? 1 : 0), ttl) != 0;
                        }
                    }
                }

                public bool DequeueEnabled {
                    get {
                        return ClientCoreLoader.IsDequeueEnabled(m_cs.Handle) != 0;
                    }
                }

                public DateTime LastMessageTime {
                    get {
                        ulong seconds = ClientCoreLoader.GetLastQueueMessageTime(m_cs.Handle);
                        DateTime dt = new DateTime(2013, 1, 1, 0, 0, 0, DateTimeKind.Utc);
                        if (DateTime.Now.IsDaylightSavingTime())
                            dt = dt.AddSeconds(3600);
                        dt = dt.AddSeconds(seconds);
                        return dt;
                    }
                }

                public bool AppendTo(IClientQueue[] clientQueues)
                {
                    if (clientQueues == null || clientQueues.Length == 0)
                        return true;
                    List<IntPtr> qs = new List<IntPtr>();
                    foreach (IClientQueue cq in clientQueues)
                    {
                        qs.Add(cq.Handle);
                    }
                    return AppendTo(qs.ToArray());
                }

                public bool AppendTo(IntPtr[] queueHandles)
                {
                    if (queueHandles == null || queueHandles.Length == 0)
                        return true;
                    unsafe
                    {
                        fixed (IntPtr* p = queueHandles)
                        {
                            return ClientCoreLoader.PushQueueTo(m_cs.Handle, p, (uint)queueHandles.Length) != 0;
                        }
                    }
                }

                public bool AppendTo(IClientQueue clientQueue)
                {
                    if (clientQueue == null)
                        return true;
                    IntPtr[] queueHandles = { clientQueue.Handle };
                    return AppendTo(queueHandles);
                }

                public bool EnsureAppending(IClientQueue clientQueue)
                {
                    if (clientQueue == null)
                        return true;
                    IntPtr[] queueHandles = { clientQueue.Handle };
                    return EnsureAppending(queueHandles);
                }

                public bool EnsureAppending(IClientQueue[] clientQueues)
                {
                    if (clientQueues == null || clientQueues.Length == 0)
                        return true;
                    List<IntPtr> qs = new List<IntPtr>();
                    foreach (IClientQueue cq in clientQueues)
                    {
                        qs.Add(cq.Handle);
                    }
                    return EnsureAppending(qs.ToArray());
                }

                public bool EnsureAppending(IntPtr[] queueHandles)
                {
                    if (!Available)
                        return false;
                    if (QueueStatus != tagQueueStatus.qsMergePushing)
                        return true;
                    if (queueHandles == null || queueHandles.Length == 0)
                        return true;
                    List<IntPtr> vHandles = new List<IntPtr>();
                    foreach (IntPtr h in queueHandles)
                    {
                        if (ClientCoreLoader.GetClientQueueStatus(h) != tagQueueStatus.qsMergeComplete)
                            vHandles.Add(h);
                    }
                    if (vHandles.Count > 0)
                    {
                        return AppendTo(vHandles.ToArray());
                    }
                    Reset();
                    return true;
                }

                public bool RoutingQueueIndex {
                    get {
                        return ClientCoreLoader.IsRoutingQueueIndexEnabled(m_cs.Handle) != 0;
                    }

                    set {
                        ClientCoreLoader.EnableRoutingQueueIndex(m_cs.Handle, (byte)(value ? 1 : 0));
                    }
                }

                public IntPtr Handle {
                    get { return m_cs.Handle; }
                }

                #endregion

                #region IMessageQueueBasic Members

                public void StopQueue()
                {
                    ClientCoreLoader.StopQueue(m_cs.Handle, (byte)0);
                }

                public void StopQueue(bool permanent)
                {
                    ClientCoreLoader.StopQueue(m_cs.Handle, (byte)(permanent ? 1 : 0));
                }

                public void Reset()
                {
                    ClientCoreLoader.ResetQueue(m_cs.Handle);
                }

                public ulong CancelQueuedMessages(ulong startIndex, ulong endIndex)
                {
#if WINCE
                    return ClientCoreLoader.CancelQueuedRequestsByIndex(m_cs.Handle, (uint)startIndex, (uint)endIndex);
#else
                    return ClientCoreLoader.CancelQueuedRequestsByIndex(m_cs.Handle, startIndex, endIndex);
#endif
                }

                public uint MessagesInDequeuing {
                    get {
                        return ClientCoreLoader.GetMessagesInDequeuing(m_cs.Handle);
                    }
                }

                public ulong MessageCount {
                    get {
                        return ClientCoreLoader.GetMessageCount(m_cs.Handle);
                    }
                }

                public ulong QueueSize {
                    get {
                        return ClientCoreLoader.GetQueueSize(m_cs.Handle);
                    }
                }

                public bool Available {
                    get {
                        return ClientCoreLoader.IsQueueStarted(m_cs.Handle) != 0;
                    }
                }

                public bool Secure {
                    get { return ClientCoreLoader.IsQueueSecured(m_cs.Handle) != 0; }
                }

                public bool DequeueShared {
                    get {
                        return ClientCoreLoader.IsDequeueShared(m_cs.Handle) != 0;
                    }
                }
                public ulong LastIndex {
                    get {
                        return ClientCoreLoader.GetQueueLastIndex(m_cs.Handle);
                    }
                }

                public string QueueFileName {
                    get {
                        IntPtr p = ClientCoreLoader.GetQueueFileName(m_cs.Handle);
                        if (p == IntPtr.Zero)
                            return "";
                        unsafe
                        {
                            return new string((sbyte*)p);
                        }
                    }
                }

                public string QueueName {
                    get {
                        IntPtr p = ClientCoreLoader.GetQueueName(m_cs.Handle);
                        if (p == IntPtr.Zero)
                            return "";
                        unsafe
                        {
                            return new string((sbyte*)p);
                        }
                    }
                }

                public bool AbortJob()
                {
                    CAsyncServiceHandler ash = m_cs.CurrentHandler;
                    lock (ash.m_cs)
                    {
                        int aborted = ash.GetCallbacks().Count - m_nQIndex;
                        if (ClientCoreLoader.AbortJob(m_cs.Handle) != 0)
                        {
                            ash.EraseBack(aborted);
                            return true;
                        }
                    }
                    return false;
                }

                public bool StartJob()
                {
                    CAsyncServiceHandler ash = m_cs.CurrentHandler;
                    lock (ash.m_cs)
                    {
                        m_nQIndex = ash.GetCallbacks().Count;
                    }
                    return ClientCoreLoader.StartJob(m_cs.Handle) != 0;
                }

                public bool EndJob()
                {
                    return ClientCoreLoader.EndJob(m_cs.Handle) != 0;
                }

                public ulong JobSize {
                    get { return ClientCoreLoader.GetJobSize(m_cs.Handle); }
                }

                public ulong RemoveByTTL()
                {
                    return ClientCoreLoader.RemoveQueuedRequestsByTTL(m_cs.Handle);
                }

                public tagQueueStatus QueueStatus {
                    get {
                        return ClientCoreLoader.GetClientQueueStatus(m_cs.Handle);
                    }
                }

                public uint TTL {
                    get {
                        return ClientCoreLoader.GetTTL(m_cs.Handle);
                    }
                }

                public tagOptimistic Optimistic {
                    get {
                        return ClientCoreLoader.GetOptimistic(m_cs.Handle);
                    }
                    set {
                        ClientCoreLoader.SetOptimistic(m_cs.Handle, value);
                    }
                }

                #endregion
            }

            private CClientQueueImpl m_qm;

            public IClientQueue ClientQueue {
                get {
                    return m_qm;
                }
            }

            class CUCertImpl : IUcert
            {
                private CClientSocket m_cs;

                internal CUCertImpl(CClientSocket cs)
                {
                    m_cs = cs;
                    IntPtr p = ClientCoreLoader.GetUCert(cs.Handle);
                    if (p != IntPtr.Zero)
                    {
                        CertInfoIntenal cii = new CertInfoIntenal();
                        System.Runtime.InteropServices.Marshal.PtrToStructure(p, cii);
                        Set(cii);
                    }
                }

                public unsafe override string Verify(out int errCode)
                {
                    int ec = 0;
                    sbyte* str = (sbyte*)ClientCoreLoader.Verify(m_cs.Handle, ref ec);
                    errCode = ec;
                    return new string(str);
                }
            };
            private CUCertImpl m_cert = null;

            public IUcert UCert {
                get {
                    return m_cert;
                }
            }

            public bool Sendable {
                get {
                    return ((ClientCoreLoader.IsOpened(m_h) != 0) || ClientCoreLoader.IsQueueStarted(m_h) != 0);
                }
            }

            class CPushImpl : IUPushClient
            {
                private CClientSocket m_cs;

                internal CPushImpl(CClientSocket cs)
                {
                    m_cs = cs;
                    m_lstPublish = new UDelegate<DOnPublish>(cs.m_cs);
                    m_lstPublishEx = new UDelegate<DOnPublishEx>(cs.m_cs);
                    m_lstSubscribe = new UDelegate<DOnSubscribe>(cs.m_cs);
                    m_lstUnsubscribe = new UDelegate<DOnUnsubscribe>(cs.m_cs);
                    m_lstUser = new UDelegate<DOnSendUserMessage>(cs.m_cs);
                    m_lstUserEx = new UDelegate<DOnSendUserMessageEx>(cs.m_cs);
                }

                #region IUPushClient Members

                private UDelegate<DOnPublish> m_lstPublish;
                public event DOnPublish OnPublish {
                    add {
                        m_lstPublish.add(value);
                    }
                    remove {
                        m_lstPublish.remove(value);
                    }
                }

                private UDelegate<DOnPublishEx> m_lstPublishEx;
                public event DOnPublishEx OnPublishEx {
                    add {
                        m_lstPublishEx.add(value);
                    }
                    remove {
                        m_lstPublishEx.remove(value);
                    }
                }

                private UDelegate<DOnSendUserMessage> m_lstUser;
                public event DOnSendUserMessage OnSendUserMessage {
                    add {
                        m_lstUser.add(value);
                    }
                    remove {
                        m_lstUser.remove(value);
                    }
                }

                private UDelegate<DOnSendUserMessageEx> m_lstUserEx;
                public event DOnSendUserMessageEx OnSendUserMessageEx {
                    add {
                        m_lstUserEx.add(value);
                    }
                    remove {
                        m_lstUserEx.remove(value);
                    }
                }

                private UDelegate<DOnSubscribe> m_lstSubscribe;
                public event DOnSubscribe OnSubscribe {
                    add {
                        m_lstSubscribe.add(value);
                    }
                    remove {
                        m_lstSubscribe.remove(value);
                    }
                }

                private UDelegate<DOnUnsubscribe> m_lstUnsubscribe;
                public event DOnUnsubscribe OnUnsubscribe {
                    add {
                        m_lstUnsubscribe.add(value);
                    }
                    remove {
                        m_lstUnsubscribe.remove(value);
                    }
                }

                #endregion

                #region IUPush Members

                public bool Publish(object Message, uint[] groups)
                {
                    if (groups == null || groups.Length == 0)
                        throw new ArgumentException("Must specify an array of group identification numbers");

                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        byte[] msg = su.Save(Message).m_bytes;
                        unsafe
                        {
                            fixed (byte* message = msg)
                            {
                                IntPtr m = new IntPtr(message);
                                fixed (uint* grps = groups)
                                {
                                    IntPtr g = new IntPtr(grps);
                                    return ClientCoreLoader.Speak(m_cs.Handle, m, su.UQueue.GetSize(), g, groups.Length) != 0;
                                }
                            }
                        }
                    }
                }

                public bool Publish(byte[] Message, uint[] groups)
                {
                    int lenMsg;
                    int lenGroups;

                    if (Message == null)
                        lenMsg = 0;
                    else
                        lenMsg = Message.Length;
                    if (groups == null)
                        lenGroups = 0;
                    else
                        lenGroups = groups.Length;
                    unsafe
                    {
                        fixed (byte* data = Message)
                        {
                            IntPtr m = new IntPtr(data);
                            fixed (uint* gp = groups)
                            {
                                IntPtr g = new IntPtr(gp);
                                return ClientCoreLoader.SpeakEx(m_cs.Handle, m, lenMsg, g, lenGroups) != 0;
                            }
                        }
                    }
                }

                public bool Subscribe(uint[] groups)
                {
                    int len;
                    if (groups == null)
                        len = 0;
                    else
                        len = groups.Length;
                    unsafe
                    {
                        fixed (uint* grps = groups)
                        {
                            IntPtr g = new IntPtr(grps);
                            return ClientCoreLoader.Enter(m_cs.Handle, g, len) != 0;
                        }
                    }
                }

                public bool Unsubscribe()
                {
                    ClientCoreLoader.Exit(m_cs.Handle);
                    return true;
                }

                public bool SendUserMessage(object message, string userId)
                {
                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        byte[] msg = su.Save(message).m_bytes;
                        unsafe
                        {
                            fixed (byte* data = msg)
                            {
                                IntPtr m = new IntPtr(data);
                                return ClientCoreLoader.SendUserMessage(m_cs.Handle, userId, m, su.UQueue.GetSize()) != 0;
                            }
                        }
                    }
                }

                public bool SendUserMessage(string userId, byte[] Message)
                {
                    int len;
                    if (Message == null)
                        len = 0;
                    else
                        len = Message.Length;
                    unsafe
                    {
                        fixed (byte* data = Message)
                        {
                            IntPtr m = new IntPtr(data);
                            return ClientCoreLoader.SendUserMessageEx(m_cs.Handle, userId, m, len) != 0;
                        }
                    }
                }

                internal void OnEnter(IntPtr handler, CMessageSender sender, uint[] group, uint count)
                {
                    lock (m_cs.m_cs)
                    {
                        foreach (var el in m_lstSubscribe)
                        {
                            el.Invoke(m_cs, sender, group);
                        }
                    }
                }

                internal void OnExit(IntPtr handler, CMessageSender sender, uint[] group, uint count)
                {
                    lock (m_cs.m_cs)
                    {
                        foreach (var el in m_lstUnsubscribe)
                        {
                            el.Invoke(m_cs, sender, group);
                        }
                    }
                }

                internal void OnBEx(IntPtr handler, CMessageSender sender, uint[] group, uint count, byte[] message, uint size)
                {
                    lock (m_cs.m_cs)
                    {
                        foreach (var el in m_lstPublishEx)
                        {
                            el.Invoke(m_cs, sender, group, message);
                        }
                    }
                }

                internal void OnB(IntPtr handler, CMessageSender sender, uint[] group, uint count, object vtMessage)
                {
                    lock (m_cs.m_cs)
                    {
                        foreach (var el in m_lstPublish)
                        {
                            el.Invoke(m_cs, sender, group, vtMessage);
                        }
                    }
                }

                internal void OnPUMessage(IntPtr handler, CMessageSender sender, object vtMessage)
                {
                    lock (m_cs.m_cs)
                    {
                        foreach (var el in m_lstUser)
                        {
                            el.Invoke(m_cs, sender, vtMessage);
                        }
                    }
                }

                internal void OnPUMessageEx(IntPtr handler, CMessageSender sender, byte[] msg)
                {
                    lock (m_cs.m_cs)
                    {
                        foreach (var el in m_lstUserEx)
                        {
                            el.Invoke(m_cs, sender, msg);
                        }
                    }
                }

                #endregion
            }

            private CPushImpl m_PushImpl;
            private IntPtr m_h;
        }
    }
}
