/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using USOCKETLib; //you may need it for accessing various constants
using System.Threading;
using System.Collections.Generic;

namespace Suntico
{
    namespace Server
    {
        public delegate long DClientStartTrans(long Clue);
        public delegate long DClientEndTrans(long Clue);
        public delegate void DConfirmed(long Clue);
        public delegate void DConnected(CSunticoPeer SunticoPeer);
        public delegate void DDisconnected(CSunticoPeer SunticoPeer);
        public delegate void DGenericObject(long Clue, SocketProAdapter.CUQueue Queue);

        //server implementation for service CSunticoAsyncHandler
        public class CSunticoPeer : SocketProAdapter.ServerSide.CAdoClientPeer
        {
            private bool m_bDoingTransaction = false;
            private System.Threading.AutoResetEvent m_event = new System.Threading.AutoResetEvent(true);

            private static object m_csDic = new object();
            private static Dictionary<string, CSunticoPeer> m_dicUserPeer = new Dictionary<string, CSunticoPeer>();

            private byte m_ClientConnectionIndex = 0;

            public event DClientStartTrans OnClientStartTrans;
            public event DClientEndTrans OnClientEndTrans;
            public event DDataReader OnClientDataReader;
            public event DDataTable OnClientDataTable;
            public event DDataSet OnClientDataSet;
            public event DStringObject OnClientStringObject;
            public event DGenericObject OnGenericObject;

            /// <summary>
            /// Seek a client connection from a given user id
            /// </summary>
            /// <param name="userId">A case-insensitive user id</param>
            /// <returns>A valid reference returned if found. Otherwise, it returns null</returns>
            public static CSunticoPeer Seek(string userId)
            {
                if (userId == null)
                    userId = "";
                userId = userId.ToLower();
                lock (m_csDic)
                {
                    if (m_dicUserPeer.ContainsKey(userId))
                        return m_dicUserPeer[userId];
                    return null;
                }
            }

            protected override void OnSwitchFrom(int nServiceID)
            {
                int[] groups = { 1 };

                //subscribe event
                Push.Enter(groups);

                m_bDoingTransaction = false;
                m_ClientConnectionIndex = 0;
            }

            private void ClientFakeSlow(byte index)
            {
                string user = UserID.ToLower();
                lock (m_csDic)
                {
                    m_dicUserPeer[user] = this;
                }
                m_ClientConnectionIndex = index;
                CSunticoServer.SunticoServer.OnConnected(this);
            }

            /// <summary>
            /// 0 -- master, 1, 2, ... for backup
            /// </summary>
            public byte ClientConnectionIndex
            {
                get
                {
                    return m_ClientConnectionIndex;
                }
            }

            protected override void OnReleaseResource(bool bClosing, int nInfo)
            {
                if (bClosing)
                {
                    //closing the socket with error code = nInfo
                }
                else
                {
                    //switch to a new service with the service id = nInfo
                }

                string user = UserID.ToLower();
                lock (m_csDic)
                {
                    m_dicUserPeer.Remove(user);
                }

                CSunticoServer.SunticoServer.OnDisconnected(this);

                //release all of your resources here as early as possible
                m_bDoingTransaction = false;
                m_ClientConnectionIndex = 0;

                //clean all of subscribers
                OnClientStartTrans = null;
                OnClientEndTrans = null;
                OnClientDataReader = null;
                OnClientDataTable = null;
                OnClientDataSet = null;
                OnClientStringObject = null;
                OnGenericObject = null;

                m_event.Set();
            }

            private bool IsOk(int res)
            {
                return (res != CClientPeer.REQUEST_CANCELED && res != CClientPeer.SOCKET_NOT_FOUND);
            }

            public bool SendBeginTrans(long Clue = 0)
            {
                bool b;
                if (m_bDoingTransaction)
                    throw new InvalidProgramException("Can not start a new transaction without closing previous transaction");
                m_bDoingTransaction = true;
                b = IsBatching;
                if (!b)
                    b = StartBatching();
                return IsOk(SendResult(Const.idCloudStartTrans, Clue));
            }

            private DConfirmed m_confirm;
            public bool SendEndTrans(DConfirmed confirm, long Clue = 0)
            {
                bool b;
                if (!m_bDoingTransaction)
                    throw new InvalidProgramException("Must start a new transaction first by calling the method CSunticoPeer::SendBeginTrans");
                m_confirm = confirm;
                b = IsOk(SendResult(Const.idCloudEndTrans, Clue));
                m_bDoingTransaction = false;
                if (b)
                {
                    b = IsBatching;
                    if (b)
                        b = CommitBatching(); //push all batched results onto client in one shot
                    m_event.Reset();
                }
                return b;
            }

