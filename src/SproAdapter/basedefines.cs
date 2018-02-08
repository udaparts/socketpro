using System;
using System.Runtime.InteropServices;

namespace SocketProAdapter
{
    public enum tagZipLevel : int
    {
        zlDefault = 0,
        zlBestSpeed = 1,
        zlBestCompression = 2
    }

    public enum tagSocketOption : int
    {
        soTcpNoDelay = 1,
        soReuseAddr = 4,
        soKeepAlive = 8,
        soSndBuf = 0x1001, /* send buffer size */
        soRcvBuf = 0x1002, /* receive buffer size */
    }

    public enum tagSocketLevel : int
    {
        slTcp = 6,
        slSocket = 0xFFFF,
    }

    public enum tagOperationSystem : int
    {
        osWin = 0,
        osApple = 1,
        osMac = osApple,
        osUnix = 2,
        osLinux = osUnix,
        osBSD = osUnix,
        osAndroid = 3,
        osWinCE = 4, /**< Old window pocket pc, ce or smart phone devices*/
        osWinPhone = osWinCE,
    }

    struct Defines
    {
        public static tagOperationSystem OperationSystem;

        static Defines()
        {
            switch (System.Environment.OSVersion.Platform)
            {
                case PlatformID.Win32S:
                case PlatformID.Win32NT:
                case PlatformID.Win32Windows:
                case PlatformID.Xbox:
                    OperationSystem = tagOperationSystem.osWin;
                    break;
                case PlatformID.WinCE:
                    OperationSystem = tagOperationSystem.osWinCE;
                    break;
                case PlatformID.Unix:
                    OperationSystem = tagOperationSystem.osUnix;
                    break;
#if WINCE
#else
                case PlatformID.MacOSX:
                    OperationSystem = tagOperationSystem.osMac;
                    break;
#endif
                default:
                    throw new NotSupportedException("Platform not supported");
            }
        }
    }

    public enum tagThreadApartment
    {
        /// no COM apartment involved
        taNone = 0,

        /// STA apartment
        taApartment = 1,

        /// MTA (free) or neutral apartments
        taFree = 2,
    }

    public enum tagBaseRequestID : ushort
    {
        idUnknown = 0,
        idSwitchTo = 1,
        idRouteeChanged = (idSwitchTo + 1),
        idEncrypted = (idRouteeChanged + 1),
        idBatchZipped = (idEncrypted + 1),
        idCancel = (idBatchZipped + 1),
        idGetSockOptAtSvr = (idCancel + 1),
        idSetSockOptAtSvr = (idGetSockOptAtSvr + 1),
        idDoEcho = (idSetSockOptAtSvr + 1),
        idTurnOnZipAtSvr = (idDoEcho + 1),
        idStartBatching = (idTurnOnZipAtSvr + 1),
        idCommitBatching = (idStartBatching + 1),
        idShrinkMemoryAtSvr = (idCommitBatching + 1),
        idSetRouting = (idShrinkMemoryAtSvr + 1),
        idPing = (idSetRouting + 1),
        idEnableClientDequeue = (idPing + 1),
        idServerException = (idEnableClientDequeue + 1),
        idAllMessagesDequeued = (idServerException + 1),
        idHttpClose = (idAllMessagesDequeued + 1), //SocketPro HTTP Close
        idSetZipLevelAtSvr = (idHttpClose + 1),
        idStartJob = (idSetZipLevelAtSvr + 1),
        idEndJob = (idStartJob + 1),
        idRoutingData = (idEndJob + 1),
        idDequeueConfirmed = (idRoutingData + 1),
        idMessageQueued = (idDequeueConfirmed + 1),
        idStartQueue = (idMessageQueued + 1),
        idStopQueue = (idStartQueue + 1),
        idRoutePeerUnavailable = (idStopQueue + 1),

        idReservedOne = 0x100,
        idReservedTwo = 0x2001
    }

    public enum tagChatRequestID : ushort
    {
        idEnter = 65,
        idSpeak = 66,
        idSpeakEx = 67,
        idExit = 68,
        idSendUserMessage = 69,
        idSendUserMessageEx = 70,
    }

    public static class BaseServiceID
    {
        public const uint sidReserved1 = 1;
        public const uint sidStartup = 0x100;
        public const uint sidChat = (sidStartup + 1);
        public const uint sidHTTP = (sidChat + 1);
        public const uint sidFile = (sidHTTP + 1);
        public const uint sidODBC = (sidFile + 1);
        public const uint sidReserved = 0x10000000;
        public const uint sidQueue = (sidReserved + 0xEFFF0000);
    }

