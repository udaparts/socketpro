using System;
using System.Runtime.InteropServices;
using System.Security;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        delegate void SocketPoolCallback(uint poolId, tagSocketPoolEvent spc, IntPtr h);
        delegate void POnSocketClosed(IntPtr handler, int nError);
        delegate void POnHandShakeCompleted(IntPtr handler, int nError);
        delegate void POnSocketConnected(IntPtr handler, int nError);
        delegate void POnRequestProcessed(IntPtr handler, ushort requestId, uint len);
        delegate void POnBaseRequestProcessed(IntPtr handler, ushort requestId);
        delegate void POnAllRequestsProcessed(IntPtr handler, ushort lastRequestId);
        delegate void POnPostProcessing(IntPtr handler, uint hint, ulong data);

        //CE/mobile platforms do not support the marshals UnmanagedType.LPArray or UnmanagedType.LPStr, 
        delegate void POnEnter(IntPtr handler, IntPtr sender, IntPtr groups, uint count);
        delegate void POnExit(IntPtr handler, IntPtr sender, IntPtr groups, uint count);
        delegate void POnSpeakEx(IntPtr handler, IntPtr sender, IntPtr groups, uint count, IntPtr message, uint size);
        delegate void POnSendUserMessageEx(IntPtr handler, IntPtr sender, IntPtr message, uint size);
        delegate void POnServerException(IntPtr handler, ushort requestId, [MarshalAs(UnmanagedType.LPWStr)]string errMessage, IntPtr errWhere, int errCode);
        delegate void POnSpeak(IntPtr handler, IntPtr sender, IntPtr groups, uint count, IntPtr message, uint size);
        delegate void POnSendUserMessage(IntPtr handler, IntPtr sender, IntPtr message, uint size);
        delegate byte PCertificateVerifyCallback([MarshalAs(UnmanagedType.I1)]byte preverified, int depth, int errorCode, IntPtr errMessage, IntPtr ci);

        /*
        delegate void POnEnter(IntPtr handler, CMessageSender sender, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]uint[] group, uint count);
        delegate void POnExit(IntPtr handler, CMessageSender sender, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]uint[] group, uint count);
        delegate void POnSpeakEx(IntPtr handler, CMessageSender sender, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]uint[] group, uint count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 5)] byte[] message, uint size);
        delegate void POnSendUserMessageEx(IntPtr handler, CMessageSender sender, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] byte[] message, uint size);
        delegate void POnServerException(IntPtr handler, ushort requestId, [MarshalAs(UnmanagedType.LPWStr)]string errMessage, [MarshalAs(UnmanagedType.LPStr)]string errWhere, int errCode);
        delegate void POnSpeak(IntPtr handler, CMessageSender sender, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]uint[] group, uint count, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 5)] byte[] message, uint size);
        delegate void POnSendUserMessage(IntPtr handler, CMessageSender sender, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] byte[] message, uint size);
        */

        [StructLayout(LayoutKind.Sequential)]
        class CertInfoIntenal
        {
            public IntPtr Issuer;
            public IntPtr Subject;
            public IntPtr NotBefore;
            public IntPtr NotAfter;
            [MarshalAs(UnmanagedType.I1)]
            public byte Validity;
            public IntPtr SigAlg;
            public IntPtr CertPem;
            public IntPtr SessionInfo;
            public int PKSize;
            public IntPtr PublicKey;
            public int AlgSize;
            public IntPtr Algorithm;
            public int SNSize;
            public IntPtr SerialNumber;
        };

        //make it works within SQL Server
#if WINCE
        //there is no attribute SuppressUnmanagedCodeSecurity available for wince .NET
#else
        [SuppressUnmanagedCodeSecurity]
