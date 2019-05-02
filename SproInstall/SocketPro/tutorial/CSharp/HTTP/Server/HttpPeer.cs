using System;
using System.Collections.Generic;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.IO;

class CHttpPeer : CHttpPeerBase
{
    private int m_nIndex;
    private void DownloadFile(string strFile)
    {
        int res;
        int nBufferSize = 10240;
        FileStream readIn = new FileStream(strFile, FileMode.Open, FileAccess.Read);
        //tell a client the size of the coming file
        bool ok = SetResponseHeader("Content-Length", readIn.Length.ToString());

        byte[] buffer = new byte[nBufferSize];
        int nRead = readIn.Read(buffer, 0, nBufferSize);
        while (nRead > 0)
        {
            res = SendReturnData((short)USOCKETLib.tagHttpRequestID.idGet, buffer, nRead);
            if (res < 0) //client cancels downloading or shuts down TCP/IP connection
                break;
            nRead = readIn.Read(buffer, 0, nBufferSize);
        }
        readIn.Close();

    }
    private void GetFile(string strFile)
    {
        m_UQueue.SetHeadPosition();
        FileStream readIn = new FileStream(strFile, FileMode.Open, FileAccess.Read);
        byte[] buffer = new byte[10240];
        int nRead = readIn.Read(buffer, 0, 10240);
        while (nRead > 0)
        {
            m_UQueue.Push(buffer, nRead);
            nRead = readIn.Read(buffer, 0, 10240);
        }
        readIn.Close();
    }

