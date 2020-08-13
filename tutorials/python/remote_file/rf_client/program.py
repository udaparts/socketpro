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

        def cb_download(file, downloaded):
            print('Downloading rate: ' + '{:.2f}'.format(downloaded * 100 / file.FileSize) + '%')

        def cb_upload(file, uploaded):
            print('Uploading rate: ' + '{:.2f}'.format(uploaded * 100 / file.FileSize) + '%')

        try:
            fut0 = rf.download(LocalFile, RemoteFile, cb_download)
            RemoteFile += '.copy'
            fut1 = rf.upload(LocalFile, RemoteFile, cb_upload)
            print(fut0.result())
            print(fut1.result())
        except CServerError as ex:  # an exception from remote server
            print(ex)
        except CSocketError as ex:  # a communication error
            print(ex)
        print('Press key ENTER to shutdown the demo application ......')
        sys.stdin.readline()