    public static class BaseExceptionCode
    {
        public const uint becBAD_DESERIALIZATION = 0xAAAA0000;
        public const uint becSERIALIZATION_NOT_SUPPORTED = 0xAAAA0001;
        public const uint becBAD_OPERATION = 0xAAAA0002;
        public const uint becBAD_INPUT = 0xAAAA0003;
        public const uint becNOT_SUPPORTED = 0xAAAA0004;
        public const uint becSTL_EXCEPTION = 0xAAAA0005;
        public const uint becUNKNOWN_EXCEPTION = 0xAAAA0006;
        public const uint becQUEUE_FILE_NOT_AVAILABLE = 0xAAAA0007;
        public const uint becALREADY_DEQUEUED = 0xAAAA0008;
        public const uint becROUTEE_DISCONNECTED = 0xAAAA0009;
    }

    public enum tagEncryptionMethod : int
    {
        NoEncryption = 0,
        TLSv1 = 1,
    }

    public enum tagShutdownType : int
    {
        stReceive = 0,
        stSend = 1,
        stBoth = 2
    }

    public enum tagQueueStatus : int
    {
        /// <summary>
        /// everything is fine
        /// </summary>
        qsNormal = 0,

        /// <summary>
        /// Queued messages merged completely
        /// </summary>
        qsMergeComplete = 1,

        /// <summary>
        /// Message replication started but not completed yet
        /// </summary>
        qsMergePushing = 2,

        /// <summary>
        /// Message replicated incompletely from a source queue
        /// </summary>
        qsMergeIncomplete = 3,

        /// <summary>
        /// A set of messages as a job are incompletely queued 
        /// </summary>
        qsJobIncomplete = 4,

        /// <summary>
        /// A message queued incompletely because of application crash or unexpected termination
        /// </summary>
        qsCrash = 5,

        /// <summary>
        /// Queue file open error
        /// </summary>
        qsFileError = 6,

        /// <summary>
        /// Queue file opened but can not decrypt existing queued messages beacuse of bad password found
        /// </summary>
        qsBadPassword = 7,

        /// <summary>
        /// Duplicate name error
        /// </summary>
        qsDuplicateName = 8,
    }

    public enum tagVariantDataType : ushort
    {
        sdVT_EMPTY = 0,
        sdVT_NULL = 1,
        sdVT_I2 = 2,
        sdVT_I4 = 3,
        sdVT_R4 = 4,
        sdVT_R8 = 5,
        sdVT_CY = 6,
        sdVT_BSTR = 8,
        sdVT_DATE = 7,
        sdVT_BOOL = 11,
        sdVT_VARIANT = 12,
        sdVT_DECIMAL = 14,
        sdVT_I1 = 16,
        sdVT_UI1 = 17,
        sdVT_UI2 = 18,
        sdVT_UI4 = 19,
        sdVT_I8 = 20,
        sdVT_UI8 = 21,
        sdVT_INT = 22,
        sdVT_UINT = 23,
        sdVT_XML = 35,
        sdVT_FILETIME = 64,
        sdVT_CLSID = 72,
        sdVT_BYTES = 128,
        sdVT_STR = 129,
        sdVT_WSTR = 130,
        sdVT_USERIALIZER_OBJECT = 0xD00,
        sdVT_NETObject = 0xE00,
        sdVT_TIMESPAN = 0xC00,
        sdVT_DATETIMEOFFSET = 0xB00,
        sdVT_ARRAY = 0x2000,
    }

    public enum tagOptimistic
    {
        oMemoryCached = 0,
        oSystemMemoryCached = 1,
        oDiskCommitted = 2
    }

    public interface IMessageQueueBasic
    {
        /// <summary>
        /// Stop message queue without removing message queue file
        /// </summary>
        void StopQueue();

        /// <summary>
        /// Stop message queue
        /// </summary>
        /// <param name="permanent">A boolean value to determine if the message queue file is permanently removed</param>
        void StopQueue(bool permanent);

        /// <summary>
        /// Remove queued messages according to given message indexes
        /// </summary>
        /// <param name="startIndex">A start index</param>
        /// <param name="endIndex">An end index</param>
        /// <returns>The number of messages removed</returns>
        ulong CancelQueuedMessages(ulong startIndex, ulong endIndex);

        /// <summary>
        /// Remove messages according to time-to-live
        /// </summary>
        /// <returns>The number of messages removed</returns>
        ulong RemoveByTTL();

