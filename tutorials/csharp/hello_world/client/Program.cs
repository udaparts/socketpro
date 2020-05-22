using System;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        using (CSocketPool<HelloWorld> spHw = new CSocketPool<HelloWorld>(true)) //true -- automatic reconnecting
        {
            //optionally start a persistent queue at client side to ensure auto failure recovery and once-only delivery
            //spHw.QueueName = "helloworld";

            bool ok = spHw.StartSocketPool(cc, 1, 1);
            HelloWorld hw = spHw.Seek(); //or HelloWorld hw = spHw.Lock();

            //process requests one by one synchronously
            Console.WriteLine(hw.SayHello("Jone", "Dole"));
            hw.Sleep(5000);
            CMyStruct msOriginal = CMyStruct.MakeOne();
            CMyStruct ms = hw.Echo(msOriginal);

            //asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Jack", "Smith", (ar) =>
            {
                string ret;
                ar.Load(out ret);
                Console.WriteLine(ret);
            });
            CAsyncServiceHandler.DAsyncResultHandler arh = null;
            ok = hw.SendRequest(hwConst.idSleepHelloWorld, (int)5000, arh);
            ok = hw.SendRequest(hwConst.idEchoHelloWorld, msOriginal, (ar) =>
            {
                ar.Load(out ms);
            });
            ok = hw.WaitAll();
            Console.WriteLine("Press ENTER key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
