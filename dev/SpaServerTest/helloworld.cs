using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

class CHelloWorld : CClientPeer
{
    [RequestAttr(HwConst.idSayHello, true)]
    string SayHello(string name, int index)
    {
        string str = "Hello to " + name + " with index = " + index.ToString();
        Console.WriteLine(str);
        return str;
    }
}
