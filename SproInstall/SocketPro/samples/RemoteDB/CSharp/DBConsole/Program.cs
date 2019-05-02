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
    class Program
    {
        [MTAThread]
        static void Main(string[] args)
        {
            CSocketPool<CAsynDBLite> dbPool = new CSocketPool<CAsynDBLite>();
            
            //start a pool socket with one thread, one socket, and one DB handler.
            if (dbPool.StartSocketPool( "localhost", //remote host server
                                        17001, //port for a SocketPro server
                                        "SocketPro", //user id
                                        "PassOne", //password
                                        1, //sockets per thread
                                        2, //number of threads in the pool
                                        tagEncryptionMethod.NoEncryption, //encription method
                                        false //enable online compression or not
                                        ))
            {
                //get an instance of raw COM socket pool object for your debug
                USocketPoolClass spc = dbPool.GetUSocketPool();

                CAsynDBLite DBLite0 = dbPool.Lock();
                CAsynDBLite DBLite1 = dbPool.Lock();

                DBLite0.BeginBatch();
                DBLite0.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb");
                DBLite0.OpenRowset("Select * from Customers", "Customers");
                DBLite0.CommitBatch(null); //send all of requests in batch for processing

                DBLite1.BeginBatch();
                DBLite1.ConnectDB("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=nwind3.mdb");
                DBLite1.OpenRowset("Select * from Products", "Products");
                DBLite1.CommitBatch(null); //send all of requests in batch for processing
                
                //cooperatively blocking
                DBLite0.GetAttachedClientSocket().WaitAll();

                //cooperatively blocking
                DBLite1.GetAttachedClientSocket().WaitAll();


                DataTable dt0 = DBLite0.CurrentDataTable;
                DataTable dt1 = DBLite1.CurrentDataTable;

                //return locked sockets back into pool for reuse
                dbPool.Unlock(DBLite0);
                dbPool.Unlock(DBLite1);

                dbPool.ShutdownPool();
            }
        }
    }
}
