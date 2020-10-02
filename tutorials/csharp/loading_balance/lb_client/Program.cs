using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Collections.Generic;
using System.Threading.Tasks;
class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("This is a client. Remote router host: ");
        CConnectionContext cc = new CConnectionContext(Console.ReadLine(), 20901, "lb_client", "pwd_lb_client");
        using (CSocketPool<Pi> spPi = new CSocketPool<Pi>())
        {
            spPi.QueueName = "lbqueue";
            if (!spPi.StartSocketPool(cc, 1))
            {
                Console.WriteLine("No connection to " + cc.Host);
                Console.WriteLine("Press ENTER key to kill the demo ......");
                Console.ReadLine();
                return;
            }
            Pi pi = spPi.SeekByQueue();

            int nDivision = 1000, nNum = 10000000;
            double dPi = 0, dStep = 1.0 / nNum / nDivision;
            List<Task<CScopeUQueue>> vtR = new List<Task<CScopeUQueue>>();
            for (int n = 0; n < nDivision; ++n)
            {
                double dStart = (double)n / nDivision;
                vtR.Add(pi.send(piConst.idComputePi, dStart, dStep, nNum));
            }

            foreach (var t in vtR)
            {
                CScopeUQueue sb = t.Result;
                dPi += sb.Load<double>();
                Console.WriteLine("dStart: " + sb.Load<double>());
            }

            Console.WriteLine("pi: {0}, returns: {1}", dPi, vtR.Count);
            Console.WriteLine("Press ENTER key to kill the demo ......");
            Console.ReadLine();
        }
    }
}
