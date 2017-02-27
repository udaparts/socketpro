package remote_file.client;

import SPA.ClientSide.*;

public class Program {

    public static void main(String[] args) {
        String res;
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "MyUserId", "MyPassword");
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        CSocketPool<RemotingFile> spRf = new CSocketPool<>(RemotingFile.class);

        boolean ok = spRf.StartSocketPool(cc, 1, 1);
        if (!ok) {
            System.out.println("Can not connect to remote server");
            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
            return;
        }
        RemotingFile rf = spRf.Seek();
        CStreamHelper.DProgress progress = new CStreamHelper.DProgress() {
            @Override
            public void invoke(CStreamHelper sender, long pos) {
                System.out.println("Downloading progress = " + (pos * 100) / sender.getDownloadingStreamSize());
            }
        };
        rf.getStreamHelper().Progress = progress;
        System.out.println("Input a remote file to download ......");
        String RemoteFile = scanner.nextLine();
        String LocalFile = "spfile.test";
        try {
            //downloading test
            try (java.io.FileOutputStream s = new java.io.FileOutputStream(LocalFile, true)) {
                res = rf.getStreamHelper().Download(s, RemoteFile);
                if (res.length() == 0 && rf.WaitAll()) {
                    s.flush();
                    System.out.println("Successful to download file " + RemoteFile);
                } else {
                    System.out.println("Failed to download file " + RemoteFile);
                }
            }
            //uploading test
            RemoteFile = "spfile.testr";
            try (java.io.FileInputStream inFile = new java.io.FileInputStream(LocalFile)) {
                final long FileSize = inFile.getChannel().size();
                rf.getStreamHelper().Progress = new CStreamHelper.DProgress() {
                    @Override
                    public void invoke(CStreamHelper sender, long pos) {
                        System.out.println("Uploading progress = " + (pos * 100) / FileSize);
                    }
                };
                res = rf.getStreamHelper().Upload(inFile, RemoteFile);
                if (res.length() == 0 && rf.WaitAll()) {
                    System.out.println("Successful to upload file " + LocalFile);
                } else {
                    System.out.println("Failed to upload file " + LocalFile);
                }
            }
        } catch (java.io.IOException err) {
            System.out.println(err.getMessage());
        }
        System.out.println("Press key ENTER to shutdown the demo application ......");
        scanner.nextLine();
    }
}
