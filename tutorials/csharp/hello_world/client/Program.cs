using System;
using System.Threading.Tasks;
using SocketProAdapter.ClientSide;
class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        using (CSocketPool<HelloWorld> spHw = new CSocketPool<HelloWorld>(true)) //1, true -- automatic reconnecting
        {
            //optionally start a persistent queue at client side to ensure auto failure recovery and once-only delivery
            //spHw.QueueName = "helloworld";

            bool ok = spHw.StartSocketPool(cc, 1); //2
            HelloWorld hw = spHw.Seek(); //3 or HelloWorld hw = spHw.Lock();

            //process requests one by one synchronously
            Console.WriteLine(hw.Async<string, string, string>(hwConst.idSayHelloHelloWorld, "Jone", "Dole").Result); //4
            hw.Async(hwConst.idSleepHelloWorld, (uint)5000).Wait(); //5
            CMyStruct msOrig = CMyStruct.MakeOne();
            CMyStruct ms = hw.Async<CMyStruct, CMyStruct>(hwConst.idEchoHelloWorld, msOrig).Result; //6

            //asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Jack", "Smith", (ar) => { //7
                string ret; ar.Load(out ret); Console.WriteLine(ret);
            });
            CAsyncServiceHandler.DAsyncResultHandler arh = null;
            ok = hw.SendRequest(hwConst.idSleepHelloWorld, (int)5000, arh); //8
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Donald", "Trump", (ar) => { //9
                string ret; ar.Load(out ret); Console.WriteLine(ret);
            });
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Hillary", "Clinton", (ar) => { //10
                string ret; ar.Load(out ret); Console.WriteLine(ret);
            });
            Task<CMyStruct> task = hw.Async<CMyStruct, CMyStruct>(hwConst.idEchoHelloWorld, msOrig); //11
            ms = task.Result; //12
            Console.WriteLine("Press ENTER key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
