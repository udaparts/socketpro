using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;

namespace RAdoServer
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class RAdoHost
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		static void Main(string[] args)
		{
            CMySocketProServer MySocketProServer = new CMySocketProServer();
            if (!MySocketProServer.Run(20901))
            {
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            }
            Console.WriteLine("Input a line to close the application ......");
            string str = Console.ReadLine();
            MySocketProServer.StopSocketProServer();
		}
	}
}
