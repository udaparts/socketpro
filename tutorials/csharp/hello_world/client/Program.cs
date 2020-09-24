using System;
using SocketProAdapter.ClientSide;
using Task = System.Threading.Tasks.Task<SocketProAdapter.CScopeUQueue>;
class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "hwClientUserId", "password4hwClient");
        using (CSocketPool<HelloWorld> spHw = new CSocketPool<HelloWorld>())
        {
            //optionally start a persistent queue at client side to
            //ensure auto failure recovery and once-only delivery
            //spHw.QueueName = "helloworld";
            CMyStruct ms, msOrig = CMyStruct.MakeOne();
            if (spHw.StartSocketPool(cc, 1))
            {
                HelloWorld hw = spHw.Seek();
                try
                {
                    //process requests one by one synchronously
                    Task t0 = hw.send(hwConst.idSayHelloHelloWorld, "John", "Dole");
                    Console.WriteLine(t0.Result.Load<string>());
                    Task t1 = hw.send(hwConst.idSleepHelloWorld, (int)4000);
                    Console.WriteLine("Returned buffer size should be " + t1.Result.UQueue.Size + " because server returns nothing");
                    Task t2 = hw.send(hwConst.idEchoHelloWorld, msOrig);
                    ms = t2.Result.Load<CMyStruct>();

                    //All requests are streamed with in-line batch for the best network efficiency
                    t0 = hw.send(hwConst.idSayHelloHelloWorld, "John", "Dole");
                    t1 = hw.send(hwConst.idSleepHelloWorld, (int)4000);
                    t2 = hw.send(hwConst.idEchoHelloWorld, msOrig);
                    Task t3 = hw.send(hwConst.idSayHelloHelloWorld, "Jack", "Smith");
                    Task t4 = hw.send(hwConst.idSayHelloHelloWorld, "Donald", "Trump");
                    Task t5 = hw.send(hwConst.idSleepHelloWorld, (int)15000);
                    Task t6 = hw.send(hwConst.idSayHelloHelloWorld, "Hillary", "Clinton");
                    Task t7 = hw.send(hwConst.idEchoHelloWorld, msOrig);
                    //hw.Socket.Cancel();
                    Console.WriteLine(t0.Result.Load<string>());
                    Console.WriteLine("Returned buffer size should be " + t1.Result.UQueue.Size + " because server returns nothing");
                    ms = t2.Result.Load<CMyStruct>();
                    Console.WriteLine(t3.Result.Load<string>());
                    Console.WriteLine(t4.Result.Load<string>());
                    Console.WriteLine("Returned buffer size should be " + t5.Result.UQueue.Size + " because server returns nothing");
                    Console.WriteLine(t6.Result.Load<string>());
                    ms = t7.Result.Load<CMyStruct>();
                }
                catch (AggregateException ex)
                {
                    foreach (Exception e in ex.InnerExceptions)
                    {
                        //An exception from server (CServerError), Socket closed
                        //after sending a request (CSocketError) or canceled (CSocketError),
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
                    //bad operations such as invalid arguments,
                    //bad operations and de-serialization errors, and so on
                    Console.WriteLine(ex);
                }
            }
            else
            {
                Console.WriteLine("No connection to server with error message: " + spHw.Sockets[0].ErrorMsg);
            }
            Console.WriteLine("Press ENTER key to kill the demo ......");
            Console.ReadLine();
        }
    }
}
