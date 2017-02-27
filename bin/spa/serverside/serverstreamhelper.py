
from spa import CStreamSerializationHelper as ssh, CUQueue
from spa.serverside.scoreloader import SCoreLoader as scl
from spa.serverside.socketpeer import CSocketPeer
import os
from ctypes import c_ubyte

class CStreamHelper(ssh):
    @staticmethod
    def WriteDataFromClientToServer(q, receiver):
        ssh.Write(receiver, q)

    @staticmethod
    def ReadDataFromServerToClient(hPeer, source):
        sent = 0
        bs = ssh.Read(source)
        read = len(bs)
        while read > 0:
            buffer = (c_ubyte * read).from_buffer(bytearray(bs))
            res = scl.SendReturnData(hPeer, ssh.idReadDataFromServerToClient, read, buffer)
            if res == CSocketPeer.REQUEST_CANCELED or res == CSocketPeer.REQUEST_CANCELED:
                break
            sent += res
            bs = ssh.Read(source)
            read = len(bs)
        return sent

    @staticmethod
    def DownloadFile(hPeer, filePath):
        f = None
        fileSize = -1
        errMsg = ''
        try:
            f = open(filePath, 'rb')
            f.seek(0, os.SEEK_END)
            fileSize = f.tell()
            f.seek(0, os.SEEK_SET)
            mem = (c_ubyte * 1)()
            ok = scl.MakeRequest(hPeer, ssh.idReadDataFromServerToClient, mem, 0)
        except Exception as err:
            errMsg = err.message
        return fileSize, errMsg, f
