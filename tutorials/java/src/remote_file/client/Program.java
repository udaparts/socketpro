package remote_file.client;

import SPA.ClientSide.*;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "MyUserId", "MyPassword");
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        try (CSocketPool<CStreamingFile> spRf = new CSocketPool<>(CStreamingFile.class)) {
            if (!spRf.StartSocketPool(cc, 1)) {
                System.out.println("Can not connect to remote server");
            } else {
                CStreamingFile rf = spRf.Seek();
                System.out.println("Input a remote file to download ......");
                String RemoteFile = scanner.nextLine();
                String LocalFile = "spfile.test";
                UFuture<ErrInfo> fd = rf.download(LocalFile, RemoteFile, (csf, downloaded) -> {
                    System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
                });
                //uploading test
                RemoteFile += ".copy";
                UFuture<ErrInfo> fu = rf.upload(LocalFile, RemoteFile, (csf, uploaded) -> {
                    System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
                });
                try {
                    System.out.println(fd.get());
                    System.out.println(fu.get());
                } catch (SPA.CServerError | CSocketError ex) {
                    System.out.println(ex);
                }
            }
            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
        }
    }
}
