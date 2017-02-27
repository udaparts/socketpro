
from spa.clientside import *
from asyncfile import CRemoteFile
import os

with CSocketPool(CRemoteFile) as spFile:
    cc = CConnectionContext('localhost', 20901, 'PythonUser', 'TooMuchSecret')
    ok = spFile.StartSocketPool(cc, 1, 1)
    if not ok:
        print('Can not connect to remote server')
    else:
        rf = spFile.Seek()
        print('Input a remote file to download ......')
        RemoteFile = sys.stdin.readline()
        LocalFile = "spfile.test"
        with open(LocalFile, 'wb') as s:
            def progress(sender, pos):
                print ('Downloading progress = ' + str((pos * 100) / sender.DownloadingStreamSize))
            rf.StreamHelper.Progress = progress
            res = rf.StreamHelper.Download(s, RemoteFile)
            if res == '' and rf.WaitAll():
                print('Successful to download file ' + RemoteFile)
            else:
                print('Failed to download file ' + RemoteFile + ', error message = ' + res)
        RemoteFile = "spfile.testr"
        with open(LocalFile, 'rb') as s:
            s.seek(0, os.SEEK_END)
            FileSize = s.tell()
            s.seek(0, os.SEEK_SET)
            def progress(sender, pos):
                print('Uploading progress = ' + str((pos * 100) / FileSize))
            rf.StreamHelper.Progress = progress
            res = rf.StreamHelper.Upload(s, RemoteFile)
            if res == '' and rf.WaitAll():
                print('Successful to upload file ' + LocalFile)
            else:
                print('Failed to upload file ' + LocalFile + ', error message = ' + res)
        print('Press key ENTER to shutdown the demo application ......')
        sys.stdin.readline()