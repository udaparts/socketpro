using System;
using SocketProAdapter.ServerSide;

class Program {
    static void Main(string[] args) {
        using (CSqlPlugin plugin = new CSqlPlugin()) {
            if (!plugin.Run(20901))
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            Console.WriteLine("Input a line to close the application ......");
            Console.ReadLine();
        }
    }
}
