/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

//server implementation for service HelloWorld
public class HelloWorldPeer : CClientPeer
{
    public static string ToString(uint[] groups)
    {
        int n = 0;
        string s = "[";
        foreach (uint id in groups)
        {
            if (n != 0)
                s += ", ";
            s += id;
            ++n;
        }
        s += "]";
        return s;
    }

    protected override void OnSwitchFrom(uint oldServiceId)
    {
        uint[] chat_groups = { 1, 3 };
        Push.Subscribe(chat_groups);
    }

    protected override void OnSubscribe(uint[] groups)
    {
        Console.WriteLine(UID + " subscribes for groups " + ToString(groups));
    }

    protected override void OnUnsubscribe(uint[] groups)
    {
        Console.WriteLine(UID + " unsubscribes for groups " + ToString(groups));
    }

    protected override void OnPublish(object message, uint[] groups)
    {
        Console.WriteLine(UID + " publishes a message (" + message + ") to groups " + ToString(groups));
    }

    protected override void OnSendUserMessage(string receiver, object message)
    {
        Console.WriteLine(UID + " sends a message (" + message + ") to " + receiver);
    }

	[RequestAttr(hwConst.idSayHelloHelloWorld)]
	private string SayHello(string firstName, string lastName)
	{
        //processed within main thread
        System.Diagnostics.Debug.Assert(CSocketProServer.IsMainThread);

        //notify a message to groups [2, 3] at server side
        Push.Publish("Say hello from " + firstName + " " + lastName, 2, 3);

        string res = "Hello " + firstName + " " + lastName;
        Console.WriteLine(res);
        return res;
	}

	[RequestAttr(hwConst.idSleepHelloWorld, true)] //true -- slow request
	private void Sleep(int ms)
	{
        //processed within a worker thread
        System.Diagnostics.Debug.Assert(!CSocketProServer.IsMainThread);

        System.Threading.Thread.Sleep(ms);
	}

    [RequestAttr(hwConst.idEchoHelloWorld)]
    private CMyStruct Echo(CMyStruct ms)
    {
        return ms;
    }
}

