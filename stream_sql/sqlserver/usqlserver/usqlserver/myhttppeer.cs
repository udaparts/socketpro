
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

/// <summary>
/// This is a skeleton code for HTTP/web socket, which is not fully implemented.
/// Extends its functionalities or delete the class from your project at your will.
/// </summary>
public class CMyHttpPeer : CHttpPeerBase
{
    protected override void OnSubscribe(uint[] groups)
    {
        
    }

    protected override void OnUnsubscribe(uint[] groups)
    {
        
    }

    protected override void OnPublish(object message, uint[] groups)
    {
       
    }

    protected override void OnSendUserMessage(string receiver, object message)
    {
        
    }

    protected override bool DoAuthentication(string userId, string password)
    {
        Push.Subscribe(1, 2, 7);
        Console.Write("User id = " + userId);
        Console.WriteLine(", password = " + password);
        return true; //true -- permitted; and false -- denied
    }

    protected override void OnGet()
    {
        if (Path.LastIndexOf('.') != 1)
            DownloadFile(Path.Substring(1));
        else
            SendResult("test result --- GET ---");
    }

    protected override void OnPost()
    {
        uint res = SendResult("+++ POST +++ test result");
    }

    protected override void OnUserRequest()
    {
        switch (RequestName)
        {
            case "sleep":
                int ms = int.Parse(Args[0].ToString());
                Sleep(ms);
                SendResult("");
                break;
            case "sayHello":
                SendResult(SayHello(Args[0].ToString(), Args[1].ToString()));
                break;
            default:
                SendResult("");
                break;
        }
    }

    private string SayHello(string firstName, string lastName)
    {
        //notify a message to groups [2, 3] at server side
        Push.Publish("Say hello from " + firstName + " " + lastName, 2, 3);

        return "Hello " + firstName + " " + lastName;
    }

    private void Sleep(int ms)
    {
        System.Threading.Thread.Sleep(ms);
    }
}