        /// <summary>
        /// Abort current transaction messages
        /// </summary>
        /// <returns>True if successful; and false if failed</returns>
        bool AbortJob();

        /// <summary>
        /// Start a message transaction
        /// </summary>
        /// <returns>True if successful; and false if failed</returns>
        bool StartJob();

        /// <summary>
        /// Commit a message transaction
        /// </summary>
        /// <returns>True if successful; and false if failed</returns>
        bool EndJob();

        /// <summary>
        /// Discard all of persistent messages
        /// </summary>
        void Reset();

        /// <summary>
        /// The number of messages during dequeuing
        /// </summary>
        uint MessagesInDequeuing { get; }
        ulong MessageCount { get; }
        ulong QueueSize { get; }
        bool Available { get; }
        bool Secure { get; }
        string QueueFileName { get; }
        string QueueName { get; }

        /// <summary>
        /// The size of messages within a transaction
        /// </summary>
        ulong JobSize { get; }

        /// <summary>
        /// A boolean value indicating if the message queue is able to be dequeued among multiple sessions
        /// </summary>
        bool DequeueShared { get; }
        ulong LastIndex { get; }

        /// <summary>
        /// A status value for message queue opened
        /// </summary>
        tagQueueStatus QueueStatus { get; }

        /// <summary>
        /// A time-to-live number in seconds
        /// </summary>
        uint TTL { get; }

        /// <summary>
        /// Date time for queued last message
        /// </summary>
        DateTime LastMessageTime { get; }

        /// <summary>
        /// A value for how to flush message writing into hard disk. It defaults to the value oSystemMemoryCached.
        /// If the property is set to oSystemMemorycached or oDiskCommitted, queue file stream is immediately flushed into system memory or hard disk, respectively. Otherwise, queue file stream may be flushed with delay.
        /// Queue file stream will be flushed into system memory or hard disk whenever setting the property to oSystemMemorycached or oDiskCommitted.
        /// </summary>
        tagOptimistic Optimistic { get; set; }
    }

#if USQLSERVER

#else

    namespace ClientSide
    {
        public interface IClientQueue : IMessageQueueBasic
        {
            /// <summary>
            /// Open a persistent file for a message queue
            /// </summary>
            /// <param name="qName">Message queue name or a full path to message queue file</param>
            /// <param name="ttl">Time-to-live in seconds</param>
            /// <returns>True if successful and false if failed</returns>
            /// <remarks>To reopen an existing secure message queue file, the method may fail if current password is different from original one. There are a number of situations leading the failures of this method</remarks>
            bool StartQueue(string qName, uint ttl);

            /// <summary>
            /// Open a persistent file for a message queue
            /// </summary>
            /// <param name="qName">Message queue name or a full path to message queue file</param>
            /// <param name="ttl">Time-to-live in seconds</param>
            /// <param name="secure">A boolean value default to true to indicate if queued messages should be encrypted by password</param>
            /// <returns>True if successful and false if failed</returns>
            /// <remarks>To reopen an existing secure message queue file, the method may fail if current password is different from original one. There are a number of situations leading the failures of this method</remarks>
            bool StartQueue(string qName, uint ttl, bool secure);

            /// <summary>
            /// Open a persistent file for a message queue
            /// </summary>
            /// <param name="qName">Message queue name or a full path to message queue file</param>
            /// <param name="ttl">Time-to-live in seconds</param>
            /// <param name="secure">A boolean value default to true to indicate if queued messages should be encrypted by password</param>
            /// <param name="dequeueShared">A boolean value default to false to indicate if there are two or more sessions to dequeue messages</param>
            /// <returns>True if successful and false if failed</returns>
            /// <remarks>To reopen an existing secure message queue file, the method may fail if current password is different from original one. There are a number of situations leading the failures of this method</remarks>
            bool StartQueue(string qName, uint ttl, bool secure, bool dequeueShared);

            /// <summary>
            /// A property indicating if dequeuing message queue is enabled
            /// </summary>
            bool DequeueEnabled { get; }

            /// <summary>
            /// Replicate all messages within this queue onto one target queue
            /// </summary>
            /// <param name="clientQueue">A target queue for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, a target queue should be already opened and available</returns>
            bool AppendTo(IClientQueue clientQueue);

            /// <summary>
            /// Replicate all messages within this queue onto an array of queues
            /// </summary>
            /// <param name="clientQueues">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool AppendTo(IClientQueue[] clientQueues);

