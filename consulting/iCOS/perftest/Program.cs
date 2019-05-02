using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Threading;

class Program
{
    static CConnectionContext[,] loadConnectionContexts(string ipAddr)
    {
        CConnectionContext[,] ccs = new CConnectionContext[4, 1];
        /*
        ccs[0, 0] = new CConnectionContext("localhost", 20901, "SocketPro", "PassOne");
        ccs[1, 0] = new CConnectionContext("192.168.1.109", 20901, "SocketPro", "PassOne");
        ccs[2, 0] = new CConnectionContext("192.168.1.109", 20901, "SocketPro", "PassOne");
        ccs[3, 0] = new CConnectionContext("192.168.1.109", 20901, "SocketPro", "PassOne");
        */

        ccs[0, 0] = new CConnectionContext(ipAddr, 20901, "SocketPro", "PassOne");
        ccs[1, 0] = new CConnectionContext(ipAddr, 20901, "SocketPro", "PassOne");
        ccs[2, 0] = new CConnectionContext(ipAddr, 20901, "SocketPro", "PassOne");
        ccs[3, 0] = new CConnectionContext(ipAddr, 20901, "SocketPro", "PassOne");

        return ccs;
    }

    static List<string> m_lst = new List<string> { "127.0.0.1", "111.123.212.145", "112.123.212.145", "113.123.212.145", "111.222.121.109" };
    static Random m_random = new Random();

    static List<string> PrepareFakeIpAddresses(uint count)
    {
        List<string> lst = new List<string>();
        for (uint n = 0; n < count; ++n)
        {
            lst.Add(m_lst[m_random.Next(m_lst.Count)]);
        }
        return lst;
    }

    //Overhead of using async and await in C# 4.5 -- 2.5 μs per asynchronous call (http://nzbart.blogspot.com/2013/07/overhead-of-using-async-and-await-in-c.html)
    static async void Test(List<string> ips)
    {
        iCOS.CRCode crc;
        iCOS.Lookup lookup = iCOS.Lookup.GeoIp;
        List<Task<uint>> lstCodes = new List<Task<uint>>(ips.Count);
        foreach (string ip in ips)
        {
            lstCodes.Add(lookup.DoLookup(ip));
        }

        foreach (Task<uint> tc in lstCodes)
        {
            crc = iCOS.Lookup.ToCRCode(await tc);
        }
        lstCodes.Clear();
    }

    static void Main(string[] args)
    {
        int n;
        System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
        Console.WriteLine("tell me ip address:");
        CConnectionContext[,] ccs = loadConnectionContexts(Console.ReadLine());
        int threads = ccs.GetLength(0);
        int sockets_per_thread = ccs.GetLength(1);

        //start one instance of lookup with one socket pool connected to a number of remote SocketPro servers
        bool working = iCOS.Lookup.Initialize(ccs);
        Console.WriteLine("Start memory = " + GC.GetTotalMemory(false));
        List<string> ips = PrepareFakeIpAddresses(2 * 1024 * 1024);
        threads = 2;
        System.Threading.Thread[] vThreads = new Thread[threads];
        sw.Start();
        for (n = 0; n < threads; ++n)
        {
            vThreads[n] = new Thread(() =>
            {
                Test(ips);
            });
            vThreads[n].Start();
        }

        for (n = 0; n < threads; ++n)
        {
            vThreads[n].Join();
        }

        sw.Stop();
        //shutdown the single instance at last
        iCOS.Lookup.Shutdown();
        Console.WriteLine("End memory = " + GC.GetTotalMemory(false));
        Console.WriteLine("Time required for {0} ip addresses = {1}", ips.Count * threads, sw.ElapsedMilliseconds);
        GC.Collect();
        Console.WriteLine("After-Connect memory = " + GC.GetTotalMemory(false));
        Console.WriteLine("Press key ENTER to shutdown the application ......");
        Console.ReadLine();
    }
}

