import sys

class BaseServiceID(object):
    sidReserved1 = 1
    sidStartup = 256
    sidChat = 257
    sidHTTP = 258
    sidFile = 259
    sidODBC = 260
    sidReserved = 0x10000000
    sidQueue = (sidReserved + 0xEFFF0000)

class tagBaseRequestID(object):
    idUnknown = 0
    idSwitchTo = 1
    idRouteeChanged = 2
    idEncrypted = 3
    idBatchZipped = 4
    idCancel = 5
    idGetSockOptAtSvr = 6
    idSetSockOptAtSvr = 7
    idDoEcho = 8
    idTurnOnZipAtSvr = 9
    idStartBatching = 10
    idCommitBatching = 11
    idShrinkMemoryAtSvr = 12
    idSetRouting = 13
    idPing = 14
    idEnableClientDequeue = 15
    idServerException = 16
    idAllMessagesDequeued = 17
    idHttpClose = 18 #SocketPro HTTP Close
    idSetZipLevelAtSvr = 19
    idStartJob = 20
    idEndJob = 21
    idRoutingData = 22
    idDequeueConfirmed = 23
    idMessageQueued = 24
    idStartQueue = 25
    idStopQueue = 26
    idRoutePeerUnavailable = 27
    idReservedOne = 0x100
    idReservedTwo = 0x2001

class tagChatRequestID(object):
    idEnter = 65
    idSpeak = 66
    idSpeakEx = 67
    idExit = 68
    idSendUserMessage = 69
    idSendUserMessageEx = 70

class tagEncryptionMethod(object):
    NoEncryption = 0
    TLSv1 = 1

class tagOperationSystem(object):
    osWin = 0
    osApple = 1
    osUnix = 2
    osAndroid = 3
    osWinCE = 4 #< Old window pocket pc, ce or smart phone devices

class tagQueueStatus(object):

    """
    everything is fine
    """
    qsNormal = 0

    """
    Queued messages merged completely
    """
    qsMergeComplete = 1

    """
    Message replication started but not completed yet
    """
    qsMergePushing = 2

    """
    Message replicated incompletely from a source queue
    """
    qsMergeIncomplete = 3

    """
    A set of messages as a job are incompletely queued
    """
    qsJobIncomplete = 4

    """
    A message queued incompletely because of application crash or unexpected termination
    """
    qsCrash = 5

    """
    Queue file open error
    """
    qsFileError = 6

    """
    Queue file opened but can not decrypt existing queued messages because of bad password found
    """
    qsBadPassword = 7

    """
    Duplicate name error
    """
    qsDuplicateName = 8

class tagShutdownType(object):
    stReceive = 0
    stSend = 1
    stBoth = 2

class tagSocketLevel(object):
    slTcp = 6
    slSocket = 0xFFFF

class tagSocketOption(object):
    soTcpNoDelay = 1
    soReuseAddr = 4
    soKeepAlive = 8
    soSndBuf = 0x1001 # send buffer size
    soRcvBuf = 0x1002 # receive buffer size

class tagThreadApartment(object):

    """
    no COM apartment involved
    """
    taNone = 0

    """
    STA apartment
    """
    taApartment = 1

    """
    MTA (free) or neutral apartments
    """
    taFree = 2

class tagZipLevel(object):
    zlDefault = 0
    zlBestSpeed = 1
    zlBestCompression = 2

class tagOptimistic(object):
    oMemoryCached = 0
    oSystemMemoryCached = 1
    oDiskCommitted = 2

class tagVariantDataType(object):
    sdVT_EMPTY = 0
    sdVT_NULL = 1
    sdVT_I2 = 2
    sdVT_I4 = 3
    sdVT_R4 = 4
    sdVT_R8 = 5
    sdVT_CY = 6
    sdVT_BSTR = 8
    sdVT_DATE = 7
    sdVT_BOOL = 11
    sdVT_VARIANT = 12
    sdVT_DECIMAL = 14
    sdVT_I1 = 16
    sdVT_UI1 = 17
    sdVT_UI2 = 18
    sdVT_UI4 = 19
    sdVT_I8 = 20
    sdVT_UI8 = 21
    sdVT_INT = 22
    sdVT_UINT = 23
    sdVT_XML = 35
    sdVT_FILETIME = 64
    sdVT_CLSID = 72
    sdVT_BYTES = 128
    sdVT_STR = 129
    sdVT_WSTR = 130
    sdVT_USERIALIZER_OBJECT = 0xD00
    sdVT_NETObject = 0xE00
    sdVT_TIMESPAN = 0xC00
    sdVT_DATETIMEOFFSET = 0xB00
    sdVT_ARRAY = 0x2000

class BaseExceptionCode(object):
    becBAD_DESERIALIZATION = 0xAAAA0000
    becSERIALIZATION_NOT_SUPPORTED = 0xAAAA0001
    becBAD_OPERATION = 0xAAAA0002
    becBAD_INPUT = 0xAAAA0003
    becNOT_SUPPORTED = 0xAAAA0004
    becSTL_EXCEPTION = 0xAAAA0005
    becUNKNOWN_EXCEPTION = 0xAAAA0006
    becQUEUE_FILE_NOT_AVAILABLE = 0xAAAA0007
    becALREADY_DEQUEUED = 0xAAAA0008
    becROUTEE_DISCONNECTED = 0xAAAA0009

