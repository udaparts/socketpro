package remote_file.client;

import SPA.ClientSide.*;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "MyUserId", "MyPassword");
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        try (CSocketPool<CStreamingFile> spRf = new CSocketPool<>(CStreamingFile.class)) {
            do {
                if (!spRf.StartSocketPool(cc, 1, 1)) {
                    System.out.println("Can not connect to remote server");
                    break;
                }
                CStreamingFile rf = spRf.Seek();
                System.out.println("Input a remote file to download ......");
                String RemoteFile = scanner.nextLine();
                String LocalFile = "spfile.test";
                if (!rf.Download(LocalFile, RemoteFile, (csf, res, errMsg) -> {
                    if (res != 0) {
                        System.out.println("Error code: " + res + ", error message: " + errMsg);
                    } else {
                        System.out.println("Downloading " + csf.getRemoteFile() + " completed");
                    }
                }, (csf, downloaded) -> {
                    //downloading progress
                    System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
                })) {
                    System.out.println(rf.getAttachedClientSocket().getErrorMsg());
                    break;
                }
                //uploading test
                RemoteFile += ".copy";
                if (!rf.Upload(LocalFile, RemoteFile, (csf, res, errMsg) -> {
                    if (res != 0) {
                        System.out.println("Error code: " + res + ", error message: " + errMsg);
                    } else {
                        System.out.println("Uploading " + csf.getLocalFile() + " completed");
                    }
                }, (csf, uploaded) -> {
                    //uploading progress
                    System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
                })) {
                    System.out.println(rf.getAttachedClientSocket().getErrorMsg());
                    break;
                }
                if (!rf.WaitAll()) {
                    System.out.println(rf.getAttachedClientSocket().getErrorMsg());
                    break;
                }
            } while (false);
            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
        }
    }
}
