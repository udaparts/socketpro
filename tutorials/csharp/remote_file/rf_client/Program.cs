using System;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote SocketPro file streaming server:");
        CConnectionContext cc = new CConnectionContext(Console.ReadLine(), 20901, "MyUserId", "MyPassword");
        using (CSocketPool<CStreamingFile> spRf = new CSocketPool<CStreamingFile>())
        {
            if (!spRf.StartSocketPool(cc, 1))
            {
                Console.WriteLine("Can not connect to remote server and press ENTER key to shutdown the application ......");
                Console.ReadLine(); return;
            }
            CStreamingFile rf = spRf.Seek();
            Console.WriteLine("Input a remote file path:");
            //test both downloading and uploading files in file stream (it is different from byte stream)
            string RemoteFile = Console.ReadLine();
            string LocalFile = "spfile1.test";
            try
            {
                //downloading test
                var td = rf.download(LocalFile, RemoteFile, (file, downloaded) =>
                {
                    Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
                });

                //uploading test
                RemoteFile += ".copy";
                var tu = rf.upload(LocalFile, RemoteFile, (file, uploaded) =>
                {
                    Console.WriteLine("Uploading rate: {0}%", uploaded * 100 / file.FileSize);
                });
                Console.WriteLine(td.Result);
                Console.WriteLine();
                Console.WriteLine(tu.Result);
            }
            catch (AggregateException ex)
            {
                foreach (Exception e in ex.InnerExceptions)
                {
                    //An exception from server (CServerError), Socket closed after sending a request (CSocketError) or request canceled (CSocketError),
                    Console.WriteLine(e);
                }
            }
            catch (CSocketError ex)
            {
                //Socket is already closed before sending a request
                Console.WriteLine(ex);
            }
            catch (Exception ex)
            {
                //bad operations such as invalid arguments, bad operations and de-serialization errors, and so on
                Console.WriteLine(ex);
            }
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
