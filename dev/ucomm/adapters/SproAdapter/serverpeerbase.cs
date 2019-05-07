using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public class CSocketPeer
        {
            internal CUQueue m_qBuffer = new CUQueue();
            internal bool m_bRandom = false;
            internal ulong m_sh = 0;
            internal CBaseService m_Service;

            public const uint SOCKET_NOT_FOUND = uint.MaxValue;
            public const uint REQUEST_CANCELED = uint.MaxValue - 1;
            public const uint BAD_OPERATION = uint.MaxValue - 2;
            public const uint RESULT_SENDING_FAILED = uint.MaxValue - 3;

            internal bool m_endian = false;
            internal tagOperationSystem m_os = SocketProAdapter.Defines.OperationSystem;

            internal CSocketPeer()
            {
            }

            internal ushort m_CurrReqID = (ushort)tagBaseRequestID.idSwitchTo;
            public ushort CurrentRequestID
            {
                get
                {
                    return m_CurrReqID;
                }
            }

            public uint CurrentRequestLen
            {
                get
                {
                    return ServerCoreLoader.GetCurrentRequestLen(m_sh);
                }
            }

            public ulong BytesReceived
            {
                get
                {
                    return ServerCoreLoader.GetBytesReceived(m_sh);
                }
            }

            public ulong BytesSent
            {
                get
                {
                    return ServerCoreLoader.GetBytesSent(m_sh);
                }
            }

            public uint CountOfJoinedChatGroups
            {
                get
                {
                    return ServerCoreLoader.GetCountOfJoinedChatGroups(m_sh);
                }
            }

            public int ErrorCode
            {
                get
                {
                    return ServerCoreLoader.GetServerSocketErrorCode(m_sh);
                }
            }

            public string ErrorMessage
            {
                get
                {
                    sbyte[] errMsg = new sbyte[4097];
                    unsafe
                    {
                        fixed (sbyte* p = errMsg)
                        {
                            ServerCoreLoader.GetServerSocketErrorMessage(m_sh, p, 4097);
                            return new string(p);
                        }
                    }
                }
            }

            public string GetPeerName(out uint port)
            {
                uint myPort = 0;
                sbyte[] addr = new sbyte[256];
                unsafe
                {
                    fixed (sbyte* str = addr)
                    {
                        ServerCoreLoader.GetPeerName(m_sh, ref myPort, str, 256);
                    }
                    port = myPort;
                    fixed (sbyte* p = addr)
                    {
                        return new string(p);
                    }
                }
            }

            public string GetPeerName()
            {
                uint port;
                return GetPeerName(out port);
            }

            public uint BytesInReceivingBuffer
            {
                get
                {
                    return ServerCoreLoader.GetRcvBytesInQueue(m_sh);
                }
            }

            public uint BytesInSendingBuffer
            {
                get
                {
                    return ServerCoreLoader.GetSndBytesInQueue(m_sh);
                }
            }

            public static ulong RequestCount
            {
                get
                {
                    return ServerCoreLoader.GetRequestCount();
                }
            }

            public ulong SocketNativeHandle
            {
                get
                {
                    return ServerCoreLoader.GetSocketNativeHandle(m_sh);
                }
            }

            public CBaseService BaseService
            {
                get
                {
                    return m_Service;
                }
            }

            protected bool Random
            {
                get
                {
                    return m_bRandom;
                }
            }

            protected CUQueue UQueue
            {
                get
                {
                    return m_qBuffer;
                }
            }

            public ulong Handle
            {
                get
                {
                    lock (m_Service.m_cs)
                    {
                        return m_sh;
                    }
                }
            }

            public IntPtr SSL
            {
                get
                {
                    return ServerCoreLoader.GetSSL(m_sh);
                }
            }

            public string UID
            {
                get
                {
                    return CSocketProServer.CredentialManager.GetUserID(m_sh);
                }
                set
                {
                    ServerCoreLoader.SetUserID(m_sh, value);
                }
            }

            public uint SvsID
            {
                get
                {
                    return ServerCoreLoader.GetSvsID(m_sh);
                }
            }

            public bool Batching
            {
                get
                {
                    return ServerCoreLoader.IsBatching(m_sh);
                }
            }

            public bool IsCanceled
            {
                get
                {
                    return ServerCoreLoader.IsCanceled(m_sh);
                }
            }

            public bool IsFakeRequest
            {
                get
                {
                    return ServerCoreLoader.IsFakeRequest(m_sh);
                }
            }

            public ulong CurrentRequestIndex
            {
                get
                {
                    return ServerCoreLoader.GetCurrentRequestIndex(m_sh);
                }
            }

            public static bool IsMainThread
            {
                get
                {
                    return ServerCoreLoader.IsMainThread();
                }
            }

            public bool Connected
            {
                get
                {
                    return ServerCoreLoader.IsOpened(m_sh);
                }
            }

            public void DropCurrentSlowRequest()
            {
                ServerCoreLoader.DropCurrentSlowRequest(m_sh);
            }

            public void Close()
            {
                ServerCoreLoader.Close(m_sh);
            }

            public void PostClose()
            {
                ServerCoreLoader.PostClose(m_sh, 0);
            }

            public void PostClose(int errCode)
            {
                ServerCoreLoader.PostClose(m_sh, errCode);
            }

            public uint RequestsInQueue
            {
                get
                {
                    return ServerCoreLoader.QueryRequestsInQueue(m_sh);
                }
            }

            public uint SendExceptionResult(string errMessage, string errWhere)
            {
                ulong reqIndex = CurrentRequestIndex;
                if (reqIndex == ulong.MaxValue)
                    return ServerCoreLoader.SendExceptionResult(m_sh, errMessage, errWhere, 0, 0);
                return ServerCoreLoader.SendExceptionResultIndex(m_sh, reqIndex, errMessage, errWhere, 0, 0);
            }

            public uint SendExceptionResult(string errMessage, string errWhere, uint errCode)
            {
                ulong reqIndex = CurrentRequestIndex;
                if (reqIndex == ulong.MaxValue)
                    return ServerCoreLoader.SendExceptionResult(m_sh, errMessage, errWhere, 0, errCode);
                return ServerCoreLoader.SendExceptionResultIndex(m_sh, reqIndex, errMessage, errWhere, 0, errCode);
            }

            public uint SendExceptionResult(string errMessage, string errWhere, uint errCode, ushort requestId)
            {
                ulong reqIndex = CurrentRequestIndex;
                if (reqIndex == ulong.MaxValue)
                    return ServerCoreLoader.SendExceptionResult(m_sh, errMessage, errWhere, requestId, errCode);
                return ServerCoreLoader.SendExceptionResultIndex(m_sh, reqIndex, errMessage, errWhere, requestId, errCode);
            }

            public uint SendExceptionResult(string errMessage, string errWhere, uint errCode, ushort requestId, ulong reqIndex)
            {
                if (reqIndex == ulong.MaxValue)
                    return ServerCoreLoader.SendExceptionResult(m_sh, errMessage, errWhere, requestId, errCode);
                return ServerCoreLoader.SendExceptionResultIndex(m_sh, reqIndex, errMessage, errWhere, requestId, errCode);
            }

            protected virtual void OnSwitchFrom(uint oldServiceId)
            {

            }

            internal void OnSwitch(uint oldServiceId)
            {
                OnSwitchFrom(oldServiceId);
            }

            protected virtual void OnSlowRequestProcessed(ushort reqId)
            {

            }

            internal virtual void OnSRProcessed(ushort reqId)
            {
                OnSlowRequestProcessed(reqId);
            }

            protected virtual void OnReleaseResource(bool bClosing, uint info)
            {

            }

            internal void OnRelease(bool bClosing, uint info)
            {
                OnReleaseResource(bClosing, info);
            }

            protected virtual void OnBaseRequestCame(tagBaseRequestID reqId)
            {

            }

            internal void OnBRCame(ushort reqId)
            {
                OnBaseRequestCame((tagBaseRequestID)reqId);
            }

            protected virtual void OnChatRequestCame(tagChatRequestID chatRequestId)
            {

            }

            internal void OnCRCame(tagChatRequestID chatRequestId)
            {
                OnChatRequestCame(chatRequestId);
            }

            protected virtual void OnRequestArrive(ushort requestId, uint len)
            {

            }

            internal void OnRArrive(ushort requestId, uint len)
            {
                OnRequestArrive(requestId, len);
            }

            protected virtual void OnSubscribe(uint[] groups)
            {

            }

            protected virtual void OnUnsubscribe(uint[] groups)
            {

            }

            protected virtual void OnPublish(object message, uint[] groups)
            {

            }

            protected virtual void OnSendUserMessage(string receiver, object message)
            {

            }

            protected virtual void OnResultsSent()
            {

            }

            internal void OnRSent()
            {
                OnResultsSent();
            }

            public uint[] ChatGroups
            {
                get
                {
                    uint res = ServerCoreLoader.GetCountOfJoinedChatGroups(m_sh);
                    uint[] groups = new uint[res];
                    if (res > 0)
                    {
                        unsafe
                        {
                            fixed (uint* p = groups)
                            {
                                ServerCoreLoader.GetJoinedGroupIds(m_sh, p, res);
                            }
                        }
                    }
                    return groups;
                }
            }

            internal virtual void OnChatComing(tagChatRequestID chatRequestID)
            {
                uint svsId = ServerCoreLoader.GetSvsID(m_sh);
                CUQueue q = m_qBuffer;
                if (svsId != BaseServiceID.sidHTTP)
                {
                    bool endian = false;
                    tagOperationSystem os = ServerCoreLoader.GetPeerOs(m_sh, ref endian);
                    q.Endian = endian;
                    q.OS = os;
                }
                switch (chatRequestID)
                {
                    case tagChatRequestID.idEnter:
                        {
                            object objGroups;
                            q.Load(out objGroups);
                            OnSubscribe((uint[])objGroups);
                        }
                        break;
                    case tagChatRequestID.idExit:
                        {
                            OnUnsubscribe(ChatGroups);
                        }
                        break;
                    case tagChatRequestID.idSendUserMessage:
                        {
                            object msg;
                            string user;
                            q.Load(out user).Load(out msg);
                            OnSendUserMessage(user, msg);
                        }
                        break;
                    case tagChatRequestID.idSpeak:
                        {
                            object msg;
                            object groups;
                            q.Load(out groups).Load(out msg);
                            OnPublish(msg, (uint[])groups);
                        }
                        break;
                    default:
                        ServerCoreLoader.SendExceptionResult(m_sh, "Unexpected chat request", Environment.StackTrace, (ushort)chatRequestID, 0);
                        break;
                }
            }
        }
    }
}
