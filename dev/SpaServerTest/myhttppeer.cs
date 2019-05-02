using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Diagnostics;

public class CMyHttpPeer : CHttpPeerBase
{
    protected override bool DoAuthentication(string userId, string password)
    {
        Push.Subscribe(1, 2, 7);
        Console.Write("User id = " + userId);
        Console.WriteLine(", password = " + password);
        return true;
    }

    protected override void OnGet()
    {
        switch (Path)
        {
            default:
                DownloadFile(Path.Substring(1));
                break;
        }
    }

    protected override void OnPost()
    {
        uint res = SendResult("+++ POST +++ test result");
    }

    protected override void OnUserRequest()
    {
        switch (RequestName)
        {
            case "doRequest":
                {
                    foreach (object obj in Args)
                    {
                        if (obj == null)
                            Console.WriteLine("null");
                        else
                            Console.WriteLine(obj.ToString());
                    }
                }
                SendResult("2012-08-23T18:55:11.052Z");
                break;
            case "doEnter":
                SendResult("Ok -- doEnter");
                break;
            case "doSpeak":
                SendResult("Ok -- doSpeak");
                break;
            case "doSendUserMsg":
                Push.SendUserMessage(Args[1], (string)Args[0]);
                SendResult("Ok -- doSendUserMsg");
                break;
            case "doExit":
                Push.Unsubscribe();
                SendResult("Ok -- doExit");
                break;
            case "doException":
                SendExceptionResult("doException", Environment.StackTrace);
                break;
            case "sendLargeText":
                SendResult((string)Args[0]);
                break;
            default:
                SetResponseCode(501);
                SendResult("Method not implemented");
                break;
        }
    }
}