#endif
        static class ClientCoreLoader
        {
            const string CLIENT_CORE_DLL = "usocket";

            static ClientCoreLoader()
            {
                switch (System.Environment.OSVersion.Platform)
                {
                    case PlatformID.Win32S:
                    case PlatformID.Win32NT:
                    case PlatformID.Win32Windows:
                    case PlatformID.WinCE:
                    case PlatformID.Xbox:
                        break;
                    default:
                        ClientCoreLoader.UseUTF16();
                        break;
                }
            }

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void UseUTF16();

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint CreateSocketPool(SocketPoolCallback spc, uint maxSocketsPerThread, uint maxThreads, [MarshalAs(UnmanagedType.I1)]byte bAvg, tagThreadApartment ta);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte DestroySocketPool(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte GetQueueAutoMergeByPool(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetQueueAutoMergeByPool(uint poolId, [MarshalAs(UnmanagedType.I1)]byte autoMerge);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr FindAClosedSocket(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte AddOneThreadIntoPool(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetLockedSockets(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetIdleSockets(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetConnectedSockets(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte DisconnectAll(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr LockASocket(uint poolId, uint timeout, IntPtr hSameThread);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte UnlockASocket(uint poolId, IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetSocketsPerThread(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsAvg(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetDisconnectedSockets(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetThreadCount(uint poolId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void Close(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetCountOfRequestsQueued(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ushort GetCurrentRequestID(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetCurrentResultSize(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsDequeuedMessageAborted(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void AbortDequeuedMessage(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern tagEncryptionMethod GetEncryptionMethod(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern tagConnectionState GetConnectionState(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern int GetErrorCode(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static unsafe extern uint GetErrorMessage(IntPtr h, sbyte* str, uint bufferLen);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetSocketPoolId(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.U1)]
            internal static extern byte IsOpened(IntPtr h);

            //[DllImport(CLIENT_CORE_DLL)]
            //internal static unsafe extern uint RetrieveResult(IntPtr h, byte* buffer, uint size);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern byte SendRequest(IntPtr h, ushort reqId, byte* buffer, uint len);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnHandShakeCompleted(IntPtr h, POnHandShakeCompleted p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnRequestProcessed(IntPtr h, POnRequestProcessed p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnSocketClosed(IntPtr h, POnSocketClosed p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnSocketConnected(IntPtr h, POnSocketConnected p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnBaseRequestProcessed(IntPtr h, POnBaseRequestProcessed p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnAllRequestsProcessed(IntPtr h, POnAllRequestsProcessed p);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte WaitAll(IntPtr h, uint nTimeout);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte Cancel(IntPtr h, uint requestsQueued);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsRandom(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetBytesInSendingBuffer(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetBytesInReceivingBuffer(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsBatching(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetBytesBatched(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte StartBatching(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte CommitBatching(IntPtr h, [MarshalAs(UnmanagedType.I1)]byte batchingAtServerSide);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte AbortBatching(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetUserID(IntPtr h, [MarshalAs(UnmanagedType.LPWStr)]string userId);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern unsafe uint GetUID(IntPtr h, char* userId, uint chars);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetPassword(IntPtr h, [MarshalAs(UnmanagedType.LPWStr)]string password);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SwitchTo(IntPtr h, uint serviceId);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte Connect(IntPtr h, IntPtr host, uint portNumber, [MarshalAs(UnmanagedType.I1)]byte sync, [MarshalAs(UnmanagedType.I1)]byte v6);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte Enter(IntPtr h, IntPtr groups, int count);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte Speak(IntPtr h, IntPtr message, uint size, IntPtr groups, int count);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SpeakEx(IntPtr h, IntPtr message, int size, IntPtr groups, int count);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SendUserMessage(IntPtr h, [MarshalAs(UnmanagedType.LPWStr)]string userId, IntPtr message, uint size);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SendUserMessageEx(IntPtr h, [MarshalAs(UnmanagedType.LPWStr)]string userId, IntPtr message, int size);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte StartQueue(IntPtr h, IntPtr qName, [MarshalAs(UnmanagedType.I1)]byte secure, [MarshalAs(UnmanagedType.I1)]byte dequeueShared, uint ttl);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SetVerifyLocation(IntPtr certFile);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetClientWorkDirectory(IntPtr dir);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void Exit(IntPtr h);

#if WINCE
            //wince pinvoke doesn't support ulong marshalling for input and return.
            //we use uint instead!
            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetBytesReceived(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetBytesSent(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetSocketNativeHandle(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetMessageCount(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetQueueSize(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetQueueLastIndex(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint CancelQueuedRequestsByIndex(IntPtr h, uint startIndex, uint endIndex);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint RemoveQueuedRequestsByTTL(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetLastQueueMessageTime(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetJobSize(IntPtr h);
#else
            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetBytesReceived(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetBytesSent(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetSocketNativeHandle(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetMessageCount(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetQueueSize(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetQueueLastIndex(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong CancelQueuedRequestsByIndex(IntPtr h, ulong startIndex, ulong endIndex);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong RemoveQueuedRequestsByTTL(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetLastQueueMessageTime(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ulong GetJobSize(IntPtr h);
#endif

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnEnter2(IntPtr h, POnEnter p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnExit2(IntPtr h, POnExit p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnSpeakEx2(IntPtr h, POnSpeakEx p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnSendUserMessageEx2(IntPtr h, POnSendUserMessageEx p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern tagOperationSystem GetPeerOs(IntPtr h, ref bool endian);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnServerException(IntPtr h, POnServerException p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnSendUserMessage2(IntPtr h, POnSendUserMessage p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnSpeak2(IntPtr h, POnSpeak p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetZip(IntPtr h, [MarshalAs(UnmanagedType.I1)]byte zip);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte GetZip(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetZipLevel(IntPtr h, tagZipLevel zl);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern tagZipLevel GetZipLevel(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetCurrentServiceId(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void StopQueue(IntPtr h, [MarshalAs(UnmanagedType.I1)]byte permanent);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte DequeuedResult(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetMessagesInDequeuing(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsQueueSecured(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsQueueStarted(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte DoEcho(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SetSockOpt(IntPtr h, tagSocketOption optName, int optValue, tagSocketLevel level);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SetSockOptAtSvr(IntPtr h, tagSocketOption optName, int optValue, tagSocketLevel level);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte TurnOnZipAtSvr(IntPtr h, [MarshalAs(UnmanagedType.I1)]byte enableZip);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte SetZipLevelAtSvr(IntPtr h, tagZipLevel zipLevel);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetRecvTimeout(IntPtr h, uint timeout);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetRecvTimeout(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetConnTimeout(IntPtr h, uint timeout);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetConnTimeout(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetAutoConn(IntPtr h, [MarshalAs(UnmanagedType.I1)]byte autoConnecting);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte GetAutoConn(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern ushort GetServerPingTime(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetSSL(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IgnoreLastRequest(IntPtr h, ushort reqId);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsDequeueEnabled(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetQueueName(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetQueueFileName(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern byte GetPeerName(IntPtr h, ref uint peerPort, sbyte* addr, ushort chars);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetUCert(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr Verify(IntPtr h, ref int errCode);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte AbortJob(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte StartJob(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte EndJob(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsRouteeRequest(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetRouteeCount(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern byte SendRouteeResult(IntPtr h, ushort reqId, byte* buffer, uint len);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsDequeueShared(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern tagQueueStatus GetClientQueueStatus(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static unsafe extern byte PushQueueTo(IntPtr srcHandle, IntPtr* targetHandles, uint count);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetTTL(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void ResetQueue(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsClientQueueIndexPossiblyCrashed();

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetClientWorkDirectory();

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern uint GetNumberOfSocketPools();

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsRouting(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetEncryptionMethod(IntPtr h, tagEncryptionMethod em);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void Shutdown(IntPtr h, tagShutdownType how);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetUClientSocketVersion();

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetMessageQueuePassword(IntPtr pwd);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetCertificateVerifyCallback(PCertificateVerifyCallback p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void EnableRoutingQueueIndex(IntPtr h, [MarshalAs(UnmanagedType.I1)]byte enable);

            [DllImport(CLIENT_CORE_DLL)]
            [return: MarshalAs(UnmanagedType.I1)]
            internal static extern byte IsRoutingQueueIndexEnabled(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetUClientAppName();

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern IntPtr GetResultBuffer(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern tagOptimistic GetOptimistic(IntPtr h);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOptimistic(IntPtr h, tagOptimistic optimistic);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetLastCallInfo(IntPtr str);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void SetOnPostProcessing(IntPtr h, POnPostProcessing p);

            [DllImport(CLIENT_CORE_DLL)]
            internal static extern void PostProcessing(IntPtr h, uint hint, ulong data);
        }
    }
}
