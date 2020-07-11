using SocketProAdapter.ClientSide;

public class HelloWorld : CAsyncServiceHandler
{
    public HelloWorld()
        : base(hwConst.sidHelloWorld)
    {
    }
}
