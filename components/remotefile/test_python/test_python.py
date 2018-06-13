
from spa.clientside import *

with CSocketPool(CStreamingFile) as spFile:
    print('Remote SocketPro file streaming server:')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'PythonUser', 'TooMuchSecret')
    ok = spFile.StartSocketPool(cc, 1, 1)
    if not ok:
        print('Can not connect to remote server')
    else:
        rf = spFile.Seek()

    def cbDownload(file, res, errmsg):
        if res:
            print('Error code ' + str(res) + ', error message: ' + errmsg)
        else:
            print('Downloading ' + file.RemoteFile + ' completed')

    def cbDProgress(file, downloaded):
        print('Downloading rate: ' + str(downloaded * 100 / file.FileSize) + '%')

    RemoteFile = 'jvm.lib'
    LocalFile = 'spfile1.test'
    ok = rf.Download(LocalFile, RemoteFile, cbDownload, cbDProgress)

    LocalFile = "spfile2.test"
    RemoteFile = "libboost_wave-vc100-mt-sgd-1_60.lib"
    ok = rf.Download(LocalFile, RemoteFile, cbDownload)

    LocalFile = "spfile3.test"
    RemoteFile = "libboost_coroutine-vc100-mt-s-1_60.lib"
    ok = rf.Download(LocalFile, RemoteFile, cbDownload)

    LocalFile = "spfile4.test"
    RemoteFile = "libboost_serialization-vc100-mt-s-1_60.lib"
    ok = rf.Download(LocalFile, RemoteFile, cbDownload)

    LocalFile = "spfile5.test"
    RemoteFile = "libboost_math_tr1f-vc100-mt-sgd-1_60.lib"
    ok = rf.Download(LocalFile, RemoteFile, cbDownload)

    ok = rf.WaitAll()

    def cbUpload(file, res, errmsg):
        if res:
            print('Error code ' + str(res) + ', error message: ' + errmsg)
        else:
            print('Uploading ' + file.RemoteFile + ' completed')
    def cbUProgress(file, uploaded):
        print('Uploading rate: ' + str(uploaded * 100 / file.FileSize) + '%')

    LocalFile = 'spfile1.test'
    RemoteFile = 'jvm_copy.lib'
    ok = rf.Upload(LocalFile, RemoteFile, cbUpload, cbUProgress)

    LocalFile = 'spfile2.test'
    RemoteFile = 'libboost_wave-vc100-mt-sgd-1_60_copy.lib'
    ok = rf.Upload(LocalFile, RemoteFile, cbUpload)

    LocalFile = 'spfile3.test'
    RemoteFile = 'libboost_coroutine-vc100-mt-s-1_60_copy.lib'
    ok = rf.Upload(LocalFile, RemoteFile, cbUpload)

    LocalFile = 'spfile4.test'
    RemoteFile = 'libboost_serialization-vc100-mt-s-1_60_copy.lib'
    ok = rf.Upload(LocalFile, RemoteFile, cbUpload)

    LocalFile = 'spfile5.test'
    RemoteFile = 'libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib'
    ok = rf.Upload(LocalFile, RemoteFile, cbUpload)

    ok = rf.WaitAll()

    print('Press key ENTER to shutdown the demo application ......')
    sys.stdin.readline()