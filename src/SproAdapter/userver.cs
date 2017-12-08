using System;
using System.Reflection;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public class CSocketProServer : IDisposable
        {
            private DOnAccept m_onAccept;
            private DOnClose m_onClose;
            private DOnIdle m_onIdle;
            private DOnIsPermitted m_onIsPermitted;
            private DOnSSLHandShakeCompleted m_shc;

            internal static void TE(tagThreadEvent te)
            {
                if (ThreadEvent != null)
                    ThreadEvent.Invoke(te);
            }

            private void Init(int param)
            {
                bool ok = ServerCoreLoader.InitSocketProServer(param);
                m_sps = this;
                m_onAccept = new DOnAccept(OnAccept);
                ServerCoreLoader.SetOnAccept(m_onAccept);
                m_onClose = new DOnClose(OnClose);
                ServerCoreLoader.SetOnClose(m_onClose);
                m_onIdle = new DOnIdle(OnIdle);
                ServerCoreLoader.SetOnIdle(m_onIdle);
                m_onIsPermitted = new DOnIsPermitted(IsPermitted);
                ServerCoreLoader.SetOnIsPermitted(m_onIsPermitted);
                m_shc = new DOnSSLHandShakeCompleted(OnSSLShakeCompleted);
                ServerCoreLoader.SetOnSSLHandShakeCompleted(m_shc);
            }

            public static event DOnThreadEvent ThreadEvent = null;

            /// <summary>
            /// Use the method for debugging crash within cross development environments.
            /// </summary>
            /// <param name="str">A string will be sent to server core library to be output into a crash text file</param>
            public static void SetLastCallInfo(string str)
            {
                if (str == null)
                    str = "";
                unsafe
                {
                    fixed (byte* data = System.Text.Encoding.ASCII.GetBytes(str))
                    {
                        IntPtr p = new IntPtr(data);
                        ServerCoreLoader.SetLastCallInfo(p);
                    }
                }
            }

            protected CSocketProServer(int param)
            {
                if (m_sps != null)
                    throw new InvalidOperationException("SocketPro doesn't allow multiple instances at the same time");
                Init(param);
            }

            protected CSocketProServer()
            {
                if (m_sps != null)
                    throw new InvalidOperationException("SocketPro doesn't allow multiple instances at the same time");
                Init(0);
            }

            protected virtual bool OnSettingServer()
            {
                return true;
            }

            public static bool Running
            {
                get
                {
                    return ServerCoreLoader.IsRunning();
                }
            }

            public static CSocketProServer Server
            {
                get
                {
                    return m_sps;
                }
            }

            public static bool IsMainThread
            {
                get
                {
                    return ServerCoreLoader.IsMainThread();
                }
            }

            public static bool SSLEnabled
            {
                get
                {
                    return ServerCoreLoader.IsServerSSLEnabled();
                }
            }

            public static int LastSocketError
            {
                get
                {
                    return ServerCoreLoader.GetServerErrorCode();
                }
            }

            public static string ErrorMessage
            {
                get
                {
                    sbyte[] msg = new sbyte[4097];
                    unsafe
                    {
                        fixed (sbyte* p = msg)
                        {
                            uint res = ServerCoreLoader.GetServerErrorMessage(p, 4097);
                            return new string(p);
                        }
                    }
                }

            }

            public static ulong RequestCount
            {
                get
                {
                    return ServerCoreLoader.GetRequestCount();
                }
            }

            public static uint CountOfClients
            {
                get
                {
                    return ServerCoreLoader.GetCountOfClients();
                }
            }

            public static uint[] Services
            {
                get
                {
                    uint count = ServerCoreLoader.GetCountOfServices();
                    uint[] svcs = new uint[count];
                    if (count > 0)
                    {
                        unsafe
                        {
                            fixed (uint* p = svcs)
                            {
                                ServerCoreLoader.GetServices(p, count);
                            }
                        }
                    }
                    return svcs;
                }
            }

            public unsafe string Version
            {
                get
                {
                    unsafe
                    {
                        return new string((sbyte*)ServerCoreLoader.GetUServerSocketVersion());
                    }
                }
            }

            public static ulong GetClient(uint index)
            {
                return ServerCoreLoader.GetClient(index);
            }

            public static string LocalName
            {
                get
                {
                    sbyte[] str = new sbyte[256];
                    unsafe
                    {
                        fixed (sbyte* p = str)
                        {
                            uint res = ServerCoreLoader.GetLocalName(p, 256);
                            return new string(p);
                        }
                    }
                }
            }

            protected virtual void OnAccept(ulong hSocket, int nError)
            {

            }

            protected virtual void OnClose(ulong hSocket, int nError)
            {

            }

            private bool IsPermitted(ulong hSocket, uint nSvsID)
            {
                return OnIsPermitted(hSocket, CredentialManager.GetUserID(hSocket), CredentialManager.GetPassword(hSocket), nSvsID);
            }

            protected virtual bool OnIsPermitted(ulong hSocket, string userId, string password, uint nSvsID)
            {
                return true;
            }

            protected virtual void OnIdle(ulong milliseconds)
            {
                if (CScopeUQueue.MemoryConsumed / 1024 > CScopeUQueue.SHARED_BUFFER_CLEAN_SIZE)
                    CScopeUQueue.DestroyUQueuePool();
            }

            protected virtual void OnSSLShakeCompleted(ulong hSocket, int errCode)
            {
            }

            public virtual void PostQuit()
            {
                ServerCoreLoader.PostQuitPump();
            }

            public bool Run(uint port)
            {
                return Run(port, 32, false);
            }

            public bool Run(uint port, uint maxBacklog)
            {
                return Run(port, maxBacklog, false);
            }

            public virtual bool Run(uint port, uint maxBacklog, bool v6Supported)
            {
                if (!OnSettingServer())
                    return false;
                Type type = GetType();
                FieldInfo[] fis = type.GetFields(BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.GetField | BindingFlags.Static);
                foreach (FieldInfo fi in fis)
                {
                    ServiceAttr[] sas = (ServiceAttr[])fi.GetCustomAttributes(typeof(ServiceAttr), true);
                    if (sas != null && sas.Length > 0)
                    {
                        CBaseService bs = (CBaseService)fi.GetValue(this);
                        if (!bs.AddMe(sas[0].ServiceID, sas[0].ThreadApartment))
                            throw new InvalidOperationException("Failed in registering service = " + sas[0].ServiceID + ", and check if the service ID is duplicated with the previous one or if the service ID is less or equal to SocketProAdapter.BaseServiceID.sidReserved");
                    }
                }
                lock (CBaseService.m_csService)
                {
                    if (!CBaseService.m_bRegEvent)
                    {
                        ServerCoreLoader.SetThreadEvent(TE);
                        CBaseService.m_bRegEvent = true;
                    }
                }
                bool ok = ServerCoreLoader.StartSocketProServer(port, maxBacklog, v6Supported);
                CBaseService.m_nMainThreads = uint.MaxValue;
                return ok;
            }

            public virtual void StopSocketProServer()
            {
                Clean();
                ServerCoreLoader.PostQuitPump();
                ServerCoreLoader.StopSocketProServer();
            }

            public void UseSSL(string certFile, string keyFile, string pwdForPrivateKeyFile)
            {
                UseSSL(certFile, keyFile, pwdForPrivateKeyFile, "");
            }

            public void UseSSL(string certFile, string keyFile, string pwdForPrivateKeyFile, string dhFile)
            {
                ServerCoreLoader.SetCertFile(certFile);
                ServerCoreLoader.SetPrivateKeyFile(keyFile);
                ServerCoreLoader.SetPKFPassword(pwdForPrivateKeyFile);
                ServerCoreLoader.SetDHParmsFile(dhFile);
                ServerCoreLoader.SetDefaultEncryptionMethod(tagEncryptionMethod.TLSv1);
            }

            public struct SwitchError
            {
                public const int seERROR_WRONG_SWITCH = 0x7FFFF100;
                public const int seERROR_AUTHENTICATION_FAILED = 0x7FFFF101;
                public const int seERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE = 0x7FFFF102;
                public const int seERROR_NOT_SWITCHED_YET = 0x7FFFF103;
                public const int seERROR_BAD_REQUEST = 0x7FFFF104;
            };

            public struct Config
            {
                public static uint MainThreads
                {
                    get
                    {
                        return ServerCoreLoader.GetMainThreads();
                    }
                }

                public static uint MaxThreadIdleTimeBeforeSuicide
                {
                    get
                    {
                        return ServerCoreLoader.GetMaxThreadIdleTimeBeforeSuicide();
                    }
                    set
                    {
                        ServerCoreLoader.SetMaxThreadIdleTimeBeforeSuicide(value);
                    }
                }

                public static uint MaxConnectionsPerClient
                {
                    get
                    {
                        return ServerCoreLoader.GetMaxConnectionsPerClient();
                    }

                    set
                    {
                        ServerCoreLoader.SetMaxConnectionsPerClient(value);
                    }
                }

                public static uint TimerElapse
                {
                    get
                    {
                        return ServerCoreLoader.GetTimerElapse();
                    }
                    set
                    {
                        ServerCoreLoader.SetTimerElapse(value);
                    }
                }

                public static uint SMInterval
                {
                    get
                    {
                        return ServerCoreLoader.GetSMInterval();
                    }
                    set
                    {
                        ServerCoreLoader.SetSMInterval(value);

                    }
                }

                public static uint PingInterval
                {
                    get
                    {
                        return ServerCoreLoader.GetPingInterval();
                    }
                    set
                    {
                        ServerCoreLoader.SetPingInterval(value);
                    }

                }

                public static bool DefaultZip
                {
                    get
                    {
                        return ServerCoreLoader.GetDefaultZip();
                    }
                    set
                    {
                        ServerCoreLoader.SetDefaultZip(value);
                    }
                }

                static public tagEncryptionMethod DefaultEncryptionMethod
                {
                    get
                    {
                        return ServerCoreLoader.GetDefaultEncryptionMethod();
                    }
                    set
                    {
                        ServerCoreLoader.SetDefaultEncryptionMethod(value);
                    }
                }

                public static uint SwitchTime
                {
                    get
                    {
                        return ServerCoreLoader.GetSwitchTime();
                    }
                    set
                    {
                        ServerCoreLoader.SetSwitchTime(value);
                    }
                }

                public static tagAuthenticationMethod AuthenticationMethod
                {
                    get
                    {
                        return ServerCoreLoader.GetAuthenticationMethod();
                    }
                    set
                    {
                        ServerCoreLoader.SetAuthenticationMethod(value);
                    }
                }

                public static bool SharedAM
                {
                    get
                    {
                        return ServerCoreLoader.GetSharedAM();
                    }
                    set
                    {
                        ServerCoreLoader.SetSharedAM(value);
                    }
                }

                public static string Password
                {
                    set
                    {
                        ServerCoreLoader.SetPKFPassword(value);
                    }
                }
            }

            public struct DllManager
            {
                public static IntPtr AddALibrary(string libFile)
                {
                    return ServerCoreLoader.AddADll(libFile, 0);
                }

                public static IntPtr AddALibrary(string libFile, int param)
                {
                    return ServerCoreLoader.AddADll(libFile, param);
                }

                public static bool RemoveALibrary(IntPtr hLib)
                {
                    return ServerCoreLoader.RemoveADllByHandle(hLib);
                }
            }

            public struct PushManager
            {
                public static bool Publish(byte[] Message, params uint[] Groups)
                {
                    uint size;
                    uint len;
                    if (Groups == null)
                        len = 0;
                    else
                        len = (uint)Groups.Length;
                    if (Message == null)
                        size = 0;
                    else
                        size = (uint)Message.Length;
                    unsafe
                    {
                        fixed (byte* buffer = Message)
                        {
                            fixed (uint* p = Groups)
                            {
                                return ServerCoreLoader.SpeakExPush(buffer, size, p, len);
                            }
                        }
                    }
                }
                public static bool SendUserMessage(string UserId, byte[] Message)
                {
                    uint size;
                    if (Message == null)
                        size = 0;
                    else
                        size = (uint)Message.Length;
                    unsafe
                    {
                        fixed (byte* p = Message)
                        {
                            return ServerCoreLoader.SendUserMessageExPush(UserId, p, size);
                        }
                    }
                }

                public static bool Publish(object Message, params uint[] Groups)
                {
                    uint len;
                    if (Groups == null)
                        len = 0;
                    else
                        len = (uint)Groups.Length;
                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        CUQueue q = su.UQueue;
                        q.Save(Message);
                        unsafe
                        {
                            fixed (byte* buffer = q.m_bytes)
                            {
                                fixed (uint* p = Groups)
                                {
                                    return ServerCoreLoader.SpeakPush(buffer, q.GetSize(), p, len);
                                }
                            }
                        }
                    }
                }

                public static bool SendUserMessage(object Message, string UserId)
                {
                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        CUQueue q = su.UQueue;
                        q.Save(Message);
                        unsafe
                        {
                            fixed (byte* p = q.m_bytes)
                            {
                                return ServerCoreLoader.SendUserMessagePush(UserId, p, q.GetSize());
                            }
                        }
                    }
                }

                public static void AddAChatGroup(uint groupId)
                {
                    ServerCoreLoader.AddAChatGroup(groupId, "");
                }

                public static void AddAChatGroup(uint groupId, string description)
                {
                    ServerCoreLoader.AddAChatGroup(groupId, description);
                }

                public static void RemoveChatGroup(uint chatGroupId)
                {
                    ServerCoreLoader.RemoveChatGroup(chatGroupId);
                }

                public static string GetAChatGroup(uint groupId)
                {
                    char[] des = new char[2048];
                    unsafe
                    {
                        fixed (char* str = des)
                        {
                            ServerCoreLoader.GetAChatGroup(groupId, str, 2048);
                        }
                    }
                    return new string(des);
                }

                public static uint CountOfChatGroups
                {
                    get
                    {
                        return ServerCoreLoader.GetCountOfChatGroups();
                    }
                }

                public static uint[] AllCreatedChatGroups
                {
                    get
                    {
                        uint count = ServerCoreLoader.GetCountOfChatGroups();
                        uint[] grp = new uint[count];
                        if (count > 0)
                        {
                            unsafe
                            {
                                fixed (uint* p = grp)
                                {
                                    ServerCoreLoader.GetAllCreatedChatGroups(p, count);
                                }
                            }
                        }
                        return grp;
                    }
                }
            };

            public struct QueueManager
            {
                public static CServerQueue StartQueue(string qName, uint ttl)
                {
                    return new CServerQueue(ServerCoreLoader.StartQueue(qName, true, ttl));
                }

                public static CServerQueue StartQueue(string qName, uint ttl, bool dequeueShared)
                {
                    return new CServerQueue(ServerCoreLoader.StartQueue(qName, dequeueShared, ttl));
                }

                public static bool StopQueue(string qName)
                {
                    return ServerCoreLoader.StopQueueByName(qName, false);
                }

                public static bool StopQueue(string qName, bool permanent)
                {
                    return ServerCoreLoader.StopQueueByName(qName, permanent);
                }

                public static bool IsQueueStarted(string qName)
                {
                    return ServerCoreLoader.IsQueueStartedByName(qName);
                }

                public static bool IsQueueSecured(string qName)
                {
                    return ServerCoreLoader.IsQueueSecuredByName(qName);
                }

                public static bool IsServerQueueIndexPossiblyCrashed
                {
                    get
                    {
                        return ServerCoreLoader.IsServerQueueIndexPossiblyCrashed();
                    }
                }

                public static string MessageQueuePassword
                {
                    set
                    {
                        ServerCoreLoader.SetMessageQueuePassword(value);
                    }
                }

                public static string WorkDirectory
                {
                    get
                    {
                        return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetServerWorkDirectory());
                    }
                    set
                    {
                        ServerCoreLoader.SetServerWorkDirectory(value);
                    }
                }
            };

            public struct Router
            {
                /// <summary>
                /// Set a route with two given service ids
                /// </summary>
                /// <param name="serviceId0">The first service Id</param>
                /// <param name="serviceId1">The second service id</param>
                /// <returns>True if successful; and false if failed</returns>
                /// <remarks>If any one of the two given service ids does not exist, the route is broken</remarks>
                public static bool SetRouting(uint serviceId0, uint serviceId1)
                {
                    return ServerCoreLoader.SetRouting(serviceId0, tagRoutingAlgorithm.raDefault, serviceId1, tagRoutingAlgorithm.raDefault);
                }

                /// <summary>
                /// Set a route with two given service ids
                /// </summary>
                /// <param name="serviceId0">The first service Id</param>
                /// <param name="ra0">Routing algorithm for serviceId0. It is default to raDefault</param>
                /// <param name="serviceId1">The second service id</param>
                /// <param name="ra1">Routing algorithm for serviceId1. It is default to raDefault</param>
                /// <returns>True if successful; and false if failed</returns>
                /// <remarks>If any one of the two given service ids does not exist, the route is broken</remarks>
                public static bool SetRouting(uint serviceId0, tagRoutingAlgorithm ra0, uint serviceId1, tagRoutingAlgorithm ra1)
                {
                    return ServerCoreLoader.SetRouting(serviceId0, ra0, serviceId1, ra1);
                }

                /// <summary>
                /// Query a routee service id from a given service id
                /// </summary>
                /// <param name="serviceId">A given service id</param>
                /// <returns>A valid routee service id if this service id is valid and set to be routed</returns>
                public static uint CheckRouting(uint serviceId)
                {
                    return ServerCoreLoader.CheckRouting(serviceId);
                }
            };

            public struct CredentialManager
            {
                public static bool HasUserId(string userId)
                {
                    return ServerCoreLoader.HasUserId(userId);
                }

                public static string GetUserID(ulong hSocket)
                {
                    char[] id = new char[256];
                    unsafe
                    {
                        fixed (char* p = id)
                        {
                            uint res = ServerCoreLoader.GetUID(hSocket, p, 256);
                            return new string(p);
                        }
                    }
                }

                public static bool SetUserID(ulong hSocket, string userId)
                {
                    return ServerCoreLoader.SetUserID(hSocket, userId);
                }

                public static string GetPassword(ulong hSocket)
                {
                    char[] pwd = new char[256];
                    unsafe
                    {
                        fixed (char* p = pwd)
                        {
                            uint res = ServerCoreLoader.GetPassword(hSocket, p, 256);
                            return new string(p);
                        }
                    }
                }

                public static bool SetPassword(ulong hSocket, string password)
                {
                    return ServerCoreLoader.SetPassword(hSocket, password);
                }
            };

            #region IDisposable Members

            private static CSocketProServer m_sps = null;

            private void Clean()
            {
                if (m_sps != null)
                {
                    ServerCoreLoader.SetOnAccept(null);
                    ServerCoreLoader.SetOnClose(null);
                    ServerCoreLoader.SetOnIdle(null);
                    ServerCoreLoader.SetOnIsPermitted(null);
                    ServerCoreLoader.SetOnSSLHandShakeCompleted(null);
                    lock (CBaseService.m_csService)
                    {
                        if (CBaseService.m_bRegEvent)
                        {
                            ServerCoreLoader.SetThreadEvent(null);
                            CBaseService.m_bRegEvent = false;
                        }
                    }
                    m_sps = null;
                }
            }

            public void Dispose()
            {
                Clean();
            }

            #endregion
        }

        public class CSocketProService<TPeer> : CBaseService
            where TPeer : CSocketPeer, new()
        {
            protected override CSocketPeer GetPeerSocket()
            {
                return new TPeer();
            }

            private void SetMethods(Type typePeer)
            {
                string metName;
                MethodInfo[] mis = typePeer.GetMethods(BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.DeclaredOnly);
                foreach (MethodInfo mi in mis)
                {
                    RequestAttr[] ras = (RequestAttr[])mi.GetCustomAttributes(typeof(RequestAttr), true);
                    if (ras != null && ras.Length > 0)
                    {
                        if (ras[0].RequestID <= (ushort)SocketProAdapter.tagBaseRequestID.idReservedTwo)
                            throw new InvalidProgramException("The request ID must be larger than ocketProAdapter.tagBaseRequestID.idReservedTwo");
                        if (m_dicMethod.ContainsKey(ras[0].RequestID))
                        {
                            if (mi.IsVirtual)
                                continue;
                            else
                                throw new InvalidProgramException("The request ID (" + ras[0].RequestID.ToString() + ") can not be duplicated within the same service");
                        }
                        uint input = 0;
                        uint output = 0;
                        ParameterInfo[] pis = mi.GetParameters();
                        bool nonVoid = (mi.ReturnType != typeof(void));
                        Type[] gTypes = new Type[pis.Length + (nonVoid ? 1 : 0)];
                        int index = 0;
                        foreach (ParameterInfo pi in pis)
                        {
                            if (pi.IsOut && pi.IsIn)
                                throw new InvalidOperationException("Reference parameter not supported");
                            else if (pi.IsOut || pi.ParameterType.IsByRef)
                            {
                                ++output;
                                string typeName = pi.ParameterType.FullName.Substring(0, pi.ParameterType.FullName.IndexOf('&'));
                                gTypes[index] = Type.GetType(typeName);
                                if (gTypes[index] == null)
                                {
                                    foreach (var a in AppDomain.CurrentDomain.GetAssemblies())
                                    {
                                        Type type = a.GetType(typeName);
                                        if (type != null)
                                        {
                                            gTypes[index] = type;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                ++input;
                                gTypes[index] = pi.ParameterType;
                            }
                            ++index;
                        }

                        if (nonVoid)
                        {
                            gTypes[input + output] = mi.ReturnType;
                            ++output;
                        }

                        if (input > 10)
                            throw new InvalidProgramException("The count of inputs can not be over 10");

                        if (output > 5)
                            throw new InvalidProgramException("The count of outputs, including its return type of data, can not be over 5");

                        if (nonVoid)
                            metName = string.Format("RM_I{0}_R{1}", input, output);
                        else
                            metName = string.Format("M_I{0}_R{1}", input, output);

                        MethodInfo gmiGeneric;
                        MethodInfo gmi = typeof(CClientPeer).GetMethod(metName, BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.ExactBinding);
                        if (output + input > 0)
                            gmiGeneric = gmi.MakeGenericMethod(gTypes);
                        else
                            gmiGeneric = gmi;
                        m_dicMethod.Add(ras[0].RequestID, gmiGeneric);

                        //register a slow request
                        if (ras[0].SlowRequest)
                        {
                            AddSlowRequest(ras[0].RequestID);
                        }
                    }
                }
            }

            public override bool AddMe(uint svsId, tagThreadApartment ta)
            {
                m_dicMethod.Clear();
                if (base.AddMe(svsId, ta))
                {
                    if (svsId == BaseServiceID.sidHTTP)
                    {
                        bool ok = AddSlowRequest((ushort)tagHttpRequestID.idPost);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idGet);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idConnect);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idHead);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idMultiPart);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idOptions);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idPut);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idTrace);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idUserRequest);
                        ok = AddSlowRequest((ushort)tagHttpRequestID.idDelete);
                    }
                    else
                    {
                        Type typePeer = typeof(TPeer);
                        while (typePeer != null)
                        {
                            SetMethods(typePeer);
                            typePeer = typePeer.BaseType;
                        }
                    }
                    return true;
                }
                return false;
            }
        }
    }
}
