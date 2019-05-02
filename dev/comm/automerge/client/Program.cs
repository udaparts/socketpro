
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        CClientSocket.QueueConfigure.MessageQueuePassword = "MyPwdForMsgQueue";
        if (System.Environment.OSVersion.Platform == PlatformID.Unix)
            CClientSocket.QueueConfigure.WorkDirectory = "/home/yye/sp_test/";
        else
            CClientSocket.QueueConfigure.WorkDirectory = "c:\\sp_test";

        uint j;
        Console.WriteLine("Give number of sessions:");
        string s = Console.ReadLine();
        while (!uint.TryParse(s, out j))
        {
            s = Console.ReadLine();
        }

        CConnectionContext[,] cc = new CConnectionContext[j, 1];
        for (uint m = 0; m < j; ++m)
        {
            Console.WriteLine("Give ip address {0}:", m + 1);
            string ipaddress = Console.ReadLine();
            cc[m, 0] = new CConnectionContext(ipaddress, 20901, "lb_client", "pwd_lb_client");
        }

        using (CSocketPool<Pi> spPi = new CSocketPool<Pi>(true)) //true -- automatic reconnecting
        {
            bool ok = spPi.StartSocketPool(cc);
            spPi.QueueAutoMerge = true;
            do
            {
                uint index = 0;
                foreach (CClientSocket cs in spPi.Sockets)
                {
                    //use persistent queue to ensure auto failure recovery and at-least-once or once-only delivery
                    ok = cs.ClientQueue.StartQueue("pi_queue" + index, 24 * 3600, (cs.EncryptionMethod == tagEncryptionMethod.TLSv1));
                    ++index;
                }
                double dPi = 0.0;
                int nDivision = 1000;
                int nNum = 10000000;
                double dStep = 1.0 / nNum / nDivision;
                int nReturns = 0;
                for (int n = 0; n < nDivision; ++n)
                {
                    double dStart = (double)n / nDivision;
                    Pi pi = spPi.SeekByQueue();
                    ok = pi.SendRequest(piConst.idComputePi, dStart, dStep, nNum, (ar) =>
                    {
                        double res;
                        ar.Load(out res);
                        dPi += res;
                        ++nReturns;
                    });
                }
                foreach (CClientSocket cs in spPi.Sockets)
                {
                    cs.WaitAll();
                }
                Console.WriteLine("Your pi = {0}, returns = {1} and give a postive number to continue .....", dPi, nReturns);
                int data;
                s = Console.ReadLine();
                while (!int.TryParse(s, out data))
                {
                    s = Console.ReadLine();
                }
                if (data <= 0)
                    break;
            } while (true);
            Console.WriteLine("Press ENTER key to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
