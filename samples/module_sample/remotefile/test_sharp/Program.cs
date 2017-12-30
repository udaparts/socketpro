using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "MyUserId", "MyPassword");
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
            Console.WriteLine("Input a remote file to download ......");
            string RemoteFile1 = Console.ReadLine(); //jvm.lib
            string LocalFile = "spfile1.test";
            //test both downloading and uploading files in file stream (it is different from byte stream)
            //downloading test
            ok = rf.Download(LocalFile, RemoteFile1, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", RemoteFile1);
            }, (file, downloaded) =>
            {
                //downloading progress
                Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile2.test";
            string RemoteFile2 = "libboost_wave-vc100-mt-sgd-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile2, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", RemoteFile2);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile3.test";
            string RemoteFile3 = "libboost_coroutine-vc100-mt-s-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile3, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", RemoteFile3);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile4.test";
            string RemoteFile4 = "libboost_serialization-vc100-mt-s-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile4, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", RemoteFile4);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile5.test";
            string RemoteFile5 = "libboost_math_tr1f-vc100-mt-sgd-1_60.lib";
            ok = rf.Download(LocalFile, RemoteFile5, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Downloading {0} completed", RemoteFile5);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Downloading rate: {0}%", downloaded * 100 / file.FileSize);
            });
            ok = rf.WaitAll();

            LocalFile = "spfile1.test";
            RemoteFile1 = "jvm_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile1, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", RemoteFile1);
            }, (file, downloaded) =>
            {
                //uploading progress
                Console.WriteLine("Uploading rate: {0}%", downloaded * 100 / file.FileSize);
            });
            
            LocalFile = "spfile2.test";
            RemoteFile2 = "libboost_wave-vc100-mt-sgd-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile2, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", RemoteFile2);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile3.test";
            RemoteFile3 = "libboost_coroutine-vc100-mt-s-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile3, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", RemoteFile3);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile4.test";
            RemoteFile4 = "libboost_serialization-vc100-mt-s-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile4, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", RemoteFile4);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", downloaded * 100 / file.FileSize);
            });

            LocalFile = "spfile5.test";
            RemoteFile5 = "libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
            ok = rf.Upload(LocalFile, RemoteFile5, (file, res, errMsg) =>
            {
                if (res != 0)
                    Console.WriteLine("Error code: {0}, error message: {1}", res, errMsg);
                else
                    Console.WriteLine("Uploading {0} completed", RemoteFile5);
            }, (file, downloaded) =>
            {
                //Console.WriteLine("Uploading rate: {0}%", downloaded * 100 / file.FileSize);
            });
            
            ok = rf.WaitAll();
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}
