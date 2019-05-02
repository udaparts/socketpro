using System;
using System.Runtime.InteropServices;
using Microsoft.SqlServer.Server;

namespace SpMSSqlPush
{
    [StructLayout(LayoutKind.Sequential)]
    struct POINT
    {
        public int X;
        public int Y;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct MSG
    {
        public IntPtr hwnd;
        public uint message;
        public IntPtr wParam;
        public IntPtr lParam;
        public uint time;
        public POINT pt;
    }

    /// <summary>
    /// An interface to a connection context for accessing a remote SocketPro server.
    /// </summary>
    interface IConnectionContext
    {
        /// <summary>
        /// An IP address to a remote SocketPro server like '111.222.212.121', 'www.mydomain.com', or 'MyServerName'
        /// </summary>
        string   IpAddress { get; set; }
        /// <summary>
        /// A remote SocketPro server port number.
        /// </summary>
        int Port { get; set; }

        /// <summary>
        /// A case in-sensitive user id string.
        /// </summary>
        string UserId { get; set; }

        /// <summary>
        /// A case sensitive password string.
        /// </summary>
        string Password { get;  set; }

        /// <summary>
        /// Enable or disable online compression. It defaults to false.
        /// </summary>
        bool Zip { get; set; }

        /// <summary>
        /// Encryption method used for data transaction between a remote SocketPro server and SQL server. It defaults to no encryption.
        /// </summary>
        USOCKETLib.tagEncryptionMethod EncryptionMethod { get; set; }

        bool Strict { get; set;}

        string Verify { get; set;}

        int VerifyCode { get; set;}
    }

    /// <summary>
    /// A sealed class exposing two static methods for sending messages through SocketPro.
    /// </summary>
    public sealed class Messenger
    {
        private Messenger()
        {
        }

        private class ConnectionContextImpl : IConnectionContext
        {
            bool m_bStrict = false;
            string m_strLocation = "";
            int m_nVerifyCode = 0;
            string m_strIpAddr = "";
            string m_strUserId = "";
            string m_strPassword = "";
            int m_nPort = 0;
            bool m_bZip = false;
            USOCKETLib.tagEncryptionMethod m_EncryptionMethod = USOCKETLib.tagEncryptionMethod.NoEncryption;

            public bool Strict { get { return m_bStrict; } set { m_bStrict = value; } }

            public string Verify { get { return m_strLocation; } set { m_strLocation = value; } }

            public int VerifyCode { get { return m_nVerifyCode; } set { m_nVerifyCode = value; } }

            public string IpAddress
            {
                get
                {
                    return m_strIpAddr;
                }
                set
                {
                    m_strIpAddr = "";
                    if(value != null)
                        m_strIpAddr = value;
                }
            }

            public int Port
            {
                get
                {
                    return m_nPort;
                }
                set
                {
                    m_nPort = value;
                }
            }

            public string UserId
            {
                get
                {
                   return m_strUserId;
                }
                set
                {
                    m_strUserId = "";
                    if(value != null)
                        m_strUserId = value;
                }
            }

            public string Password
            {
                get
                {
                    return m_strPassword;
                }
                set { 
                    m_strPassword = "";
                    if (value != null)
                        m_strPassword = value;
                }
            }

            public bool Zip
            {
                get
                {
                    return m_bZip;
                }
                set
                {
                    m_bZip = value;
                }
            }

            public USOCKETLib.tagEncryptionMethod EncryptionMethod
            {
                get
                {
                    return m_EncryptionMethod;
                }
                set
                {
                    m_EncryptionMethod = value;
                }
            }
        }
        private static IConnectionContext m_cc = new ConnectionContextImpl();
        private static USOCKETLib.USocketClass m_ClientSocket = null;
        private static System.Threading.AutoResetEvent m_event = new System.Threading.AutoResetEvent(false);
        private static object m_locker = new object();
        private static System.Threading.Thread m_thread = null;
        private static uint m_dwUIThreadId = 0;

        [return: MarshalAs(UnmanagedType.Bool)]
        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool PostThreadMessage(uint threadId, uint msg, UIntPtr wParam, IntPtr lParam);

        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        [DllImport("kernel32.dll")]
        private static extern uint GetCurrentThreadId();

        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool GetMessage(out MSG lpMsg, IntPtr hWnd, uint wMsgFilterMin,
           uint wMsgFilterMax);

        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        [DllImport("user32.dll")]
        static extern IntPtr DispatchMessage([In] ref MSG lpmsg);


        static Messenger()
        { 
        }

        /// <summary>
        /// A structure of connection context for accessing a remote SocketPro server
        /// </summary>
        //private static IConnectionContext ConnectionContext
        //{
        //    get { return m_cc; }
        //}

        private const uint MSG_CONNECT_SOCKET = 0x450;

