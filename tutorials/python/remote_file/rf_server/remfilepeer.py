
from spa.serverside import CClientPeer, CStreamHelper, CUQueue

class RemotingFilePeer(CClientPeer):
    def __init__(self):
        self._m_f = None

    def _clean(self):
        if not self._m_f is None:
            self._m_f.close()
            self._m_f = None

    def OnReleaseResource(self, closing, info):
        self._clean()

    def StartDownloadingFile(self):
        self._clean()
        filePath = self.UQueue.LoadString()
        size, errMsg, self._m_f = CStreamHelper.DownloadFile(self.Handle, filePath)
        return CUQueue().SaveLong(size).SaveString(errMsg)

    def MoveDataFromServerToClient(self):
        CStreamHelper.ReadDataFromServerToClient(self.Handle, self._m_f)

    def WaitDownloadCompleted(self):
        self._clean()

    def StartUploadingFile(self):
        self._clean()
        errMsg = ''
        filePath = self.UQueue.LoadString()
        try:
            self._m_f = open(filePath, 'wb')
        except Exception as err:
            errMsg = err.message
        return CUQueue().SaveString(errMsg)

    def MoveDataFromClientToServer(self):
        CStreamHelper.WriteDataFromClientToServer(self.UQueue, self._m_f)

    def WaitUploadingCompleted(self):
        self._clean()