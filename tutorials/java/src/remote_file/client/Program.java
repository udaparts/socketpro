package remote_file.client;

import SPA.ClientSide.*;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "MyUserId", "MyPassword");
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        CSocketPool<CStreamingFile> spRf = new CSocketPool<>(CStreamingFile.class);

        boolean ok = spRf.StartSocketPool(cc, 1, 1);
        if (!ok) {
            System.out.println("Can not connect to remote server");
            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
            return;
        }
        CStreamingFile rf = spRf.Seek();
        System.out.println("Input a remote file to download ......");
        String RemoteFile = scanner.nextLine();
        String LocalFile = "spfile.test";
        ok = rf.Download(LocalFile, RemoteFile, new CStreamingFile.DDownload() {
            @Override
            public void invoke(CStreamingFile csf, int res, String errMsg) {
                if (res != 0) {
                    System.out.println("Error code: " + res + ", error message: " + errMsg);
                } else {
                    System.out.println("Downloading " + csf.getRemoteFile() + " completed");
                }
            }
        }, new CStreamingFile.DTransferring() {
            @Override
            public void invoke(CStreamingFile csf, long downloaded) {
                //downloading progress
                System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
            }
        });
        ok = rf.WaitAll();
        //uploading test
        RemoteFile += ".copy";
        ok = rf.Upload(LocalFile, RemoteFile, new CStreamingFile.DUpload() {
            @Override
            public void invoke(CStreamingFile csf, int res, String errMsg) {
                if (res != 0) {
                    System.out.println("Error code: " + res + ", error message: " + errMsg);
                } else {
                    System.out.println("Uploading " + csf.getLocalFile() + " completed");
                }
            }
        }, new CStreamingFile.DTransferring() {
            @Override
            public void invoke(CStreamingFile csf, long uploaded) {
                //downloading progress
                System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
            }
        });
        ok = rf.WaitAll();
        System.out.println("Press key ENTER to shutdown the demo application ......");
        scanner.nextLine();
    }
}
