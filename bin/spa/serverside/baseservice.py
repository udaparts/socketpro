
import threading, traceback, sys
from spa.serverside.scoreloader import SCoreLoader as scl, CSvsContext
from ctypes import c_ushort, c_char, c_bool
from spa.serverside import *

class CBaseService(object):
    _m_csService = threading.Lock()
    _m_lstService = []

    def __init__(self):
        self._m_cs = threading.Lock()
        self._m_sc = CSvsContext()
        self._m_lstPeer = []
        self._m_lstDeadPeer = []
        self._m_dicMethod = {}
        self._m_svsId = 0

    @abstractmethod
    def GetPeerSocket(self):
        raise NotImplementedError("Please implement this method")

    def _ReleasePeer(self, hSocket, bClosing, info):
        found = self.Seek(hSocket)
        if not found is None:
            with self._m_cs:
                found.OnReleaseResource(bClosing, info)
                found._m_qBuffer.SetSize(0)
                found._m_sh = 0
                self._m_lstPeer.remove(found)
                self._m_lstDeadPeer.append(found)

    def _CreatePeer(self, hSocket, newServiceId):
        sp = None
        with self._m_cs:
            if len(self._m_lstDeadPeer) > 0:
                sp = self._m_lstDeadPeer.pop()
        if sp is None:
            sp = self.GetPeerSocket()
        sp._m_Service = self
        sp._m_sh = hSocket
        sp._m_random = self.ReturnRandom
        with self._m_cs:
            self._m_lstPeer.append(sp)
        return sp

    def RemoveMe(self):
        if self._m_svsId > 0:
            scl.RemoveASvsContext(self._m_svsId)
            with CBaseService._m_csService:
                CBaseService._m_lstService.remove(self)
            self._m_svsId = 0

    def _OnBaseCame(self, hSocket, reqId):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        if self._m_svsId == BaseServiceID.sidHTTP and reqId == tagBaseRequestID.idPing:
            sp._m_vArg = []
            sp._m_WebRequestName = None
        sp.OnBaseRequestCame(reqId)

    def _OnChatCame(self, hSocket, reqId):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        sp.OnChatRequestCame(reqId)

    def _OnChatComing(self, hSocket, chatReqId, len):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        buffer = (c_char * len)()
        res = scl.RetrieveBuffer(hSocket, len, buffer, True)
        assert res == len
        sp._m_qBuffer = CUQueue(buffer)
        sp._OnChatComing(chatReqId)

    def _OnClose(self, hSocket, nError):
        self._ReleasePeer(hSocket, True, nError)

    def _calling(self, sp, reqId, len, slow=False):
        ret = 0
        try:
            if sp.SvsID == BaseServiceID.sidHTTP:
                return sp._OnHttpRequestArrive(reqId, sp._m_qBuffer.GetSize())
            else:
                funcName = self._m_dicMethod.get(reqId, None)
                if not funcName:
                    if slow:
                        ret = sp.OnSlowRequestArrive(reqId, len)
                    else:
                        sp.OnFastRequestArrive(reqId, len)
                else:
                    res = sp.__getattribute__(funcName)()
                    if isinstance(res, CUQueue):
                        sp.SendResult(res, reqId)
                    elif isinstance(res, CScopeUQueue):
                        sp.SendResult(res, reqId)
                        res.Dispose()
                    else:
                        sp.SendResult(None, reqId)
        except Exception as ex:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            stack = repr(traceback.format_exception(exc_type, exc_value, exc_traceback))
            scl.SendExceptionResult(sp.Handle, ex.message, stack, reqId, 0)
        if slow:
            return ret

    def _OnFast(self, hSocket, reqId, len):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        self._calling(sp, reqId, len, False)

    def _OnSlow(self, reqId, len, hSocket):
        sp = self.Seek(hSocket)
        if sp is None:
            return -1
        return self._calling(sp, reqId, len, True)

    def _OnReqArrive(self, hSocket, reqId, len):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        buffer = (c_char * len)()
        res = scl.RetrieveBuffer(hSocket, len, buffer, False)
        assert res == len
        sp._m_qBuffer = CUQueue(buffer)
        if self._m_svsId == BaseServiceID.sidHTTP:
            sp._m_WebRequestName = None
            sp._m_vArg = []
            if reqId == tagHttpRequestID.idUserRequest:
                sp._m_WebRequestName = sp._m_qBuffer.LoadAString()
                count = sp._m_qBuffer.LoadUInt()
                while count > 0:
                    sp._m_vArg.append(sp._m_qBuffer.LoadObject())
                    count -= 1
        sp.OnRequestArrive(reqId, len)

    def _OnSwitch(self, hSocket, oldServiceId, newServiceId):
        bsOld = None
        if oldServiceId != BaseServiceID.sidStartup:
            bsOld = CBaseService.SeekService(int(oldServiceId))
            if not bsOld is None:
                bsOld._ReleasePeer(hSocket, False, newServiceId)
        bsNew = CBaseService.SeekService(int(newServiceId))
        if not bsNew is None:
            sp = bsNew._CreatePeer(hSocket, newServiceId)
            if newServiceId == BaseServiceID.sidHTTP:
                sp._m_bHttpOk = False
            sp._m_qBuffer = CUQueue(bytearray(0))
            sp.OnSwitchFrom(oldServiceId)

    def _OnResultsSent(self, hSocket):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        sp.OnResultsSent()

    def _OnSRProcessed(self, hSocket, reqId):
        sp = self.Seek(hSocket)
        if sp is None:
            return
        sp.OnSlowRequestProcessed(reqId)

    def _OnHttpAuthentication(self, hSocket, userId, password):
        hp = self.Seek(hSocket)
        if hp is None:
            return
        hp._m_bHttpOk = hp.DoAuthentication(userId, password)
        return hp._m_bHttpOk

    """
    <summary>
    Register a service
    </summary>
    <param name="svsId">A service id</param>
    <param name="ta">Thread apartment for windows default to tagThreadApartment.taNone. It is ignored on non-windows platforms</param>
    <returns>True if successful. Otherwise false if failed</returns>
    """
    def AddMe(self, svsId, ta=tagThreadApartment.taNone):
        self._m_sc = CSvsContext()
        self._m_sc.m_ta = ta
        self._m_sc.m_OnSwitchTo = scl.POnSwitchTo(self._OnSwitch)
        self._m_sc.m_OnRequestArrive = scl.POnRequestArrive(self._OnReqArrive)
        self._m_sc.m_OnFastRequestArrive = scl.POnFastRequestArrive(self._OnFast)
        self._m_sc.m_OnBaseRequestCame = scl.POnBaseRequestCame(self._OnBaseCame)
        self._m_sc.m_OnRequestProcessed = scl.POnRequestProcessed(self._OnSRProcessed)
        self._m_sc.m_OnClose = scl.POnClose(self._OnClose)
        self._m_sc.m_SlowProcess = scl.PSLOW_PROCESS(self._OnSlow)
        self._m_sc.m_OnChatRequestComing = scl.POnChatRequestComing(self._OnChatComing)
        self._m_sc.m_OnChatRequestCame = scl.POnChatRequestCame(self._OnChatCame)
        self._m_sc.m_OnResultsSent = scl.POnResultsSent(self._OnResultsSent)
        self._m_sc.m_OnHttpAuthentication = scl.POnHttpAuthentication(self._OnHttpAuthentication)

        if svsId != 0 and scl.AddSvsContext(svsId, self._m_sc):
            self._m_svsId = svsId
            with CBaseService._m_csService:
                CBaseService._m_lstService.append(self)
            return True
        return False

    def __del__(self):
        if not self._m_sc.m_OnClose is None:
            with CBaseService._m_csService:
                CBaseService._m_lstService.remove(self)
            self._m_sc = CSvsContext()

    @property
    def SvsID(self):
        return self._m_svsId

    @property
    def CountOfSlowRequests(self):
        return scl.GetCountOfSlowRequests(self._m_svsId)

    @property
    def AllSlowRequestIds(self):
        sr = (c_ushort * 4097)()
        res = scl.GetAllSlowRequestIds(self._m_svsId, sr, 4097)
        return list(sr)[0:res]

    @property
    def ReturnRandom(self):
        return scl.GetReturnRandom(self._m_svsId)

    @ReturnRandom.setter
    def ReturnRandom(self, value):
        scl.SetReturnRandom(self._m_svsId, value)

    """
    <summary>
    Register a slow request
    </summary>
    <param name="reqId">A request id</param>
    <returns>True if successful. Otherwise false if failed</returns>
    """
    def AddSlowRequest(self, reqId):
        return scl.AddSlowRequest(self._m_svsId, reqId)

    def RemoveSlowRequest(self, reqId):
        scl.RemoveSlowRequest(self._m_svsId, reqId)

    def RemoveAllSlowRequests(self):
        scl.RemoveAllSlowRequests(self._m_svsId)

    """
    <summary>
    Make a request processed at router for a routee
    </summary>
    <param name="reqId">A request id</param>
    <returns>True if successful. Otherwise, false if failed</returns>
    """
    def AddAlphaRequest(self, reqId):
        return scl.AddAlphaRequest(self._m_svsId, reqId)

    @property
    def AlphaRequestIds(self):
        sr = (c_ushort * 4097)()
        res = scl.GetAlphaRequestIds(self._m_svsId, sr, 4097)
        return list(sr)[0:res]

    def Seek(self, hSocket):
        with self._m_cs:
            for sp in self._m_lstPeer:
                if sp._m_sh == hSocket:
                    return sp
        return None

    @staticmethod
    def SeekService(serviceId):
        with CBaseService._m_csService:
            for bs in CBaseService._m_lstService:
                if bs.SvsID == serviceId:
                    return bs
        return None