        private static void SetConnectionString(string strConnection)
        {
            //if there is no connection string available, use an existing one instead
            if (strConnection == null)
                return;
            strConnection = strConnection.Trim();
            if (strConnection.Length == 0)
                return;
            ConnectionContextImpl cc = new ConnectionContextImpl();
            System.Collections.Generic.Dictionary<string, string> dic = new System.Collections.Generic.Dictionary<string, string>();
            do
            {
                if (strConnection == null)
                    break;
                string[] lst = strConnection.Split(';');
                foreach (string str in lst)
                {
                    string[] pair = str.Split('=');
                    if (pair.Length != 2)
                        continue;
                    dic.Add(pair[0].Trim().ToLower(), pair[1].Trim());
                }
                if (dic.ContainsKey("host"))
                    cc.IpAddress = dic["host"].ToLower();
                if (dic.ContainsKey("server"))
                    cc.IpAddress = dic["server"].ToLower();
                if (dic.ContainsKey("uid"))
                    cc.UserId = dic["uid"].ToLower(); ;
                if (dic.ContainsKey("userid"))
                    cc.UserId = dic["userid"].ToLower(); ;
                if (dic.ContainsKey("pwd"))
                    cc.Password = dic["pwd"];
                if (dic.ContainsKey("password"))
                    cc.Password = dic["password"];
                if (dic.ContainsKey("port"))
                {
                    try
                    {
                        cc.Port = int.Parse(dic["port"]);
                    }
                    catch { }
                }

                if (dic.ContainsKey("zip"))
                {
                    try
                    {
                        cc.Zip = (int.Parse(dic["zip"]) != 0);
                    }
                    catch
                    {
                        try
                        {
                            cc.Zip = bool.Parse(dic["zip"]);
                        }
                        catch { }
                    }
                }

                if (dic.ContainsKey("encryption"))
                {
                    try
                    {
                        int nMethod = int.Parse(dic["encryption"]);
                        cc.EncryptionMethod = (USOCKETLib.tagEncryptionMethod)nMethod;
                    }
                    catch { }
                }

                if (cc.EncryptionMethod != USOCKETLib.tagEncryptionMethod.NoEncryption)
                {
                    if (dic.ContainsKey("strict"))
                    {
                        try
                        {
                            int strict = int.Parse(dic["strict"]);
                            cc.Strict = (strict > 0);
                        }
                        catch { }
                    }

                    if (dic.ContainsKey("verify"))
                    {
                        cc.Verify = dic["verify"];
                    }
                }
            } while (false);

            lock (m_locker)
            {
                if (m_ClientSocket.Socket <= 0)
                    m_cc = cc;
                else if (m_cc.EncryptionMethod != cc.EncryptionMethod ||
                    m_cc.IpAddress != cc.IpAddress ||
                    m_cc.Port != cc.Port ||
                    m_cc.UserId != cc.UserId ||
                    m_cc.Password != cc.Password)
                {
                    //disconnect old one, and use new connection context instead
                    m_ClientSocket.Disconnect();
                    m_cc = cc;
                }
                else if(m_cc.Zip != cc.Zip)
                {
                    m_ClientSocket.ZipIsOn = cc.Zip;
                    m_ClientSocket.TurnOnZipAtSvr(cc.Zip);
                    m_cc = cc;
                }
            }
        }

        [System.MTAThread]
        private static void StartUIThread()
        {
            MSG msg;
            m_dwUIThreadId = GetCurrentThreadId();
            try
            {
                m_ClientSocket = new USOCKETLib.USocketClass();
                m_ClientSocket.ConnTimeout = 1000;
                m_ClientSocket.OnSocketClosed += new USOCKETLib._IUSocketEvent_OnSocketClosedEventHandler(m_ClientSocket_OnSocketClosed);
                m_ClientSocket.OnSocketConnected += new USOCKETLib._IUSocketEvent_OnSocketConnectedEventHandler(m_ClientSocket_OnSocketConnected);
                m_ClientSocket.OnRequestProcessed += new USOCKETLib._IUSocketEvent_OnRequestProcessedEventHandler(m_ClientSocket_OnRequestProcessed);
            }
            catch {
                //Exception may happen because there is no proper version of npUSocket.dll available.
                m_ClientSocket = null; 
            }
            bool bSuc = m_event.Set();
            if (m_ClientSocket == null)
                return;
            while(GetMessage(out msg, IntPtr.Zero, 0, 0))
            {
                switch (msg.message)
                {
                    case MSG_CONNECT_SOCKET:
                        m_ClientSocket.EncryptionMethod = (short)m_cc.EncryptionMethod;
                        m_ClientSocket.ZipIsOn = m_cc.Zip;
                        m_ClientSocket.Disconnect();
                        m_ClientSocket.ConnTimeout = 60000; //60 seconds only for connection time-out
                        m_ClientSocket.RecvTimeout = 1000;
                        try
                        {
                            m_ClientSocket.Connect(m_cc.IpAddress,
                                m_cc.Port,
                                false, //connect to a remote SocketPro asynchronously
                                0,
                                0,
                                0,
                                0);
                        }
                        catch {
                            bSuc = m_event.Set(); 
                        }
                        break;
                    default:
                        do
                        {
                            if (msg.message >= 1921)
                                break;
                            if (m_ClientSocket.Socket <= 0)
                                break;
                            m_ClientSocket.DoEcho();
                        } while (false);
                        break;
                }
                DispatchMessage(ref msg);
            }
        }

