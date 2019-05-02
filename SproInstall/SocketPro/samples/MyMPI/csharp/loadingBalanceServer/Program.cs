using System;
using System.Collections.Generic;
using System.Text;

namespace LoadingBalance
{
    class Program
    {
        static void Main(string[] args)
        {
            CMySocketProServer MySocketProServer = new CMySocketProServer();
            if (!MySocketProServer.Run(20910))
            {
                Console.WriteLine("Error code = " + SocketProAdapter.ServerSide.CSocketProServer.LastSocketError.ToString());
            }
            Console.WriteLine("Input a line to close the application ......");
            string str = Console.ReadLine();
            MySocketProServer.StopSocketProServer();
        }
    }
}
