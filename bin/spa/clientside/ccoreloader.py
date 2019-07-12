
from ctypes import *
from sys import platform as os
from spa.clientside.certinfo import PCertInfo
from spa.clientside.messagesender import CMessageSender, USocket_Client_Handle

class CCoreLoader(object):
    _ucsLib_ = None
    _IsWin_ = (os == "win32")
    if _IsWin_:
        _ucsLib_ = WinDLL("usocket.dll")
    else:
        _ucsLib_ = CDLL("libusocket.so")

    #typedef void (CALLBACK *POnSocketClosed) (USocket_Client_Handle handler, int nError);
    if _IsWin_:
        POnSocketClosed = WINFUNCTYPE(None, USocket_Client_Handle, c_int)
    else:
        POnSocketClosed = CFUNCTYPE(None, USocket_Client_Handle, c_int)

    #typedef void (CALLBACK *POnHandShakeCompleted) (USocket_Client_Handle handler, int nError);
    if _IsWin_:
        POnHandShakeCompleted = WINFUNCTYPE(None, USocket_Client_Handle, c_int)
    else:
        POnHandShakeCompleted = CFUNCTYPE(None, USocket_Client_Handle, c_int)

    #typedef void (CALLBACK *POnSocketConnected) (USocket_Client_Handle handler, int nError);
    if _IsWin_:
        POnSocketConnected = WINFUNCTYPE(None, USocket_Client_Handle, c_int)
    else:
        POnSocketConnected = CFUNCTYPE(None, USocket_Client_Handle, c_int)

    #typedef void (CALLBACK *POnRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
    if _IsWin_:
        POnRequestProcessed = WINFUNCTYPE(None, USocket_Client_Handle, c_ushort, c_uint)
    else:
        POnRequestProcessed = CFUNCTYPE(None, USocket_Client_Handle, c_ushort, c_uint)

    #typedef void (CALLBACK *POnBaseRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId);
    if _IsWin_:
        POnBaseRequestProcessed = WINFUNCTYPE(None, USocket_Client_Handle, c_ushort)
    else:
        POnBaseRequestProcessed = CFUNCTYPE(None, USocket_Client_Handle, c_ushort)

    #typedef void (CALLBACK *POnEnter2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
    if _IsWin_:
        POnEnter = WINFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint)
    else:
        POnEnter = CFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint)

    #typedef void (CALLBACK *POnExit2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
    if _IsWin_:
        POnExit = WINFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint)
    else:
        POnExit = CFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint)

    #typedef void (CALLBACK *POnSpeakEx2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
    if _IsWin_:
        POnSpeakEx = WINFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint, POINTER(c_ubyte), c_uint)
    else:
        POnSpeakEx = CFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint, POINTER(c_ubyte), c_uint)

    #typedef void (CALLBACK *POnSpeak2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
    if _IsWin_:
        POnSpeak = WINFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint, POINTER(c_ubyte), c_uint)
    else:
        POnSpeak = CFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_uint), c_uint, POINTER(c_ubyte), c_uint)

    #typedef void (CALLBACK *POnSendUserMessage2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned char *message, unsigned int size);
    if _IsWin_:
        POnSendUserMessage = WINFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_ubyte), c_uint)
    else:
        POnSendUserMessage = CFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_ubyte), c_uint)

    #typedef void (CALLBACK *POnSendUserMessageEx2) (USocket_Client_Handle handler, SPA::ClientSide::CMessageSender *sender, const unsigned char *pMessage, unsigned int size);
    if _IsWin_:
        POnSendUserMessageEx = WINFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_ubyte), c_uint)
    else:
        POnSendUserMessageEx = CFUNCTYPE(None, USocket_Client_Handle, POINTER(CMessageSender), POINTER(c_ubyte), c_uint)

    #typedef void (CALLBACK *POnServerException) (USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
    if _IsWin_:
        POnServerException = WINFUNCTYPE(None, USocket_Client_Handle, c_ushort, c_wchar_p, c_char_p, c_uint)
    else:
        POnServerException = CFUNCTYPE(None, USocket_Client_Handle, c_ushort, c_wchar_p, c_char_p, c_uint)

    #typedef void (CALLBACK *POnAllRequestsProcessed) (USocket_Client_Handle handler, unsigned short lastRequestId);
    if _IsWin_:
        POnAllRequestsProcessed = WINFUNCTYPE(None, USocket_Client_Handle, c_ushort)
    else:
        POnAllRequestsProcessed = CFUNCTYPE(None, USocket_Client_Handle, c_ushort)

    # typedef void (CALLBACK *POnPostProcessing) (USocket_Client_Handle handler, unsigned int hint, SPA::UINT64 data);
    if _IsWin_:
        POnPostProcessing = WINFUNCTYPE(None, USocket_Client_Handle, c_uint, c_uint64)
    else:
        POnPostProcessing = CFUNCTYPE(None, USocket_Client_Handle, c_uint, c_uint64)

    """
    enum tagSocketPoolEvent {
        speUnknown = -1,
        speStarted = 0,
        speCreatingThread = 1,
        speThreadCreated = 2,
        speConnecting = 3,
        speConnected = 4,
        speKillingThread = 5,
        speShutdown = 6,
        speUSocketCreated = 7,
        speHandShakeCompleted = 8,
        speLocked = 9,
        speUnlocked = 10,
        speThreadKilled = 11,
        speClosingSocket = 12,
        speSocketClosed = 13,
        speUSocketKilled = 14,
        speTimer = 15
    };
    """
    #typedef void(CALLBACK *PSocketPoolCallback) (unsigned int, SPA::ClientSide::tagSocketPoolEvent, USocket_Client_Handle);
    if _IsWin_:
        PSocketPoolCallback = WINFUNCTYPE(None, c_uint, c_int, USocket_Client_Handle)
    else:
        PSocketPoolCallback = CFUNCTYPE(None, c_uint, c_int, USocket_Client_Handle)

    #typedef bool(CALLBACK *PCertificateVerifyCallback) (bool, int, int, const char *errMsg, SPA::ClientSide::CertInfo*);
    if _IsWin_:
        PCertificateVerifyCallback = WINFUNCTYPE(c_bool, c_bool, c_int, c_int, c_char_p, PCertInfo)
    else:
        PCertificateVerifyCallback = CFUNCTYPE(c_bool, c_bool, c_int, c_int, c_char_p, PCertInfo)

    #unsigned int WINAPI CreateSocketPool(PSocketPoolCallback spc, unsigned int maxSocketsPerThread, unsigned int maxThreads = 0, bool bAvg = true, SPA::tagThreadApartment ta = SPA::taNone);
    CreateSocketPool = _ucsLib_.CreateSocketPool
    CreateSocketPool.argtypes = [PSocketPoolCallback, c_uint, c_uint, c_bool, c_int]
    CreateSocketPool.restype = c_uint

    #bool WINAPI DestroySocketPool(unsigned int poolId);
    DestroySocketPool = _ucsLib_.DestroySocketPool
    DestroySocketPool.argtypes = [c_uint]
    DestroySocketPool.restype = c_bool

    #unsigned int WINAPI GetNumberOfSocketPools();
    GetNumberOfSocketPools = _ucsLib_.GetNumberOfSocketPools
    GetNumberOfSocketPools.argtypes = []
    GetNumberOfSocketPools.restype = c_uint

    #USocket_Client_Handle WINAPI FindAClosedSocket(unsigned int poolId);
    FindAClosedSocket = _ucsLib_.FindAClosedSocket
    FindAClosedSocket.argtypes = [c_uint]
    FindAClosedSocket.restype = c_uint64

    #bool WINAPI AddOneThreadIntoPool(unsigned int poolId);
    AddOneThreadIntoPool = _ucsLib_.AddOneThreadIntoPool
    AddOneThreadIntoPool.argtypes = [c_uint]
    AddOneThreadIntoPool.restype = c_bool

    #unsigned int WINAPI GetLockedSockets(unsigned int poolId);
    GetLockedSockets = _ucsLib_.GetLockedSockets
    GetLockedSockets.argtypes = [c_uint]
    GetLockedSockets.restype = c_uint

    #unsigned int WINAPI GetIdleSockets(unsigned int poolId);
    GetIdleSockets = _ucsLib_.GetIdleSockets
    GetIdleSockets.argtypes = [c_uint]
    GetIdleSockets.restype = c_uint

    #unsigned int WINAPI GetConnectedSockets(unsigned int poolId);
    GetConnectedSockets = _ucsLib_.GetConnectedSockets
    GetConnectedSockets.argtypes = [c_uint]
    GetConnectedSockets.restype = c_uint

    #bool WINAPI DisconnectAll(unsigned int poolId);
    DisconnectAll = _ucsLib_.DisconnectAll
    DisconnectAll.argtypes = [c_uint]
    DisconnectAll.restype = c_bool

    #USocket_Client_Handle WINAPI LockASocket(unsigned int poolId, unsigned int timeout, USocket_Client_Handle hSameThread = NULL);
    LockASocket = _ucsLib_.LockASocket
    LockASocket.argtypes = [c_uint, c_uint, USocket_Client_Handle]
    LockASocket.restype = USocket_Client_Handle

    #bool WINAPI UnlockASocket(unsigned int poolId, USocket_Client_Handle h);
    UnlockASocket = _ucsLib_.UnlockASocket
    UnlockASocket.argtypes = [c_uint, USocket_Client_Handle]
    UnlockASocket.restype = c_bool

    #unsigned int WINAPI GetSocketsPerThread(unsigned int poolId);
    GetSocketsPerThread = _ucsLib_.GetSocketsPerThread
    GetSocketsPerThread.argtypes = [c_uint]
    GetSocketsPerThread.restype = c_uint

    #bool WINAPI IsAvg(unsigned int poolId);
    IsAvg = _ucsLib_.IsAvg
    IsAvg.argtypes = [c_uint]
    IsAvg.restype = c_bool

    #unsigned int WINAPI GetDisconnectedSockets(unsigned int poolId);
    GetDisconnectedSockets = _ucsLib_.GetDisconnectedSockets
    GetDisconnectedSockets.argtypes = [c_uint]
    GetDisconnectedSockets.restype = c_uint

    #unsigned int WINAPI GetThreadCount(unsigned int poolId);
    GetThreadCount = _ucsLib_.GetThreadCount
    GetThreadCount.argtypes = [c_uint]
    GetThreadCount.restype = c_uint

    #void WINAPI Close(USocket_Client_Handle h);
    Close = _ucsLib_.Close
    Close.argtypes = [USocket_Client_Handle]
    Close.restype = None

    #bool WINAPI Connect(USocket_Client_Handle h, const char* host, unsigned int portNumber, bool sync = false, bool v6 = false);
    Connect = _ucsLib_.Connect
    Connect.argtypes = [USocket_Client_Handle, c_char_p, c_uint, c_bool, c_bool]
    Connect.restype = c_bool

    #unsigned int WINAPI GetCountOfRequestsQueued(USocket_Client_Handle h);
    GetCountOfRequestsQueued = _ucsLib_.GetCountOfRequestsQueued
    GetCountOfRequestsQueued.argtypes = [USocket_Client_Handle]
    GetCountOfRequestsQueued.restype = c_uint

    #unsigned short WINAPI GetCurrentRequestID(USocket_Client_Handle h);
    GetCurrentRequestID = _ucsLib_.GetCurrentRequestID
    GetCurrentRequestID.argtypes = [USocket_Client_Handle]
    GetCurrentRequestID.restype = c_ushort

    #unsigned int WINAPI GetCurrentResultSize(USocket_Client_Handle h);
    GetCurrentResultSize = _ucsLib_.GetCurrentResultSize
    GetCurrentResultSize.argtypes = [USocket_Client_Handle]
    GetCurrentResultSize.restype = c_uint

    #int WINAPI GetErrorCode(USocket_Client_Handle h);
    GetErrorCode = _ucsLib_.GetErrorCode
    GetErrorCode.argtypes = [USocket_Client_Handle]
    GetErrorCode.restype = c_int

    #unsigned int WINAPI GetErrorMessage(USocket_Client_Handle h, char *str, unsigned int bufferLen);
    GetErrorMessage = _ucsLib_.GetErrorMessage
    GetErrorMessage.argtypes = [USocket_Client_Handle, POINTER(c_char), c_uint]
    GetErrorMessage.restype = c_uint

    #unsigned int WINAPI GetSocketPoolId(USocket_Client_Handle h);
    GetSocketPoolId = _ucsLib_.GetSocketPoolId
    GetSocketPoolId.argtypes = [USocket_Client_Handle, c_char_p, c_uint]
    GetSocketPoolId.restype = c_uint

    #bool WINAPI IsOpened(USocket_Client_Handle h);
    IsOpened = _ucsLib_.IsOpened
    IsOpened.argtypes = [USocket_Client_Handle]
    IsOpened.restype = c_bool

    #unsigned int WINAPI RetrieveResult(USocket_Client_Handle h, unsigned char *pBuffer, unsigned int size);
    RetrieveResult = _ucsLib_.RetrieveResult
    RetrieveResult.argtypes = [USocket_Client_Handle, POINTER(c_char), c_int]
    RetrieveResult.restype = c_uint

    #bool WINAPI SendRequest(USocket_Client_Handle h, unsigned short reqId, const unsigned char *pBuffer, unsigned int len);
    SendRequest = _ucsLib_.SendRequest
    SendRequest.argtypes = [USocket_Client_Handle, c_ushort, POINTER(c_ubyte), c_uint]
    SendRequest.restype = c_bool

    #void WINAPI SetOnHandShakeCompleted(USocket_Client_Handle h, POnHandShakeCompleted p);
    SetOnHandShakeCompleted = _ucsLib_.SetOnHandShakeCompleted
    SetOnHandShakeCompleted.argtypes = [USocket_Client_Handle, POnHandShakeCompleted]
    SetOnHandShakeCompleted.restype = None

    #void WINAPI SetOnRequestProcessed(USocket_Client_Handle h, POnRequestProcessed p);
    SetOnRequestProcessed = _ucsLib_.SetOnRequestProcessed
    SetOnRequestProcessed.argtypes = [USocket_Client_Handle, POnRequestProcessed]
    SetOnRequestProcessed.restype = None

    #void WINAPI SetOnSocketClosed(USocket_Client_Handle h, POnSocketClosed p);
    SetOnSocketClosed = _ucsLib_.SetOnSocketClosed
    SetOnSocketClosed.argtypes = [USocket_Client_Handle, POnSocketClosed]
    SetOnSocketClosed.restype = None

    #void WINAPI SetOnSocketConnected(USocket_Client_Handle h, POnSocketConnected p);
    SetOnSocketConnected = _ucsLib_.SetOnSocketConnected
    SetOnSocketConnected.argtypes = [USocket_Client_Handle, POnSocketConnected]
    SetOnSocketConnected.restype = None

    #void WINAPI SetOnBaseRequestProcessed(USocket_Client_Handle h, POnBaseRequestProcessed p);
    SetOnBaseRequestProcessed = _ucsLib_.SetOnBaseRequestProcessed
    SetOnBaseRequestProcessed.argtypes = [USocket_Client_Handle, POnBaseRequestProcessed]
    SetOnBaseRequestProcessed.restype = None

    #void WINAPI SetOnAllRequestsProcessed(USocket_Client_Handle h, POnAllRequestsProcessed p);
    SetOnAllRequestsProcessed = _ucsLib_.SetOnAllRequestsProcessed
    SetOnAllRequestsProcessed.argtypes = [USocket_Client_Handle, POnAllRequestsProcessed]
    SetOnAllRequestsProcessed.restype = None

    #bool WINAPI WaitAll(USocket_Client_Handle h, unsigned int nTimeout = (~0));
    WaitAll = _ucsLib_.WaitAll
    WaitAll.argtypes = [USocket_Client_Handle, c_uint]
    WaitAll.restype = c_bool

    #bool WINAPI Cancel(USocket_Client_Handle h, unsigned int requestsQueued = (~0));
    Cancel = _ucsLib_.Cancel
    Cancel.argtypes = [USocket_Client_Handle, c_uint]
    Cancel.restype = c_bool

    #bool WINAPI IsRandom(USocket_Client_Handle h);
    IsRandom = _ucsLib_.IsRandom
    IsRandom.argtypes = [USocket_Client_Handle]
    IsRandom.restype = c_bool

    #unsigned int WINAPI GetBytesInSendingBuffer(USocket_Client_Handle h);
    GetBytesInSendingBuffer = _ucsLib_.GetBytesInSendingBuffer
    GetBytesInSendingBuffer.argtypes = [USocket_Client_Handle]
    GetBytesInSendingBuffer.restype = c_uint

    #unsigned int WINAPI GetBytesInReceivingBuffer(USocket_Client_Handle h);
    GetBytesInReceivingBuffer = _ucsLib_.GetBytesInReceivingBuffer
    GetBytesInReceivingBuffer.argtypes = [USocket_Client_Handle]
    GetBytesInReceivingBuffer.restype = c_uint

    #bool WINAPI IsBatching(USocket_Client_Handle h);
    IsBatching = _ucsLib_.IsBatching
    IsBatching.argtypes = [USocket_Client_Handle]
    IsBatching.restype = c_bool

    #unsigned int WINAPI GetBytesBatched(USocket_Client_Handle h);
    GetBytesBatched = _ucsLib_.GetBytesBatched
    GetBytesBatched.argtypes = [USocket_Client_Handle]
    GetBytesBatched.restype = c_uint

    #bool WINAPI StartBatching(USocket_Client_Handle h);
    StartBatching = _ucsLib_.StartBatching
    StartBatching.argtypes = [USocket_Client_Handle]
    StartBatching.restype = c_bool

    #bool WINAPI CommitBatching(USocket_Client_Handle h, bool batchingAtServerSide);
    CommitBatching = _ucsLib_.CommitBatching
    CommitBatching.argtypes = [USocket_Client_Handle, c_bool]
    CommitBatching.restype = c_bool

    #bool WINAPI AbortBatching(USocket_Client_Handle h);
    AbortBatching = _ucsLib_.AbortBatching
    AbortBatching.argtypes = [USocket_Client_Handle]
    AbortBatching.restype = c_bool

    #SPA::UINT64 WINAPI GetBytesReceived(USocket_Client_Handle h);
    GetBytesReceived = _ucsLib_.GetBytesReceived
    GetBytesReceived.argtypes = [USocket_Client_Handle]
    GetBytesReceived.restype = c_uint64

    #SPA::UINT64 WINAPI GetBytesSent(USocket_Client_Handle h);
    GetBytesSent = _ucsLib_.GetBytesSent
    GetBytesSent.argtypes = [USocket_Client_Handle]
    GetBytesSent.restype = c_uint64

    #void WINAPI SetUserID(USocket_Client_Handle h, const wchar_t *userId);
    SetUserID = _ucsLib_.SetUserID
    SetUserID.argtypes = [USocket_Client_Handle, c_wchar_p]
    SetUserID.restype = None

    #unsigned int WINAPI GetUID(USocket_Client_Handle h, wchar_t *userId, unsigned int chars);
    GetUID = _ucsLib_.GetUID
    GetUID.argtypes = [USocket_Client_Handle, POINTER(c_wchar), c_uint]
    GetUID.restype = c_uint

    #void WINAPI SetPassword(USocket_Client_Handle h, const wchar_t *password);
    SetPassword = _ucsLib_.SetPassword
    SetPassword.argtypes = [USocket_Client_Handle, c_wchar_p]
    SetPassword.restype = None

    #bool WINAPI SwitchTo(USocket_Client_Handle h, unsigned int serviceId);
    SwitchTo = _ucsLib_.SwitchTo
    SwitchTo.argtypes = [USocket_Client_Handle, c_uint]
    SwitchTo.restype = c_bool

    #bool WINAPI Enter(USocket_Client_Handle h, const unsigned int *pChatGroupId, unsigned int count);
    Enter = _ucsLib_.Enter
    Enter.argtypes = [USocket_Client_Handle, POINTER(c_uint), c_uint]
    Enter.restype = c_bool

    #void WINAPI Exit(USocket_Client_Handle h);
    Exit = _ucsLib_.Exit
    Exit.argtypes = [USocket_Client_Handle]
    Exit.restype = None

    #bool WINAPI Speak(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
    Speak = _ucsLib_.Speak
    Speak.argtypes = [USocket_Client_Handle, POINTER(c_ubyte), c_uint, POINTER(c_uint), c_uint]
    Speak.restype = c_bool

    #bool WINAPI SpeakEx(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
    SpeakEx = _ucsLib_.SpeakEx
    SpeakEx.argtypes = [USocket_Client_Handle, POINTER(c_ubyte), c_uint, POINTER(c_uint), c_uint]
    SpeakEx.restype = c_bool

    #bool WINAPI SendUserMessage(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    SendUserMessage = _ucsLib_.SendUserMessage
    SendUserMessage.argtypes = [USocket_Client_Handle, c_wchar_p, POINTER(c_ubyte), c_uint]
    SendUserMessage.restype = c_bool

    #bool WINAPI SendUserMessageEx(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    SendUserMessageEx = _ucsLib_.SendUserMessageEx
    SendUserMessageEx.argtypes = [USocket_Client_Handle, c_wchar_p, POINTER(c_ubyte), c_uint]
    SendUserMessageEx.restype = c_bool

    #SPA::UINT64 WINAPI GetSocketNativeHandle(USocket_Client_Handle h);
    GetSocketNativeHandle = _ucsLib_.GetSocketNativeHandle
    GetSocketNativeHandle.argtypes = [USocket_Client_Handle]
    GetSocketNativeHandle.restype = c_int64

    #void WINAPI SetOnEnter2(USocket_Client_Handle h, POnEnter2 p);
    SetOnEnter = _ucsLib_.SetOnEnter2
    SetOnEnter.argtypes = [USocket_Client_Handle, POnEnter]
    SetOnEnter.restype = None

    #void WINAPI SetOnExit2(USocket_Client_Handle h, POnExit2 p);
    SetOnExit = _ucsLib_.SetOnExit2
    SetOnExit.argtypes = [USocket_Client_Handle, POnExit]
    SetOnExit.restype = None

    #void WINAPI SetOnSpeakEx2(USocket_Client_Handle h, POnSpeakEx2 p);
    SetOnSpeakEx = _ucsLib_.SetOnSpeakEx2
    SetOnSpeakEx.argtypes = [USocket_Client_Handle, POnSpeakEx]
    SetOnSpeakEx.restype = None

    #void WINAPI SetOnSendUserMessageEx2(USocket_Client_Handle h, POnSendUserMessageEx2 p);
    SetOnSendUserMessageEx = _ucsLib_.SetOnSendUserMessageEx2
    SetOnSendUserMessageEx.argtypes = [USocket_Client_Handle, POnSendUserMessageEx]
    SetOnSendUserMessageEx.restype = None

    """
    /**
     * Defines for supported operation systems
     */
    enum tagOperationSystem {
        osWin = 0,
        osApple = 1,
        osMac = osApple,
        osUnix = 2,
        osLinux = osUnix,
        osBSD = osUnix,
        osAndroid = 3,
        osWinCE = 4, /**< Old window pocket pc, ce or smart phone devices*/
        osWinPhone = osWinCE,
    };
    """
    #SPA::tagOperationSystem WINAPI GetPeerOs(USocket_Client_Handle h, bool *endian);
    GetPeerOs = _ucsLib_.GetPeerOs
    GetPeerOs.argtypes = [USocket_Client_Handle, POINTER(c_bool)]
    GetPeerOs.restype = c_int

    #void WINAPI SetOnServerException(USocket_Client_Handle h, POnServerException p);
    SetOnServerException = _ucsLib_.SetOnServerException
    SetOnServerException.argtypes = [USocket_Client_Handle, POnServerException]
    SetOnServerException.restype = None

    #void WINAPI SetOnSendUserMessage(USocket_Client_Handle h, POnSendUserMessage2 p);
    SetOnSendUserMessage = _ucsLib_.SetOnSendUserMessage2
    SetOnSendUserMessage.argtypes = [USocket_Client_Handle, POnSendUserMessage]
    SetOnSendUserMessage.retype = None

    #void WINAPI SetOnSpeak(USocket_Client_Handle h, POnSpeak2 p);
    SetOnSpeak = _ucsLib_.SetOnSpeak2
    SetOnSpeak.argtypes = [USocket_Client_Handle, POnSpeak]
    SetOnSpeak.retype = None

    #void WINAPI SetZip(USocket_Client_Handle h, bool zip);
    SetZip = _ucsLib_.SetZip
    SetZip.argtypes = [USocket_Client_Handle, c_bool]
    SetZip.restype = None

    #bool WINAPI GetZip(USocket_Client_Handle h);
    GetZip = _ucsLib_.GetZip
    GetZip.argtypes = [USocket_Client_Handle]
    GetZip.restype = c_bool

    """
    /**
     * Defines for supported compression options
     */
    typedef enum tagZipLevel {
        zlDefault = 0,
        zlBestSpeed = 1,
        zlBestCompression = 2
    } ZipLevel;
    """
    #SPA::tagZipLevel WINAPI GetZipLevel(USocket_Client_Handle h);
    GetZipLevel = _ucsLib_.GetZipLevel
    GetZipLevel.argtypes = [USocket_Client_Handle]
    GetZipLevel.restype = c_int

    #void WINAPI SetZipLevel(USocket_Client_Handle h, SPA::tagZipLevel zl);
    SetZipLevel = _ucsLib_.SetZipLevel
    SetZipLevel.argtypes = [USocket_Client_Handle, c_int]
    SetZipLevel.restype = None

    #bool WINAPI SetZipLevelAtSvr(USocket_Client_Handle h, SPA::tagZipLevel zipLevel);
    SetZipLevelAtSvr = _ucsLib_.SetZipLevelAtSvr
    SetZipLevelAtSvr.argtypes = [USocket_Client_Handle, c_int]
    SetZipLevelAtSvr.restype = c_bool

    #unsigned int WINAPI GetCurrentServiceId(USocket_Client_Handle h);
    GetCurrentServiceId = _ucsLib_.GetCurrentServiceId
    GetCurrentServiceId.argtypes = [USocket_Client_Handle]
    GetCurrentServiceId.restype = c_uint

    #bool WINAPI StartQueue(USocket_Client_Handle h, const char *qName, bool secure, bool dequeueShared, unsigned int ttl);
    StartQueue = _ucsLib_.StartQueue
    StartQueue.argtypes = [USocket_Client_Handle, c_char_p, c_bool, c_bool, c_uint]
    StartQueue.restype = c_bool

    #void WINAPI StopQueue(USocket_Client_Handle h, bool permanent);
    StopQueue = _ucsLib_.StopQueue
    StopQueue.argtypes = [USocket_Client_Handle, c_bool]
    StopQueue.restype = None

    #bool WINAPI DequeuedResult(USocket_Client_Handle h);
    DequeuedResult = _ucsLib_.DequeuedResult
    DequeuedResult.argtypes = [USocket_Client_Handle]
    DequeuedResult.restype = c_bool

    #unsigned int WINAPI GetMessagesInDequeuing(USocket_Client_Handle h);
    GetMessagesInDequeuing = _ucsLib_.GetMessagesInDequeuing
    GetMessagesInDequeuing.argtypes = [USocket_Client_Handle]
    GetMessagesInDequeuing.restype = c_uint

    #SPA::UINT64 WINAPI GetMessageCount(USocket_Client_Handle h);
    GetMessageCount = _ucsLib_.GetMessageCount
    GetMessageCount.argtypes = [USocket_Client_Handle]
    GetMessageCount.restype = c_uint64

    #SPA::UINT64 WINAPI GetQueueSize(USocket_Client_Handle h);
    GetQueueSize = _ucsLib_.GetQueueSize
    GetQueueSize.argtypes = [USocket_Client_Handle]
    GetQueueSize.restype = c_uint64

    #bool WINAPI IsQueueSecured(USocket_Client_Handle h);
    IsQueueSecured = _ucsLib_.IsQueueSecured
    IsQueueSecured.argtypes = [USocket_Client_Handle]
    IsQueueSecured.restype = c_bool

    #bool WINAPI IsQueueStarted(USocket_Client_Handle h);
    IsQueueStarted = _ucsLib_.IsQueueStarted
    IsQueueStarted.argtypes = [USocket_Client_Handle]
    IsQueueStarted.restype = c_bool

    #const char* WINAPI GetQueueName(USocket_Client_Handle h);
    GetQueueName = _ucsLib_.GetQueueName
    GetQueueName.argtypes = [USocket_Client_Handle]
    GetQueueName.restype = c_char_p

    #const char* WINAPI GetQueueFileName(USocket_Client_Handle h);
    GetQueueFileName = _ucsLib_.GetQueueFileName
    GetQueueFileName.argtypes = [USocket_Client_Handle]
    GetQueueFileName.restype = c_char_p

    #bool WINAPI DoEcho(USocket_Client_Handle h);
    DoEcho = _ucsLib_.DoEcho
    DoEcho.argtypes = [USocket_Client_Handle]
    DoEcho.restype = c_bool

    #bool WINAPI TurnOnZipAtSvr(USocket_Client_Handle h, bool enableZip);
    TurnOnZipAtSvr = _ucsLib_.TurnOnZipAtSvr
    TurnOnZipAtSvr.argtypes = [USocket_Client_Handle, c_bool]
    TurnOnZipAtSvr.restype = c_bool

    #bool WINAPI GetPeerName(USocket_Client_Handle h, unsigned int *peerPort, char *ipAddr, unsigned short chars);
    GetPeerName = _ucsLib_.GetPeerName
    GetPeerName.argtypes = [USocket_Client_Handle, POINTER(c_uint), POINTER(c_char), c_uint]
    GetPeerName.restype = c_bool

    #void WINAPI SetRecvTimeout(USocket_Client_Handle h, unsigned int timeout);
    SetRecvTimeout = _ucsLib_.SetRecvTimeout
    SetRecvTimeout.argtypes = [USocket_Client_Handle, c_uint]
    SetRecvTimeout.restype = None

    #unsigned int WINAPI GetRecvTimeout(USocket_Client_Handle h);
    GetRecvTimeout = _ucsLib_.GetRecvTimeout
    GetRecvTimeout.argtypes = [USocket_Client_Handle]
    GetRecvTimeout.restype = c_uint

    #void WINAPI SetConnTimeout(USocket_Client_Handle h, unsigned int timeout);
    SetConnTimeout = _ucsLib_.SetConnTimeout
    SetConnTimeout.argtypes = [USocket_Client_Handle, c_uint]
    SetConnTimeout.restype = None

    #unsigned int WINAPI GetConnTimeout(USocket_Client_Handle h);
    GetConnTimeout = _ucsLib_.GetConnTimeout
    GetConnTimeout.argtypes = [USocket_Client_Handle]
    GetConnTimeout.restype = c_uint

    #void WINAPI SetAutoConn(USocket_Client_Handle h, bool autoConnecting);
    SetAutoConn = _ucsLib_.SetAutoConn
    SetAutoConn.argtypes = [USocket_Client_Handle, c_bool]
    SetAutoConn.restype = None

    #bool WINAPI GetAutoConn(USocket_Client_Handle h);
    GetAutoConn = _ucsLib_.GetAutoConn
    GetAutoConn.argtypes = [USocket_Client_Handle]
    GetAutoConn.restype = c_bool

    #unsigned short WINAPI GetServerPingTime(USocket_Client_Handle h);
    GetServerPingTime = _ucsLib_.GetServerPingTime
    GetServerPingTime.argtypes = [USocket_Client_Handle]
    GetServerPingTime.restype = c_ushort

    #SPA::ClientSide::CertInfo* WINAPI GetUCert(USocket_Client_Handle h);
    GetUCert = _ucsLib_.GetUCert
    GetUCert.argtypes = [USocket_Client_Handle]
    GetUCert.restype = PCertInfo

    #SPA::ClientSide::IUcert* WINAPI GetUCertEx(USocket_Client_Handle h);

    #void* WINAPI GetSSL(USocket_Client_Handle h);
    GetSSL = _ucsLib_.GetSSL
    GetSSL.argtypes = [USocket_Client_Handle]
    GetSSL.restype = c_void_p

    #bool WINAPI IgnoreLastRequest(USocket_Client_Handle h, unsigned short reqId);
    IgnoreLastRequest = _ucsLib_.IgnoreLastRequest
    IgnoreLastRequest.argtypes = [USocket_Client_Handle, c_ushort]
    IgnoreLastRequest.restype = c_bool

    #bool WINAPI SetVerifyLocation(const char *certFile);
    SetVerifyLocation = _ucsLib_.SetVerifyLocation
    SetVerifyLocation.argtypes = [c_char_p]
    SetVerifyLocation.restype = c_bool

    #const char* WINAPI Verify(USocket_Client_Handle h, int *errCode);
    Verify = _ucsLib_.Verify
    Verify.argtypes = [USocket_Client_Handle, POINTER(c_int)]
    Verify.restype = c_char_p

    #bool WINAPI IsDequeueEnabled(USocket_Client_Handle h);
    IsDequeueEnabled = _ucsLib_.IsDequeueEnabled
    IsDequeueEnabled.argtypes = [USocket_Client_Handle]
    IsDequeueEnabled.restype = c_bool

    #bool WINAPI AbortJob(USocket_Client_Handle h);
    AbortJob = _ucsLib_.AbortJob
    AbortJob.argtypes = [USocket_Client_Handle]
    AbortJob.restype = c_bool

    #bool WINAPI StartJob(USocket_Client_Handle h);
    StartJob = _ucsLib_.StartJob
    StartJob.argtypes = [USocket_Client_Handle]
    StartJob.restype = c_bool

    #bool WINAPI EndJob(USocket_Client_Handle h);
    EndJob = _ucsLib_.EndJob
    EndJob.argtypes = [USocket_Client_Handle]
    EndJob.restype = c_bool

    #SPA::UINT64 WINAPI GetJobSize(USocket_Client_Handle h);
    GetJobSize = _ucsLib_.GetJobSize
    GetJobSize.argtypes = [USocket_Client_Handle]
    GetJobSize.restype = c_uint64

    #bool WINAPI IsRouteeRequest(USocket_Client_Handle h);
    IsRouteeRequest = _ucsLib_.IsRouteeRequest
    IsRouteeRequest.argtypes = [USocket_Client_Handle]
    IsRouteeRequest.restype = c_bool

    #bool WINAPI SendRouteeResult(USocket_Client_Handle h, unsigned short reqId, const unsigned char *buffer, unsigned int len);
    SendRouteeResult = _ucsLib_.SendRouteeResult
    SendRouteeResult.argtypes = [USocket_Client_Handle, c_ushort, POINTER(c_ubyte), c_uint]
    SendRouteeResult.restype = c_bool

    #unsigned int WINAPI GetRouteeCount(USocket_Client_Handle h);
    GetRouteeCount = _ucsLib_.GetRouteeCount
    GetRouteeCount.argtypes = [USocket_Client_Handle]
    GetRouteeCount.restype = c_uint

    #void WINAPI UseUTF16();
    UseUTF16 = _ucsLib_.UseUTF16
    UseUTF16.argtypes = []
    UseUTF16.restype = None

    #SPA::UINT64 WINAPI GetQueueLastIndex(USocket_Client_Handle h);
    GetQueueLastIndex = _ucsLib_.GetQueueLastIndex
    GetQueueLastIndex.argtypes = [USocket_Client_Handle]
    GetQueueLastIndex.restype = c_uint64

    #SPA::UINT64 WINAPI CancelQueuedRequestsByIndex(USocket_Client_Handle h, SPA::UINT64 startIndex, SPA::UINT64 endIndex);
    CancelQueuedRequestsByIndex = _ucsLib_.CancelQueuedRequestsByIndex
    CancelQueuedRequestsByIndex.argtypes = [USocket_Client_Handle, c_uint64, c_uint64]
    CancelQueuedRequestsByIndex.restype = c_uint64

    #bool WINAPI IsDequeueShared(USocket_Client_Handle h);
    IsDequeueShared = _ucsLib_.IsDequeueShared
    IsDequeueShared.argtypes = [USocket_Client_Handle]
    IsDequeueShared.restype = c_bool

    """
    enum tagQueueStatus {
        //everything is fine
        qsNormal = 0,

        qsMergeComplete = 1,

        //merge push not completed yet
        qsMergePushing = 2,

        //merge incomplete (job incomplete or crash)
        qsMergeIncomplete = 3,

        //job incomplete (crash or endjob not found)
        qsJobIncomplete = 4,

        //an incomplete message detected
        qsCrash = 5,

        //file open error
        qsFileError = 6,

        //queue file opened but can't decrypt existing queued messages because of bad password found
        qsBadPassword = 7,

        //duplicate name error
        qsDuplicateName = 8,
    };
    """
    #SPA::tagQueueStatus WINAPI GetClientQueueStatus(USocket_Client_Handle h);
    GetClientQueueStatus = _ucsLib_.GetClientQueueStatus
    GetClientQueueStatus.argtypes = [USocket_Client_Handle]
    GetClientQueueStatus.restype = c_int

    #bool WINAPI PushQueueTo(USocket_Client_Handle src, const USocket_Client_Handle *targets, unsigned int count);
    PushQueueTo = _ucsLib_.PushQueueTo
    PushQueueTo.argtypes = [USocket_Client_Handle, POINTER(USocket_Client_Handle), c_uint]
    PushQueueTo.restype = c_bool

    #unsigned int WINAPI GetTTL(USocket_Client_Handle h);
    GetTTL = _ucsLib_.GetTTL
    GetTTL.argtypes = [USocket_Client_Handle]
    GetTTL.restype = c_uint

    #SPA::UINT64 WINAPI RemoveQueuedRequestsByTTL(USocket_Client_Handle h);
    RemoveQueuedRequestsByTTL = _ucsLib_.RemoveQueuedRequestsByTTL
    RemoveQueuedRequestsByTTL.argtypes = [USocket_Client_Handle]
    RemoveQueuedRequestsByTTL.restype = c_uint64

    #void WINAPI ResetQueue(USocket_Client_Handle h);
    ResetQueue = _ucsLib_.ResetQueue
    ResetQueue.argtypes = [USocket_Client_Handle]
    ResetQueue.restype = None

    #void WINAPI SetClientWorkDirectory(const char *dir);
    SetClientWorkDirectory = _ucsLib_.SetClientWorkDirectory
    SetClientWorkDirectory.argtypes = [c_char_p]
    SetClientWorkDirectory.restype = None

    #const char* WINAPI GetClientWorkDirectory();
    GetClientWorkDirectory = _ucsLib_.GetClientWorkDirectory
    GetClientWorkDirectory.argtypes = []
    GetClientWorkDirectory.restype = c_char_p

    #SPA::UINT64 WINAPI GetLastQueueMessageTime(USocket_Client_Handle h);
    GetLastQueueMessageTime = _ucsLib_.GetLastQueueMessageTime
    GetLastQueueMessageTime.argtypes = [USocket_Client_Handle]
    GetLastQueueMessageTime.restype = c_uint64

    #bool WINAPI IsRouting(USocket_Client_Handle h);
    IsRouting = _ucsLib_.IsRouting
    IsRouting.argtypes = [USocket_Client_Handle]
    IsRouting.restype = c_bool

    #void WINAPI AbortDequeuedMessage(USocket_Client_Handle h);
    AbortDequeuedMessage = _ucsLib_.AbortDequeuedMessage
    AbortDequeuedMessage.argtypes = [USocket_Client_Handle]
    AbortDequeuedMessage.restype = None

    #bool WINAPI IsDequeuedMessageAborted(USocket_Client_Handle h);
    IsDequeuedMessageAborted = _ucsLib_.IsDequeuedMessageAborted
    IsDequeuedMessageAborted.argtypes = [USocket_Client_Handle]
    IsDequeuedMessageAborted.restype = c_bool

    #bool WINAPI IsClientQueueIndexPossiblyCrashed();
    IsClientQueueIndexPossiblyCrashed = _ucsLib_.IsClientQueueIndexPossiblyCrashed
    IsClientQueueIndexPossiblyCrashed.argtypes = []
    IsClientQueueIndexPossiblyCrashed.restype = c_bool

    """
    enum tagEncryptionMethod {
        NoEncryption = 0,
        TLSv1 = 1,
    };
    """
    #void WINAPI SetEncryptionMethod(USocket_Client_Handle h, SPA::tagEncryptionMethod em);
    SetEncryptionMethod = _ucsLib_.SetEncryptionMethod
    SetEncryptionMethod.argtypes = [USocket_Client_Handle, c_int]
    SetEncryptionMethod.restype = None

    #SPA::tagEncryptionMethod WINAPI GetEncryptionMethod(USocket_Client_Handle h);
    GetEncryptionMethod = _ucsLib_.GetEncryptionMethod
    GetEncryptionMethod.argtypes = [USocket_Client_Handle]
    GetEncryptionMethod.restype = c_int

    """
    enum tagSocketLevel {
        slTcp = 6,
        slSocket = 0xFFFF,
    };

    enum tagSocketOption {
        soTcpNoDelay = 1,
        soReuseAddr = 4,
        soKeepAlive = 8,
        soSndBuf = 0x1001, /* send buffer size */
        soRcvBuf = 0x1002, /* receive buffer size */
    };
    """
    #bool WINAPI SetSockOpt(USocket_Client_Handle h, SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level);
    SetSockOpt = _ucsLib_.SetSockOpt
    SetSockOpt.argtypes = [USocket_Client_Handle, c_int, c_int, c_int]
    SetSockOpt.restype = c_bool

    #bool WINAPI SetSockOptAtSvr(USocket_Client_Handle h, SPA::tagSocketOption optName, int optValue, SPA::tagSocketLevel level);
    SetSockOptAtSvr = _ucsLib_.SetSockOptAtSvr
    SetSockOptAtSvr.argtypes = [USocket_Client_Handle, c_int, c_int, c_int]
    SetSockOptAtSvr.restype = c_bool

    """
    enum tagShutdownType {
        stReceive = 0,
        stSend = 1,
        stBoth = 2
    };
    """
    #void WINAPI Shutdown(USocket_Client_Handle h, SPA::tagShutdownType how);
    Shutdown = _ucsLib_.Shutdown
    Shutdown.argtypes = [USocket_Client_Handle, c_int]
    Shutdown.restype = None

    #const char* WINAPI GetUClientSocketVersion();
    GetUClientSocketVersion = _ucsLib_.GetUClientSocketVersion
    GetUClientSocketVersion.argtypes = []
    GetUClientSocketVersion.restype = c_char_p

    #void WINAPI SetMessageQueuePassword(const char *pwd);
    SetMessageQueuePassword = _ucsLib_.SetMessageQueuePassword
    SetMessageQueuePassword.argtypes = [c_char_p]
    SetMessageQueuePassword.restype = None

    """
    enum tagConnectionState {
        csClosed = 0,
        csConnecting = 1,
        csSslShaking = 2,
        csClosing = 3,
        csConnected = 4,
        csSwitched = 5
    };
    """
    #SPA::ClientSide::tagConnectionState WINAPI GetConnectionState(USocket_Client_Handle h);
    GetConnectionState = _ucsLib_.GetConnectionState
    GetConnectionState.argtypes = [USocket_Client_Handle]
    GetConnectionState.restype = c_int

    #void WINAPI SetCertificateVerifyCallback(PCertificateVerifyCallback cvc);
    SetCertificateVerifyCallback = _ucsLib_.SetCertificateVerifyCallback
    SetCertificateVerifyCallback.argtypes = [PCertificateVerifyCallback]
    SetCertificateVerifyCallback.restype = None

    #void WINAPI EnableRoutingQueueIndex(USocket_Client_Handle h, bool enable);
    EnableRoutingQueueIndex = _ucsLib_.EnableRoutingQueueIndex
    EnableRoutingQueueIndex.argtypes = [USocket_Client_Handle, c_bool]
    EnableRoutingQueueIndex.restype = None

    #bool WINAPI IsRoutingQueueIndexEnabled(USocket_Client_Handle h);
    IsRoutingQueueIndexEnabled = _ucsLib_.IsRoutingQueueIndexEnabled
    IsRoutingQueueIndexEnabled.argtypes = [USocket_Client_Handle]
    IsRoutingQueueIndexEnabled.restype = c_bool

    #const char* WINAPI GetUClientAppName();
    GetUClientAppName = _ucsLib_.GetUClientAppName
    GetUClientAppName.argtypes = []
    GetUClientAppName.restype = c_char_p

    """
    enum tagOptimistic {
        oMemoryCached = 0,
        oSystemMemoryCached = 1,
        oDiskCommitted = 2
    };
    """
    #SPA::tagOptimistic WINAPI GetOptimistic(USocket_Client_Handle h);
    GetOptimistic = _ucsLib_.GetOptimistic
    GetOptimistic.argtypes = [USocket_Client_Handle]
    GetOptimistic.restype = c_int

    #void WINAPI SetOptimistic(USocket_Client_Handle h, SPA::tagOptimistic bOptimistic);
    SetOptimistic = _ucsLib_.SetOptimistic
    SetOptimistic.argtypes = [USocket_Client_Handle, c_int]
    SetOptimistic.restype = None

    #void WINAPI SetLastCallInfo(const char *str);
    SetLastCallInfo = _ucsLib_.SetLastCallInfo
    SetLastCallInfo.argtypes = [c_char_p]
    SetLastCallInfo.restype = None

    #bool WINAPI GetQueueAutoMergeByPool(unsigned int poolId);
    GetQueueAutoMergeByPool = _ucsLib_.GetQueueAutoMergeByPool
    GetQueueAutoMergeByPool.argtypes = [c_uint]
    GetQueueAutoMergeByPool.restype = c_bool

    #void WINAPI SetQueueAutoMergeByPool(unsigned int poolId, bool autoMerge);
    SetQueueAutoMergeByPool = _ucsLib_.SetQueueAutoMergeByPool
    SetQueueAutoMergeByPool.argtypes = [c_uint, c_bool]
    SetQueueAutoMergeByPool.restype = None

    # void WINAPI SetOnPostProcessing(USocket_Client_Handle h, POnPostProcessing p);
    SetOnPostProcessing = _ucsLib_.SetOnPostProcessing
    SetOnPostProcessing.argtypes = [USocket_Client_Handle, POnPostProcessing]
    SetOnPostProcessing.restype = None

    #void WINAPI PostProcessing(USocket_Client_Handle h, unsigned int hint, SPA::UINT64 data);
    PostProcessing = _ucsLib_.PostProcessing
    PostProcessing.argtypes = [USocket_Client_Handle, c_uint, c_uint64]
    PostProcessing.restype = None

