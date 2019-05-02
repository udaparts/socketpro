using System;
using System.Collections.Generic;
using System.Text;
using System.Data;

using UDBLib;
using USOCKETLib;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.ClientSide.RemoteDB;

namespace DBConsole
{
    class CMyDBSyn : CAsynDBLite
    {
        public bool Connect(string strConn)
        {
            ConnectDB(strConn);
            GetAttachedClientSocket().WaitAll();
            return DBConnected;
        }

        public void Disconnect()
        {
            DisconnectDB();
            GetAttachedClientSocket().WaitAll();
        }

        public bool OpenRowset(string strSQL)
        {
            OpenRowset(strSQL, "MyTable", tagCursorType.ctStatic, CAsynDBLite.Readonly, 20, -1);
            GetAttachedClientSocket().WaitAll();
            return (CurrentDataTable.Columns.Count != 0);
        }

        public bool IsEOF()
        {
            return (CurrentDataTable.Rows.Count == 0);
        }

        public void MoveFirst()
        {
            FirstBatch();
            GetAttachedClientSocket().WaitAll();
        }

        public void MoveNext()
        {
            MoveNext(0);
        }

        public void MoveNext(int nSkip)
        {
            NextBatch(nSkip);
            GetAttachedClientSocket().WaitAll();
        }

        public void MovePrev()
        {
            MoveNext(-2);
        }

        public void MoveLast()
        {
            LastBatch();
            GetAttachedClientSocket().WaitAll();
        }
    }

    class Program
    {
        [MTAThread]
        static void Main(string[] args)
        {
            CSocketPool<CMyDBSyn> dbPool = new CSocketPool<CMyDBSyn>();
            
            //start a pool socket with one thread, one socket, and one DB handler.
            if (dbPool.StartSocketPool( "localhost", //remote host server
                                        17001, //port for a SocketPro server
                                        "SocketPro", //user id
                                        "PassOne", //password
                                        1, //sockets per thread
                                        1, //number of threads in th epool
                                        tagEncryptionMethod.NoEncryption, //encription method
                                        false //enable online compression or not
                                        ))
            {
                CMyDBSyn myDB = dbPool.Lock();
                int nCount = 0;
//                myDB.ConnectDB("Provider=sqlncli;Data Source=localhost\\sqlexpress;Initial Catalog=northwind;Integrated Security=SSPI");
                if (myDB.Connect("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb"))
                {
                    if (myDB.OpenRowset("Select * from Orders"))
                    {
                        while (!myDB.IsEOF())
                        {
                            nCount += myDB.CurrentDataTable.Rows.Count;

                            // process your batch records here

                            myDB.MoveNext();
                        }
                    }
                }
                dbPool.ShutdownPool();
                Console.WriteLine("Recods fetched = " + nCount.ToString());
                Console.WriteLine("Presss the key <ENTER> to exit the application");
                Console.ReadLine();
            }
        }
    }
}
