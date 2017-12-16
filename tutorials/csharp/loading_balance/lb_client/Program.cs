using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        bool ok;
        CClientSocket.QueueConfigure.MessageQueuePassword = "MyPwdForMsgQueue";
        if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            CClientSocket.QueueConfigure.WorkDirectory = "/home/yye/sp_test/";
        else
            CClientSocket.QueueConfigure.WorkDirectory = "c:\\sp_test";
        Console.WriteLine("This is a client. Remote router host: ");
        CConnectionContext cc = new CConnectionContext(Console.ReadLine(), 20901, "lb_client", "pwd_lb_client");
        using (CSocketPool<Pi> spPi = new CSocketPool<Pi>(true)) //true -- automatic reconnecting
        {
            ok = spPi.StartSocketPool(cc, 1, 1);
            CClientSocket cs = spPi.Sockets[0];

            //use persistent queue to ensure auto failure recovery and at-least-once or once-only delivery
            ok = cs.ClientQueue.StartQueue("pi_queue", 24 * 3600, (cs.EncryptionMethod == tagEncryptionMethod.TLSv1));
            cs.ClientQueue.RoutingQueueIndex = true;
            
            Pi pi = spPi.AsyncHandlers[0];
            pi.WaitAll(); //make sure all existing queued requests are processed before executing next requests

            double dPi = 0.0;
            int nDivision = 1000;
            int nNum = 10000000;
            double dStep = 1.0 / nNum / nDivision;
            int nReturns = 0;
            for (int n = 0; n < nDivision; ++n) {
                double dStart = (double) n / nDivision;
                ok = pi.SendRequest(piConst.idComputePi, dStart, dStep, nNum, (ar) => {
                    double res;
                    ar.Load(out res);
                    dPi += res;
                    ++nReturns;
                });
            }
            ok = pi.WaitAll();
            Console.WriteLine("Your pi = {0}, returns = {1}", dPi, nReturns);
            Console.WriteLine("Press ENTER key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}

