using System;
using System.Collections.Generic;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.IO;

[Serializable]
class CMyCall
{
    public string method;
    public string version;
    public string callIndex;
    public Dictionary<string, object> parameters;
}

class CHttpPeer : CHttpPushPeer
{
    private static List<string> m_lstAllowIpAddress = new List<string>();
    static CHttpPeer()
    {
        m_lstAllowIpAddress.Add("127.0.0.1");
        m_lstAllowIpAddress.Add("10.1.100.112");
        m_lstAllowIpAddress.Add("10.1.100.103");
        
        //my others ....
    }

    protected override void OnChatRequestComing(USOCKETLib.tagChatRequestID ChatRequestId, object Param0, object Param1)
    {
        string str = "";
        int[] Groups;
        switch (ChatRequestId)
        {
            case USOCKETLib.tagChatRequestID.idEnter:
            case USOCKETLib.tagChatRequestID.idXEnter:
                Groups = (int[])Param0;
                foreach (int n in Groups)
                {
                    if (str.Length > 0) str += ", ";
                    str += n.ToString();
                }
                Console.WriteLine("User {0} joins chat groups {1}", UserID, str);
                break;
            case USOCKETLib.tagChatRequestID.idSpeak:
            case USOCKETLib.tagChatRequestID.idXSpeak:
                Groups = (int[])Param1;
                foreach (int n in Groups)
                {
                    if (str.Length > 0) str += ", ";
                    str += n.ToString();
                }
                Console.WriteLine("User {0} sends a message '{1}' to chat groups {2}", UserID, Param0.ToString(), str);
                break;
            case USOCKETLib.tagChatRequestID.idExit:
                Console.WriteLine("User {0} exits his or her chat groups", UserID);
                break;
            case USOCKETLib.tagChatRequestID.idSendUserMessage:
                if (Param0 == null)
                    Param0 = "null";
                if (Param1 == null)
                    Param1 = "null";
                Console.WriteLine("User {0} sends a message '{1}' to {2}", UserID, Param1.ToString(), Param0.ToString());
                break;
            default:
                break;
        }
    }

    protected override int OnSlowRequestArrive(short sRequestID, int nLen)
    {
        /*
         SocketPro tries to hide browser-related problems as many as possible. However, 
         you may still use the below code to solve a problem that SocketPro does not handle well 
		 usually by setting a proper response header 
        */
        
        //http://www.useragentstring.com/pages/useragentstring.php
        string strUserAgent = Headers["User-Agent"].ToLower();

        Console.WriteLine("BrowserIpAddress = " + BrowserIpAddress + ", data = " + Query);
        return base.OnSlowRequestArrive(sRequestID, nLen);
    }

    protected override bool IsOk(Dictionary<String, object> parameters)
    {
        int nPort = 0;
        string strIpAddress = GetPeerName(ref nPort);

        //control clients from a list of allowed ip addresses.
        //especially useful for cross-domain HTTP Push.
        //return (m_lstAllowIpAddress.IndexOf(strIpAddress) != -1);

        return true;
    }

    protected override bool OnDownloading(string strFile)
    {
        Console.WriteLine(strFile);
        return true;
    }

    protected override string OnProcessingRequest(string strMethod, string strVersion, string strId, Dictionary<string, object> mapParams)
    {
        switch (PathName)
        {
            case "/MyChannel":
                {
                    CMyCall myCall = new CMyCall();
                    myCall.method = strMethod;
                    myCall.version = strVersion;
                    myCall.callIndex = strId;
                    myCall.parameters = mapParams;
                    
                    System.Web.Script.Serialization.JavaScriptSerializer jss = new System.Web.Script.Serialization.JavaScriptSerializer();
                    string str = jss.Serialize(myCall);
                    Console.WriteLine(str);
                    return str;
                }
            case "/TOne":
                switch(strMethod)
                {
                    case "Echo":
                        return mapParams["obj"].ToString();
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        throw new Exception("Request not supported");
    }
}