        static void m_ClientSocket_OnSocketClosed(int hSocket, int lError)
        {
            m_event.Set();
        }

        static void m_ClientSocket_OnRequestProcessed(int hSocket, short nRequestID, int lLen, int lLenInBuffer, short sFlag)
        {
            if (sFlag == (short)USOCKETLib.tagReturnFlag.rfCompleted && nRequestID == (short)USOCKETLib.tagBaseRequestID.idSwitchTo)
            {
                m_event.Set();
            }
        }

        static void m_ClientSocket_OnSocketConnected(int hSocket, int lError)
        {
            if (lError == 0)
            {
                if (m_ClientSocket.EncryptionMethod != (short)USOCKETLib.tagEncryptionMethod.NoEncryption)
                {
                    bool ok = false;
                    do
                    {
                        USOCKETLib.IUCert Cert = (USOCKETLib.IUCert)m_ClientSocket.PeerCertificate;
                        if (Cert == null)
                        {
                            m_cc.VerifyCode = 7;
                            break;
                        }

                        if (m_cc.Verify.Length > 0)
                            Cert.VerifyLocation = m_cc.Verify;
                        int res = 0;
                        try
                        {
                            string strErr = Cert.Verify(out res);
                        }
                        catch(COMException err)
                        {
                            m_cc.VerifyCode = 8;
                            break;
                        }

                        if (m_cc.Verify.Length > 0 && res < 0 && Cert.Subject.ToLower().IndexOf(m_cc.Verify) == -1)
                        {
                            m_cc.VerifyCode = 8;
                            break;
                        }

                        if (res != 0 && m_cc.Strict)
                        {
                            m_cc.VerifyCode = 9;
                            break;
                        }
                        ok = true;
                    } while (false);
                    if (!ok)
                    {
                        m_ClientSocket.Disconnect();
                        return;
                    }
                }

                m_ClientSocket.UserID = m_cc.UserId;
                m_ClientSocket.Password = m_cc.Password;
                m_ClientSocket.SetSockOpt((int)USOCKETLib.tagSocketOption.soSndBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);
                m_ClientSocket.SetSockOpt((int)USOCKETLib.tagSocketOption.soRcvBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);
                m_ClientSocket.SetSockOpt((int)USOCKETLib.tagSocketOption.soTcpNoDelay, 1, (int)USOCKETLib.tagSocketLevel.slTcp);
                m_ClientSocket.StartBatching();
                m_ClientSocket.SwitchTo((int)USOCKETLib.tagServiceID.sidChat);
                m_ClientSocket.TurnOnZipAtSvr(m_cc.Zip);
                m_ClientSocket.SetSockOptAtSvr((int)USOCKETLib.tagSocketOption.soSndBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);
                m_ClientSocket.SetSockOptAtSvr((int)USOCKETLib.tagSocketOption.soRcvBuf, 116800, (int)USOCKETLib.tagSocketLevel.slSocket);
                m_ClientSocket.CommitBatching(false);
            } 
        }

        private static int StartUIThreadInternally()
        {
            lock (m_locker)
            {
                if (m_thread == null)
                {
                    //start a UI thread for hosting USocket COM object
                    m_thread = new System.Threading.Thread(new System.Threading.ThreadStart(StartUIThread));
                    m_thread.Start();
                    m_event.WaitOne();
                }
                if (m_thread == null)
                    return 6;
                if (m_ClientSocket == null)
                {
                    m_thread = null;
                    return 5;
                }
            }
            return 0;
        }

        private static int DoConnection()
        {
            bool bSuc;
            int nIndex = 0;
            bool bSocketAvailable = false;
            lock (m_locker)
            {
                if (m_ClientSocket.Socket <= 0)
                {
                    bSuc = m_event.Reset();
                    while (!PostThreadMessage(m_dwUIThreadId, MSG_CONNECT_SOCKET, UIntPtr.Zero, IntPtr.Zero))
                    {
                        System.Threading.Thread.Sleep(1);
                        nIndex++;
                        if (nIndex > 5)
                            break;
                    }
                    if (nIndex > 5)
                        return 4;
                    bSocketAvailable = ((bSuc = m_event.WaitOne(61000)) && m_ClientSocket.Rtn == 0 && m_ClientSocket.Socket > 0);
                }
                else
                    bSocketAvailable = true;

                if (m_cc.VerifyCode != 0)
                    return m_cc.VerifyCode;
            }
            return bSocketAvailable ? 0 : 1;
        }

