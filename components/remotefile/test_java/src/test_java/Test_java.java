package test_java;

import SPA.ClientSide.*;

public class Test_java {

    public static void main(String[] args) {
        System.out.println("Remote SocketPro file streaming server:");
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        CConnectionContext cc = new CConnectionContext(scanner.nextLine(), 20901, "MyUserId", "MyPassword");
        try (CSocketPool<CStreamingFile> spRf = new CSocketPool<>(CStreamingFile.class)) {
            boolean ok = spRf.StartSocketPool(cc, 1);
            if (!ok) {
                System.out.println("Can not connect to remote server");
                System.out.println("Press key ENTER to shutdown the demo application ......");
                scanner.nextLine();
                return;
            }
            CStreamingFile rf = spRf.Seek();

            //test both downloading and uploading files in file stream (it is different from byte stream)
            String RemoteFile = "jvm.lib";
            String LocalFile = "spfile1.test";
            UFuture<ErrInfo> fd0 = rf.download(LocalFile, RemoteFile, (CStreamingFile csf, long downloaded) -> {
                //downloading progress
                System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
            });

            RemoteFile = "libboost_wave-vc100-mt-sgd-1_60.lib";
            LocalFile = "spfile2.test";
            UFuture<ErrInfo> fd1 = rf.download(LocalFile, RemoteFile);

            RemoteFile = "libboost_coroutine-vc100-mt-s-1_60.lib";
            LocalFile = "spfile3.test";
            UFuture<ErrInfo> fd2 = rf.download(LocalFile, RemoteFile);

            RemoteFile = "libboost_serialization-vc100-mt-s-1_60.lib";
            LocalFile = "spfile4.test";
            UFuture<ErrInfo> fd3 = rf.download(LocalFile, RemoteFile);

            RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60.lib";
            LocalFile = "spfile5.test";
            UFuture<ErrInfo> fd4 = rf.download(LocalFile, RemoteFile);

            //uploading test
            LocalFile = "spfile1.test";
            RemoteFile = "jvm_copy.lib";
            UFuture<ErrInfo> fu0 = rf.upload(LocalFile, RemoteFile, (CStreamingFile csf, long uploaded) -> {
                //downloading progress
                System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
            });

            LocalFile = "spfile2.test";
            RemoteFile = "libboost_wave-vc100-mt-sgd-1_60_copy.lib";
            UFuture<ErrInfo> fu1 = rf.upload(LocalFile, RemoteFile);

            LocalFile = "spfile3.test";
            RemoteFile = "libboost_coroutine-vc100-mt-s-1_60_copy.lib";
            UFuture<ErrInfo> fu2 = rf.upload(LocalFile, RemoteFile);

            LocalFile = "spfile4.test";
            RemoteFile = "libboost_serialization-vc100-mt-s-1_60_copy.lib";
            UFuture<ErrInfo> fu3 = rf.upload(LocalFile, RemoteFile);

            LocalFile = "spfile5.test";
            RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
            UFuture<ErrInfo> fu4 = rf.upload(LocalFile, RemoteFile);
            try {
                ErrInfo ei = fd0.get();
                ei = fd1.get();
                ei = fd2.get();
                ei = fd3.get();
                ei = fd4.get();
                ei = fu0.get();
                ei = fu1.get();
                ei = fu2.get();
                ei = fu3.get();
                ei = fu4.get();
                ei = null;
            } catch (SPA.CServerError ex) {

            } catch (CSocketError ex) {

            }
            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
        }
    }
}
