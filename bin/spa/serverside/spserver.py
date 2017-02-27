
from spa.serverside.scoreloader import SCoreLoader as scl, CConfigImpl, CQueueManagerImpl
from ctypes import c_char, c_uint, c_wchar
from spa import tagEncryptionMethod, classproperty
from spa.serverside import tagRoutingAlgorithm

class CSocketProServer(object):
    _m_sps_ = None
    def __init__(self, param=0):
        if not CSocketProServer._m_sps_ is None:
            raise Exception("SocketPro doesn't allow multiple instances at the same time")
        CSocketProServer._m_sps_ = self
        ok = scl.InitSocketProServer(param)
        self._m_onAccept_ = scl.POnAccept(self.OnAccept)
        scl.SetOnAccept(self._m_onAccept_)
        self._m_onClose_ = scl.POnClose(self.OnClose)
        scl.SetOnClose(self._m_onClose_)
        self._m_onIdle_ = scl.POnIdle(self.OnIdle)
        scl.SetOnIdle(self._m_onIdle_)
        self._m_onIsPermitted_ = scl.POnIsPermitted(self._IsPermitted_)
        scl.SetOnIsPermitted(self._m_onIsPermitted_)
        self._m_shc_ = scl.POnSSLHandShakeCompleted(self.OnSSLShakeCompleted)
        scl.SetOnSSLHandShakeCompleted(self._m_shc_)

    def __del__(self):
        self._Cleanup_()

    def _Cleanup_(self):
        if CSocketProServer._m_sps_ is None:
            return
        CSocketProServer._m_sps_ = None
        self.StopSocketProServer()
        scl.UninitSocketProServer()

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self._Cleanup_()

    def OnSettingServer(self):
        return True

    @classproperty
    def Running(cls):
        return scl.IsRunning()

    @classproperty
    def Server(cls):
         return CSocketProServer._m_sps_

    @classproperty
    def IsMainThread(cls):
        return scl.IsMainThread()

    @classproperty
    def SSLEnabled(cls):
        return scl.IsServerSSLEnabled()

    @classproperty
    def LastSocketError(cls):
        return scl.GetServerErrorCode()

    @classproperty
    def ErrorMessage(cls):
        msg = (c_char * 4097)()
        res = scl.GetServerErrorMessage(msg, 4097)
        return msg.value.decode('latin-1')

    @classproperty
    def CountOfClients(cls):
        return scl.GetCountOfClients()

    @classproperty
    def RequestCount(cls):
        return scl.GetRequestCount()

    @classproperty
    def Services(cls):
        count = scl.GetCountOfServices()
        svs = (c_uint * count)()
        res = scl.GetServices(svs, count)
        return list(svs)

    @classproperty
    def Version(cls):
        return scl.GetUServerSocketVersion().decode('latin-1')

    @staticmethod
    def GetClient(index):
        return scl.GetClient(index)

    @classproperty
    def LocalName(cls):
        str = (c_char * 256)()
        res = scl.GetLocalName(str, 256)
        return str.value.decode('latin-1')

    def _IsPermitted_(self, hSocket, serviceId):
        return self.OnIsPermitted(hSocket, CSocketProServer.CredentialManager.GetUserID(hSocket), CSocketProServer.CredentialManager.GetPassword(hSocket), serviceId)

    def OnIsPermitted(self, hSocket, userId, password, serviceId):
        return True

    def OnAccept(self, hSocket, errCode):
        pass

    def OnClose(self, hSocket, errCode):
        pass

    def OnIdle(self, milliseconds):
        pass

    def OnSSLShakeCompleted(self, hSocket, errCode):
        pass

    def PostQuit(self):
        scl.PostQuitPump()

    @staticmethod
    def SetLastCallInfo(str):
        scl.SetLastCallInfo(str.encode('latin-1'))

    def Run(self, port, maxBacklog=32, v6=False):
        if not self.OnSettingServer():
            return False
        return scl.StartSocketProServer(port, maxBacklog, v6)

    def StopSocketProServer(self):
        scl.PostQuitPump()
        scl.StopSocketProServer()

    def UseSSL(self, certFile, keyFile, pwdForPrivateKeyFile, dhFile=''):
        scl.SetCertFile(certFile.encode('latin-1'))
        scl.SetPrivateKeyFile(keyFile.encode('latin-1'))
        scl.SetPKFPassword(pwdForPrivateKeyFile.encode('latin-1'))
        scl.SetDHParmsFile(dhFile.encode('latin-1'))
        scl.SetDefaultEncryptionMethod(tagEncryptionMethod.TLSv1)

    class SwitchError(object):
        seERROR_WRONG_SWITCH = 0x7FFFF100
        seERROR_AUTHENTICATION_FAILED = 0x7FFFF101
        seERROR_SERVICE_NOT_FOUND_AT_SERVER_SIDE = 0x7FFFF102
        seERROR_NOT_SWITCHED_YET = 0x7FFFF103
        seERROR_BAD_REQUEST = 0x7FFFF104

    Config = CConfigImpl()

    class DllManager(object):
        @staticmethod
        def AddALibrary(libFile, param=0):
            return scl.AddADll(libFile.encode('latin-1'), param)

        @staticmethod
        def RemoveALibrary(hLib):
            return scl.RemoveADllByHandle(hLib)

    class PushManager(object):
        @classproperty
        def CountOfChatGroups(cls):
            return scl.GetCountOfChatGroups()

        @classproperty
        def AllCreatedChatGroups(cls):
            count = scl.GetCountOfChatGroups()
            grp = (c_uint * count)()
            scl.GetAllCreatedChatGroups(grp, count)
            return list(grp)

        @staticmethod
        def AddAChatGroup(groupId, description=u''):
            scl.AddAChatGroup(groupId, description)

        @staticmethod
        def RemoveChatGroup(groupId):
            scl.RemoveChatGroup(groupId)

        @staticmethod
        def GetAChatGroup(groupId):
            des = (c_wchar * 2048)()
            scl.GetAChatGroup(groupId, des, 2048)
            return des.value

        @staticmethod
        def Publish(message, groups, hint=''):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            q = CUQueue().SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return scl.SpeakPush(bytes, q.GetSize(), arr, size)

        @staticmethod
        def SendUserMessage(message, userId, hint=''):
            if userId is None:
                userId = u''
            q = CUQueue().SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return scl.SendUserMessagePush(userId, bytes, q.GetSize())

        @staticmethod
        def PublishEx(message, groups):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            if message is None:
                message = ()
            arrMessage = (c_ubyte * len(message))(*message)
            return scl.SpeakExPush(arrMessage, len(message), arr, size)

        @staticmethod
        def SendUserMessageEx(userId, message):
            if userId is None:
                userId = u''
            if message is None:
                message = ()
            arrMessage = (c_ubyte * len(message))(*message)
            return scl.SendUserMessageExPush(userId, arrMessage, len(message))

    QueueManager = CQueueManagerImpl()

    class Router(object):

        #<summary>
        #Set a route with two given service ids
        #</summary>
        #<param name="serviceId0">The first service Id</param>
        #<param name="serviceId1">The second service id</param>
        #<param name="ra0">Routing algorithm for serviceId0. It is default to raDefault</param>
        #<param name="ra1">Routing algorithm for serviceId1. It is default to raDefault</param>
        #<returns>True if successful; and false if failed</returns>
        #<remarks>If any one of the two given service ids does not exist, the route is broken</remarks>
        @staticmethod
        def SetRouting(serviceId0, serviceId1, ra0=tagRoutingAlgorithm.raDefault, ra1=tagRoutingAlgorithm.raDefault):
            return scl.SetRouting(serviceId0, ra0, serviceId1, ra1)

        #<summary>
        #Query a routee service id from a given service id
        #</summary>
        #<param name="serviceId">A given service id</param>
        #<returns>A valid routee service id if this service id is valid and set to be routed</returns>
        @staticmethod
        def CheckRouting(serviceId):
            return scl.CheckRouting(serviceId)

    class CredentialManager(object):
        @staticmethod
        def HasUserId(userId):
            return scl.HasUserId(userId)

        @staticmethod
        def GetUserID(hSocket):
            str = (c_wchar * 256)()
            res = scl.GetUID(hSocket, str, 256)
            return str.value

        @staticmethod
        def SetUserID(hSocket, userId):
            return scl.SetUserID(hSocket, userId)

        @staticmethod
        def GetPassword(hSocket):
            str = (c_wchar * 256)()
            res = scl.GetPassword(hSocket, str, 256)
            return str.value

        @staticmethod
        def SetPassword(hSocket, password):
            return scl.SetPassword(hSocket, password)