            public bool SendStringObject(StringObjectType sot, string str)
            {
                if (!m_bDoingTransaction)
                    throw new InvalidProgramException("Must start a new transaction first by calling the method CSunticoPeer::SendBeginTrans");
                return IsOk(SendResult(Const.idCloudSendString, (int)sot, str));
            }

            public bool SendGenericObject<T>(long Clue, T obj) where T : SocketProAdapter.IUSerializer, new()
            {
                if (!m_bDoingTransaction)
                    throw new InvalidProgramException("Must start a new transaction first by calling the method CSunticoPeer::SendBeginTrans");
                return IsOk(SendResult(Const.idCloudSendObject, Clue, obj));
            }

            private void BeginTrans(long data, out long BeginTransRtn)
            {
                if (OnClientStartTrans != null)
                    BeginTransRtn = OnClientStartTrans.Invoke(data);
                else
                    BeginTransRtn = 0;
            }

            private void Commit(long data, out long CommitRtn)
            {
                if (OnClientEndTrans != null)
                    CommitRtn = OnClientEndTrans.Invoke(data);
                else
                    CommitRtn = 0;
            }

            private void ClientSendDataReader()
            {
                System.Data.DataTable dt = m_AdoSerialier.CurrentDataTable;
                if (OnClientDataReader != null)
                    OnClientDataReader.Invoke(dt);
            }

            private void ClientSendDataSet()
            {
                System.Data.DataSet ds = m_AdoSerialier.CurrentDataSet;
                if (OnClientDataSet != null)
                    OnClientDataSet.Invoke(ds);
            }

            private void ClientSendDataTable()
            {
                System.Data.DataTable dt = m_AdoSerialier.CurrentDataTable;
                if (OnClientDataTable != null)
                    OnClientDataTable.Invoke(dt);
            }

            public long Confirmation
            {
                get
                {
                    return m_ClientConfirmation;
                }
            }

            /// <summary>
            /// Wait for confirmation from client. Note that you can call it from your worker threads only
            /// NEVER call it within threads managed by SocketPro server! 
            /// Otherwise, it is a dead lock for either whole system (main thread) or one connection (SocketPro worker thread)
            /// </summary>
            /// <param name="ClientConfirmation">A sequential number from a client point</param>
            /// <param name="timeout">Timeout in ms</param>
            /// <returns>True for successful, and false for failed or timeout</returns>
            public bool WaitConfirmation(ref long ClientConfirmation, int timeout = 30000)
            {
                bool ok = m_event.WaitOne(timeout);
                if (ok)
                    ClientConfirmation = m_ClientConfirmation;
                return ok;
            }

            private long m_ClientConfirmation;
            private void ClientConfirmation(long Clue)
            {
                m_event.Set();
                m_ClientConfirmation = Clue;
                if (m_confirm != null)
                    m_confirm.Invoke(Clue);
            }

            protected override void OnFastRequestArrive(short sRequestID, int nLen)
            {
                int res;
                switch (sRequestID)
                {
                    case CAsyncAdoSerializationHelper.idDataSetHeaderArrive:
                    case CAsyncAdoSerializationHelper.idDataTableHeaderArrive:
                    case CAsyncAdoSerializationHelper.idDataReaderHeaderArrive:
                        base.OnFastRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
                        break;
                    case Const.idClientBeginTrans:
                        res = M_I1_R1<long, long>(BeginTrans);
                        break;
                    case Const.idClientCommit:
                        res = M_I1_R1<long, long>(Commit);
                        break;
                    case Const.idClientConfirmation:
                        res = M_I1_R0<long>(ClientConfirmation);
                        break;
                    default:
                        break;
                }
            }

            private void ClientSendString(string str, int objectType)
            {
                StringObjectType sot = (StringObjectType)objectType;
                if (OnClientStringObject != null)
                    OnClientStringObject.Invoke(sot, str);
            }

            private void ClientSendObject(long Hint)
            {
                if (OnGenericObject != null)
                    OnGenericObject.Invoke(Hint, m_UQueue);
            }

