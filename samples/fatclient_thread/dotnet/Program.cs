using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote host: ");
        string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "usqlite_client", "pwd_for_usqlite");
        using (CSocketPool<CSqlite> spSqlite = new CSocketPool<CSqlite>())
        {

        }
    }
}

