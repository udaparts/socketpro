
from spa.clientside import CAsyncServiceHandler, CStreamHelper
from consts import RemFileConst

class CRemoteFile(CAsyncServiceHandler):
    def __init__(self):
        super(CRemoteFile, self).__init__(RemFileConst.sidRemotingFile)
        self._m_sh = CStreamHelper(self)

    @property
    def StreamHelper(self):
        return self._m_sh