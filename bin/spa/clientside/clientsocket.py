
import threading
from spa.clientside.ccoreloader import CCoreLoader as ccl
from spa.clientside.ccoreloader import CQueueConfigureImpl, CSSLConfigureImpl
from ctypes import c_uint, c_ubyte, c_char, c_uint64, c_int, byref, memmove, addressof, create_unicode_buffer, create_string_buffer, c_bool
from spa.clientside import tagQueueStatus, IClientQueue
from spa.memqueue import CUQueue
from spa import tagBaseRequestID, classproperty, IPushEx, BaseServiceID
from spa.clientside.messagesender import CSender, USocket_Client_Handle

import time

class CClientSocket:
    DEFAULT_RECV_TIMEOUT = 30000
    DEFAULT_CONN_TIMEOUT = 30000

    class CPushImpl(IPushEx):
        def __init__(self, cs):
            self.OnPublish = None
            self.OnPublishEx = None
            self.OnSendUserMessage = None
            self.OnSendUserMessageEx = None
            self.OnSubscribe = None
            self.OnUnsubscribe = None
            self._m_cs_ = cs

        def __del__(self):
            self._m_cs_ = None

        def Publish(self, message, groups, hint=''):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            q = CUQueue().SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return ccl.Speak(self._m_cs_.Handle, bytes, q.GetSize(), arr, size)

        def SendUserMessage(self, message, userId, hint=''):
            if userId is None:
                userId = u''
            q = CUQueue().SaveObject(message, hint)
            bytes = (c_ubyte * q.GetSize()).from_buffer(q._m_bytes_)
            return ccl.SendUserMessage(self._m_cs_.Handle, userId, bytes, q.GetSize())

        def Subscribe(self, groups):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            return ccl.Enter(self._m_cs_.Handle, arr, size)

        def Unsubscribe(self):
            ccl.Exit(self._m_cs_.Handle)
            return True

        def PublishEx(self, message, groups):
            if groups is None:
                groups = ()
            size = len(groups)
            arr = (c_uint * size)(*groups)
            if message is None:
                message = ()
            msize = len(message)
            arrMessage = (c_ubyte * msize).from_buffer(message)
            return ccl.SpeakEx(self._m_cs_.Handle, arrMessage, msize, arr, size)

        def SendUserMessageEx(self, userId, message):
            if userId is None:
                userId = u''
            if message is None:
                message = ()
            msize = len(message)
            arrMessage = (c_ubyte * msize).from_buffer(message)
            return ccl.SendUserMessageEx(self._m_cs_.Handle, userId, arrMessage, msize)

    class CClientQueueImpl(IClientQueue):
        def __init__(self, cs):
            self._m_cs_ = cs
            self._nQIndex_ = 0

        def StartQueue(self, qName, ttl, secure=True, dequeueShared=False):
            if qName is None or len(qName) == 0:
                raise ValueError('Invalid queue file name')
            return ccl.StartQueue(self._m_cs_.Handle, qName.encode('latin-1'), secure, dequeueShared, ttl)

        @property
        def RoutingQueueIndex(self):
            return ccl.IsRoutingQueueIndexEnabled(self._m_cs_.Handle)

        @RoutingQueueIndex.setter
        def RoutingQueueIndex(self, value):
            ccl.EnableRoutingQueueIndex(self._m_cs_.Handle, value)

        @property
        def DequeueEnabled(self):
            return ccl.IsDequeueEnabled(self._m_cs_.Handle)

        def AppendTo(self, queues):
            if queues is None:
                return True
            queues = list(queues)
            count = len(queues)
            if count == 0:
                return True
            index = 0
            handles = (USocket_Client_Handle * count)()
            for q in queues:
                if isinstance(q, CClientSocket) or isinstance(q, IClientQueue):
                    handles[index] = q.Handle
                else:
                    handles[index] = q
                index += 1
            return ccl.PushQueueTo(self._m_cs_.Handle, handles, count)

        def EnsureAppending(self, queues):
            if not self.Available:
                return False
            if self.QueueStatus != tagQueueStatus.qsMergePushing:
                return True
            if queues is None:
                return True
            queues = list(queues)
            if len(queues) == 0:
                return True
            handles = []
            for q in queues:
                h = 0
                if isinstance(q, CClientSocket) or isinstance(q, IClientQueue):
                    h = q.Handle
                else:
                    h = q
                if ccl.GetClientQueueStatus(h) != tagQueueStatus.qsMergeComplete:
                    handles.append(h)
            if len(handles) > 0:
                return self.AppendTo(handles)
            self.Reset()
            return True

        @property
        def Handle(self):
            return self._m_cs_.Handle

        @property
        def LastMessageTime(self):
            seconds = ccl.GetLastQueueMessageTime(self._m_cs_.Handle)
            temp = time.strptime('1 Jan 2013', '%d %b %Y')
            seconds += time.mktime(temp)
            return float(seconds)

        def StopQueue(self, permanent=False):
            ccl.StopQueue(self._m_cs_.Handle, permanent)

        def CancelQueuedMessages(self, startIndex, endIndex):
            return ccl.CancelQueuedRequestsByIndex(self._m_cs_.Handle, startIndex, endIndex)

        def RemoveByTTL(self):
            return ccl.RemoveQueuedRequestsByTTL(self._m_cs_.Handle)

        def AbortJob(self):
            ash = self._m_cs_.CurrentHandler
            with ash._lock_:
                aborted = len(ash._m_kvCallback_) - self._nQIndex_
                if ccl.AbortJob(self._m_cs_.Handle):
                    while aborted > 0:
                        p = ash._m_kvCallback_.pop()
                        if p and p[1] and p[1].Discarded:
                            p[1].Discarded(ash, True)
                        aborted -= 1
                    return True
            return False

        def StartJob(self):
            ash = self._m_cs_.CurrentHandler
            with ash._lock_:
                self._nQIndex_ = len(ash._m_kvCallback_)
            return ccl.StartJob(self._m_cs_.Handle)

        def EndJob(self):
            return ccl.EndJob(self._m_cs_.Handle)

        def Reset(self):
            ccl.ResetQueue(self._m_cs_.Handle)

        @property
        def MessagesInDequeuing(self):
            return ccl.GetMessagesInDequeuing(self._m_cs_.Handle)

        @property
        def MessageCount(self):
            return ccl.GetMessageCount(self._m_cs_.Handle)

        @property
        def QueueSize(self):
            return ccl.GetQueueSize(self._m_cs_.Handle)

        @property
        def Available(self):
            return ccl.IsQueueStarted(self._m_cs_.Handle)

        @property
        def Secure(self):
            return ccl.IsQueueSecured(self._m_cs_.Handle)

        @property
        def QueueFileName(self):
            return ccl.GetQueueFileName(self._m_cs_.Handle).decode('latin-1')

        @property
        def QueueName(self):
            return ccl.GetQueueName(self._m_cs_.Handle).decode('latin-1')

        @property
        def JobSize(self):
            return ccl.GetJobSize(self._m_cs_.Handle)

        @property
        def DequeueShared(self):
            return ccl.IsDequeueShared(self._m_cs_.Handle)

        @property
        def LastIndex(self):
            ccl.GetQueueLastIndex(self._m_cs_.Handle)

        @property
        def QueueStatus(self):
            return ccl.GetClientQueueStatus(self._m_cs_.Handle)

        @property
        def TTL(self):
            return ccl.GetTTL(self._m_cs_.Handle)

        @property
        def Optimistic(self):
            return ccl.GetOptimistic(self._m_cs_.Handle)

        @Optimistic.setter
        def Optimistic(self, value):
            ccl.SetOptimistic(self._m_cs_.Handle, value)

    def __init__(self, handle):
        self._m_h_ = handle
        self._m_cc_ = None
        self._m_lstAsh_ = []
        self._m_cert_ = None
        self._m_random_ = False
        self._m_currSvsId = BaseServiceID.sidStartup
        #typedef void (CALLBACK *POnHandShakeCompleted) (USocket_Client_Handle handler, int nError);
        self._m_hsc_ = ccl.POnHandShakeCompleted(self._hsc_)
        #typedef void (CALLBACK *POnRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
        self._m_rp_ = ccl.POnRequestProcessed(self._rp_)
        #typedef void (CALLBACK *POnSocketClosed) (USocket_Client_Handle handler, int nError);
        self._m_ss_ = ccl.POnSocketClosed(self._ss_)
        #typedef void (CALLBACK *POnSocketConnected) (USocket_Client_Handle handler, int nError);
        self._m_sc_ = ccl.POnSocketConnected(self._sc_)
        #typedef void (CALLBACK *POnBaseRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId);
        self._m_brp_ = ccl.POnBaseRequestProcessed(self._brp_)
        #typedef void (CALLBACK *POnAllRequestsProcessed) (USocket_Client_Handle handler, unsigned short lastRequestId);
        self._m_arp_ = ccl.POnAllRequestsProcessed(self._arp_)
        #typedef void (CALLBACK *POnServerException) (USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
        self._m_se_ = ccl.POnServerException(self._se_)
        #typedef void (CALLBACK *POnSpeakEx2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
        self._m_sx_ = ccl.POnSpeakEx(self._sx_)
        #typedef void (CALLBACK *POnEnter2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
        self._m_enter_ = ccl.POnEnter(self._enter_)
        #typedef void (CALLBACK *POnExit2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
        self._m_exit_ = ccl.POnExit(self._exit_)
        #typedef void (CALLBACK *POnSpeak2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
        self._m_s_ = ccl.POnSpeak(self._s_)
        #typedef void (CALLBACK *POnSendUserMessageEx2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned char *pMessage, unsigned int size);
        self._m_sume_ = ccl.POnSendUserMessageEx(self._sume_)
        #typedef void (CALLBACK *POnSendUserMessage2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned char *message, unsigned int size);
        self._sum_ = ccl.POnSendUserMessage(self._sum_)

        ccl.SetOnHandShakeCompleted(self._m_h_, self._m_hsc_)
        ccl.SetOnRequestProcessed(self._m_h_, self._m_rp_)
        ccl.SetOnSocketClosed(self._m_h_, self._m_ss_)
        ccl.SetOnSocketConnected(self._m_h_, self._m_sc_)
        ccl.SetOnBaseRequestProcessed(self._m_h_, self._m_brp_)
        ccl.SetOnAllRequestsProcessed(self._m_h_, self._m_arp_)
        ccl.SetOnServerException(self._m_h_, self._m_se_)
        ccl.SetOnSpeakEx(self._m_h_, self._m_sx_)
        ccl.SetOnEnter(self._m_h_, self._m_enter_)
        ccl.SetOnExit(self._m_h_, self._m_exit_)
        ccl.SetOnSpeak(self._m_h_, self._m_s_)
        ccl.SetOnSendUserMessageEx(self._m_h_, self._m_sume_)
        ccl.SetOnSendUserMessage(self._m_h_, self._sum_)

        self.HandShakeCompleted = None
        self.RequestProcessed = None
        self.SocketClosed = None
        self.SocketConnected = None
        self.BaseRequestProcessed = None
        self.AllRequestsProcessed = None
        self.ServerException = None
        self._m_PushImpl_ = CClientSocket.CPushImpl(self)
        self._m_qm_ = CClientSocket.CClientQueueImpl(self)

    @property
    def Push(self):
        return self._m_PushImpl_

    @property
    def ClientQueue(self):
        return self._m_qm_

    @property
    def UCert(self):
        return self._m_cert_

    def _hsc_(self, handler, nError):
        if not self.HandShakeCompleted is None:
            self.HandShakeCompleted(self, nError)

    def _rp_(self, handler, reqId, size):
        ash = self._Seek_(self.CurrentServiceID)
        if not ash is None:
            mem = (c_char * size)()
            res = ccl.RetrieveResult(handler, mem, size)
            if res != size:
                if res == 0:
                    return #socket closed
                msg = 'Wrong number of bytes retrieved (expected = ' + str(size) + ' and obtained = ' + str(res)
                raise ValueError(msg)
            q = CUQueue(mem)
            ash._OnRR_(reqId, q)
        if not self.RequestProcessed is None:
            self.RequestProcessed(self, reqId, size)

    def _ss_(self, handler, nError):
        ash = self._Seek_(self.CurrentServiceID)
        if ash and not self._m_qm_.Available:
            ash.CleanCallbacks()
        if not self.SocketClosed is None:
            self.SocketClosed(self, nError)

    class CUCertImpl(object):
        def __init__(self, ci, cs=None):
            self._m_cs_ = cs
            self._m_ci_ = ci

        def Verify(self):
            code = c_int()
            res = ccl.Verify(self._m_cs_.Handle, byref(code))
            return code.value, res.decode('utf-8')

        @property
        def Issuer(self):
            return self._m_ci_.contents.Issuer.decode('utf-8')

        @property
        def Subject(self):
            return self._m_ci_.contents.Subject.decode('utf-8')

        @property
        def NotBefore(self):
            return self._m_ci_.contents.NotBefore.decode('utf-8')

        @property
        def NotAfter(self):
            return self._m_ci_.contents.NotAfter.decode('utf-8')

        @property
        def Validity(self):
            return self._m_ci_.contents.Validity

        @property
        def SigAlg(self):
            return self._m_ci_.contents.SigAlg.decode('utf-8')

        @property
        def CertPem(self):
            return self._m_ci_.contents.CertPem.decode('utf-8')

        @property
        def SessionInfo(self):
            return self._m_ci_.contents.SessionInfo.decode('utf-8')

        @property
        def PublicKey(self):
            index = 0
            keySize = self._m_ci_.contents.PKSize
            data = (c_ubyte * keySize)()
            for b in data:
                data[index] = self._m_ci_.contents.PublicKey[index]
                index += 1
            return list(data)

        @property
        def Algorithm(self):
            index = 0
            size = self._m_ci_.contents.AlgSize
            data = (c_ubyte * size)()
            for b in data:
                data[index] = self._m_ci_.contents.Algorithm[index]
                index += + 1
            return list(data)

        @property
        def SerialNumber(self):
            index = 0
            size = self._m_ci_.contents.SNSize
            data = (c_ubyte * size)()
            for b in data:
                data[index] = self._m_ci_.contents.SerialNumber[index]
                index += + 1
            return list(data)

    def _sc_(self, handler, nError):
        if nError == 0 and ccl.GetSSL(self._m_h_) != 0:
            pCertInfo = ccl.GetUCert(self._m_h_)
            self._m_cert_ = CClientSocket.CUCertImpl(pCertInfo, self)
        else:
            self._m_cert_ = None
        if not self.SocketConnected is None:
            self.SocketConnected(self, nError)

    def _brp_(self, handler, reqId):
        if reqId == tagBaseRequestID.idSwitchTo:
            self._m_random_ = ccl.IsRandom(self._m_h_)
            self._m_currSvsId = ccl.GetCurrentServiceId(self._m_h_)
        ash = self._Seek_(self.CurrentServiceID)
        if not ash is None:
            ash.OnBaseRequestProcessed(reqId)
            if reqId == tagBaseRequestID.idCancel:
                ash.CleanCallbacks()
        if not self.BaseRequestProcessed is None:
            self.BaseRequestProcessed(self, reqId)

    def _arp_(self, handler, reqId):
        ash = self._Seek_(self.CurrentServiceID)
        if ash:
            ash.OnAllProcessed()
        if not self.AllRequestsProcessed is None:
            self.AllRequestsProcessed(self, reqId)

    def _se_(self, handler, reqId, errMessage, errWhere, errCode):
        ash = self._Seek_(self.CurrentServiceID)
        if not ash is None:
            ash._OnSE_(reqId, errMessage, errWhere, errCode)
        if not self.ServerException is None:
            self.ServerException(self, reqId, errMessage, errWhere, errCode)

    def _Seek_(self, svsId):
        for ash in self._m_lstAsh_:
            if ash.SvsID == svsId:
                return ash
        return None

    @staticmethod
    def _makeGroups_(pGroup, count): #pGroup = POINTER(c_uint)
        groups = (c_int * count)()
        memmove(addressof(groups), pGroup, count * 4)
        return list(groups)

    @staticmethod
    def _prepareBytes(message, size):
        bytes = (c_char * size)()
        memmove(addressof(bytes), message, size)
        return bytes

    def _sx_(self, handler, sender, pGroup, count, pMessage, size):
        if not self.Push.OnPublishEx is None:
            self.Push.OnPublishEx(self, CSender(sender), CClientSocket._makeGroups_(pGroup, count), CClientSocket._prepareBytes(pMessage, size))

    def _enter_(self, handler, sender, pGroup, count):
        if not self.Push.OnSubscribe is None:
            self.Push.OnSubscribe(self, CSender(sender), CClientSocket._makeGroups_(pGroup, count))

    def _exit_(self, handler, sender, pGroup, count):
        if not self.Push.OnUnsubscribe is None:
            self.Push.OnUnsubscribe(self, CSender(sender), CClientSocket._makeGroups_(pGroup, count))

    def _s_(self, handler, sender, pGroup, count, pMessage, size):
        if not self.Push.OnPublish is None:
            self.Push.OnPublish(self, CSender(sender), CClientSocket._makeGroups_(pGroup, count), CUQueue(CClientSocket._prepareBytes(pMessage, size)).LoadObject())

    def _sum_(self, handler, sender, pMessage, size):
        if not self.Push.OnSendUserMessage is None:
            self.Push.OnSendUserMessage(self, CSender(sender), CUQueue(CClientSocket._prepareBytes(pMessage, size)).LoadObject())

    def _sume_(self, handler, sender, pMessage, size):
        if not self.Push.OnSendUserMessageEx is None:
            self.Push.OnSendUserMessageEx(self, CSender(sender), CClientSocket._prepareBytes(pMessage, size))

    def __del__(self):
        for h in self._m_lstAsh_:
            h._SetNull_()
        self._m_lstAsh_ = []

    def _Attach_(self, ash):
        if ash is None:
            return
        for h in self._m_lstAsh_:
            if h.SvsID == ash.SvsID:
                return False
        self._m_lstAsh_.append(ash)

    def _Detach_(self, ash):
        if ash is None:
            return
        self._m_lstAsh_.remove(ash)
        ash._SetNull_()

    def WaitAll(self, timeOut = 0xffffffff):
        if ccl.IsBatching(self._m_h_):
            raise Exception("Can't call the method WaitAll during batching requests")
        if ccl.IsQueueStarted(self._m_h_) and ccl.GetJobSize(self._m_h_) > 0:
            raise Exception("Can't call the method WaitAll during enqueuing transactional requests")
        return ccl.WaitAll(self._m_h_, timeOut)

    def Cancel(self):
        if ccl.IsBatching(self._m_h_):
            raise Exception("Can't call the method Cancel during batching requests")
        return ccl.Cancel(self._m_h_, -1)

    def GetPeerName(self):
        port = c_uint()
        addr = (c_char * 256)()
        res = ccl.GetPeerName(self._m_h_, byref(port), addr, 256)
        return addr.value.decode('latin-1'), port.value

    def DoEcho(self):
        return ccl.DoEcho(self._m_h_)

    def GetOS(self):
        b = c_bool()
        os = ccl.GetPeerOs(self._m_h_, byref(b))
        return os, b.value

    @staticmethod
    def SetLastCallInfo(str):
        ccl.SetLastCallInfo(str.encode('latin-1'))

    @property
    def DequeuedMessageAborted(self):
        ccl.AbortDequeuedMessage(self._m_h_)

    @property
    def AutoConn(self):
        return ccl.GetAutoConn(self._m_h_)

    @AutoConn.setter
    def AutoConn(self, value):
        ccl.SetAutoConn(self._m_h_, value)

    @property
    def Batching(self):
        return ccl.IsBatching(self._m_h_)

    @property
    def Handle(self):
        return self._m_h_

    @AutoConn.setter
    def AutoConn(self, value):
        ccl.SetAutoConn(self._m_h_, value)

    @property
    def BytesBatched(self):
        return ccl.GetBytesBatched(self._m_h_)

    @property
    def BytesInReceivingBuffer(self):
        return ccl.GetBytesInReceivingBuffer(self._m_h_)

    @property
    def BytesInSendingBuffer(self):
        return ccl.GetBytesInSendingBuffer(self._m_h_)

    @property
    def BytesReceived(self):
        return ccl.GetBytesReceived(self._m_h_)

    @property
    def BytesSent(self):
        return ccl.GetBytesSent(self._m_h_)

    @property
    def Connected(self):
        return ccl.IsOpened(self._m_h_)

    @property
    def ConnectingTimeout(self):
        return ccl.GetConnTimeout(self._m_h_)

    @ConnectingTimeout.setter
    def ConnectingTimeout(self, value):
        ccl.SetConnTimeout(self._m_h_, value)

    @property
    def CurrentRequestID(self):
        return ccl.GetCurrentRequestID(self._m_h_)

    @property
    def ConnectionContext(self):
        return self._m_cc_

    @ConnectionContext.setter
    def ConnectionContext(self, value):
        self._m_cc_ = value

    @property
    def CurrentResultSize(self):
        return ccl.GetCurrentResultSize(self._m_h_)

    @property
    def CurrentServiceID(self):
        return self._m_currSvsId

    @property
    def CurrentHandler(self):
        return self._Seek_(self._m_currSvsId)

    @property
    def EncryptionMethod(self):
        return ccl.GetEncryptionMethod(self._m_h_)

    @EncryptionMethod.setter
    def EncryptionMethod(self, value):
        ccl.SetEncryptionMethod(self._m_h_, value)

    @property
    def RouteeCount(self):
        return ccl.GetRouteeCount(self._m_h_)

    @property
    def ConnectionState(self):
        return ccl.GetConnectionState(self._m_h_)

    @property
    def Routing(self):
        return ccl.IsRouting(self._m_h_)

    @property
    def Random(self):
        return self._m_random_

    @property
    def Sendable(self):
        return (ccl.IsOpened(self._m_h_) or ccl.IsQueueStarted(self._m_h_))

    @property
    def ServerPingTime(self):
        return ccl.GetServerPingTime(self._m_h_)

    @property
    def UID(self):
        str = create_unicode_buffer(256)
        res = ccl.GetUID(self._m_h_, str, 256)
        return str.value

    @UID.setter
    def UID(self, value):
        ccl.SetUserID(self._m_h_, value)

    @property
    def Zip(self):
        return ccl.GetZip(self._m_h_)

    @Zip.setter
    def Zip(self, value):
        ccl.SetZip(self._m_h_, value)

    @property
    def SocketNativeHandle(self):
        return ccl.GetSocketNativeHandle(self._m_h_)

    @property
    def SslHandle(self):
        return ccl.GetSSL(self._m_h_)

    @property
    def CountOfRequestsInQueue(self):
        return ccl.GetCountOfRequestsQueued(self._m_h_)

    @property
    def ErrorCode(self):
        return ccl.GetErrorCode(self._m_h_)

    @property
    def ZipLevel(self):
        return ccl.GetZipLevel(self._m_h_)

    @ZipLevel.setter
    def ZipLevel(self, value):
        ccl.SetZipLevel(self._m_h_, value)

    @property
    def ErrorMessage(self):
        errMsg = create_string_buffer(2048)
        res = ccl.GetErrorMessage(self._m_h_, errMsg, 2048)
        return errMsg.value.decode('latin-1')

    @property
    def ReceivingTimeout(self):
        return ccl.GetRecvTimeout(self._m_h_)

    @ReceivingTimeout.setter
    def ReceivingTimeout(self, value):
        ccl.SetRecvTimeout(self._m_h_, value)

    @classproperty
    def Version(cls):
        return ccl.GetUClientSocketVersion().decode('latin-1')

    SSL = CSSLConfigureImpl()
    QueueConfigure = CQueueConfigureImpl()