class CQueueConfigureImpl(object):
    @property
    def WorkDirectory(self):
        return CCoreLoader.GetClientWorkDirectory().decode('latin-1')

    @WorkDirectory.setter
    def WorkDirectory(self, value):
        CCoreLoader.SetClientWorkDirectory(value.encode('latin-1'))

    @property
    def IsClientQueueIndexPossiblyCrashed(self):
        return CCoreLoader.IsClientQueueIndexPossiblyCrashed()

    def _set_wo(self, value):
        CCoreLoader.SetMessageQueuePassword(value.encode('latin-1'))
    MessageQueuePassword = property(None, _set_wo)

class CSSLConfigureImpl(object):
    def _set_cvc(self, cvc):
        if not cvc is None:
            self._m_cvc_ = CCoreLoader.PCertificateVerifyCallback(cvc)
            CCoreLoader.SetCertificateVerifyCallback(self._m_cvc_)
        else:
            CCoreLoader.SetCertificateVerifyCallback(CCoreLoader.PCertificateVerifyCallback())
            self._m_cvc_ = None
    def _get_cvc(self):
        return self._m_cvc_
    CertificateVerify = property(_get_cvc, _set_cvc)

    def SetVerifyLocation(self, certFile):
        if certFile is None or len(certFile) == 0:
            raise ValueError("Invalid queue file name")
        CCoreLoader.SetVerifyLocation(certFile.encode('latin-1'))