        /// <summary>
        /// Send a message onto one given user.
        /// </summary>
        /// <param name="Message">A message.</param>
        /// <param name="UserId">A case-insensitive user id.</param>
        /// <returns>If successfull, it returns 0. Otherwise, there is an error. 
        /// 1 -- no socekt available; 2 -- Failed in sending a message even though Socket is available;
        /// 3 -- Invalid user id; 4 -- System error; 5 -- No COM usocket available; and 6 -- No UI worker thread available.
        /// </returns>
        [SqlFunction(Name = "SendUserMessage")]
        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        public static int SendUserMessage(string Message, string UserId)
        {
            if (UserId == null)
                return 3;
            UserId = UserId.Trim();
            if (UserId.Length == 0)
                return 3;
            try
            {
                lock (m_locker)
                {
                    if(IsConnected())
                        m_ClientSocket.SendUserMessage(UserId, Message);
                    else
                        return 1;
                }
            }
            catch
            {
                return 2;
            }
            return 0;
        }

        /// <summary>
        /// Check if there is a connection to a remote SocketPro server available.
        /// </summary>
        /// <returns>True if connected. Otherwise, it is false.</returns>
        [SqlFunction(Name = "IsConnected")]
        public static bool IsConnected()
        {
            lock (m_locker)
            {
                if (m_ClientSocket == null)
                    return false;
                return (m_ClientSocket.Socket > 0);
            }
        }


        /// <summary>
        /// Build a connection to a given remote SocketPro server.
        /// </summary>
        /// <param name="ConnectionString">A connection string to a remote SocketPro server like 'server=myserver;port=20901;pwd=PassOne;uid=SocketPro;zip=0;encryption=0'. If the connection string is null or empty, a previous connection string will be used.</param>
        /// <returns>If successfull, it returns 0. Otherwise, there is an error. 
        /// 1 -- no socekt available; 2 -- Failed in sending a message even though Socket is available;
        /// 3 -- No group id available; 4 -- System error; 5 -- No COM usocket available; and 6 -- No UI worker thread available.
        /// </returns>
        [SqlFunction(Name = "Connect")]
        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        public static int Connect(string ConnectionString)
        {
            lock (m_locker)
            {
                int res = StartUIThreadInternally();
                if (res != 0)
                    return res;
                SetConnectionString(ConnectionString);
                if (IsConnected())
                    return 0;
               
                return DoConnection();
            }
        }

        /// <summary>
        /// Close a connection to a remote SocketPro server.
        /// </summary>
        /// <returns>It always returns zero.</returns>
        [SqlFunction(Name = "Disconnect")]
        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        public static int Disconnect()
        {
            lock (m_locker)
            {
                if (m_ClientSocket == null)
                    return 0;
                m_ClientSocket.Disconnect();
                return 0;
            }
        }

        /// <summary>
        /// Send a message onto one or more groups of clients.
        /// </summary>
        /// <param name="Message">A message.</param>
        /// <param name="Groups">An arry of chat group ids like '1,2,3,15,......'.</param>
        /// <returns>If successfull, it returns 0. Otherwise, there is an error. 
        /// 1 -- no socekt available; 2 -- Failed in sending a message even though Socket is available;
        /// 3 -- No group id available; 4 -- System error; 5 -- No COM usocket available; and 6 -- No UI worker thread available.
        /// </returns>
        [SqlFunction(Name = "Notify")]
        //[System.Security.Permissions.PermissionSet(System.Security.Permissions.SecurityAction.Assert, Unrestricted = true)]
        public static int Notify(string Message, string Groups)
        {
            if (Groups == null || Groups.Length == 0)
                return 3;
            Groups = Groups.Trim(' ', '}', '{', '[', ']', '(', ')');
            System.Collections.Generic.List<int> lstGroups = new System.Collections.Generic.List<int>();
            string[] lstNumber = Groups.Split(';', ',', '|', ':');
            foreach (string str in lstNumber)
            {
                try
                {
                    lstGroups.Add(int.Parse(str));
                }
                catch { }
            }
            int[] myGroups = lstGroups.ToArray();
            if (myGroups.Length == 0)
                return 3;
            try
            {
                lock (m_locker)
                {
                    if (IsConnected())
                        m_ClientSocket.XSpeak(Message, myGroups);
                    else
                        return 1;
                }
            }
            catch
            {
                return 2;
            }
            return 0;
        }
    };
}
