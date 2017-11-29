
import threading
from spa.clientside import CCachedBaseHandler

from sharedstruct import *


class CWebAsyncHandler(CCachedBaseHandler):
    """
    First of all, all four methods are thread-safe.

    Second, all methods are ended with two inputs, a callback for returning result and a callback
    for tracking potential socket close event after sending

    Third, a method always returns a call index. If the returned call index is zero, socket is closed before calling.
    Otherwise, the method is put on wire successfully

    Fourth, all methods are supporting random request stream returning

    At last, you can recall a method if socket is closed for fault tolerance
    """

    #  static members
    _csSS = threading.Lock()
    _ssIndex = 0  #protected by _csSS

    def __init__(self):
        super(CWebAsyncHandler, self).__init__(sidStreamSystem)
        self._mapMMA = {}
        self._mapSession = {}
        self._mapUpload = {}
        self._mapRentalDateTimes = {}

    def QueryPaymentMaxMinAvgs(self, filter, dMma, dClosed=None):
        def arh(ar):
            index = ar.LoadLong()
            res = ar.LoadInt()
            errMsg = ar.LoadString()
            mma = ar.LoadByClass(CMaxMinAvg)
            p = None
            with self._csCache:
                p = self._mapMMA.pop(index)
            if p and p.first:
                p.first(index, mma, res, errMsg)
        callIndex = 0
        with CWebAsyncHandler._csSS:
            CWebAsyncHandler._ssIndex += 1
            callIndex = CWebAsyncHandler._ssIndex
        with self._csCache:
            self._mapMMA[callIndex] = Pair(dMma, dClosed)
        def closed():
            p = None
            with self._csCache:
                p = self._mapMMA.pop(callIndex)
            if p and p.second:
                p.second(callIndex)
        with CScopeUQueue() as q:
            q.SaveLong(callIndex).SaveString(filter)
            if not self.SendRequest(idQueryMaxMinAvgs, q, arh, closed):
                with self._csCache:
                    p = self._mapMMA.pop(callIndex)
                return 0
        return callIndex

    def GetMasterSlaveConnectedSessions(self, dMscs, dClosed=None):
        def arh(ar):
            index = ar.LoadLong()
            master_connections = ar.LoadInt()
            slave_conenctions = ar.LoadInt()
            p = None
            with self._csCache:
                p = self._mapSession.pop(index)
            if p and p.first:
                p.first(index, master_connections, slave_conenctions)
        callIndex = 0
        with CWebAsyncHandler._csSS:
            CWebAsyncHandler._ssIndex += 1
            callIndex = CWebAsyncHandler._ssIndex
        with self._csCache:
            self._mapSession[callIndex] = Pair(dMscs, dClosed)
        def closed():
            p = None
            with self._csCache:
                p = self._mapSession.pop(callIndex)
            if p and p.second:
                p.second(callIndex)
        with CScopeUQueue() as q:
            q.SaveLong(callIndex)
            if not self.SendRequest(idGetMasterSlaveConnectedSessions, q, arh, closed):
                with self._csCache:
                    p = self._mapSession.pop(callIndex)
                return 0
        return callIndex

    def UploadEmployees(self, vData, dUe, dClosed=None):
        assert isinstance(vData, list)
        def arh(ar):
            index = ar.LoadLong()
            errCode = ar.LoadInt()
            errMsg = ar.LoadString()
            vId = ar.LoadByClass(CLongArray)
            p = None
            with self._csCache:
                p = self._mapUpload.pop(index)
            if p and p.first:
                p.first(index, errCode, errMsg, vId)
        callIndex = 0
        with CWebAsyncHandler._csSS:
            CWebAsyncHandler._ssIndex += 1
            callIndex = CWebAsyncHandler._ssIndex
        with self._csCache:
            self._mapUpload[callIndex] = Pair(dUe, dClosed)
        def closed():
            p = None
            with self._csCache:
                p = self._mapUpload.pop(callIndex)
            if p and p.second:
                p.second(callIndex)
        with CScopeUQueue() as q:
            q.SaveLong(callIndex)
            #pack vData into memory
            q.SaveInt(len(vData))
            for d in vData:
                q.SaveObject(d)
            if not self.SendRequest(idUploadEmployees, q, arh, closed):
                with self._csCache:
                    p = self._mapUpload.pop(callIndex)
                return 0
        return callIndex

    def GetRentalDateTimes(self, rentalId, dRdt, dClosed=None):
        def arh(ar):
            index = ar.LoadLong()
            dates = ar.LoadByClass(CRentalDateTimes)
            errCode = ar.LoadInt()
            errMsg = ar.LoadString()
            p = None
            with self._csCache:
                p = self._mapRentalDateTimes.pop(index)
            if p and p.first:
                p.first(index, dates, errCode, errMsg)
        callIndex = 0
        with CWebAsyncHandler._csSS:
            CWebAsyncHandler._ssIndex += 1
            callIndex = CWebAsyncHandler._ssIndex
        with self._csCache:
            self._mapRentalDateTimes[callIndex] = Pair(dRdt, dClosed)
        def closed():
            p = None
            with self._csCache:
                p = self._mapRentalDateTimes.pop(callIndex)
            if p and p.second:
                p.second(callIndex)
        with CScopeUQueue() as q:
            q.SaveLong(callIndex).SaveLong(rentalId)
            if not self.SendRequest(idGetRentalDateTimes, q, arh, closed):
                with self._csCache:
                    p = self._mapRentalDateTimes.pop(callIndex)
                return 0
        return callIndex
