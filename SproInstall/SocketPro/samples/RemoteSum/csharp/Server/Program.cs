using System;
using System.Collections.Generic;
using System.Text;
using SocketProAdapter.ServerSide;

namespace Server
{
    class Program
    {
        static void Main(string[] args)
        {
            CMySocketProServer MySocketProServer = new CMySocketProServer();
            if (!MySocketProServer.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            string str = Console.ReadLine();
            MySocketProServer.StopSocketProServer(); //or MySocketProServer.Dispose();
        }
    }
}
