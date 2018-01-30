
from spa.clientside import CCachedBaseHandler
from sharedstruct import *

class CWebAsyncHandler(CCachedBaseHandler):
    """
    First of all, all four methods are thread-safe.

    Second, all methods are ended with two inputs, a callback for returning result and a callback
    for tracking potential socket close event after sending

    At last, you can recall a method if socket is closed for fault tolerance
    """

    def __init__(self):
        super(CWebAsyncHandler, self).__init__(sidStreamSystem)

    def QueryPaymentMaxMinAvgs(self, filter, dMma, discarded=None):
        def arh(ar):
            res = ar.LoadInt()
            errMsg = ar.LoadString()
            mma = ar.LoadByClass(CMaxMinAvg)
            if dMma:
                dMma(mma, res, errMsg)
        with CScopeUQueue() as q:
            q.SaveString(filter)
            return self.SendRequest(idQueryMaxMinAvgs, q, arh, discarded)

    def GetMasterSlaveConnectedSessions(self, dMscs, discarded=None):
        def arh(ar):
            master_connections = ar.LoadInt()
            slave_conenctions = ar.LoadInt()
            if dMscs:
                dMscs(master_connections, slave_conenctions)
        return self.SendRequest(idGetMasterSlaveConnectedSessions, None, arh, discarded)

    def UploadEmployees(self, vData, dUe, discarded=None):
        assert isinstance(vData, list)
        def arh(ar):
            errCode = ar.LoadInt()
            errMsg = ar.LoadString()
            vId = ar.LoadByClass(CLongArray)
            if dUe:
                dUe(errCode, errMsg, vId)
        with CScopeUQueue() as q:
            #pack vData into memory
            q.SaveUInt(len(vData))
            for d in vData:
                q.SaveObject(d)
            return self.SendRequest(idUploadEmployees, q, arh, discarded)

    def GetRentalDateTimes(self, rentalId, dRdt, discarded=None):
        def arh(ar):
            dates = ar.LoadByClass(CRentalDateTimes)
            errCode = ar.LoadInt()
            errMsg = ar.LoadString()
            if dRdt:
                dRdt(dates, errCode, errMsg)
        with CScopeUQueue() as q:
            q.SaveLong(rentalId)
            return self.SendRequest(idGetRentalDateTimes, q, arh, discarded)
