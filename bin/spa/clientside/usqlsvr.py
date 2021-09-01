from spa.clientside.asyncdbhandler import CAsyncDBHandler
from spa import BaseServiceID, tagBaseRequestID

class CSqlServer(CAsyncDBHandler):
    # asynchronous MS SQL stream service id
    sidMsSql = BaseServiceID.sidReserved + 0x6FFFFFF2

    READ_ONLY = 0x20000000
    USE_ENCRYPTION = 0x40000000

    def __init__(self, sid=sidMsSql):
        super(CSqlServer, self).__init__(sid)