            protected override int OnSlowRequestArrive(short sRequestID, int nLen)
            {
                int res;
                switch (sRequestID)
                {
                    case CAsyncAdoSerializationHelper.idDataReaderRecordsArrive:
                    case CAsyncAdoSerializationHelper.idDataTableRowsArrive:
                        base.OnSlowRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
                        break;
                    case Const.idClientSendObject:
                        res = M_I1_R0<long>(ClientSendObject);
                        break;
                    case Const.idClientSendString:
                        res = M_I2_R0<string, int>(ClientSendString);
                        break;
                    case Const.idClientFakeSlow:
                        res = M_I1_R0<byte>(ClientFakeSlow);
                        break;
                    case CAsyncAdoSerializationHelper.idEndDataTable:
                        base.OnFastRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
                        ClientSendDataTable();
                        break;
                    case CAsyncAdoSerializationHelper.idEndDataReader:
                        base.OnFastRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
                        ClientSendDataReader();
                        break;
                    case CAsyncAdoSerializationHelper.idEndDataSet:
                        base.OnFastRequestArrive(sRequestID, nLen); //chain down to CAdoClientPeer for processing
                        ClientSendDataSet();
                        break;
                    default:
                        break;
                }
                return 0;
            }

        }

        public class CSunticoServer : CSocketProServer
        {
            internal static CSunticoServer SunticoServer;
            public CSunticoServer()
            {
                SunticoServer = this;
            }

            public event DConnected Connected;

            internal void OnConnected(CSunticoPeer p)
            {
                if (Connected != null)
                    Connected.Invoke(p);
            }

            internal void OnDisconnected(CSunticoPeer p)
            {
                if (Disconnected != null)
                    Disconnected.Invoke(p);
            }

            public event DDisconnected Disconnected;

            protected override bool OnSettingServer()
            {
                //try amIntegrated and amMixed instead by yourself
                Config.AuthenticationMethod = tagAuthenticationMethod.amOwn;

                //add service(s) into SocketPro server
                AddServices();
                SetBuiltinChatService();

                //You may set others here

                return true; //true -- ok; false -- no listening server
            }

            protected override void OnAccept(int hSocket, int nError)
            {
                //when a socket is initially established
            }

            protected override bool OnIsPermitted(int hSocket, int nSvsID)
            {
                string userId = CSocketProServer.GetUserID(hSocket);
                Console.WriteLine(userId + " switched to service = " + nSvsID);

                //give permission to all
                return true;
            }

            protected override void OnClose(int hSocket, int nError)
            {


                Console.WriteLine("Socket closed = " + hSocket);
            }

            CSocketProService<CSunticoPeer> m_SunticoComm = new CSocketProService<CSunticoPeer>();
            CSocketProService<CPushPeer> m_OnlineMessenger = new CSocketProService<CPushPeer>();
            //One SocketPro server supports any number of services. You can list them here!

            private void AddServices()
            {
                bool ok;

                //No COM -- taNone; STA COM -- taApartment; and Free COM -- taFree
                ok = m_SunticoComm.AddMe(Const.sidSunticoComm, 0, tagThreadApartment.taNone);
                //If ok is false, very possibly you have two services with the same service id!

                ok = m_SunticoComm.AddSlowRequest(CAsyncAdoSerializationHelper.idDataReaderRecordsArrive);
                ok = m_SunticoComm.AddSlowRequest(CAsyncAdoSerializationHelper.idDataTableRowsArrive);
                ok = m_SunticoComm.AddSlowRequest(CAsyncAdoSerializationHelper.idEndDataReader);
                ok = m_SunticoComm.AddSlowRequest(CAsyncAdoSerializationHelper.idEndDataSet);
                ok = m_SunticoComm.AddSlowRequest(CAsyncAdoSerializationHelper.idEndDataTable);
                ok = m_SunticoComm.AddSlowRequest(Const.idClientSendString);
                ok = m_SunticoComm.AddSlowRequest(Const.idClientSendObject);
                ok = m_SunticoComm.AddSlowRequest(Const.idClientFakeSlow);

                //Add all of other services into SocketPro server here!

                //enable builtin chat service for broadcasting message
                ok = m_OnlineMessenger.AddMe((int)USOCKETLib.tagServiceID.sidChat);
            }

            private void SetBuiltinChatService()
            {
                bool ok = PushManager.AddAChatGroup(1, "All Customers");
            }

        }

    }
}

