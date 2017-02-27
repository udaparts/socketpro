using System;

namespace PerfSvr
{
    class Program
    {
        static void Main(string[] args)
        {
            CMySocketProServer MySocketProServer = new CMySocketProServer();
            if (!MySocketProServer.Run(21911))
            {
                Console.WriteLine("Error code = " + SocketProAdapter.ServerSide.CSocketProServer.LastSocketError.ToString());
            }
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
            MySocketProServer.StopSocketProServer();
        }
    }
}