class Pair(object):
    def __init__(self, reqId, cb):
        self.first = reqId
        self.second = cb

from abc import abstractmethod, abstractproperty
class IUSerializer(object):
    @abstractmethod
    def LoadFrom(self, q):
        raise NotImplementedError("Please implement this method")

    @abstractmethod
    def SaveTo(self, q):
        raise NotImplementedError("Please implement this method")

class IMessageQueueBasic(object):
    """
    <summary>
    Stop message queue
    </summary>
    <param name="permanent">A boolean value to determine if the message queue file is permanently removed</param>
    """
    @abstractmethod
    def StopQueue(self, permanent=False):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Remove queued messages according to given message indexes
    </summary>
    <param name="startIndex">A start index</param>
    <param name="endIndex">An end index</param>
    <returns>The number of messages removed</returns>
    """
    @abstractmethod
    def CancelQueuedMessages(self, startIndex, endIndex):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Remove messages according to time-to-live
    </summary>
    <returns>The number of messages removed</returns>
    """
    @abstractmethod
    def RemoveByTTL(self):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Abort current transaction messages
    </summary>
    <returns>True if successful; and false if failed</returns>
    """
    @abstractmethod
    def AbortJob(self):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Start a message transaction
    </summary>
    <returns>True if successful; and false if failed</returns>
    """
    @abstractmethod
    def StartJob(self):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Commit a message transaction
    </summary>
    <returns>True if successful; and false if failed</returns>
    """
    @abstractmethod
    def EndJob(self):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Discard all of persistent messages
    </summary>
    """
    @abstractmethod
    def Reset(self):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Replicate all messages within this queue onto an array of queues
    </summary>
    <param name="queues">An array of target queues for appending messages from this queue</param>
    <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
    """
    @abstractmethod
    def AppendTo(self, queues):
        raise NotImplementedError("Please implement this method")

    """
    <summary>
    Ensure previous replication in case an application was crashed previously. Call this method one time only and as early as possible
    </summary>
    <param name="queues">An array of target queues for appending messages from this queue</param>
    <returns>True for success; and false for fail. To make the call success, all of target queues should be already opened and available</returns>
    """
    @abstractmethod
    def EnsureAppending(self, queues):
        raise NotImplementedError("Please implement this method")

    #<summary>
    #The number of messages during dequeuing
    #</summary>
    @abstractproperty
    def MessagesInDequeuing(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def MessageCount(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def QueueSize(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def Available(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def Secure(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def QueueFileName(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def QueueName(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #The size of messages within a transaction
    #</summary>
    @abstractproperty
    def JobSize(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #A boolean value indicating if the message queue is able to be dequeued among multiple sessions
    #</summary>
    @abstractproperty
    def DequeueShared(self):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def LastIndex(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #A status value for message queue opened
    #</summary>
    @abstractproperty
    def QueueStatus(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #A time-to-live number in seconds
    #</summary>
    @abstractproperty
    def TTL(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #Date time for queued last message
    #</summary>
    @abstractproperty
    def LastMessageTime(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #A bool value for enqueuing or dequeuing optimistically
    #If the property is set to false, queue file stream is immediately flushed. Otherwise, queue file stream may be flushed with delay.
    #Queue file stream will be flushed whenever setting the property to false.
    #</summary>
    @abstractproperty
    def Optimistic(self):
        raise NotImplementedError("Please implement this property")

    #<summary>
    #A bool value for enqueuing or dequeuing optimistically
    #If the property is set to false, queue file stream is immediately flushed. Otherwise, queue file stream may be flushed with delay.
    #Queue file stream will be flushed whenever setting the property to false.
    #</summary>
    @abstractproperty
    @Optimistic.setter
    def Optimistic(self, value):
        raise NotImplementedError("Please implement this property")

    @abstractproperty
    def Handle(self):
        raise NotImplementedError("Please implement this property")

class IPush(object):
    @abstractmethod
    def Publish(self, message, groups, hint=''):
        raise NotImplementedError("Please implement this method")

    @abstractmethod
    def SendUserMessage(self, message, userId, hint=''):
        raise NotImplementedError("Please implement this method")

    @abstractmethod
    def Subscribe(self, groups):
        raise NotImplementedError("Please implement this method")

    @abstractmethod
    def Unsubscribe(self):
        raise NotImplementedError("Please implement this method")

class IPushEx(IPush):
    @abstractmethod
    def PublishEx(self, message, groups):
        raise NotImplementedError("Please implement this method")

    @abstractmethod
    def SendUserMessageEx(self, userId, message):
        raise NotImplementedError("Please implement this method")

class classproperty(object):
    def __init__(self, getter):
        self.getter = getter
    def __get__(self, instance, owner):
        return self.getter(owner)

from spa.memqueue import CUQueue, CScopeUQueue

isVersion3 = (sys.version_info[0] >= 3)
isVersion342 = (sys.version_info[0] * 100 + sys.version_info[1] * 10 + sys.version_info[2] >= 342)
isAwaitable = (sys.version_info[0] * 100 + sys.version_info[1] * 10 + sys.version_info[2] >= 350)

from spa.dataset import CTable, CDataSet
from spa.poolbase import CMasterSlaveBase
from spa.sqlpool import CSqlMasterPool
from spa.masterpool import CMasterPool
