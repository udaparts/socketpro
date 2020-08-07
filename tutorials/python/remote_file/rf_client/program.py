from spa.clientside import *

with CSocketPool(CStreamingFile) as spFile:
    print('Remote SocketPro file streaming server:')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'PythonUser', 'TooMuchSecret')
    ok = spFile.StartSocketPool(cc, 1)
    if not ok:
        print('Can not connect to remote server ' + cc.Host)
    else:
        rf = spFile.Seek()
        print('Input a remote file to download ......')
        RemoteFile = sys.stdin.readline().strip()
        LocalFile = 'spfile.test'
        def cbDownload(file, res, errmsg):
            if res:
                print('Error code ' + str(res) + ', error message: ' + errmsg)
            else:
                print('Downloading ' + file.RemoteFile + ' completed')
        def cbDProgress(file, downloaded):
            print('Downloading rate: ' + str(downloaded * 100 / file.FileSize) + '%')
        ok = rf.Download(LocalFile, RemoteFile, cbDownload, cbDProgress)
        # ok = rf.WaitAll()

        RemoteFile += '.copy'
        def cbUpload(file, res, errmsg):
            if res:
                print('Error code ' + str(res) + ', error message: ' + errmsg)
            else:
                print('Uploading ' + file.RemoteFile + ' completed')
        def cbUProgress(file, uploaded):
            print('Uploading rate: ' + str(uploaded * 100 / file.FileSize) + '%')
        ok = rf.Upload(LocalFile, RemoteFile, cbUpload, cbUProgress)
        ok = rf.WaitAll()

        print('Press key ENTER to shutdown the demo application ......')
        sys.stdin.readline()