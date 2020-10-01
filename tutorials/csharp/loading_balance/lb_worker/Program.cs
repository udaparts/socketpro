using System;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("This is worker client. Remote router host: ");
        CConnectionContext cc = new CConnectionContext(Console.ReadLine(), 20901, "lb_worker", "pwdForlb_worker");
        using (CSocketPool<PiWorker> spPi = new CSocketPool<PiWorker>())
        {
            spPi.StartSocketPool(cc, 1, (uint)Environment.ProcessorCount);
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