class CSocketProService(CBaseService):
    def __init__(self, clsSocketPeer, svsId, dicRequests, ta=tagThreadApartment.taNone):
        super(CSocketProService, self).__init__()
        self._m_SocketPeer = clsSocketPeer
        if not self.AddMe(svsId, ta):
            raise Exception('Error in registering service (' + str(svsId) + ')')
        if svsId == BaseServiceID.sidHTTP:
            slows = [tagHttpRequestID.idPost,
                     tagHttpRequestID.idGet,
                     tagHttpRequestID.idConnect,
                     tagHttpRequestID.idHead,
                     tagHttpRequestID.idMultiPart,
                     tagHttpRequestID.idOptions,
                     tagHttpRequestID.idPut,
                     tagHttpRequestID.idTrace,
                     tagHttpRequestID.idDelete,
                     tagHttpRequestID.idUserRequest]
            for rid in slows:
                self.AddSlowRequest(rid)
        else:
            for rid, req in dicRequests.items():
                if isinstance(req, str):
                    self._m_dicMethod[rid] = req
                elif isinstance(req, list) or isinstance(req, tuple):
                    self._m_dicMethod[rid] = req[0]
                    if len(req) > 1 and req[1]:
                        self.AddSlowRequest(rid)

    def GetPeerSocket(self):
        return self._m_SocketPeer()



