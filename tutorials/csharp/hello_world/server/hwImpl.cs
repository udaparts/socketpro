/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter.ServerSide;

//server implementation for service HelloWorld
public class HelloWorldPeer : CClientPeer
{
    [RequestAttr(hwConst.idSayHelloHelloWorld)]
    private string SayHello(string firstName, string lastName)
    {
        if (firstName == null || firstName.Length == 0)
        {
            throw new SocketProAdapter.CServerError(123456, "First name cannot be empty");
        }
        string res = "Hello " + firstName + " " + lastName;
        Console.WriteLine(res);
        return res;
    }

    [RequestAttr(hwConst.idSleepHelloWorld, true)] //true -- slow request
    private void Sleep(int ms)
    {
        if (ms < 0)
        {
            throw new SocketProAdapter.CServerError(654321, "Sleep time cannot be less than zero");
        }
        System.Threading.Thread.Sleep(ms);
    }

    [RequestAttr(hwConst.idEchoHelloWorld)]
    private CMyStruct Echo(CMyStruct ms)
    {
        return ms;
    }
}
