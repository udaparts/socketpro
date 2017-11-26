
class tagAuthenticationMethod(object):
    amOwn = 0
    amMixed = (amOwn + 1)
    amIntegrated = (amMixed + 1)
    amTrusted = (amIntegrated + 1)

class tagHttpMethod(object):
    hmUnknown = 0
    hmGet = 1
    hmPost = 2
    hmHead = 3
    hmPut = 4
    hmDelete = 5
    hmOptions = 6
    hmTrace = 7
    hmConnect = 8

class tagTransport(object):
    tUnknown = -1
    tWebSocket = 0
    tFlash = 1
    tAjax = 2
    tScript = 3

class tagTransferEncoding(object):
    teUnknown = 0
    teChunked = 1
    teCompress = 2
    teDeflate = 3
    teGZip = 4
    teIdentity = 5

class tagContentMultiplax(object):
    cmUnknown = 0
    cmMixed = 1
    cmAlternative = 2
    cmDigest = 3
    cmParallel = 4
    cmFormData = 5
    cmReport = 6
    cmSigned = 7
    cmEncrypted = 8
    cmRelated = 9
    cmByteRanges = 10

class tagRoutingAlgorithm(object):
    raDefault = 0
    raRandom = 1
    raAverage = 2

class tagHttpRequestID(object):
    idGet = 129
    idPost = 130
    idHead = 131
    idPut = 132
    idDelete = 133
    idOptions = 134
    idTrace = 135
    idConnect = 136
    idMultiPart = 137 #HTTP POST MUTIPLE PART
    idUserRequest = 138 #SocketPro HTTP User Request

from spa import *
IServerQueue = IMessageQueueBasic

from spa.serverside.baseservice import CSocketProService
from spa.serverside.serverqueue import CServerQueue
from spa.serverside.httppeer import CHttpPeerBase
from spa.serverside.clientpeer import CClientPeer
from spa.serverside.spserver import CSocketProServer
from spa.serverside.serverstreamhelper import CStreamHelper
from spa.serverside.cachebpeer import CCacheBasePeer
