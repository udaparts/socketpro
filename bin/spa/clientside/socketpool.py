
import threading
from spa.clientside.ccoreloader import CCoreLoader as ccl
from spa import classproperty, CScopeUQueue
from spa.memqueue import CUQueue
from spa.clientside import tagSocketPoolEvent, BaseServiceID, tagEncryptionMethod, tagSocketOption, tagSocketLevel, tagConnectionState, tagOperationSystem, tagThreadApartment
from spa.clientside.conncontext import CConnectionContext
from spa.clientside.asynchandler import CAsyncServiceHandler
from spa.clientside.clientsocket import CClientSocket
import multiprocessing

class CSocketPool(object):
    DEFAULT_QUEUE_TIME_TO_LIVE = 24 * 3600

    #<summary>
    #Create an instance of socket pool
    #</summary>
    #<param name="clsAsyncHandler">An async handler derived from CAsyncServiceHandler</param>
    #<param name="autoconn">All sockets will support auto connection if true; and not if false</param>
    #<param name="recvtimeout">Request receiving timeout in milliseconds</param>
    #<param name="conntimeout">Socket connection timeout in milliseconds</param>
    #<param name="serviceid">Service id which defaults to 0</param>
    def __init__(self, clsAsyncHandler, autoconn=True, recvtimeout=CClientSocket.DEFAULT_RECV_TIMEOUT, conntimeout=CClientSocket.DEFAULT_CONN_TIMEOUT, serviceid=0):
        self._autoConn_ = autoconn
        self._recvTimeout_ = recvtimeout
        self._connTimeout_ = conntimeout
        self._serviceId_ = serviceid
        self.SocketPoolEvent = None
        self.DoSslServerAuthentication = None
        self._PoolId_ = 0
        self._lock_ = threading.Lock()
        self._m_dicSocketHandler_ = {}
        self._m_cbPool_ = ccl.PSocketPoolCallback(self._spe_)
        self._m_mcc_ = [[]]
        self._m_cls_ = clsAsyncHandler
        self._queue_name_ = ''

    def __del__(self):
        self.DoSslServerAuthentication = None
        self.SocketPoolEvent = None
        self.ShutdownPool()

    def _MapToHandler_(self, h):
        with self._lock_:
            for cs in self._m_dicSocketHandler_.keys():
                if cs.Handle == h:
                    return self._m_dicSocketHandler_[cs]
        return None

    def _CopyCC_(self, cc):
        threads = len(cc)
        socketsPerThread = len(cc[0])
        if socketsPerThread * threads == 0:
            raise ValueError("Must set connection context argument properly")
        with self._lock_:
            self._m_mcc_ = [[None for c in range(socketsPerThread)] for r in range(threads)]
            for c in range(socketsPerThread):
                for r in range(threads):
                    src = cc[r][c]
                    if src is None:
                         raise ValueError("Must set connection context argument properly")
                    else:
                        conn_context = CConnectionContext(src.Host, src.Port, src.UserId, src._Password_, src.EncrytionMethod, src.Zip, src.V6)
                        self._m_mcc_[r][c] = conn_context

    @property
    def AsyncHandlers(self):
        with self._lock_:
            return list(self._m_dicSocketHandler_.values())

    @property
    def Sockets(self):
        with self._lock_:
            return list(self._m_dicSocketHandler_.keys())

    @property
    def PoolId(self):
        with self._lock_:
            return self._PoolId_

    @property
    def Avg(self):
        with self._lock_:
            return ccl.IsAvg(self._PoolId_)

    @property
    def ConnectedSockets(self):
        with self._lock_:
            return ccl.GetConnectedSockets(self._PoolId_)

    @property
    def DisconnectedSockets(self):
        with self._lock_:
            return ccl.GetDisconnectedSockets(self._PoolId_)

    @property
    def IdleSockets(self):
        with self._lock_:
            return ccl.GetIdleSockets(self._PoolId_)

    @property
    def LockedSockets(self):
        with self._lock_:
            return ccl.GetLockedSockets(self._PoolId_)

    @property
    def QueueAutoMerge(self):
        with self._lock_:
            return ccl.GetQueueAutoMergeByPool(self._PoolId_)

    @QueueAutoMerge.setter
    def QueueAutoMerge(self, merge):
        with self._lock_:
            return ccl.SetQueueAutoMergeByPool(self._PoolId_, merge)

    @property
    def SocketsPerThread(self):
        with self._lock_:
            return ccl.GetSocketsPerThread(self._PoolId_)

    @property
    def ThreadsCreated(self):
        with self._lock_:
            return ccl.GetThreadCount(self._PoolId_)

    @property
    def Started(self):
        with self._lock_:
            return (self._PoolId_ != 0)

    @property
    def QueueName(self):
        with self._lock_:
            return self._queue_name_

    def _StopPoolQueue_(self):
        for cs in self._m_dicSocketHandler_.keys():
            cq = cs.ClientQueue
            if cq and cq.Available:
                cq.StopQueue()

    def _StartPoolQueue_(self, qName):
        index = 0
        for cs in self._m_dicSocketHandler_.keys():
            cq = cs.ClientQueue
            ok = cq.StartQueue(qName + str(index), CSocketPool.DEFAULT_QUEUE_TIME_TO_LIVE, cs.EncryptionMethod != tagEncryptionMethod.NoEncryption)

    def _SetQueue_(self, socket):
        index = 0
        for cs in self._m_dicSocketHandler_.keys():
            if cs == socket:
                if len(self._queue_name_) > 0:
                    if not cs.ClientQueue.Available:
                        cs.ClientQueue.StartQueue(self._queue_name_ + str(index), CSocketPool.DEFAULT_QUEUE_TIME_TO_LIVE, cs.EncryptionMethod != tagEncryptionMethod.NoEncryption)
                break;
            index += 1

    @QueueName.setter
    def QueueName(self, value):
        s = value.strip()
        if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
            s = s.lower()
        with self._lock_:
            if s != self._queue_name_:
                self._StopPoolQueue_()
                self._queue_name_ = s
                if len(s) > 0:
                    self._StartPoolQueue_(s)

    @property
    def Queues(self):
        q = 0
        with self._lock_:
            for cs in self._m_dicSocketHandler_.keys():
                if cs.ClientQueue.Available:
                    q += 1
        return q

    @classproperty
    def SocketPools(cls):
        return ccl.GetNumberOfSocketPools()

    def OnSocketPoolEvent(self, spe, handler):
        pass

    def _spe_(self, poolId, spe, h): #h -- usocket handle
        #print "Pool id = " + str(poolId) + ", spe = " + str(spe) + ", usocket handle = " + str(h)
        handler = self._MapToHandler_(h)
        if spe == tagSocketPoolEvent.speTimer:
            if CScopeUQueue.MemoryConsumed() / 1024 > CScopeUQueue.SHARED_BUFFER_CLEAN_SIZE:
                CScopeUQueue.DestroyUQueuePool()
        elif spe == tagSocketPoolEvent.speStarted:
            with self._lock_:
                self._PoolId_ = poolId
        elif spe == tagSocketPoolEvent.speShutdown:
            with self._lock_:
                self._PoolId_ = 0
        elif spe == tagSocketPoolEvent.speUSocketKilled:
            with self._lock_:
                del self._m_dicSocketHandler_[handler.AttachedClientSocket]
        elif spe == tagSocketPoolEvent.speUSocketCreated:
            cs = CClientSocket(h)
            ash = self._m_cls_()
            ccl.SetRecvTimeout(h, self._recvTimeout_)
            ccl.SetConnTimeout(h, self._connTimeout_)
            ccl.SetAutoConn(h, self._autoConn_)
            if ash.SvsID == 0:
                ash._m_nServiceId_ = self._serviceId_
            if ash.SvsID <= BaseServiceID.sidStartup:
                raise ValueError('Service id must be larger than SocketProAdapter.BaseServiceID.sidReserved (268435456)')
            ash._Attach_(cs)
            handler = ash
            with self._lock_:
                self._m_dicSocketHandler_[cs] = ash
        elif spe == tagSocketPoolEvent.speConnected:
            if ccl.IsOpened(h):
                cs = handler.AttachedClientSocket
                if self.DoSslServerAuthentication is not None and cs.EncryptionMethod == tagEncryptionMethod.TLSv1 and (not self.DoSslServerAuthentication(self, cs)):
                    return #don't set password or call SwitchTo in case failure of ssl server authentication on certificate from server
                ccl.SetSockOpt(h, tagSocketOption.soRcvBuf, 116800, tagSocketLevel.slSocket)
                ccl.SetSockOpt(h, tagSocketOption.soSndBuf, 116800, tagSocketLevel.slSocket)
                ccl.SetSockOpt(h, tagSocketOption.soTcpNoDelay, 1, tagSocketLevel.slTcp)
                ccl.SetPassword(h, cs.ConnectionContext._Password_)
                ok = ccl.StartBatching(h)
                ok = ccl.SwitchTo(h, handler.SvsID)
                ok = ccl.TurnOnZipAtSvr(h, cs.ConnectionContext.Zip)
                ok = ccl.SetSockOptAtSvr(h, tagSocketOption.soRcvBuf, 116800, tagSocketLevel.slSocket)
                ok = ccl.SetSockOptAtSvr(h, tagSocketOption.soSndBuf, 116800, tagSocketLevel.slSocket)
                ok = ccl.SetSockOptAtSvr(h, tagSocketOption.soTcpNoDelay, 1, tagSocketLevel.slTcp)
                self._SetQueue_(cs)
                ok = ccl.CommitBatching(h, False)
        elif spe == tagSocketPoolEvent.speQueueMergedFrom:
            self._hFrom = handler
        elif spe == tagSocketPoolEvent.speQueueMergedTo:
            self._hFrom._AppendTo_(handler)
            self._hFrom = None
        else:
            pass
        self.OnSocketPoolEvent(spe, handler)

    def _start_(self, sockets_per_thread, thread=0, avg=True, ta=0):
        with self._lock_:
            if self._PoolId_ != 0:
                return True
        return ccl.CreateSocketPool(self._m_cbPool_, sockets_per_thread, thread, avg, ta) != 0

    """
    <summary>
    Shut down the socket pool after all connections are disconnected.
    </summary>
    """
    def ShutdownPool(self):
        poolId = 0
        ok = True
        with self._lock_:
            poolId = self._PoolId_
            self._PoolId_ = 0
        if (poolId != 0):
            ok = ccl.DisconnectAll(poolId)
            ok = ccl.DestroySocketPool(poolId)
    """
    <summary>
    Disconnect all connections
    </summary>
    <returns>The method always returns true.</returns>
    """
    def DisconnectAll(self):
        poolId = 0
        with self._lock_:
            poolId = self._PoolId_
        if (poolId != 0):
            return ccl.DisconnectAll(poolId)
        return True

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.ShutdownPool()

    """
    <summary>
    Seek an async handler on the min number of requests queued in memory and its associated socket connection
    </summary>
    <returns>An async handler if found; and null or nothing if no connection is found</returns>
    """
    def Seek(self):
        h = None
        with self._lock_:
            for cs in self._m_dicSocketHandler_.keys():
                if cs.ConnectionState < tagConnectionState.csSwitched:
                    continue
                if h is None:
                    h = self._m_dicSocketHandler_[cs]
                else:
                    cs_coriq = cs.CountOfRequestsInQueue
                    h_coriq = h.AttachedClientSocket.CountOfRequestsInQueue
                    if cs_coriq < h_coriq:
                        h = self._m_dicSocketHandler_[cs]
                    elif cs_coriq == h_coriq and cs.BytesSent < h.AttachedClientSocket.BytesSent:
                        h = self._m_dicSocketHandler_[cs]
        return h

    """
    <summary>
    Seek an async handler by queue message count or its associated queue file full path or raw name.
    </summary>
    <param name="queueName">an optional queue file full path or raw name</param>
    <returns>An async handler if found; and null or nothing if none is found</returns>
    """
    def SeekByQueue(self, queueName=''):
        h = None
        if queueName is None or len(queueName) == 0:
            with self._lock_:
                merge = ccl.GetQueueAutoMergeByPool(self._PoolId_)
                for cs in self._m_dicSocketHandler_.keys():
                    if merge and cs.ConnectionState < tagConnectionState.csSwitched:
                        continue
                    cq = cs.ClientQueue
                    if not cq.Available:
                        continue
                    if h is None:
                        h = self._m_dicSocketHandler_[cs]
                    elif (cq.MessageCount < h.AttachedClientSocket.ClientQueue.MessageCount) or ((not h.AttachedClientSocket.Connected) and cs.Connected):
                        h = self._m_dicSocketHandler_[cs]
        else:
            if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
                queueName = queueName.lower()
            rawName = ''
            appName = ccl.GetClientWorkDirectory().decode('latin-1')
            with self._lock_:
                for cs in self._m_dicSocketHandler_.keys():
                    if not cs.ClientQueue.Available:
                        continue
                    if cs.ClientQueue.Secure:
                        rawName = queueName + "_" + appName + "_1.mqc"
                    else:
                        rawName = queueName + "_" + appName + "_0.mqc"
                    queueFileName = cs.ClientQueue.QueueFileName
                    length = len(queueFileName)
                    lenRaw = len(rawName)
                    if lenRaw > length:
                        continue
                    pos = queueFileName.rfind(rawName)
                    #queue file name with full path
                    if pos == 0:
                        return self._m_dicSocketHandler_[cs]
                    #queue raw name only
                    if pos + lenRaw == length:
                        return self._m_dicSocketHandler_[cs]
        return h

    """
    <summary>
    Lock an async handler on given thread handle and timeout.
    </summary>
    <param name="timeout">A timeout in milliseconds</param>
    <param name="sameThreadHandle">A handle to a thread</param>
    <returns>An async handler if successful; and null or nothing if failed</returns>
    <remarks>You must also call the method Unlock to unlock the handler and its associated socket connection for reuse.</remarks>
    """
    def Lock(self, timeout=0xffffffff, sameThreadHandle=0):
        poolId = None
        with self._lock_:
            poolId = self._PoolId_
        h = ccl.LockASocket(poolId, timeout, sameThreadHandle);
        return self._MapToHandler_(h)

    """
    <summary>
    Unlock a previously locked async handler to pool for reuse.
    </summary>
    <param name="asyncHandler_or_clientSocket">A previously locked async handler or client socket object</param>
    """
    def Unlock(self, asyncHandler_or_clientSocket):
        if asyncHandler_or_clientSocket is None:
            return
        poolId = None
        with self._lock_:
            poolId = self._PoolId_
        if isinstance(asyncHandler_or_clientSocket, CAsyncServiceHandler):
            ccl.UnlockASocket(poolId, asyncHandler_or_clientSocket.AttachedClientSocket.Handle)
        elif isinstance(asyncHandler_or_clientSocket, CClientSocket):
            ccl.UnlockASocket(poolId, asyncHandler_or_clientSocket.Handle)
        else:
            raise ValueError('Unexpected input value')

    """
    <summary>
    Start a pool of sockets with one connection context
    </summary>
    <param name="cc">A connection context structure</param>
    <param name="socketsPerThread">The number of socket connections per thread</param>
    <param name="threads">The number of threads in a pool which defaults to the number of CPU cores</param>
    <param name="avg">A boolean value for building internal socket pool, which defaults to true.</param>
    <param name="ta">A value for COM thread apartment if there is COM object involved. It is ignored on non-window platforms, and default to tagThreadApartment.taNone</param>
    <returns>False if there is no connection established; and true as long as there is one connection started</returns>
    """
    def StartSocketPool(self, cc, socketsPerThread, threads=0, avg=True, ta=tagThreadApartment.taNone):
        if not isinstance(cc, CConnectionContext):
            raise ValueError('Must input a valid CConnectionContext structure')
        if socketsPerThread <= 0:
            raise ValueError('Must input the number of sockets for each of threads')
        if threads==0:
            threads = multiprocessing.cpu_count()
        mcc = [[0 for i in range(socketsPerThread)] for i in range(threads)]
        for m in range(0, socketsPerThread):
            for n in range(0, threads):
                mcc[n][m] = cc
        return self.StartSocketPoolEx(mcc, avg, ta)

    """
    <summary>
    Start a pool of sockets with a given two-dimensional matrix of connection contexts
    </summary>
    <param name="cc">A given two-dimensional matrix of connection contexts. Its first dimension length represents the number of threads; and the second dimension length is the number of sockets per thread</param>
    <param name="avg">A boolean value for building internal socket pool, which defaults to true.</param>
    <param name="ta">A value for COM thread apartment if there is COM object involved. It is ignored on non-window platforms, and default to tagThreadApartment.taNone</param>
    <returns>False if there is no connection established; and true as long as there is one connection started</returns>
    """
    def StartSocketPoolEx(self, cc, avg=True, ta=tagThreadApartment.taNone):
        ok = False
        temp = {}
        if cc is None or len(cc)==0 or len(cc[0])==0:
            raise ValueError('Must set connection context argument properly')
        if self.Started:
            self.ShutdownPool()
        self._CopyCC_(cc)
        first = True
        threads = len(cc)
        socketsPerThread = len(cc[0])
        if not self._start_(socketsPerThread, threads, avg, ta):
            return False
        with self._lock_:
            index = 0
            keys = self._m_dicSocketHandler_.keys()
            for cs in keys:
                temp[cs] = self._m_dicSocketHandler_[cs]
                m = int(index % threads)
                n = int(index / threads)
                c = self._m_mcc_[m][n]
                if c.Host is None:
                    raise ValueError('Host string can not be null')
                c.Host = c.Host.strip()
                if len(c.Host) == 0:
                    raise ValueError('Host string must be a valid string')
                if c.Port == 0:
                    raise ValueError("Host port can't be zero")
                cs.ConnectionContext = c
                index += 1
        for cs in temp.keys():
            if cs.Connected:
                first = False
                continue
            c = cs.ConnectionContext
            ccl.SetEncryptionMethod(cs.Handle, c.EncrytionMethod)
            ccl.SetUserID(cs.Handle, c.UserId)
            ccl.SetZip(cs.Handle, c.Zip)
            if first:
                ok = ccl.Connect(cs.Handle, c.Host.encode('latin-1'), c.Port, True, c.V6)
                if ok and ccl.WaitAll(cs.Handle, 0xffffffff):
                    first = False
            else:
                ccl.Connect(cs.Handle, c.Host.encode('latin-1'), c.Port, False, c.V6)
        return self.ConnectedSockets > 0