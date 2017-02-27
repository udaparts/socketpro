using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.IO;

class Program
{
    static void Main(string[] args)
    {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "MyUserId", "MyPassword");
        using (CSocketPool<RemotingFile> spRf = new CSocketPool<RemotingFile>())
        {
            do
            {
                bool ok = spRf.StartSocketPool(cc, 1, 1);
                if (!ok)
                {
                    Console.WriteLine("Can not connect to remote server");
                    break;
                }

                RemotingFile rf = spRf.Seek();

                //downloading test
                CStreamHelper.DProgress progress = (sender, pos) =>
                {
                    Console.WriteLine("Downloading progress = " + (pos * 100) / sender.DownloadingStreamSize);
                };
                rf.StreamHelper.Progress += progress;
                Console.WriteLine("Input a remote file to download ......");
                string RemoteFile = Console.ReadLine();
                string LocalFile = "spfile.test";
                Stream s = new FileStream(LocalFile, FileMode.Append);
                string res = rf.StreamHelper.Download(s, RemoteFile);
                if (res.Length == 0 && rf.WaitAll())
                    Console.WriteLine("Successful to download file " + RemoteFile);
                else
                    Console.WriteLine("Failed to download file " + RemoteFile);
                s.Close();
                rf.StreamHelper.Progress -= progress; //remove the callback

                //uploading test
                RemoteFile = "spfile.testr";
                s = new FileStream(LocalFile, FileMode.Open);
                ulong FileSize = (ulong)s.Length;
                rf.StreamHelper.Progress += (sender, pos) =>
                {
                    Console.WriteLine("Uploading progress = " + (pos * 100) / FileSize);
                };
                res = rf.StreamHelper.Upload(s, RemoteFile);
                if (res == "" && rf.WaitAll())
                    Console.WriteLine("Successful to upload file " + LocalFile);
                else
                    Console.WriteLine("Failed to upload file " + LocalFile);
                s.Close();
            } while (false);
            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}

