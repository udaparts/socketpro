
from remfilepeer import RemotingFilePeer as rfp
from spa.serverside import CSocketProServer, CSocketProService
from consts import RemFileConst
from spa import CStreamSerializationHelper as ssh
import sys

with CSocketProServer() as server:
    map = {
        ssh.idStartDownloading : ('StartDownloadingFile', True),
        ssh.idReadDataFromServerToClient : ['MoveDataFromServerToClient', True],
        ssh.idDownloadCompleted : 'WaitDownloadCompleted',
        ssh.idStartUploading : ['StartUploadingFile', True],
        ssh.idWriteDataFromClientToServer : ('MoveDataFromClientToServer', True),
        ssh.idUploadCompleted : 'WaitUploadingCompleted'
    }
    server.RemoteFile = CSocketProService(rfp, RemFileConst.sidRemotingFile, map)
    ok = server.Run(20901)
    if not ok:
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
