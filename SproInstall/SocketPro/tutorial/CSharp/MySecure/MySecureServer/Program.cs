using System;
using System.Collections.Generic;
using System.Text;

namespace MySecureServer
{
    class Program
    {
        static void Main(string[] args)
        {
            CMySocketProServer MySocketProServer = new CMySocketProServer();
            if (!MySocketProServer.Run(20901))
            {
                Console.WriteLine("Error code = " + SocketProAdapter.ServerSide.CSocketProServer.LastSocketError.ToString());
            }
            Console.WriteLine("Input a line to close the application ......");
            string str = Console.ReadLine();
            MySocketProServer.StopSocketProServer();
        }
    }
}
