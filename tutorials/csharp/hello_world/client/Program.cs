using System;
using System.Threading.Tasks;
using SocketProAdapter.ClientSide;
using SocketProAdapter;
class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        using (CSocketPool<HelloWorld> spHw = new CSocketPool<HelloWorld>(true)) //1, true -- automatic reconnecting
        {
            //optionally start a persistent queue at client side to ensure auto failure recovery and once-only delivery
            //spHw.QueueName = "helloworld";
            CMyStruct ms, msOrig = CMyStruct.MakeOne();

            bool ok = spHw.StartSocketPool(cc, 1); //2
            HelloWorld hw = spHw.Seek(); //3 or HelloWorld hw = spHw.Lock();
            try
            {
                //All seven requests are streamed with in-line batch for the best network efficiency
                var t0 = hw.Async(hwConst.idSayHelloHelloWorld, "John", "Dole");
                var t1 = hw.Async(hwConst.idSleepHelloWorld, (int)4000);
                var t2 = hw.Async(hwConst.idEchoHelloWorld, msOrig);
                var t3 = hw.Async(hwConst.idSayHelloHelloWorld, "Jack", "Smith");
                var t4 = hw.Async(hwConst.idSayHelloHelloWorld, "Donald", "Trump");
                var t5 = hw.Async(hwConst.idSleepHelloWorld, (int)15000);
                var t6 = hw.Async(hwConst.idSayHelloHelloWorld, "Hillary", "Clinton");
                var t7 = hw.Async(hwConst.idEchoHelloWorld, msOrig);

                Console.WriteLine(t0.Result.Load<string>());
                Console.WriteLine("Returned buffer size should be " + t1.Result.UQueue.Size + " because server returns nothing");
                ms = t2.Result.Load<CMyStruct>();
                Console.WriteLine(t3.Result.Load<string>());
                Console.WriteLine(t4.Result.Load<string>());
                Console.WriteLine("Returned buffer size should be " + t5.Result.UQueue.Size + " because server returns nothing");
                Console.WriteLine(t6.Result.Load<string>());
                ms = t2.Result.Load<CMyStruct>();
            }
            catch (AggregateException ex)
            {
                foreach (Exception e in ex.InnerExceptions)
                {
                    //An exception from server (CServerError), Socket closed after sending a request (CSocketError) or request canceled (CSocketError), 
                    Console.WriteLine(e);
                }
            }
            catch (CSocketError ex)
            {
                //Socket is already closed before sending a request
                Console.WriteLine(ex);
            }
            catch (Exception ex)
            {
                //bad operations such as invalid arguments, bad operations and de-serialization errors, and so on
                Console.WriteLine(ex);
            }
            Console.WriteLine("Press ENTER key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
