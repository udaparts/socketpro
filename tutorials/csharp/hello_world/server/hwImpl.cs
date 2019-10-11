/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter.ServerSide;

//server implementation for service HelloWorld
public class HelloWorldPeer : CClientPeer
{
    [RequestAttr(hwConst.idSayHelloHelloWorld)]
    private string SayHello(string firstName, string lastName)
    {
        string res = "Hello " + firstName + " " + lastName;
        Console.WriteLine(res);
        return res;
    }

    [RequestAttr(hwConst.idSleepHelloWorld, true)] //true -- slow request
    private void Sleep(int ms)
    {
        System.Threading.Thread.Sleep(ms);
    }

    [RequestAttr(hwConst.idEchoHelloWorld)]
    private CMyStruct Echo(CMyStruct ms)
    {
        return ms;
    }
}
