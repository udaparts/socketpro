
from spa import tagOperationSystem, tagEncryptionMethod
from spa.clientside.clientsocket import CClientSocket
from spa.clientside import CConnectionContext, CSocketPool
from spa.memqueue import CUQueue
from spa.clientside import tagSocketPoolEvent

import os

class ReplicationSetting(object):
    DEAFULT_TTL = 30 * 24 * 3600 #30 days
    def __init__(self):
        # <summary>
        # An absolute path to a directory containing message queue files.
        # </summary>
        self.QueueDir = CClientSocket.QueueConfigure.WorkDirectory

        # <summary>
        # False for auto socket connecting. Otherwise, there is no auto connection.
        # </summary>
        self.NoAutoConn = False

        # <summary>
        # Time-to-live in seconds. It is ignored if persistent message queue feature is not used. If the value is not set or zero, the value will default to DEFAULT_TTL (30 days).
        # </summary>
        self.TTL = ReplicationSetting.DEAFULT_TTL

        # <summary>
        # A timeout for receiving result from remote SocketPro server. If the value is not set or it is zero, the value will default to CClientSocket.DEFAULT_RECV_TIMEOUT (30 seconds).
        # </summary>
        self.RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT
        # <summary>
        # A timeout for connecting to remote SocketPro server. If the value is not set or it is zero, the value will default to CClientSocket.DEFAULT_CONN_TIMEOUT (30 seconds).
        # </summary>
        self.ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT

    def Copy(self):
        rs = ReplicationSetting()
        rs.ConnTimeout = self.ConnTimeout
        rs.NoAutoConn = self.NoAutoConn
        rs.QueueDir = self.QueueDir
        rs.RecvTimeout = self.RecvTimeout
        rs.TTL = self.TTL
        return rs

