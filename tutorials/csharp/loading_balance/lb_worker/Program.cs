using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "lb_worker", "pwdForlb_worker");
        using (CSocketPool<PiWorker> spPi = new CSocketPool<PiWorker>(true)) //true -- automatic reconnecting
        {
            spPi.StartSocketPool(cc, 1);
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}