            /// <summary>
            /// Replicate all messages within this queue onto an array of queues
            /// </summary>
            /// <param name="queueHandles">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool AppendTo(IntPtr[] queueHandles);

            /// <summary>
            /// Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
            /// </summary>
            /// <param name="clientQueue">A target queue for appending messages from this queue </param>
            /// <returns>True for success; and false for fail. To make the call success, a target queue should be already opened and available</returns>
            bool EnsureAppending(IClientQueue clientQueue);

            /// <summary>
            /// Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
            /// </summary>
            /// <param name="clientQueues">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool EnsureAppending(IClientQueue[] clientQueues);

            /// <summary>
            /// Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
            /// </summary>
            /// <param name="queueHandles">An array of target queues for appending messages from this queue</param>
            /// <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
            bool EnsureAppending(IntPtr[] queueHandles);

            /// <summary>
            /// A bool value for enabling or disabling routing queue index. The property defaults to false.
            /// If there is only one worker application instance running (Note that one instance may have multiple socket connections), it ensures once-only delivery if this property is set to true.
            /// If there are multiple worker application instances running, you should not set this property to true! Otherwise, SocketPro may function incorrectly in dequeuing messages.
            /// </summary>
            bool RoutingQueueIndex { get; set; }


            IntPtr Handle { get; }
        }

        public delegate void DOnSubscribe(CClientSocket sender, CMessageSender messageSender, uint[] group);
        public delegate void DOnPublishEx(CClientSocket sender, CMessageSender messageSender, uint[] group, byte[] msg);
        public delegate void DOnPublish(CClientSocket sender, CMessageSender messageSender, uint[] group, object msg);
        public delegate void DOnSendUserMessage(CClientSocket sender, CMessageSender messageSender, object msg);
        public delegate void DOnSendUserMessageEx(CClientSocket sender, CMessageSender messageSender, byte[] msg);
        public delegate void DOnUnsubscribe(CClientSocket sender, CMessageSender messageSender, uint[] group);

        public interface IUPushClient : IUPushEx
        {
            event DOnPublish OnPublish;
            event DOnPublishEx OnPublishEx;
            event DOnSendUserMessage OnSendUserMessage;
            event DOnSendUserMessageEx OnSendUserMessageEx;
            event DOnSubscribe OnSubscribe;
            event DOnUnsubscribe OnUnsubscribe;
        }

        public sealed class CConnectionContext
        {
            public CConnectionContext()
            {

            }

            public CConnectionContext(string host, uint port, string userId, string password)
            {
                Host = host;
                Port = port;
                UserId = userId;
                m_Password = password;
                EncrytionMethod = tagEncryptionMethod.NoEncryption;
                Zip = false;
                V6 = false;
            }

            public CConnectionContext(string host, uint port, string userId, string password, tagEncryptionMethod em)
            {
                Host = host;
                Port = port;
                UserId = userId;
                m_Password = password;
                EncrytionMethod = em;
                Zip = false;
                V6 = false;
            }

            public CConnectionContext(string host, uint port, string userId, string password, tagEncryptionMethod em, bool zip)
            {
                Host = host;
                Port = port;
                UserId = userId;
                m_Password = password;
                EncrytionMethod = em;
                Zip = zip;
                V6 = false;
            }

            public CConnectionContext(string host, uint port, string userId, string password, tagEncryptionMethod em, bool zip, bool v6)
            {
                Host = host;
                Port = port;
                UserId = userId;
                m_Password = password;
                EncrytionMethod = em;
                Zip = zip;
                V6 = v6;
            }

            public string Password
            {
                set
                {
                    m_Password = value;
                }
            }

            internal string GetPassword()
            {
                return m_Password;
            }

            public string Host;
            public uint Port = 0;
            public string UserId;
            private string m_Password;
            public tagEncryptionMethod EncrytionMethod;
            public bool Zip = false;
            public bool V6 = false;
            public object AnyData = null;
        }

        public struct CMessageSender
        {
            public string UserId;
            public string IpAddress;
            public ushort Port;
            public int SvsID;
            public bool SelfMessage;
        }

        [StructLayout(LayoutKind.Sequential)]
        class CMessageSenderCe
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            public string UserId;
            //CE/mobile platforms don't support the marshal [MarshalAs(UnmanagedType.LPStr)]
            public IntPtr IpAddress;
            public ushort Port;
            public int SvsID;
            [MarshalAs(UnmanagedType.U1)]
            public byte SelfMessage;
        }

        public class CertInfo
        {
            internal CertInfo()
            {
            }