class CReplication(object):
    _DIR_SEP_ = '/'
    if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
        _DIR_SEP_ = '\\'

    def __del__(self):
        self._Cleanup()
    
    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self._Cleanup()
        
    def _Cleanup(self):
        if not self._m_pool_ is None:
            self._m_pool_.ShutdownPool()
            self._m_pool_ = None

    #<summary>
    #Construct a CReplication instance
    #</summary>
    #<param name="clsAsyncHandler">An async handler derived from CAsyncServiceHandler</param>
    #<param name="qms">A structure for setting its underlying socket pool and message queue directory as well as password for source queue</param>
    def __init__(self, clsAsyncHandler, qms):
        self._m_clsAsyncHandler_ = clsAsyncHandler
        self._m_mapQueueConn_ = {}
        self._m_pool_ = None
        self._CheckReplicationSetting_(qms)

    def DoSslServerAuthentication(self, cs):
        return True

    @property
    def Connections(self):
        if self._m_pool_ is None:
            return 0
        return self._m_pool_.ConnectedSockets

    @property
    def Replicable(self):
        return len(self._m_mapQueueConn_) > 1

    @property
    def SourceHandler(self):
        if self._m_pool_ is None:
            return None
        one = None
        for h in self._m_pool_.AsyncHandlers:
            one = h
        return one

    @property
    def SourceQueue(self):
        src = self.SourceHandler
        if src is None:
            return None
        return src.AttachedClientSocket.ClientQueue

    @property
    def TargetQueues(self):
        handlers = self.TargetHandlers
        if handlers is None:
            return None
        tq = []
        for h in handlers:
            tq.append(h.AttachedClientSocket.ClientQueue)
        return tq

    @property
    def TargetHandlers(self):
        if self._m_pool_ is None:
            return None
        tq = []
        for h in self._m_pool_.AsyncHandlers:
            tq.append(h)
            if len(tq) >= len(self._m_mapQueueConn_):
                break
        return tq

    @property
    def Hosts(self):
        return len(self._m_mapQueueConn_)

    @property
    def Queues(self):
        if self._m_pool_ is None:
            return 0
        return self._m_pool_.Queues

    @property
    def ReplicationSetting(self):
        return self._m_rs_

    """
    <summary>
    Make a replication. An invalid operation exception will be thrown if not replicable.
    </summary>
    <returns>True for success; and false for failure</returns>
    """
    def DoReplication(self):
        if len(self._m_mapQueueConn_) == 1:
            raise Exception('No replication is allowed because the number of target message queues less than two')
        src = self.SourceQueue
        if src is None:
            return False
        return src.AppendTo(self.TargetQueues)

    def _DoesQueueExist_(self, qName):
        ignoreCase = False
        if CUQueue.DEFAULT_OS == tagOperationSystem.osWin:
            ignoreCase = True
        for name in self._m_mapQueueConn_.keys():
            if ignoreCase:
                if name.lower() == qName.lower():
                    return True
            else:
                if name == qName:
                    return True
        return False

    def Send(self, reqId, q):
        src = self.SourceHandler
        if src is None:
            return False
        cq = src.AttachedClientSocket.ClientQueue
        if not cq.Available:
            return False
        ok = src.SendRequest(reqId, q, None)
        if self.Replicable and cq.JobSize == 0:
            ok = cq.AppendTo(self.TargetQueues)
        return ok

    def EndJob(self):
        src = self.SourceQueue
        if src is None or not src.Available:
            return False
        ok = src.EndJob()
        if ok and self.Replicable:
            ok = src.AppendTo(self.TargetQueues)
        return ok

    def StartJob(self):
        src = self.SourceQueue
        if src is None or not src.Available:
            return False
        return src.StartJob()

    def AbortJob(self):
        src = self.SourceQueue
        if src is None or not src.Available:
            return False
        return src.AbortJob()

    def _EndProcess_(self, vDbFullName, secure, mcc, ta):
        self._m_pool_ = CSocketPool(self._m_clsAsyncHandler_, not self._m_rs_.NoAutoConn, self._m_rs_.RecvTimeout, self._m_rs_.ConnTimeout)
        if secure:
            def AuthInternal(sender, cs):
                if cs.ConnectionContext.Port == 0xffff or cs.ConnectionContext.Port == -1:
                    return True
                return self.DoSslServerAuthentication(cs)
            self._m_pool_.DoSslServerAuthentication = AuthInternal

        def PoolEvent(sender, spe, handler):
            if spe == tagSocketPoolEvent.speSocketClosed:
                handler.CleanCallbacks()
            elif spe == tagSocketPoolEvent.speConnecting:
                if handler.AttachedClientSocket.ConnectionContext.Port == 0xffff or handler.AttachedClientSocket.ConnectionContext.Port == -1:
                    handler.AttachedClientSocket.ConnectingTimeout = 500
            else:
                pass
        self._m_pool_.SocketPoolEvent = PoolEvent

        ok = self._m_pool_.StartSocketPoolEx(mcc, True, ta)
        n = 0
        for s in self._m_pool_.Sockets:
            key = vDbFullName[n]
            ok = s.ClientQueue.StartQueue(key, self._m_rs_.TTL, secure)
            n += 1
        if self.Replicable:
            self.SourceQueue.EnsureAppending(self.TargetQueues)
        targetHandlers = self.TargetHandlers
        for h in targetHandlers:
            ok = h.AttachedClientSocket.DoEcho()

    def _CheckReplicationSetting_(self, qms):
        if qms.TTL == 0:
            qms.TTL = ReplicationSetting.DEAFULT_TTL
        if qms.ConnTimeout == 0:
            qms.ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT
        if qms.RecvTimeout == 0:
            qms.RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT
        if qms.QueueDir is None:
            raise ValueError('An absolute path required for working directory')
        qms.QueueDir = qms.QueueDir.strip()
        if len(qms.QueueDir) == 0:
            raise ValueError('An absolute path required for working directory')
        if not os.path.exists(qms.QueueDir):
            os.makedirs(qms.QueueDir)
        self._m_rs_ = qms.Copy()

    def _CheckConnQueueName_(self, mapQueueConn):
        self._m_mapQueueConn_.clear()
        if mapQueueConn is None or len(mapQueueConn) == 0:
            raise ValueError('One middle server required at least')
        for key, cc in mapQueueConn.items():
            if cc is None:
                raise ValueError('An invalid host found')
            if key is None or len(key) == 0:
                raise ValueError('A non-empty string for persistent queue name required for each of hosts')
            if self._DoesQueueExist_(key):
                raise Exception('Queue name duplicated -- ' + key)
            self._m_mapQueueConn_[key] = cc

    #<summary>
    #Start a socket pool for replication
    #</summary>
    #<param name="mapQueueConn">A dictionary for message queue name and connecting context. a unique name must be specified for each of connecting contexts</param>
    #<param name="rootQueueName">A string for root replication queue name. It is ignored if it is not replicable</param>
    #<param name="ta">COM thread apartment; and it defaults to taNone. It is ignored on non-window platforms</param>
    #<returns>True if there is at least one connection established; and false if there is no connection</returns>
    def Start(self, mapQueueConn, rootQueueName='', ta=0):
        self._CheckConnQueueName_(mapQueueConn)
        n = 0
        secure = False
        all = len(self._m_mapQueueConn_)
        if all > 1:
            all += 1
        if rootQueueName is None:
            rootQueueName = ''
        rootQueueName = rootQueueName.strip()
        if len(rootQueueName) == 0:
            appName = os.path.basename(__file__)
            dot = appName.rindex('.')
            if dot == -1:
                rootQueueName = appName
            else:
                rootQueueName = appName[0:dot]
        vDbFullName = []
        mcc =[[]]
        for key in self._m_mapQueueConn_.keys():
            mcc[0].append(self._m_mapQueueConn_[key])
            if not secure and mcc[0][n].EncrytionMethod == tagEncryptionMethod.TLSv1:
                secure = True
            vDbFullName.append(self._m_rs_.QueueDir + key)
            n += 1
        if all > 1:
            last = CConnectionContext('127.0.0.1', 0xffff, 'UReplication', '', tagEncryptionMethod.NoEncryption)
            if secure:
                last.EncrytionMethod = tagEncryptionMethod.TLSv1
            mcc[0].append(last)
            vDbFullName.append(self._m_rs_.QueueDir + rootQueueName)
        self._EndProcess_(vDbFullName, secure, mcc, ta)
        return self._m_pool_.ConnectedSockets > 0