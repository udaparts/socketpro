using SocketProAdapter.ClientSide;

public class HelloWorld : CAsyncServiceHandler
{
    public HelloWorld()
        : base(hwConst.sidHelloWorld)
    {
    }

    public string SayHello(string firstName, string lastName)
    {
        string SayHelloRtn;
        bool bProcessRy = ProcessR1(hwConst.idSayHelloHelloWorld, firstName, lastName, out SayHelloRtn);
        return SayHelloRtn;
    }

    public void Sleep(int ms)
    {
        bool bProcessRy = ProcessR0(hwConst.idSleepHelloWorld, ms);
    }

    public CMyStruct Echo(CMyStruct ms)
    {
        CMyStruct EchoRtn;
        bool bProcessRy = ProcessR1(hwConst.idEchoHelloWorld, ms, out EchoRtn);
        return EchoRtn;
    }
}
