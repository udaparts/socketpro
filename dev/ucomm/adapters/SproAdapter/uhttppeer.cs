using System;
using System.Collections.Generic;
using System.Text;

namespace SocketProAdapter
{
    namespace ServerSide
    {
        public class CHttpPeerBase : CSocketPeer
        {
            internal class CHttpPushImpl : IUPush
            {
                internal CSocketPeer m_sp;
                internal CHttpPushImpl(CSocketPeer sp)
                {
                    m_sp = sp;
                }

                #region IUPush Members

                public bool Publish(object Message, uint[] Groups)
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
                                    return ServerCoreLoader.Speak(m_sp.Handle, buffer, q.GetSize(), p, len);
                                }
                            }
                        }
                    }
                }

                public bool Subscribe(uint[] Groups)
                {
                    uint len;
                    if (Groups == null)
                        len = 0;
                    else
                        len = (uint)Groups.Length;
                    unsafe
                    {
                        fixed (uint* p = Groups)
                        {
                            return ServerCoreLoader.Enter(m_sp.Handle, p, len);
                        }
                    }
                }

                public bool Unsubscribe()
                {
                    ServerCoreLoader.Exit(m_sp.Handle);
                    return true;
                }

                public bool SendUserMessage(object Message, string UserId)
                {
                    using (CScopeUQueue su = new CScopeUQueue())
                    {
                        CUQueue q = su.UQueue;
                        q.Save(Message);
                        unsafe
                        {
                            fixed (byte* p = q.m_bytes)
                            {
                                return ServerCoreLoader.SendUserMessage(m_sp.Handle, UserId, p, q.GetSize());
                            }
                        }
                    }
                }

                #endregion
            }

            private CHttpPushImpl m_push;

            protected CHttpPeerBase()
            {
                m_push = new CHttpPushImpl(this);
            }

            public IUPush Push
            {
                get
                {
                    return m_push;
                }
            }

            protected virtual bool DoAuthentication(string userId, string password)
            {
                return true;
            }

            protected virtual void OnPost()
            {

            }

            protected virtual void OnGet()
            {

            }

            protected virtual void OnUserRequest()
            {

            }

            protected virtual void OnDelete()
            {

            }

            protected virtual void OnPut()
            {

            }

            protected virtual void OnHead()
            {

            }

            protected virtual void OnOptions()
            {

            }

            protected virtual void OnTrace()
            {

            }

            protected virtual void OnMultiPart()
            {

            }

            protected virtual void OnConnect()
            {
            }

            internal int OnHttpRequestArrive(ushort reqId, uint len)
            {
                tagHttpRequestID hid = (tagHttpRequestID)reqId;
                switch (hid)
                {
                    case tagHttpRequestID.idGet:
                        OnGet();
                        break;
                    case tagHttpRequestID.idPost:
                        OnPost();
                        break;
                    case tagHttpRequestID.idUserRequest:
                        OnUserRequest();
                        break;
                    case tagHttpRequestID.idDelete:
                        OnDelete();
                        break;
                    case tagHttpRequestID.idHead:
                        OnHead();
                        break;
                    case tagHttpRequestID.idMultiPart:
                        OnMultiPart();
                        break;
                    case tagHttpRequestID.idOptions:
                        OnOptions();
                        break;
                    case tagHttpRequestID.idPut:
                        OnPut();
                        break;
                    case tagHttpRequestID.idTrace:
                        OnTrace();
                        break;
                    case tagHttpRequestID.idConnect:
                        OnConnect();
                        break;
                    default:
                        SetResponseCode(501);
                        SendResult("Method not implemented");
                        break;
                }
                return reqId;
            }

            internal bool DoHttpAuth(string userId, string password)
            {
                return DoAuthentication(userId, password);
            }

            public virtual uint SendResult(string res)
            {
                Clear();
                if (res == null)
                    res = "";
                byte[] bytes = System.Text.UTF8Encoding.UTF8.GetBytes(res);
                unsafe
                {
                    fixed (byte* p = bytes)
                    {
                        return ServerCoreLoader.SendHTTPReturnDataA(Handle, p, bytes.Length);
                    }
                }
            }

            private void Clear()
            {
                if (m_ReqHeaders != null && !ServerCoreLoader.IsWebSocket(Handle))
                {
                    m_ReqHeaders.Clear();
                    m_ReqHeaders = null;
                }
            }

            public bool DownloadFile(string filePath)
            {
                Clear();
                return ServerCoreLoader.DownloadFile(Handle, filePath);
            }

            /// <summary>
            /// Begin to send HTTP result in chunk
            /// </summary>
            /// <returns>The data size in bytes</returns>
            /// <remarks>The method EndChunkResponse should be called at the end after this method is called</remarks>
            public uint StartChunkResponse()
            {
                return ServerCoreLoader.StartHTTPChunkResponse(Handle);
            }

            /// <summary>
            /// Send a chunk of data after calling the method StartChunkResponse
            /// </summary>
            /// <param name="buffer">A buffer data</param>
            /// <returns>The data size in byte</returns>
            /// <remarks>You must call the method StartChunkResponse before calling this method</remarks>
            public uint SendChunk(byte[] buffer)
            {
                if (buffer == null || buffer.Length == 0)
                    return 0;
                unsafe
                {
                    fixed (byte* p = buffer)
                    {
                        return ServerCoreLoader.SendHTTPChunk(Handle, p, (uint)buffer.Length);
                    }
                }
            }

            /// <summary>
            /// Send the last chunk of data and indicate the HTTP response is ended
            /// </summary>
            /// <param name="buffer">The last chunk of data</param>
            /// <returns>The data size in byte</returns>
            /// <remarks>You must call the method StartChunkResponse before calling this method</remarks>
            public uint EndChunkResponse(byte[] buffer)
            {
                uint len;
                if (buffer == null)
                    len = 0;
                else
                    len = (uint)buffer.Length;
                Clear();
                unsafe
                {
                    fixed (byte* p = buffer)
                    {
                        return ServerCoreLoader.EndHTTPChunkResponse(Handle, p, len);
                    }
                }
            }

            public string RequestName
            {
                get
                {
                    return m_WebRequestName;
                }
            }

            public List<object> Args
            {
                get
                {
                    return m_vArg;
                }
            }

            public bool Authenticated
            {
                get
                {
                    return m_bHttpOk;
                }
            }

            public string Path
            {
                get
                {
                    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetHTTPPath(Handle));
                }
            }
            public ulong ContentLength
            {
                get
                {
                    return ServerCoreLoader.GetHTTPContentLength(Handle);
                }
            }

            public string Query
            {
                get
                {
                    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetHTTPQuery(Handle));
                }
            }

            public tagHttpMethod HttpMethod
            {
                get
                {
                    return ServerCoreLoader.GetHTTPMethod(Handle);
                }
            }

            public bool IsWebSocket
            {
                get
                {
                    return ServerCoreLoader.IsWebSocket(Handle);
                }
            }
            public bool IsCrossDomain
            {
                get
                {
                    return ServerCoreLoader.IsCrossDomain(Handle);
                }
            }

            public double Version
            {
                get
                {
                    return ServerCoreLoader.GetHTTPVersion(Handle);
                }
            }

            public string Host
            {
                get
                {
                    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetHTTPHost(Handle));
                }
            }

            public tagTransport Transport
            {
                get
                {
                    return ServerCoreLoader.GetHTTPTransport(Handle);
                }
            }

            public tagTransferEncoding TransferEncoding
            {
                get
                {
                    return ServerCoreLoader.GetHTTPTransferEncoding(Handle);
                }
            }

            public tagContentMultiplax ContentMultiplax
            {
                get
                {
                    return ServerCoreLoader.GetHTTPContentMultiplax(Handle);
                }
            }

            public string Id
            {
                get
                {
                    return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ServerCoreLoader.GetHTTPId(Handle));
                }
            }

            public Dictionary<string, string> CurrentMultiplaxHeaders
            {
                get
                {
                    uint res;
                    Dictionary<string, string> mapHeaders = new Dictionary<string, string>();
                    CHttpHV[] hv = new CHttpHV[64];
                    unsafe
                    {
                        fixed (CHttpHV* p = hv)
                        {
                            res = ServerCoreLoader.GetHTTPCurrentMultiplaxHeaders(Handle, p, 64);
                        }
                        for (uint n = 0; n < res; ++n)
                        {
                            string header = new string((sbyte*)(hv[n].Header));
                            if (header.Length == 0)
                                continue;
                            string value = new string((sbyte*)(hv[n].Value));
                            mapHeaders[header] = value;
                        }
                    }
                    return mapHeaders;
                }
            }

            public bool SetResponseCode(uint HttpCode)
            {
                return ServerCoreLoader.SetHTTPResponseCode(Handle, HttpCode);
            }

            public bool SetResponseHeader(string uft8Header, string utf8Value)
            {
                return ServerCoreLoader.SetHTTPResponseHeader(Handle, uft8Header, utf8Value);
            }

            private Dictionary<string, string> m_ReqHeaders = null;

            public Dictionary<string, string> RequestHeaders
            {
                get
                {
                    uint res;
                    if (m_ReqHeaders != null)
                        return m_ReqHeaders;
                    m_ReqHeaders = new Dictionary<string, string>();
                    CHttpHV[] hv = new CHttpHV[64];
                    unsafe
                    {
                        fixed (CHttpHV* p = hv)
                        {
                            res = ServerCoreLoader.GetHTTPRequestHeaders(Handle, p, 64);
                        }
                        for (uint n = 0; n < res; ++n)
                        {
                            string header = new string((sbyte*)(hv[n].Header));
                            if (header.Length == 0)
                                continue;
                            string value = new string((sbyte*)(hv[n].Value));
                            m_ReqHeaders[header] = value;
                        }
                    }
                    return m_ReqHeaders;
                }
            }

            internal string m_WebRequestName;
            internal bool m_bHttpOk;
            internal List<object> m_vArg = new List<object>();
        }
    }

}
