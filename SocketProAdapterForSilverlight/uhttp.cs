using System;
using System.Windows.Browser;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        /// <summary>
        /// A callback for tracking HTTP request result.
        /// </summary>
        /// <param name="Request">A JavaScript request object.</param>
        /// <param name="strResult">A string result.</param>
        public delegate void DOnURequest(CRequest Request, string strResult);
        
        /// <summary>
        /// A delagate callback for HTTP chat-related request results.
        /// </summary>
        /// <param name="Request">A JavaScript chat-related request object.</param>
        /// <param name="Result">A chat request result from server. It could be a number, boolean, a chat id string, a server exception message string, or a JSON string in the future.</param>
        public delegate void DOnChatRequest(CRequest Request, object Result);
        
        /// <summary>
        /// A delagate callback for tracking messages from a remote SocketPro HTTP push server.
        /// </summary>
        /// <param name="Request">A listen request object.</param>
        /// <param name="HttpPush">An interface to a collection of HTTP server push messages.</param>
        public delegate void DOnChatNotification(CRequest Request, CHttpPush HttpPush);

        /// <summary>
        /// A collection of defined message types available from SocketPro HTTP push server.
        /// </summary>
        public enum MessageType
        {
            /// <summary>
            /// A normal message.
            /// </summary>
            Normal = 0,

            /// <summary>
            /// A timeout message.
            /// </summary>
            Timeout = 1,

            /// <summary>
            /// A message from SocketPro HTTP push server if the server is shutting down gracefully.
            /// </summary>
            ServerShuttingdownGracefully = 2,
        }

        /// <summary>
        /// A class containing a number of message atrributes.
        /// </summary>
        public sealed class CChatMessage
        {
            ScriptObject m_soMessage;
            internal CChatMessage(ScriptObject soMessage)
            {
                m_soMessage = soMessage;
            }

            /// <summary>
            /// A case-insensitive sender user id.
            /// </summary>
            public string Sender
            {
                get
                {
                    object obj = m_soMessage.GetProperty("sender");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }

            /// <summary>
            /// A non-zero service id of sender.
            /// </summary>
            public uint ServiceId
            {
                get
                {
                    object obj = m_soMessage.GetProperty("serviceId");
                    if (obj == null)
                        return 0;
                    return (uint)((double)obj);
                }
            }

            /// <summary>
            /// The server ip address like '111.222.212.121'.
            /// </summary>
            public string IpAddress
            {
                get
                {
                    object obj = m_soMessage.GetProperty("ip");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }

            /// <summary>
            /// A port number of sender socket. It would be 0 for SocketPro HTTP service. It will be a non-zero number for non-HTTP service.
            /// </summary>
            public uint Port
            {
                get
                {
                    object obj = m_soMessage.GetProperty("port");
                    if (obj == null)
                        return 0;
                    return (uint)((double)obj);
                }
            }

            /// <summary>
            /// The chat groups which the sender is joining to.
            /// </summary>
            public uint[] Groups
            {
                get
                {
                    object obj = m_soMessage.GetProperty("groups");
                    if (obj == null)
                        return null;
                    ScriptObject so = obj as ScriptObject;
                    if (so == null)
                        return null;
                    return so.ConvertTo<uint[]>();
                }
            }

            /// <summary>
            /// The method name used for originating the message.
            /// </summary>
            public string MethodName
            {
                get
                {
                    object obj = m_soMessage.GetProperty("method");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }

            /// <summary>
            /// A message string.
            /// </summary>
            public string Message
            {
                get
                {
                    object obj = m_soMessage.GetProperty("msg");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }

            /// <summary>
            /// A date time string when the sender sends the message.
            /// </summary>
            public string DateTime
            {
                get
                {
                    object obj = m_soMessage.GetProperty("dt");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }
        }
        /// <summary>
        /// A class represents a HTTP push result which may contain a list of messages.
        /// </summary>
        public sealed class CHttpPush
        {
            ScriptObject m_soNotification;
            internal List<CChatMessage> m_lstMessage = null;
            internal CHttpPush(ScriptObject so)
            {
                m_soNotification = so;
                ScriptObject arrayMessage = (ScriptObject)m_soNotification.GetProperty("messages");
                if (arrayMessage != null)
                {
                    m_lstMessage = new List<CChatMessage>();
                    object len = arrayMessage.GetProperty("length");
                    uint nSize = (uint)((double)len);
                    while (nSize > 0)
                    {
                        object obj = arrayMessage.Invoke("shift");
                        if (obj == null)
                            m_lstMessage.Add(null);
                        else
                        {
                            CChatMessage chatMessage = new CChatMessage((ScriptObject)obj);
                            m_lstMessage.Add(chatMessage);
                        }
                        len = arrayMessage.GetProperty("length");
                        nSize = (uint)((double)len);
                    }
                }
            }

            /// <summary>
            /// A property for a list of messages.
            /// </summary>
            public List<CChatMessage> Messages
            {
                get { return m_lstMessage; }
            }

            /// <summary>
            /// A message type, Normal, Timeout or ServerShuttingdownGracefully.
            /// </summary>
            public MessageType Type
            {
                get
                {
                    if (m_soNotification == null)
                        return MessageType.Normal;
                    object obj = m_soNotification.GetProperty("type");
                    if (obj == null)
                        return MessageType.Normal;
                    return (MessageType)(double.Parse(obj.ToString()));
                }
            }

            /// <summary>
            /// A chat or session id string.
            /// </summary>
            public string ChatId
            {
                get
                {
                    if (m_soNotification == null)
                        return "";
                    object obj = m_soNotification.GetProperty("chatId");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }
        }

 /*       /// <summary>
        /// A class to show the chat-related request result. 
        /// </summary>
        public sealed class CChatResult 
        {
            ScriptObject m_soResult;
            internal CChatResult(ScriptObject so)
            {
                m_soResult = so;
            }

            /// <summary>
            /// Request name.
            /// </summary>
            public string Name
            {
                get
                {
                    if (m_soResult == null)
                        return "";
                    object obj = m_soResult.GetProperty("method");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }

            /// <summary>
            /// A returning result. It could be false, true, chat id string, or server exception message string.
            /// </summary>
            public object Result
            {
                get
                {
                    if (m_soResult == null)
                        return false;
                    return m_soResult.GetProperty("ret");
                }
            }
        }*/

        /// <summary>
        /// A class represents a set of url-related atributes used for connecting a web browser to a remote SocketPro HTTP push server.
        /// </summary>
        public sealed class CDefaultConfig //: IDefaultConfiguration
        {
            private ScriptObject m_soDefaultConfig = null;
            private void SetInternal()
            {
                if (m_soDefaultConfig == null)
                {
                    ScriptObject so = (ScriptObject)HtmlPage.Window.GetProperty("UHTTP");
                    if (so != null)
                    {
                        m_soDefaultConfig = (ScriptObject)so.GetProperty("defaultConfig");
                    }
                }
            }

            internal CDefaultConfig()
            {
            }

            /// <summary>
            /// A protocol used. It can be either 'http:' or 'https:'.
            /// </summary>
            public string Protocol
            {
                get
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return "";
                    object obj = m_soDefaultConfig.GetProperty("protocol");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
                set
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return;
                    string str = value;
                    if(str != null && str.Length > 0)
                    {
                        str = str.Trim(' ', ':');
                        str += ':';
                    }
                    m_soDefaultConfig.SetProperty("protocol", str);
                }
            }

            /// <summary>
            /// A port number which a remote SocketPro HTTP push server is running on.
            /// </summary>
            public uint Port
            {
                get
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return 0;
                    object obj = m_soDefaultConfig.GetProperty("port");
                    if (obj == null)
                        return 0;
                    return (uint)((double)obj);
                }
                set
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return;
                    if (value < 0)
                        value = 0;
                    m_soDefaultConfig.SetProperty("port", value);
                }
            }

            /// <summary>
            /// A path name like '/UCHAT'.
            /// </summary>
            public string PathName
            {
                get
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return "";
                    object obj = m_soDefaultConfig.GetProperty("pathname");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
                set
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return;
                    string str = value;
                    if (str != null && str.Length > 0)
                    {
                        str = str.Trim('/', ' ', '.', '\\');
                        str = "/" + str;
                    }
                    m_soDefaultConfig.SetProperty("pathname", str);
                }
            }

            /// <summary>
            /// A server address for SocketPro HTTP server push. 
            /// </summary>
            public string HostName
            {
                get
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return "";
                    object obj = m_soDefaultConfig.GetProperty("hostname");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
                set
                {
                    SetInternal();
                    if (m_soDefaultConfig == null)
                        return;
                    m_soDefaultConfig.SetProperty("hostname", value);
                }
            }

        }

        /// <summary>
        /// A class represents a JavaScript request. The request object defaults to asynchronous calling. However, it can be synchronous if the request is not cross-site or cross-domain.
        /// </summary>
        public sealed class CRequest
        {
            private int m_nStatcIndex = 0;
            private bool m_bGet = false;
            private bool m_bSync = false;
            private DOnURequest m_cb;
            internal static CRequest m_listen = null;
            internal static List<CRequest> m_lstRequest = new List<CRequest>();
            private ScriptObject m_Request;
            internal CRequest(ScriptObject req)
            {
                m_Request = req;
            }

            /// <summary>
            /// Request name.
            /// </summary>
            public string Name
            {
                get
                {
                    return m_Request.GetProperty("method").ToString();
                }
            }

            /// <summary>
            /// A property indicating if the request is cross-site (or cross-domain) one.
            /// </summary>
            public bool CrossSite
            {
                get
                {
                    return (bool)m_Request.Invoke("isCrossSite");
                }
            }

            /// <summary>
            /// An positive index number for a request.
            /// </summary>
            public long CallIndex
            {
                get
                {
                    object obj = m_Request.Invoke("getCallIndex");
                    return (long)((double)obj);
                }
            }

            /// <summary>
            /// Never use this method from your code!!! It is reserved by UDAParts.
            /// </summary>
            /// <param name="req"></param>
            /// <param name="msg"></param>
            [ScriptableMember()]
            public void OnURequest(ScriptObject req, ScriptObject msg)
            {
                string str = "";
                if (msg != null)
                    str = msg.GetProperty("ret").ToString();
                CRequest request = new CRequest(req);
                foreach (CRequest one in m_lstRequest)
                {
                    if (one.CallIndex == request.CallIndex)
                    {
                        if (one.m_cb != null)
                            one.m_cb.Invoke(one, str);
                        if(one != m_listen)
                            m_lstRequest.Remove(one);
                        break;
                    }
                }
            }

            /// <summary>
            /// Abort a request.
            /// </summary>
            public void Abort()
            {
                m_Request.Invoke("abort");
                try
                {
                    m_lstRequest.Remove(this);
                }
                catch (Exception)
                {
                }
            }

            /// <summary>
            /// Make an async request to a SocketPro HTTP service server.
            /// </summary>
            /// <param name="cb">A callback.</param>
            /// <returns>A non-zero call index or id.</returns>
            public long Invoke(DOnURequest cb)
            {
                return Invoke(cb, false, false);
            }

            /// <summary>
            /// Make an async request to a SocketPro HTTP service server.
            /// </summary>
            /// <param name="cb">A callback.</param>
            /// <param name="bGet">Method (GET or POST) used for HTTP command. Note that it must be true for cross-site request even though it is set to false.</param>
            /// <returns>A non-zero call index or id.</returns>
            public long Invoke(DOnURequest cb, bool bGet)
            {
                return Invoke(cb, bGet, false);
            }

            /// <summary>
            /// Make a request to a SocketPro HTTP service server.
            /// </summary>
            /// <param name="cb">A callback.</param>
            /// <param name="bGet">Method (GET or POST) used for HTTP command. Note that it must be true for cross-site request even though it is set to false.</param>
            /// <param name="bSync">Sync or Async. Note that it must be async for cross-site (domain) even though it is set to sync.</param>
            /// <returns>A non-zero call index or id.</returns>
            public long Invoke(DOnURequest cb, bool bGet, bool bSync)
            {
                if (CrossSite)
                {
                    bGet = true;
                    bSync = false;
                }
                m_cb = cb;
                m_bSync = bSync;
                m_bGet = bGet;
                if (m_listen == null)
                {
                    m_listen = this;
                    HtmlPage.RegisterScriptableObject("URequest", m_listen);
                }
                if(UHTTP.UChat.ChatId.Length == 38)
                    Add("chatId", UHTTP.UChat.ChatId);
                object objRet = m_Request.Invoke("invoke", "UHTTP.SilverlightCallback", bGet, bSync);
                m_lstRequest.Add(this);
                return (long)((double)objRet);
            }

            /// <summary>
            /// Clear all of the pairs of parameter and value.
            /// </summary>
            public void Clear()
            {
                m_Request.Invoke("clear");
            }

            /// <summary>
            /// Addd parameter and its value in pair.
            /// </summary>
            /// <typeparam name="T">A type.</typeparam>
            /// <param name="strParameterName">A case-sensitive string for parameter name.</param>
            /// <param name="Value">A value.</param>
            public void Add<T>(string strParameterName, T Value)
            {
                if (typeof(T).IsArray)
                    throw new Exception("Use the other version of method Add instead.");
                m_Request.Invoke("add", strParameterName, Value);
            }

            private ScriptObject Add(object[] array)
            {
                ++m_nStatcIndex;
                string strTempArray = string.Format("tempArray{0}", m_nStatcIndex);
                string strEval = string.Format("UHTTP.{0}=[];", strTempArray);
                HtmlPage.Window.Eval(strEval);
                ScriptObject so = (ScriptObject)UHTTP.m_soUHTTP.GetProperty(strTempArray);
                foreach (object obj in array)
                {
                    if (obj is object[])
                    {
                        ScriptObject mySo = Add((object[])obj);
                        so.Invoke("push", mySo);
                    }
                    else if (obj is Dictionary<string, object>)
                    {
                        ScriptObject mySo = Add((Dictionary<string, object>)obj);
                        so.Invoke("push", mySo);
                    }
                    else
                        so.Invoke("push", obj);
                }
                string strDelete = string.Format("UHTTP.{0}=null;", strTempArray);
                HtmlPage.Window.Eval(strDelete);
                --m_nStatcIndex;
                return so;
            }

            private ScriptObject Add(Dictionary<string, object> map)
            {
                ++m_nStatcIndex;
                string strTempMap = string.Format("tempMap{0}", m_nStatcIndex);
                string strEval = string.Format("UHTTP.{0}={{}};", strTempMap);
                HtmlPage.Window.Eval(strEval);
                ScriptObject so = (ScriptObject)UHTTP.m_soUHTTP.GetProperty(strTempMap);
                foreach (string strKey in map.Keys)
                {
                    if (strKey == null || strKey.Length == 0) //simple check
                        throw new Exception("Wrong parameter name!");
                    object obj = map[strKey];
                    if (obj is object[])
                    {
                        ScriptObject mySo = Add((object[])obj);
                        so.SetProperty(strKey, mySo);
                    }
                    else if (obj is Dictionary<string, object>)
                    {
                        ScriptObject mySo = Add((Dictionary<string, object>)obj);
                        so.SetProperty(strKey, mySo);
                    }
                    else
                        so.SetProperty(strKey, obj);
                }
                string strDelete = string.Format("UHTTP.{0}=null;", strTempMap);
                HtmlPage.Window.Eval(strDelete);
                --m_nStatcIndex;
                return so;
            }

            /// <summary>
            /// Add an input parameter and its value in pair.
            /// </summary>
            /// <param name="strParameterName">A case-sensitive parameter name.</param>
            /// <param name="map">A map containing a collection of parameter name and value in pair.</param>
            public void Add(string strParameterName, Dictionary<string, object> map)
            {
                if (map == null)
                    m_Request.Invoke("add", strParameterName, null);
                else
                    m_Request.Invoke("add", strParameterName, Add(map));
            }

            /// <summary>
            /// Add an input parammeter and its value in pair.
            /// </summary>
            /// <param name="strParameterName">A case-sensitive parameter name.</param>
            /// <param name="array">An array of values.</param>
            public void Add(string strParameterName, object[] array)
            {
                if (array == null)
                    m_Request.Invoke("add", strParameterName, null);
                else
                    m_Request.Invoke("add", strParameterName, Add(array));
            }

            /// <summary>
            /// A script (cross domain) or XMLHttpRequest (same domain) object.
            /// </summary>
            public ScriptObject Script
            {
                get
                {
                    object objRet = m_Request.Invoke("getScript");
                    if (objRet == null)
                        return null;
                    return (ScriptObject)objRet;
                }
            }

            /// <summary>
            /// A synchronous request or asynchronous one. It must be false if the request is cross-site or cross-domain.
            /// </summary>
            public bool Sync
            {
                get
                {
                    return m_bSync;
                }
            }

            /// <summary>
            /// The property indicating if the request is sent through HTTP command GET. It must be true if the request is cross-site or cross-domain.
            /// </summary>
            public bool Get
            {
                get
                {
                    return m_bGet;
                }
            }
        }

        /// <summary>
        /// A class stands for the JavaScript object UChat in SocketProAdapter for JavaScript.
        /// Listen or subscribe request is always asynchronous one.
        /// A chatting request defaults to asynchronous one. However, it can be synchronous if it is not cross-site or cross-domain.
        /// </summary>
        public sealed class CUChat
        {
            /// <summary>
            /// An event for a list of messages from a remote SocketPro HTTP push server.
            /// </summary>
            public event DOnChatNotification OnMessage;
            
            /// <summary>
            /// An event for tracking the returning result of a chat-related request.
            /// </summary>
            public event DOnChatRequest OnChatRequest;
            
            private ScriptObject m_UChat = null;
            internal CUChat(ScriptObject uchat)
            {
                m_UChat = uchat;
            }

            internal void SetJsChat()
            {
                UHTTP.SetUHTTP();
                if (m_UChat == null)
                {
                    ScriptObject so = (ScriptObject)HtmlPage.Window.GetProperty("UChat");
                    if (so != null)
                    {
                        m_UChat = so;
                        HtmlPage.RegisterScriptableObject("UPush", this);
                    }
                }
            }

            private CRequest createReq(object obj)
            {
                if (obj == null)
                    return null;
                string str = obj.ToString();
                if (str.Length < 5)
                    return null;
                return new CRequest((ScriptObject)obj);
            }

            /// <summary>
            /// Read or set using synchronous (true) or asynchronous chatting (false). The property must be false if it is cross-site or domain.
            /// </summary>
            public bool Sync
            {
                get
                {
                    if (m_UChat == null)
                        return false;
                    object obj = m_UChat.GetProperty("sync");
                    return ((double)obj > 0);
                }
                set
                {
                    if(m_UChat != null)
                        m_UChat.SetProperty("sync", value ? 1 : 0);
                }
            }

            /// <summary>
            /// Read or set HTTP command GET (true) or POST (false) used. The property must be true if it is cross-site or domain.
            /// </summary>
            public bool Get
            {
                get
                {
                    if (m_UChat == null)
                        return false;
                    object obj = m_UChat.GetProperty("isGet");
                    return ((double)obj > 0);
                }
                set
                {
                    if (m_UChat != null)
                        m_UChat.SetProperty("isGet", value ? 1 : 0);
                }
            }

            /// <summary>
            /// Cross site (domain) or same site (domain) chatting.
            /// </summary>
            public bool CrossSite
            {
                get
                {
                    if (m_UChat == null)
                        return false;
                    object obj = m_UChat.GetProperty("crossSite");
                    return ((double)obj > 0);
                }
            }

            /// <summary>
            /// Never use this method from your code!!! It is reserved by UDAParts.
            /// </summary>
            /// <param name="req"></param>
            /// <param name="msg"></param>
            [ScriptableMember()]
            public void OnPushMessage(ScriptObject req, ScriptObject msg)
            {
                CRequest request = new CRequest(req);
                switch (request.Name)
                {
                    case "enter":
                    case "speak":
                    case "exit":
                    case "sendUserMessage":
                        if (OnChatRequest != null)
                        {
                            object objResult = null;
                            if (msg != null)
                                objResult = msg.GetProperty("ret");
                            OnChatRequest.Invoke(request, objResult);
                        }
                        break;
                    case "listen":
                        CHttpPush hp = new CHttpPush(msg);
                        if (OnMessage != null && hp.m_lstMessage != null)
                            OnMessage.Invoke(request, hp);
                        else if (hp.m_lstMessage == null && OnChatRequest != null)
                        {
                            object objResult = null;
                            if (msg != null)
                                objResult = msg.GetProperty("ret");
                            OnChatRequest.Invoke(request, objResult);
                        }
                        break;
                    default:
                        break;
                }
            }

  /*        /// <summary>
            /// Joing one or more HTTP chatting groups.
            /// </summary>
            /// <param name="strUserId">A use id to represent this client.</param>
            /// <param name="nGroups">A number (for example, 11 = 1|2|8) indicating one or more chatting groups.</param>
            /// <returns></returns>
            public CRequest Enter(string strUserId, uint nGroups)
            {
                object obj = m_UChat.Invoke("enter", strUserId, nGroups);
                return createReq(obj);
            }*/

            internal Object CreateArray(uint[] array)
            {
                string strCreateArray = "UHTTP.UINTARRAY=[";
                if (array != null)
                {
                    int j = 0;
                    foreach (uint n in array)
                    {
                        if (j > 0)
                            strCreateArray += ",";
                        strCreateArray += n.ToString();
                        j++;
                    }
                }
                strCreateArray += "];";
                return HtmlPage.Window.Eval(strCreateArray);
            }

            /// <summary>
            /// Joing one or more HTTP chatting groups.
            /// </summary>
            /// <param name="strUserId">A use id to represent this client.</param>
            /// <param name="Groups">An array of chatting group ids.</param>
            /// <returns></returns>
            public CRequest Enter(string strUserId, uint[] Groups)
            {
                if (Groups == null || Groups.Length == 0)
                    return Exit();
                SetJsChat();
                if (m_UChat == null)
                    return null;
                object objGroups = CreateArray(Groups);
                object obj = m_UChat.Invoke("enter", strUserId, objGroups);
                return createReq(obj);
            }

            /// <summary>
            /// Exit from HTTP chatting groups.
            /// </summary>
            /// <returns></returns>
            public CRequest Exit()
            {
                object obj;
                SetJsChat();
                if (m_UChat == null)
                    return null;
                try
                {
                    obj = m_UChat.Invoke("exit");
                }
                catch (Exception)
                {
                    return null;
                }
                return createReq(obj);
            }

/*
            /// <summary>
            /// Send a message onto one or more chatting groups.
            /// </summary>
            /// <param name="strMsg">A message string.</param>
            /// <param name="nGroups">A number (for example, 47 = 1|2|4|8|32) indicating one or more chatting groups.</param>
            /// <returns>A request object.</returns>
            public CRequest Speak(string strMsg, uint nGroups)
            {
                object obj = m_UChat.Invoke("speak", strMsg, nGroups);
                return createReq(obj);
            }
*/
            /// <summary>
            /// Send a message onto one or more chatting groups.
            /// </summary>
            /// <param name="strMsg">A message string.</param>
            /// <param name="Groups">An array of chatting group ids.</param>
            /// <returns>A request object.</returns>
            public CRequest Speak(string strMsg, uint[] Groups)
            {
                if (Groups == null)
                    Groups = new uint[0];
                SetJsChat();
                if (m_UChat == null)
                    return null;
                object objGroups = CreateArray(Groups);
                object obj = m_UChat.Invoke("speak", strMsg, objGroups);
                return createReq(obj);
            }

            /// <summary>
            /// Send a message to one specific client identified by a case-insensitive receiver.
            /// </summary>
            /// <param name="strMsg">A message string</param>
            /// <param name="strReceiver">A user id for a receiver. It is case-insensitive.</param>
            /// <returns>A request object.</returns>
            public CRequest SendUserMessage(string strMsg, string strReceiver)
            {
                SetJsChat();
                if (m_UChat == null)
                    return null;
                object obj = m_UChat.Invoke("sendUserMessage", strMsg, strReceiver);
                return createReq(obj);
            }

            /// <summary>
            /// Subscribe messages from SocketPro HTTP push server with delay time. Usually, you will not use this method. The library will take care of it for you on the fly.
            /// </summary>
            /// <param name="nDelayTime">A delay time in ms.</param>
            public void Subscribe(uint nDelayTime)
            {
                SetJsChat();
                if (m_UChat == null)
                    return;
                m_UChat.Invoke("subscribe", nDelayTime);
            }

            /// <summary>
            /// Indicating if it is chatting.
            /// </summary>
            public bool Chatting
            {
                get
                {
                    if (m_UChat == null)
                        return false;
                    object obj = m_UChat.Invoke("chatting");
                    if (obj == null)
                        return false;
                    return (bool)obj;
                }
            }
            /// <summary>
            /// A chat id (or session) string of GUID.
            /// </summary>
            public string ChatId
            {
                get
                {
                    if (m_UChat == null)
                        return "";
                    object obj = m_UChat.Invoke("chatId");
                    if (obj == null)
                        return "";
                    return obj.ToString();
                }
            }
        }

        /// <summary>
        /// A static class represents a singleton of the JavaScript object UHTTP inside SocketProAdapter for JavaScript.
        /// </summary>
        public static class UHTTP
        {
            private static string m_strSpAddress = "";
            /// <summary>
            /// A url address to a remote (COMET) SocketPro server.
            /// </summary>
            public static string SocketProServerAddress
            {
                get
                {
                    return m_strSpAddress;
                }
                set
                {
                    m_strSpAddress = value;
                    if (m_strSpAddress == null)
                        m_strSpAddress = "";
                    m_strSpAddress = m_strSpAddress.ToLower();
                    int nIndex = m_strSpAddress.IndexOf("ujsonxml.js");
                    if (nIndex == -1)
                    {
                        m_strSpAddress = m_strSpAddress.Trim(' ', '/', '\\', '\r', '\n', '\t');
                        m_strSpAddress += "/ujsonxml.js";
                    }
                    if (m_strSpAddress.Length > 10 && m_soUHTTP == null)
                        CreateUScript();
                }
            }

            private static void SetUjsonXamlAddress()
            {
                ScriptObjectCollection soc = (ScriptObjectCollection)HtmlPage.Document.GetElementsByTagName("script");
                foreach (ScriptObject so in soc)
                {
                    string str = so.GetProperty("src").ToString().ToLower();
                    if (str.IndexOf("ujsonxml.js") != -1)
                    {
                        m_strSpAddress = str;
                        break;
                    }
                }
            }

            private static ScriptObject CreateUScript()
            {
                HtmlElement so = HtmlPage.Document.CreateElement("script");
                so.SetProperty("type", "text/javascript");
                so.SetProperty("src", m_strSpAddress);
                so.SetProperty("charset", "utf-8");
                HtmlPage.Document.Body.AppendChild(so);
                return so;
            }
            internal static void SetUHTTP()
            {
                if (m_soUHTTP != null)
                    return;
                m_soUHTTP = (ScriptObject)HtmlPage.Window.GetProperty("UHTTP");
                if (m_soUHTTP != null)
                {
                    string strJS = null;
                    ScriptObjectCollection soc = (ScriptObjectCollection)HtmlPage.Document.GetElementsByTagName("object");
                    foreach (ScriptObject so in soc)
                    {
                        if (so == null)
                            continue;
                        string strId = null;
                        object obj = so.GetProperty("id");
                        if (obj != null)
                        {
                            strId = obj.ToString();
                            strId = strId.Trim(' ', '/', '\\', '\t', '\n');
                        }
                        string strSrc = null;
                        obj = so.GetProperty("type");
                        if(obj != null)
                            strSrc = obj.ToString().ToLower();
                        if (strSrc != null && strSrc.IndexOf("application/x-silverlight") != -1)
                        {
                            if (strId == null || strId.Length == 0)
                            {
                                strId = "myXamlFromSocketPro";
                                so.SetProperty("id", strId);
                            }
                            strJS = string.Format("UChat.onMessage=function(req,msg){{var ctrl=document.getElementById('{0}');ctrl.Content.UPush.OnPushMessage(req,msg);}};UHTTP.SilverlightCallback=function(req,msg){{if(typeof msg =='string')msg=UJSON.parse(msg);var ctrl=document.getElementById('{0}');ctrl.Content.URequest.OnURequest(req,msg);}};", strId);
                            HtmlPage.Window.Eval(strJS);
                            break;
                        }
                    }
                    if (strJS == null)
                        throw new Exception("No silverlight control with a valid id!");
                }
            }
            private static CUChat m_UChat = new CUChat(null);
            private static CDefaultConfig m_df = new CDefaultConfig();
            internal static ScriptObject m_soUHTTP = null;
            static UHTTP()
            {
                SetUjsonXamlAddress();
                SetUHTTP();
            }

            /// <summary>
            /// Create a HTTP request object.
            /// </summary>
            /// <param name="strMethodName">A method name.</param>
            /// <param name="strChannel">A channel for processing the request on server side.</param>
            /// <returns>A HTTP request object.</returns>
            public static CRequest CreateHttpRequest(string strMethodName, string strChannel)
            {
                SetUHTTP();
                if (m_soUHTTP == null)
                    return null;
                if (strChannel == null)
                    strChannel = "";
                strChannel = strChannel.Trim(' ', '/', '\\');
                strChannel = "/" + strChannel;
                string strProtocol = UHTTP.DefaultConfiguration.Protocol.Trim(':');
                string strConfig = String.Format("config={{protocol:'{0}:',port:{1},hostname:'{2}',pathname:'{3}'}};",
                    strProtocol, 
                    UHTTP.DefaultConfiguration.Port, 
                    UHTTP.DefaultConfiguration.HostName, 
                    strChannel);
                object obj = m_soUHTTP.Invoke("createRequest", strMethodName, false, null, strConfig);
                if (obj == null)
                    return null;
                CRequest Req = new CRequest((ScriptObject)obj);
                return Req;
            }

            /// <summary>
            /// Default configuration corresponding to the JavaScript object defaultConfig inside SocketProAdapter for JavaScript.
            /// </summary>
            public static CDefaultConfig DefaultConfiguration
            {
                get
                {
                    return m_df;
                }
            }

            /// <summary>
            /// A HTTP chat object corresponding to the JavaScript UChat object inside SocketProAdapter for JavaScript. 
            /// </summary>
            public static CUChat UChat
            {
                get
                {
                    m_UChat.SetJsChat();
                    return m_UChat;
                }
            }
        }
    }
}