    protected override void OnFastRequestArrive(short sRequestID, int nLen)
    {
        base.OnFastRequestArrive(sRequestID, nLen);
        switch (sRequestID)
        {
            case (short)USOCKETLib.tagHttpRequestID.idHeader:
                m_nIndex = 0;
                break;
            default:
                m_nIndex = 0;
                break;
        }
    }
    protected override int OnSlowRequestArrive(short sRequestID, int nLen)
    {
        Encoding enc = System.Text.UTF8Encoding.UTF8;
        SetResponseHeader("MyOwnHeader", "Sometext");
        switch (sRequestID)
        {
            case (short)USOCKETLib.tagHttpRequestID.idMultiPart:
                m_nIndex++;
                Console.Write("Size = ");
                Console.Write(nLen.ToString());
                Console.Write(", index = ");
                Console.WriteLine(m_nIndex.ToString());
                break;
            case (short)USOCKETLib.tagHttpRequestID.idGet:

                switch (PathName)
                {
                    case "/":
                    case "/udemo.htm":
                    case "":
                        SetResponseHeader("Content-Type", "text/html");
                        m_UQueue.SetSize(0);
                        GetFile("udemo.htm");
                        SendResult(sRequestID, m_UQueue);
                        break;
                    case "/ujsonxml.js":
                        SetResponseHeader("Content-Type", "application/x-javascript");

                        //trun off connection right after sending response
                        SetResponseHeader("Connection", "close");

                        m_UQueue.SetSize(0);
                        GetFile("ujsonxml.js");
                        SendResult(sRequestID, m_UQueue);
                        break;
                    case "/multipart.htm":
                        SetResponseHeader("Content-Type", "text/html; application/x-javascript");

                        //trun off connection right after sending response
                        SetResponseHeader("Connection", "close");

                        m_UQueue.SetSize(0);
                        GetFile("multipart.htm");
                        SendResult(sRequestID, m_UQueue);
                        break;
                    case "/fupload.htm":
                        SetResponseHeader("Content-Type", "text/html; application/javascript");

                        //trun off connection right after sending response
                        SetResponseHeader("Connection", "close");

                        m_UQueue.SetSize(0);
                        GetFile("fupload.htm");
                        SendResult(sRequestID, m_UQueue);
                        break;
                    case "/chunked.htm":
                        SetResponseHeader("Content-Type", "text/html");
                        SetResponseHeader("Transfer-Encoding", "chunked");

                        m_UQueue.SetSize(0);
                        m_UQueue.Push(enc.GetBytes("<script type='text/javascript'>"));
                        GetFile("ujsonxml.js");
                        m_UQueue.Push(enc.GetBytes("</script>"));
                        SendResult(sRequestID, m_UQueue);

                        m_UQueue.SetSize(0);
                        GetFile("chunked.htm");
                        SendResult(sRequestID, m_UQueue);

                        //tell browser the chunked response is ended
                        SendResult(sRequestID);
                        break;
                    case "/mpart":
                        SetResponseHeader("Keep-Alive", "10"); //10 seconds
					    SetResponseHeader("Content-Type", "multipart/x-mixed-replace; boundary=rnA00A");
                        
					    m_UQueue.SetSize(0);
					    SetResponseHeader("Content-Type", "application/xml; charset=utf-8");
					    m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"));
					    m_UQueue.Push(enc.GetBytes("<content>First Part</content>"));
                        SendResult(sRequestID, m_UQueue);
					    System.Threading.Thread.Sleep(5000);
    					
					    m_UQueue.SetSize(0);
					    SetResponseHeader("Content-Type", "application/xml; charset=utf-8");
					    m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"));
					    m_UQueue.Push(enc.GetBytes("<content>Second Part</content>"));
                        SendResult(sRequestID, m_UQueue);
					    System.Threading.Thread.Sleep(5000);

					    m_UQueue.SetSize(0);
					    SetResponseHeader("Content-Type", "application/xml; charset=utf-8");
					    m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"));
					    m_UQueue.Push(enc.GetBytes("<content>Third Part</content>"));
                        SendResult(sRequestID, m_UQueue);
					    System.Threading.Thread.Sleep(5000);

					    m_UQueue.SetSize(0);
					    SetResponseHeader("Content-Type", "application/xml; charset=utf-8");
					    m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"));
					    m_UQueue.Push(enc.GetBytes("<content>Fourth Part</content>"));
                        SendResult(sRequestID, m_UQueue);
                        System.Threading.Thread.Sleep(5000);

					    m_UQueue.SetSize(0);
					    SetResponseHeader("Content-Type", "application/xml; charset=utf-8");
					    m_UQueue.Push(enc.GetBytes("<?xml version='1.0'?>"));
					    m_UQueue.Push(enc.GetBytes("<content>Ended</content>"));
                        SendResult(sRequestID, m_UQueue);

					    //tell browser the multipart response is ended
                        SendResult(sRequestID);
                        break;
                    case "/sampledownload.dll":
                        SetResponseHeader("Connection", "close");
                        DownloadFile("sampledownload.dll");
                        break;
                    case "/processing":
                        SetResponseCode(500); //not implemented
                        SendResult(sRequestID);
                        break;
                    default:
                        SetResponseCode(404); //404 Not Found
                        SendResult(sRequestID);
                        break;
                }
                break;
            case (short)USOCKETLib.tagHttpRequestID.idPost:
                switch (PathName)
                {
                    case "/processing":
                        //trun off connection right after sending response
                        //SetResponseHeader("Connection", "close");

                        SetResponseHeader("Content-Type", "application/json");

                        //echo back to client
                        SendResult(sRequestID, m_UQueue);
                        break;
                    case "/uploadfile":
                        SetResponseHeader("Content-Type", "text/html; application/javascript");

                        //trun off connection right after sending response
                        SetResponseHeader("Connection", "close");

                        m_UQueue.SetSize(0);
                        GetFile("fupload.htm");
                        SendResult(sRequestID, m_UQueue);
                        break;
                    default:
                        //trun off connection right after sending response
                        SetResponseHeader("Connection", "close");

                        SetResponseCode(404); //404 Not Found
                        SendResult(sRequestID);
                        break;
                }
                break;
            default:
                break;
        }
        return 0;
    }
}