            internal unsafe void Set(CertInfoIntenal cii)
            {
                Algorithm = new byte[cii.AlgSize];
                System.Runtime.InteropServices.Marshal.Copy(cii.Algorithm, Algorithm, 0, cii.AlgSize);
                Validity = (cii.Validity != 0);

                sbyte* p = (sbyte*)cii.CertPem.ToPointer();
                CertPem = new string(p);

                p = (sbyte*)cii.Issuer.ToPointer();
                Issuer = new string(p);

                p = (sbyte*)cii.NotAfter.ToPointer();
                NotAfter = new string(p);

                p = (sbyte*)cii.NotBefore.ToPointer();
                NotBefore = new string(p);

                PublicKey = new byte[cii.PKSize];
                System.Runtime.InteropServices.Marshal.Copy(cii.PublicKey, PublicKey, 0, cii.PKSize);

                SerialNumber = new byte[cii.SNSize];
                System.Runtime.InteropServices.Marshal.Copy(cii.SerialNumber, SerialNumber, 0, cii.SNSize);

                p = (sbyte*)cii.SessionInfo.ToPointer();
                SessionInfo = new string(p);

                p = (sbyte*)cii.SigAlg.ToPointer();
                SigAlg = new string(p);

                p = (sbyte*)cii.Subject.ToPointer();
                Subject = new string(p);
            }

            public string Issuer;
            public string Subject;
            public string NotBefore;
            public string NotAfter;
            public bool Validity;
            public string SigAlg;
            public string CertPem;
            public string SessionInfo;
            public byte[] PublicKey;
            public byte[] Algorithm;
            public byte[] SerialNumber;
        };

        public abstract class IUcert : CertInfo
        {
            public abstract string Verify(out int errCode);
        }

        public enum tagConnectionState : int
        {
            csClosed = 0,
            csConnecting,
            csSslShaking,
            csClosing,
            csConnected,
            csSwitched
        };

        public enum tagSocketPoolEvent : int
        {
            speUnknown = -1,
            speStarted = 0,
            speCreatingThread,
            speThreadCreated,
            speConnecting,
            speConnected,
            speKillingThread,
            speShutdown,
            speUSocketCreated,
            speHandShakeCompleted,
            speLocked,
            speUnlocked,
            speThreadKilled,
            speClosingSocket,
            speSocketClosed,
            speUSocketKilled,
            speTimer,
            speQueueMergedFrom,
            speQueueMergedTo,
        }
    }
#endif

#if WINCE
#else
    namespace ServerSide
    {
        public enum tagAuthenticationMethod
        {
            amOwn = 0,
            amMixed = (amOwn + 1),
            amIntegrated = (amMixed + 1),
            amTrusted = (amIntegrated + 1)
        }
		;

        public enum tagHttpMethod
        {
            hmUnknown = 0,
            hmGet = 1,
            hmPost = 2,
            hmHead = 3,
            hmPut = 4,
            hmDelete = 5,
            hmOptions = 6,
            hmTrace = 7,
            hmConnect = 8,
        };

        public enum tagTransport
        {
            tUnknown = -1,
            tWebSocket = 0,
            tFlash = 1,
            tAjax = 2,
            tScript = 3,
        };

        public enum tagTransferEncoding
        {
            teUnknown = 0,
            teChunked = 1,
            teCompress = 2,
            teDeflate = 3,
            teGZip = 4,
            teIdentity = 5,
        };

        public enum tagContentMultiplax
        {
            cmUnknown = 0,
            cmMixed = 1,
            cmAlternative = 2,
            cmDigest = 3,
            cmParallel = 4,
            cmFormData = 5,
            cmReport = 6,
            cmSigned = 7,
            cmEncrypted = 8,
            cmRelated = 9,
            cmByteRanges = 10,
        };

        public enum tagRoutingAlgorithm
        {
            raDefault = 0,
            raRandom,
            raAverage,
        };

        public enum tagThreadEvent
        {
            teStarted = 0,
            teKilling = 1
        }

        [StructLayout(LayoutKind.Sequential)]
        struct CHttpHV
        {
            public IntPtr Header;
            public IntPtr Value;
        }

        public enum tagHttpRequestID
        {
            idGet = 129,
            idPost = 130,
            idHead = 131,
            idPut = 132,
            idDelete = 133,
            idOptions = 134,
            idTrace = 135,
            idConnect = 136,
            idMultiPart = 137, //HTTP POST MUTIPLE PART
            idUserRequest = 138, //SocketPro HTTP User Request
        };
    }
#endif
}
