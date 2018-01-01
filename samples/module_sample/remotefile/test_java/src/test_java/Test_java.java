package test_java;

import SPA.ClientSide.*;

public class Test_java {

    public static void main(String[] args) {
        System.out.println("Remote SocketPro file streaming server:");
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        CConnectionContext cc = new CConnectionContext(scanner.nextLine(), 20901, "MyUserId", "MyPassword");
        CSocketPool<CStreamingFile> spRf = new CSocketPool<>(CStreamingFile.class);
        boolean ok = spRf.StartSocketPool(cc, 1, 1);
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

        RemoteFile = "libboost_wave-vc100-mt-sgd-1_60.lib";
        LocalFile = "spfile2.test";
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
                //System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
            }
        });

        RemoteFile = "libboost_coroutine-vc100-mt-s-1_60.lib";
        LocalFile = "spfile3.test";
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
                //System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
            }
        });

        RemoteFile = "libboost_serialization-vc100-mt-s-1_60.lib";
        LocalFile = "spfile4.test";
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
                //System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
            }
        });

        RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60.lib";
        LocalFile = "spfile5.test";
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
                //System.out.println("Downloading rate: " + downloaded * 100 / csf.getFileSize());
            }
        });
        ok = rf.WaitAll();

        //uploading test
        LocalFile = "spfile1.test";
        RemoteFile = "jvm_copy.lib";
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

        LocalFile = "spfile2.test";
        RemoteFile = "libboost_wave-vc100-mt-sgd-1_60_copy.lib";
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
                //System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
            }
        });

        LocalFile = "spfile3.test";
        RemoteFile = "libboost_coroutine-vc100-mt-s-1_60_copy.lib";
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
                //System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
            }
        });

        LocalFile = "spfile4.test";
        RemoteFile = "libboost_serialization-vc100-mt-s-1_60_copy.lib";
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
                //System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
            }
        });

        LocalFile = "spfile5.test";
        RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
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
                //System.out.println("Uploading rate: " + uploaded * 100 / csf.getFileSize());
            }
        });
        ok = rf.WaitAll();
        System.out.println("Press key ENTER to shutdown the demo application ......");
        scanner.nextLine();
    }
}
