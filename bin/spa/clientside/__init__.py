
from spa import *

class tagConnectionState(object):
    csClosed = 0
    csConnecting = 1
    csSslShaking = 2
    csClosing = 3
    csConnected = 4
    csSwitched = 5

class tagSocketPoolEvent(object):
    speUnknown = -1
    speStarted = 0
    speCreatingThread = 1
    speThreadCreated = 2
    speConnecting = 3
    speConnected = 4
    speKillingThread = 5
    speShutdown = 6
    speUSocketCreated = 7
    speHandShakeCompleted = 8
    speLocked = 9
    speUnlocked = 10
    speThreadKilled = 11
    speClosingSocket = 12
    speSocketClosed = 13
    speUSocketKilled = 14
    speTimer = 15
    speQueueMergedFrom = 16
    speQueueMergedTo = 17

class IClientQueue(IMessageQueueBasic):

    """
    <summary>
    Open a persistent file for a message queue
    </summary>
    <param name="qName">Message queue name or a full path to message queue file</param>
    <param name="ttl">Time-to-live in seconds</param>
    <param name="secure">A boolean value default to true to indicate if queued messages should be encrypted by password</param>
    <param name="dequeueShared">A boolean value default to false to indicate if there are two or more sessions to dequeue messages</param>
    <returns>True if successful and false if failed</returns>
    <remarks>To reopen an existing secure message queue file, the method may fail if current password is different from original one. There are a number of situations leading the failures of this method</remarks>
    """
    @abstractmethod
    def StartQueue(self, qName, ttl, secure=True, dequeueShared=False):
        raise NotImplementedError("Please implement this method")

    #<summary>
    #A property indicating if dequeuing message queue is enabled
    #</summary>
    @abstractproperty
    def DequeueEnabled(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #A bool value for enabling or disabling routing queue index. The property defaults to false.
    #If there is only one worker application instance running (Note that one instance may have multiple socket connections), it ensures once-only delivery if this property is set to true.
    #If there are multiple worker application instances running, you should not set this property to true! Otherwise, SocketPro may function incorrectly in dequeuing messages.
    #</summary>
    @abstractproperty
    def RoutingQueueIndex(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    @RoutingQueueIndex.setter
    def RoutingQueueIndex(self, value):
        raise NotImplementedError("Please implement this property")

from spa.clientside.messagesender import CMessageSender
from spa.clientside.clientsocket import CClientSocket
from spa.clientside.conncontext import CConnectionContext
from spa.clientside.asynchandler import CAsyncResult, CAsyncServiceHandler
from spa.clientside.socketpool import CSocketPool
from spa.clientside.replication import ReplicationSetting, CReplication
from spa.clientside.clientstreamhelper import CStreamingFile
from spa.clientside.sqlite import CSqlite
from spa.clientside.mysql import CMysql, CSqlServer
from spa.clientside.odbc import COdbc
from spa.clientside.asyncqueue import CAsyncQueue
from spa.clientside.ufuture import UFuture
from spa.clientside.asyncdbhandler import CAsyncDBHandler
from spa.clientside.cachedhandler import CCachedBaseHandler
from spa.clientside.spmanager import SpManager
