using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("Remote SocketPro file streaming server:");
        CConnectionContext cc = new CConnectionContext(Console.ReadLine(), 20901, "MyUserId", "MyPassword");
        using (CSocketPool<CStreamingFile> spRf = new CSocketPool<CStreamingFile>())
        {
            bool ok = spRf.StartSocketPool(cc, 1, 1);
            if (!ok)
            {
                Console.WriteLine("Can not connect to remote server and press ENTER key to shutdown the application ......");
                Console.ReadLine();
                return;
            }
            CStreamingFile rf = spRf.Seek();

            //test both downloading and uploading files in file stream (it is different from byte stream)

            string RemoteFile = "jvm.lib";
            string LocalFile = "spfile1.test";
            //downloading test
            ok = rf.Download(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", file.RemoteFile);
            }, (file, downloaded) =>
            {
                //downloading progress
                Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile2.test";
            RemoteFile = "libboost_wave-vc100-mt-sgd-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", file.RemoteFile);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile3.test";
            RemoteFile = "libboost_coroutine-vc100-mt-s-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", file.RemoteFile);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile4.test";
            RemoteFile = "libboost_serialization-vc100-mt-s-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", file.RemoteFile);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile5.test";
            RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", file.RemoteFile);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });
            ok = rf.WaitAll();

            LocalFile = "spfile1.test";
            RemoteFile = "jvm_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", file.RemoteFile);
            }, (file, uploaded) =>
            {
                //uploading progress
                Console.WriteLine("Uploading rate: {0}%", uploaded * 100 / file.FileSize);
            });

            LocalFile = "spfile2.test";
            RemoteFile = "libboost_wave-vc100-mt-sgd-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", file.RemoteFile);
            }, (file, uploaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", uploaded * 100 / file.FileSize);
            });

            LocalFile = "spfile3.test";
            RemoteFile = "libboost_coroutine-vc100-mt-s-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", file.RemoteFile);
            }, (file, uploaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", uploaded * 100 / file.FileSize);
            });

            LocalFile = "spfile4.test";
            RemoteFile = "libboost_serialization-vc100-mt-s-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", file.RemoteFile);
            }, (file, uploaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", uploaded * 100 / file.FileSize);
            });

            LocalFile = "spfile5.test";
            RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", file.RemoteFile);
            }, (file, uploaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", uploaded * 100 / file.FileSize);
            });

            ok = rf.WaitAll();
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
