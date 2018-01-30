from ctypes import *
from sys import platform as os

class CHttpHV(Structure):
    _fields_ = [("Header", c_char_p),
                ("Value", c_char_p)]

class SCoreLoader(object):
    _ussLib_ = None
    _IsWin_ = (os == "win32")
    if _IsWin_:
        _ussLib_ = WinDLL("uservercore.dll")
    else:
        _ussLib_ = CDLL("libuservercore.so")

    #typedef void (CALLBACK *POnClose) (USocket_Server_Handle Handler, int errCode);
    if _IsWin_:
        POnClose = WINFUNCTYPE(None, c_uint64, c_int)
    else:
        POnClose = CFUNCTYPE(None, c_uint64, c_int)

    #typedef void (CALLBACK *POnSSLHandShakeCompleted)(USocket_Server_Handle Handler, int errCode);
    if _IsWin_:
        POnSSLHandShakeCompleted = WINFUNCTYPE(None, c_uint64, c_int)
    else:
        POnSSLHandShakeCompleted = CFUNCTYPE(None, c_uint64, c_int)

    #typedef void (CALLBACK *POnAccept)(USocket_Server_Handle Handler, int errCode);
    if _IsWin_:
        POnAccept = WINFUNCTYPE(None, c_uint64, c_int)
    else:
        POnAccept = CFUNCTYPE(None, c_uint64, c_int)

    #typedef void (CALLBACK *POnIdle)(SPA::INT64 milliseconds);
    if _IsWin_:
        POnIdle = WINFUNCTYPE(None, c_int64)
    else:
        POnIdle = CFUNCTYPE(None, c_int64)

    #typedef bool (CALLBACK *POnIsPermitted)(USocket_Server_Handle Handler, unsigned int serviceId);
    if _IsWin_:
        POnIsPermitted = WINFUNCTYPE(c_bool, c_uint64, c_uint)
    else:
        POnIsPermitted = CFUNCTYPE(c_bool, c_uint64, c_uint)

    #typedef void (CALLBACK *POnResultsSent)(USocket_Server_Handle Handler);
    if _IsWin_:
        POnResultsSent = WINFUNCTYPE(None, c_uint64)
    else:
        POnResultsSent = CFUNCTYPE(None, c_uint64)

    #typedef void (CALLBACK *POnRequestArrive)(USocket_Server_Handle Handler, unsigned short requestId, unsigned int len);
    if _IsWin_:
        POnRequestArrive = WINFUNCTYPE(None, c_uint64, c_ushort, c_int)
    else:
        POnRequestArrive = CFUNCTYPE(None, c_uint64, c_ushort, c_int)

    #typedef void (CALLBACK *POnFastRequestArrive)(USocket_Server_Handle Handler, unsigned short requestId, unsigned int len);
    if _IsWin_:
        POnFastRequestArrive = WINFUNCTYPE(None, c_uint64, c_ushort, c_int)
    else:
        POnFastRequestArrive = CFUNCTYPE(None, c_uint64, c_ushort, c_int)

    #typedef int (CALLBACK *PSLOW_PROCESS)(unsigned short requestId, unsigned int len, USocket_Server_Handle Handler);
    if _IsWin_:
        PSLOW_PROCESS = WINFUNCTYPE(c_int, c_ushort, c_uint, c_uint64)
    else:
        PSLOW_PROCESS = CFUNCTYPE(c_int, c_ushort, c_uint, c_uint64)

    #typedef void (CALLBACK *POnRequestProcessed)(USocket_Server_Handle Handler, unsigned short requestId);
    if _IsWin_:
        POnRequestProcessed = WINFUNCTYPE(None, c_uint64, c_ushort)
    else:
        POnRequestProcessed = CFUNCTYPE(None, c_uint64, c_ushort)

    #typedef void (CALLBACK *POnBaseRequestCame)(USocket_Server_Handle Handler, unsigned short requestId);
    if _IsWin_:
        POnBaseRequestCame = WINFUNCTYPE(None, c_uint64, c_ushort)
    else:
        POnBaseRequestCame = CFUNCTYPE(None, c_uint64, c_ushort)

    #typedef void (CALLBACK *POnSwitchTo)(USocket_Server_Handle Handler, unsigned int oldServiceId, unsigned int newServiceId);
    if _IsWin_:
        POnSwitchTo = WINFUNCTYPE(None, c_uint64, c_uint, c_uint)
    else:
        POnSwitchTo = CFUNCTYPE(None, c_uint64, c_uint, c_uint)

    #typedef void (CALLBACK *POnChatRequestComing) (USocket_Server_Handle handler, SPA::tagChatRequestID chatRequestID, unsigned int len);
    if _IsWin_:
        POnChatRequestComing = WINFUNCTYPE(None, c_uint64, c_ushort, c_uint)
    else:
        POnChatRequestComing = CFUNCTYPE(None, c_uint64, c_ushort, c_uint)

    #typedef void (CALLBACK *POnChatRequestCame) (USocket_Server_Handle handler, SPA::tagChatRequestID chatRequestId);
    if _IsWin_:
        POnChatRequestCame = WINFUNCTYPE(None, c_uint64, c_ushort)
    else:
        POnChatRequestCame = CFUNCTYPE(None, c_uint64, c_ushort)

    #typedef bool (CALLBACK *POnHttpAuthentication) (USocket_Server_Handle handler, const wchar_t *userId, const wchar_t *password);
    if _IsWin_:
        POnHttpAuthentication = WINFUNCTYPE(c_bool, c_uint64, c_wchar_p, c_wchar_p)
    else:
        POnHttpAuthentication = CFUNCTYPE(c_bool, c_uint64, c_wchar_p, c_wchar_p)

    #bool WINAPI InitSocketProServer(int param);
    InitSocketProServer = _ussLib_.InitSocketProServer
    InitSocketProServer.argtypes = [c_int]
    InitSocketProServer.restype = c_bool

    #void WINAPI UninitSocketProServer();
    UninitSocketProServer = _ussLib_.UninitSocketProServer
    UninitSocketProServer.argtypes = []
    UninitSocketProServer.restype = None

    #bool WINAPI StartSocketProServer(unsigned int listeningPort, unsigned int maxBacklog = 32, bool v6 = true);
    StartSocketProServer = _ussLib_.StartSocketProServer
    StartSocketProServer.argtypes = [c_uint, c_uint, c_bool]
    StartSocketProServer.restype = c_bool

    #void WINAPI StopSocketProServer();
    StopSocketProServer = _ussLib_.StopSocketProServer
    StopSocketProServer.argtypes = []
    StopSocketProServer.restype = None

    #bool WINAPI IsCanceled(USocket_Server_Handle Handler);
    IsCanceled = _ussLib_.IsCanceled
    IsCanceled.argtypes = [c_uint64]
    IsCanceled.restype = c_bool

    #bool WINAPI IsRunning();
    IsRunning = _ussLib_.IsRunning
    IsRunning.argtypes = []
    IsRunning.restype = c_bool

    #void WINAPI SetAuthenticationMethod(SPA::ServerSide::tagAuthenticationMethod am);
    SetAuthenticationMethod = _ussLib_.SetAuthenticationMethod
    SetAuthenticationMethod.argtypes = [c_int]
    SetAuthenticationMethod.restype = None

    #SPA::ServerSide::tagAuthenticationMethod WINAPI GetAuthenticationMethod();
    GetAuthenticationMethod = _ussLib_.GetAuthenticationMethod
    GetAuthenticationMethod.argtypes = []
    GetAuthenticationMethod.restype = c_int

    #void WINAPI SetSharedAM(bool b);
    SetSharedAM = _ussLib_.SetSharedAM
    SetSharedAM.argtypes = [c_bool]
    SetSharedAM.restype = None

    #bool WINAPI GetSharedAM();
    GetSharedAM = _ussLib_.GetSharedAM
    GetSharedAM.argtypes = []
    GetSharedAM.restype = c_bool

    #void WINAPI PostQuitPump();
    PostQuitPump = _ussLib_.PostQuitPump
    PostQuitPump.argtypes = []
    PostQuitPump.restype = None

    #bool WINAPI IsMainThread();
    IsMainThread = _ussLib_.IsMainThread
    IsMainThread.argtypes = []
    IsMainThread.restype = c_bool

    #bool WINAPI AddSvsContext(unsigned int serviceId, CSvsContext svsContext); //ta ignored on non-window platforms
    AddSvsContext = _ussLib_.AddSvsContext
    #AddSvsContext.argtypes = [c_uint, CSvsContext]
    AddSvsContext.restype = c_bool

    #void WINAPI RemoveASvsContext(unsigned int serviceId);
    RemoveASvsContext= _ussLib_.RemoveASvsContext
    RemoveASvsContext.argtypes = [c_uint]
    RemoveASvsContext.restype = None

    #CSvsContext WINAPI GetSvsContext(unsigned int serviceId);
    GetSvsContext = _ussLib_.GetSvsContext
    GetSvsContext.argtypes = [c_uint]
    #GetSvsContext.restype = CSvsContext

    #bool WINAPI AddSlowRequest(unsigned int serviceId, unsigned short requestId);
    AddSlowRequest = _ussLib_.AddSlowRequest
    AddSlowRequest.argtypes = [c_uint, c_ushort]
    AddSlowRequest.restype = c_bool

    #void WINAPI RemoveSlowRequest(unsigned int serviceId, unsigned short requestId);
    RemoveSlowRequest = _ussLib_.RemoveSlowRequest
    RemoveSlowRequest.argtypes = [c_uint, c_ushort]
    RemoveSlowRequest.restype = None

    #unsigned int WINAPI GetCountOfServices();
    GetCountOfServices = _ussLib_.GetCountOfServices
    GetCountOfServices.argtypes = []
    GetCountOfServices.restype = c_uint

    #unsigned int WINAPI GetServices(unsigned int *serviceIds, unsigned int count);
    GetServices = _ussLib_.GetServices
    GetServices.argtypes = [POINTER(c_uint), c_uint]
    GetServices.restype = c_uint

    #unsigned int WINAPI GetCountOfSlowRequests(unsigned int serviceId);
    GetCountOfSlowRequests = _ussLib_.GetCountOfSlowRequests
    GetCountOfSlowRequests.argtypes = [c_uint]
    GetCountOfSlowRequests.restype = c_uint

    #void WINAPI RemoveAllSlowRequests(unsigned int serviceId);
    RemoveAllSlowRequests = _ussLib_.RemoveAllSlowRequests
    RemoveAllSlowRequests.argtypes = [c_uint]
    RemoveAllSlowRequests.restype = None

    #unsigned int WINAPI GetAllSlowRequestIds(unsigned int serviceId, unsigned short *requestIds, unsigned int count);
    GetAllSlowRequestIds = _ussLib_.GetAllSlowRequestIds
    GetAllSlowRequestIds.argtypes = [c_uint, POINTER(c_ushort), c_uint]
    GetAllSlowRequestIds.restype = c_uint

    #HINSTANCE WINAPI AddADll(const char *libFile, int nParam);
    AddADll = _ussLib_.AddADll
    AddADll.argtypes = [c_char_p, c_int]
    AddADll.restype = c_void_p

    #bool WINAPI RemoveADllByHandle(HINSTANCE hInstance);
    RemoveADllByHandle = _ussLib_.RemoveADllByHandle
    RemoveADllByHandle.argtypes = [c_void_p]
    RemoveADllByHandle.restype = c_bool

    #void WINAPI SetPrivateKeyFile(const char *keyFile);
    SetPrivateKeyFile = _ussLib_.SetPrivateKeyFile
    SetPrivateKeyFile.argtypes = [c_char_p]
    SetPrivateKeyFile.restype = None

    #void WINAPI SetCertFile(const char *certFile);
    SetCertFile = _ussLib_.SetCertFile
    SetCertFile.argtypes = [c_char_p]
    SetCertFile.restype = None

    #void WINAPI SetDHParmsFile(const char *dhFile);
    SetDHParmsFile = _ussLib_.SetDHParmsFile
    SetDHParmsFile.argtypes = [c_char_p]
    SetDHParmsFile.restype = None

    #void WINAPI SetPKFPassword(const char *pwd);
    SetPKFPassword = _ussLib_.SetPKFPassword
    SetPKFPassword.argtypes = [c_char_p]
    SetPKFPassword.restype = None

    #void WINAPI SetDefaultEncryptionMethod(SPA::tagEncryptionMethod em);
    SetDefaultEncryptionMethod = _ussLib_.SetDefaultEncryptionMethod
    SetDefaultEncryptionMethod.argtypes = [c_int]
    SetDefaultEncryptionMethod.restype = None

    #SPA::tagEncryptionMethod WINAPI GetDefaultEncryptionMethod();
    GetDefaultEncryptionMethod = _ussLib_.GetDefaultEncryptionMethod
    GetDefaultEncryptionMethod.argtypes = []
    GetDefaultEncryptionMethod.restype = c_int

    #void WINAPI SetPfxFile(const char *pfxFile);
    SetPfxFile = _ussLib_.SetPfxFile
    SetPfxFile.argtypes = [c_char_p]
    SetPfxFile.restype = None

    #int WINAPI GetServerErrorCode();
    GetServerErrorCode = _ussLib_.GetServerErrorCode
    GetServerErrorCode.argtypes = []
    GetServerErrorCode.restype = c_int

    #unsigned int WINAPI GetServerErrorMessage(char *str, unsigned int bufferLen);
    GetServerErrorMessage = _ussLib_.GetServerErrorMessage
    GetServerErrorMessage.argtypes = [POINTER(c_char), c_uint]
    GetServerErrorMessage.restype = c_uint

    #bool WINAPI IsServerRunning();
    IsServerRunning = _ussLib_.IsServerRunning
    IsServerRunning.argtypes = []
    IsServerRunning.restype = c_bool

    #bool WINAPI IsServerSSLEnabled();
    IsServerSSLEnabled = _ussLib_.IsServerSSLEnabled
    IsServerSSLEnabled.argtypes = []
    IsServerSSLEnabled.restype = c_bool

    #void WINAPI SetOnAccept(POnAccept p);
    SetOnAccept = _ussLib_.SetOnAccept
    SetOnAccept.argtypes = [POnAccept]
    SetOnAccept.restype = None

    #void WINAPI Close(USocket_Server_Handle h);
    Close = _ussLib_.Close
    Close.argtypes = [c_uint64]
    Close.restype = None

    #unsigned short WINAPI GetCurrentRequestID(USocket_Server_Handle h);
    GetCurrentRequestID = _ussLib_.GetCurrentRequestID
    GetCurrentRequestID.argtypes = [c_uint64]
    GetCurrentRequestID.restype = c_ushort

    #unsigned int WINAPI GetCurrentRequestLen(USocket_Server_Handle h);
    GetCurrentRequestLen = _ussLib_.GetCurrentRequestLen
    GetCurrentRequestLen.argtypes = [c_uint64]
    GetCurrentRequestLen.restype = c_uint

    #unsigned int WINAPI GetRcvBytesInQueue(USocket_Server_Handle h);
    GetRcvBytesInQueue = _ussLib_.GetRcvBytesInQueue
    GetRcvBytesInQueue.argtypes = [c_uint64]
    GetRcvBytesInQueue.restype = c_uint

    #unsigned int WINAPI GetSndBytesInQueue(USocket_Server_Handle h);
    GetSndBytesInQueue = _ussLib_.GetSndBytesInQueue
    GetSndBytesInQueue.argtypes = [c_uint64]
    GetSndBytesInQueue.restype = c_uint

    #void WINAPI PostClose(USocket_Server_Handle h, int errCode = 0);
    PostClose = _ussLib_.PostClose
    PostClose.argtypes = [c_uint64, c_int]
    PostClose.restype = None

    #unsigned int WINAPI QueryRequestsInQueue(USocket_Server_Handle h);
    QueryRequestsInQueue = _ussLib_.QueryRequestsInQueue
    QueryRequestsInQueue.argtypes = [c_uint64]
    QueryRequestsInQueue.restype = c_uint

    #unsigned int WINAPI RetrieveBuffer(USocket_Server_Handle h, unsigned int bufferSize, unsigned char *buffer, bool peek = false);
    RetrieveBuffer = _ussLib_.RetrieveBuffer
    RetrieveBuffer.argtypes = [c_uint64, c_uint, POINTER(c_char), c_bool]
    RetrieveBuffer.restype = c_uint

    #void WINAPI SetOnSSLHandShakeCompleted(POnSSLHandShakeCompleted p);
    SetOnSSLHandShakeCompleted = _ussLib_.SetOnSSLHandShakeCompleted
    SetOnSSLHandShakeCompleted.argtypes = [POnSSLHandShakeCompleted]
    SetOnSSLHandShakeCompleted.restype = None

    #void WINAPI SetOnClose(POnClose p);
    SetOnClose = _ussLib_.SetOnClose
    SetOnClose.argtypes = [POnClose]
    SetOnClose.restype = None

    #void WINAPI SetOnIdle(POnIdle p);
    SetOnIdle = _ussLib_.SetOnIdle
    SetOnIdle.argtypes = [POnIdle]
    SetOnIdle.restype = None

    #bool WINAPI IsOpened(USocket_Server_Handle h);
    IsOpened = _ussLib_.IsOpened
    IsOpened.argtypes = [c_uint64]
    IsOpened.restype = c_bool

    #SPA::UINT64 WINAPI GetBytesReceived(USocket_Server_Handle h);
    GetBytesReceived = _ussLib_.GetBytesReceived
    GetBytesReceived.argtypes = [c_uint64]
    GetBytesReceived.restype = c_uint64

    #SPA::UINT64 WINAPI GetBytesSent(USocket_Server_Handle h);
    GetBytesSent = _ussLib_.GetBytesSent
    GetBytesSent.argtypes = [c_uint64]
    GetBytesSent.restype = c_uint64

    #unsigned int WINAPI SendReturnData(USocket_Server_Handle h, unsigned short requestId, unsigned int bufferSize, const unsigned char *buffer);
    SendReturnData = _ussLib_.SendReturnData
    SendReturnData.argtypes = [c_uint64, c_ushort, c_uint, POINTER(c_ubyte)]
    SendReturnData.restype = c_uint

    #unsigned int WINAPI GetSvsID(USocket_Server_Handle h);
    GetSvsID = _ussLib_.GetSvsID
    GetSvsID.argtypes = [c_uint64]
    GetSvsID.restype = c_uint

    #int WINAPI GetServerSocketErrorCode(USocket_Server_Handle h);
    GetServerSocketErrorCode = _ussLib_.GetServerSocketErrorCode
    GetServerSocketErrorCode.argtypes = [c_uint64]
    GetServerSocketErrorCode.restype = c_int

    #unsigned int WINAPI GetServerSocketErrorMessage(USocket_Server_Handle h, char *str, unsigned int bufferLen);
    GetServerSocketErrorMessage = _ussLib_.GetServerSocketErrorMessage
    GetServerSocketErrorMessage.argtypes = [c_uint64, POINTER(c_char), c_uint]
    GetServerSocketErrorMessage.restype = c_uint

    #bool WINAPI IsBatching(USocket_Server_Handle h);
    IsBatching = _ussLib_.IsBatching
    IsBatching.argtypes = [c_uint64]
    IsBatching.restype = c_bool

    #unsigned int WINAPI GetBytesBatched(USocket_Server_Handle h);
    GetBytesBatched = _ussLib_.GetBytesBatched
    GetBytesBatched.argtypes = [c_uint64]
    GetBytesBatched.restype = c_uint

    #bool WINAPI StartBatching(USocket_Server_Handle h);
    StartBatching = _ussLib_.StartBatching
    StartBatching.argtypes = [c_uint64]
    StartBatching.restype = c_bool

    #bool WINAPI CommitBatching(USocket_Server_Handle h);
    CommitBatching = _ussLib_.CommitBatching
    CommitBatching.argtypes = [c_uint64]
    CommitBatching.restype = c_bool

    #bool WINAPI AbortBatching(USocket_Server_Handle h);
    AbortBatching = _ussLib_.AbortBatching
    AbortBatching.argtypes = [c_uint64]
    AbortBatching.restype = c_bool

    #bool WINAPI SetUserID(USocket_Server_Handle h, const wchar_t *userId);
    SetUserID = _ussLib_.SetUserID
    SetUserID.argtypes = [c_uint64, c_wchar_p]
    SetUserID.restype = c_bool

    #unsigned int WINAPI GetUID(USocket_Server_Handle h, wchar_t *userId, unsigned int chars);
    GetUID = _ussLib_.GetUID
    GetUID.argtypes = [c_uint64, POINTER(c_wchar), c_uint]
    GetUID.restype = c_uint

    #bool WINAPI SetPassword(USocket_Server_Handle h, const wchar_t *password);
    SetPassword = _ussLib_.SetPassword
    SetPassword.argtypes = [c_uint64, c_wchar_p]
    SetPassword.restype = c_bool

    #unsigned int WINAPI GetPassword(USocket_Server_Handle h, wchar_t *password, unsigned int chars);
    GetPassword = _ussLib_.GetPassword
    GetPassword.argtypes = [c_uint64, POINTER(c_wchar), c_uint]
    GetPassword.restype = c_uint

    #void WINAPI SetOnIsPermitted(POnIsPermitted p);
    SetOnIsPermitted = _ussLib_.SetOnIsPermitted
    SetOnIsPermitted.argtypes = [POnIsPermitted]
    SetOnIsPermitted.restype = None

    #bool WINAPI Enter(USocket_Server_Handle h, const unsigned int *chatGroupIds, unsigned int count);
    Enter = _ussLib_.Enter
    Enter.argtypes = [c_uint64, POINTER(c_uint), c_uint]
    Enter.restype = c_bool

    #void WINAPI Exit(USocket_Server_Handle h);
    Exit = _ussLib_.Exit
    Exit.argtypes = [c_uint64]
    Exit.restype = None

    #bool WINAPI Speak(USocket_Server_Handle h, const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    Speak = _ussLib_.Speak
    Speak.argtypes = [c_uint64, POINTER(c_ubyte), c_uint, POINTER(c_uint), c_uint]
    Speak.restype = c_bool

    #bool WINAPI SpeakPush(const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    SpeakPush = _ussLib_.SpeakPush
    SpeakPush.argtypes = [POINTER(c_ubyte), c_uint, POINTER(c_uint), c_uint]
    SpeakPush.restype = c_bool

    #bool WINAPI SpeakEx(USocket_Server_Handle h, const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    SpeakEx = _ussLib_.SpeakEx
    SpeakEx.argtypes = [c_uint64, POINTER(c_ubyte), c_uint, POINTER(c_uint), c_uint]
    SpeakEx.restype = c_bool

    #bool WINAPI SpeakExPush(const unsigned char *message, unsigned int size, const unsigned int *chatGroupIds, unsigned int count);
    SpeakExPush = _ussLib_.SpeakExPush
    SpeakExPush.argtypes = [POINTER(c_ubyte), c_uint, POINTER(c_uint), c_uint]
    SpeakExPush.restype = c_bool

    #bool WINAPI SendUserMessageEx(USocket_Server_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    SendUserMessageEx = _ussLib_.SendUserMessageEx
    SendUserMessageEx.argtypes = [c_uint64, c_wchar_p, POINTER(c_ubyte), c_uint]
    SendUserMessageEx.restype = c_bool

    #bool WINAPI SendUserMessageExPush(const wchar_t *userId, const unsigned char *message, unsigned int size);
    SendUserMessageExPush = _ussLib_.SendUserMessageExPush
    SendUserMessageExPush.argtypes = [c_wchar_p, POINTER(c_ubyte), c_uint]
    SendUserMessageExPush.restype = c_bool

    #bool WINAPI SendUserMessage(USocket_Server_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
    SendUserMessage = _ussLib_.SendUserMessage
    SendUserMessage.argtypes = [c_uint64, c_wchar_p, POINTER(c_ubyte), c_uint]
    SendUserMessage.restype = c_bool

    #bool WINAPI SendUserMessagePush(const wchar_t *userId, const unsigned char *message, unsigned int size);
    SendUserMessagePush = _ussLib_.SendUserMessagePush
    SendUserMessagePush.argtypes = [c_wchar_p, POINTER(c_ubyte), c_uint]
    SendUserMessagePush.restype = c_bool

    #unsigned int WINAPI GetCountOfJoinedChatGroups(USocket_Server_Handle h);
    GetCountOfJoinedChatGroups = _ussLib_.GetCountOfJoinedChatGroups
    GetCountOfJoinedChatGroups.argtypes = [c_uint64]
    GetCountOfJoinedChatGroups.restype = c_uint

    #unsigned int WINAPI GetJoinedGroupIds(USocket_Server_Handle h, unsigned int *chatGroups, unsigned int count);
    GetJoinedGroupIds = _ussLib_.GetJoinedGroupIds
    GetJoinedGroupIds.argtypes = [c_uint64, POINTER(c_uint), c_uint]
    GetJoinedGroupIds.restype = c_uint

    #bool WINAPI GetPeerName(USocket_Server_Handle h, unsigned int *peerPort, char *strPeerAddr, unsigned short chars);
    GetPeerName = _ussLib_.GetPeerName
    GetPeerName.argtypes = [c_uint64, POINTER(c_uint), POINTER(c_char), c_ushort]
    GetPeerName.restype = c_bool

    #unsigned int WINAPI GetLocalName(char *localName, unsigned short chars);
    GetLocalName = _ussLib_.GetLocalName
    GetLocalName.argtypes = [POINTER(c_char), c_ushort]
    GetLocalName.restype = c_uint

    #bool WINAPI HasUserId(const wchar_t *userId);
    HasUserId = _ussLib_.HasUserId
    HasUserId.argtypes = [c_wchar_p]
    HasUserId.restype = c_bool

    #void WINAPI DropCurrentSlowRequest(USocket_Server_Handle h);
    DropCurrentSlowRequest = _ussLib_.DropCurrentSlowRequest
    DropCurrentSlowRequest.argtypes = [c_uint64]
    DropCurrentSlowRequest.restype = None

    #void WINAPI AddAChatGroup(unsigned int chatGroupId, const wchar_t *description = nullptr);
    AddAChatGroup = _ussLib_.AddAChatGroup
    AddAChatGroup.argtypes = [c_uint, c_wchar_p]
    AddAChatGroup.restype = None

    #unsigned int WINAPI GetCountOfChatGroups();
    GetCountOfChatGroups = _ussLib_.GetCountOfChatGroups
    GetCountOfChatGroups.argtypes = []
    GetCountOfChatGroups.restype = c_uint

    #unsigned int WINAPI GetAllCreatedChatGroups(unsigned int *chatGroupIds, unsigned int count);
    GetAllCreatedChatGroups = _ussLib_.GetAllCreatedChatGroups
    GetAllCreatedChatGroups.argtypes = [POINTER(c_uint), c_uint]
    GetAllCreatedChatGroups.restype = c_uint

    #unsigned int WINAPI GetAChatGroup(unsigned int chatGroupId, wchar_t *description, unsigned int chars);
    GetAChatGroup = _ussLib_.GetAChatGroup
    GetAChatGroup.argtypes = [c_uint, POINTER(c_wchar), c_uint]
    GetAChatGroup.restype = c_uint

    #void WINAPI RemoveChatGroup(unsigned int chatGroupId);
    RemoveChatGroup = _ussLib_.RemoveChatGroup
    RemoveChatGroup.argtypes = [c_uint]
    RemoveChatGroup.restype = None

    #SPA::UINT64 WINAPI GetSocketNativeHandle(USocket_Server_Handle h);
    GetSocketNativeHandle = _ussLib_.GetSocketNativeHandle
    GetSocketNativeHandle.argtypes = [c_uint64]
    GetSocketNativeHandle.restype = c_uint64

    #SPA::tagOperationSystem WINAPI GetPeerOs(USocket_Server_Handle handler, bool *endian);
    GetPeerOs = _ussLib_.GetPeerOs
    GetPeerOs.argtypes = [c_uint64, POINTER(c_bool)]
    GetPeerOs.restype = c_int

    #unsigned int WINAPI SendExceptionResult(USocket_Server_Handle handler, const wchar_t* errMessage, const char* errWhere, unsigned short requestId = 0, unsigned int errCode = 0);
    SendExceptionResult = _ussLib_.SendExceptionResult
    SendExceptionResult.argtypes = [c_uint64, c_wchar_p, c_char_p, c_ushort, c_uint]
    SendExceptionResult.restype = c_uint

    #bool WINAPI MakeRequest(USocket_Server_Handle handler, unsigned short requestId, const unsigned char *request, unsigned int size);
    MakeRequest = _ussLib_.MakeRequest
    MakeRequest.argtypes = [c_uint64, c_ushort, POINTER(c_ubyte), c_uint]
    MakeRequest.restype = c_bool

    #unsigned int WINAPI GetHTTPRequestHeaders(USocket_Server_Handle h, SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count);
    GetHTTPRequestHeaders = _ussLib_.GetHTTPRequestHeaders
    GetHTTPRequestHeaders.argtypes = [c_uint64, POINTER(CHttpHV), c_uint]
    GetHTTPRequestHeaders.restype = c_uint

    #const char* WINAPI GetHTTPPath(USocket_Server_Handle h);
    GetHTTPPath = _ussLib_.GetHTTPPath
    GetHTTPPath.argtypes = [c_uint64]
    GetHTTPPath.restype = c_char_p

    #SPA::UINT64 WINAPI GetHTTPContentLength(USocket_Server_Handle h);
    GetHTTPContentLength = _ussLib_.GetHTTPContentLength
    GetHTTPContentLength.argtypes = [c_uint64]
    GetHTTPContentLength.restype = c_uint64

    #const char* WINAPI GetHTTPQuery(USocket_Server_Handle h);
    GetHTTPQuery = _ussLib_.GetHTTPQuery
    GetHTTPQuery.argtypes = [c_uint64]
    GetHTTPQuery.restype = c_char_p

    #bool WINAPI DownloadFile(USocket_Server_Handle handler, const char *filePath);
    DownloadFile = _ussLib_.DownloadFile
    DownloadFile.argtypes = [c_uint64, c_char_p]
    DownloadFile.restype = c_bool

    #SPA::ServerSide::tagHttpMethod WINAPI GetHTTPMethod(USocket_Server_Handle h);
    GetHTTPMethod = _ussLib_.GetHTTPMethod
    GetHTTPMethod.argtypes = [c_uint64]
    GetHTTPMethod.restype = c_int

    #bool WINAPI HTTPKeepAlive(USocket_Server_Handle h);
    HTTPKeepAlive = _ussLib_.HTTPKeepAlive
    HTTPKeepAlive.argtypes = [c_uint64]
    HTTPKeepAlive.restype = c_bool

    #bool WINAPI IsWebSocket(USocket_Server_Handle h);
    IsWebSocket = _ussLib_.IsWebSocket
    IsWebSocket.argtypes = [c_uint64]
    IsWebSocket.restype = c_bool

    #bool WINAPI IsCrossDomain(USocket_Server_Handle h);
    IsCrossDomain = _ussLib_.IsCrossDomain
    IsCrossDomain.argtypes = [c_uint64]
    IsCrossDomain.restype = c_bool

    #double WINAPI GetHTTPVersion(USocket_Server_Handle h);
    GetHTTPVersion = _ussLib_.GetHTTPVersion
    GetHTTPVersion.argtypes = [c_uint64]
    GetHTTPVersion.restype = c_double

    #bool WINAPI HTTPGZipAccepted(USocket_Server_Handle h);
    HTTPGZipAccepted = _ussLib_.HTTPGZipAccepted
    HTTPGZipAccepted.argtypes = [c_uint64]
    HTTPGZipAccepted.restype = c_bool

    #const char* WINAPI GetHTTPUrl(USocket_Server_Handle h);
    GetHTTPUrl = _ussLib_.GetHTTPUrl
    GetHTTPUrl.argtypes = [c_uint64]
    GetHTTPUrl.restype = c_char_p

    #const char* WINAPI GetHTTPHost(USocket_Server_Handle h);
    GetHTTPHost = _ussLib_.GetHTTPHost
    GetHTTPHost.argtypes = [c_uint64]
    GetHTTPHost.restype = c_char_p

    #SPA::ServerSide::tagTransport WINAPI GetHTTPTransport(USocket_Server_Handle h);
    GetHTTPTransport = _ussLib_.GetHTTPTransport
    GetHTTPTransport.argtypes = [c_uint64]
    GetHTTPTransport.restype = c_int

    #SPA::ServerSide::tagTransferEncoding WINAPI GetHTTPTransferEncoding(USocket_Server_Handle h);
    GetHTTPTransferEncoding = _ussLib_.GetHTTPTransferEncoding
    GetHTTPTransferEncoding.argtypes = [c_uint64]
    GetHTTPTransferEncoding.restype = c_int

    #SPA::ServerSide::tagContentMultiplax WINAPI GetHTTPContentMultiplax(USocket_Server_Handle h);
    GetHTTPContentMultiplax = _ussLib_.GetHTTPContentMultiplax
    GetHTTPContentMultiplax.argtypes = [c_uint64]
    GetHTTPContentMultiplax.restype = c_int

    #bool WINAPI SetHTTPResponseCode(USocket_Server_Handle h, unsigned int errCode);
    SetHTTPResponseCode = _ussLib_.SetHTTPResponseCode
    SetHTTPResponseCode.argtypes = [c_uint64, c_uint]
    SetHTTPResponseCode.restype = c_bool

    #bool WINAPI SetHTTPResponseHeader(USocket_Server_Handle h, const char *uft8Header, const char *utf8Value);
    SetHTTPResponseHeader = _ussLib_.SetHTTPResponseHeader
    SetHTTPResponseHeader.argtypes = [c_uint64, c_char_p, c_char_p]
    SetHTTPResponseHeader.restype = c_bool

    #unsigned int WINAPI SendHTTPReturnDataA(USocket_Server_Handle h, const char *str, unsigned int chars = (~0));
    SendHTTPReturnDataA = _ussLib_.SendHTTPReturnDataA
    SendHTTPReturnDataA.argtypes = [c_uint64, c_char_p, c_uint]
    SendHTTPReturnDataA.restype = c_uint

    #unsigned int WINAPI SendHTTPReturnDataW(USocket_Server_Handle h, const wchar_t *str, unsigned int chars = (~0));
    SendHTTPReturnDataW = _ussLib_.SendHTTPReturnDataW
    SendHTTPReturnDataW.argtypes = [c_uint64, c_wchar_p, c_uint]
    SendHTTPReturnDataW.restype = c_uint

    #const char* WINAPI GetHTTPId(USocket_Server_Handle h);
    GetHTTPId = _ussLib_.GetHTTPId
    GetHTTPId.argtypes = [c_uint64]
    GetHTTPId.restype = c_char_p

    #unsigned int WINAPI GetHTTPCurrentMultiplaxHeaders(USocket_Server_Handle h, SPA::ServerSide::CHttpHeaderValue *HeaderValue, unsigned int count);
    GetHTTPCurrentMultiplaxHeaders = _ussLib_.GetHTTPCurrentMultiplaxHeaders
    GetHTTPCurrentMultiplaxHeaders.argtypes = [c_uint64, POINTER(CHttpHV), c_uint]
    GetHTTPCurrentMultiplaxHeaders.restype = c_uint

    #void* WINAPI GetSSL(USocket_Server_Handle h);
    GetSSL = _ussLib_.GetSSL
    GetSSL.argtypes = [c_uint64]
    GetSSL.restype = c_void_p

    #bool WINAPI GetReturnRandom(unsigned int serviceId);
    GetReturnRandom = _ussLib_.GetReturnRandom
    GetReturnRandom.argtypes = [c_uint]
    GetReturnRandom.restype = c_bool

    #void WINAPI SetReturnRandom(unsigned int serviceId, bool random);
    SetReturnRandom = _ussLib_.SetReturnRandom
    SetReturnRandom.argtypes = [c_uint, c_bool]
    SetReturnRandom.restype = None

    #unsigned int WINAPI GetSwitchTime();
    GetSwitchTime = _ussLib_.GetSwitchTime
    GetSwitchTime.argtypes = []
    GetSwitchTime.restype = c_uint

    #void WINAPI SetSwitchTime(unsigned int switchTime);
    SetSwitchTime = _ussLib_.SetSwitchTime
    SetSwitchTime.argtypes = [c_uint]
    SetSwitchTime.restype = None

    #unsigned int WINAPI GetCountOfClients();
    GetCountOfClients = _ussLib_.GetCountOfClients
    GetCountOfClients.argtypes = []
    GetCountOfClients.restype = c_uint

    #USocket_Server_Handle WINAPI GetClient(unsigned int index);
    GetClient = _ussLib_.GetClient
    GetClient.argtypes = [c_uint]
    GetClient.restype = c_uint64

    #void WINAPI SetDefaultZip(bool zip);
    SetDefaultZip = _ussLib_.SetDefaultZip
    SetDefaultZip.argtypes = [c_bool]
    SetDefaultZip.restype = None

    #bool WINAPI GetDefaultZip();
    GetDefaultZip = _ussLib_.GetDefaultZip
    GetDefaultZip.argtypes = []
    GetDefaultZip.restype = c_bool

    #void WINAPI SetMaxConnectionsPerClient(unsigned int maxConnectionsPerClient);
    SetMaxConnectionsPerClient = _ussLib_.SetMaxConnectionsPerClient
    SetMaxConnectionsPerClient.argtypes = [c_uint]
    SetMaxConnectionsPerClient.restype = None

    #unsigned int WINAPI GetMaxConnectionsPerClient();
    GetMaxConnectionsPerClient = _ussLib_.GetMaxConnectionsPerClient
    GetMaxConnectionsPerClient.argtypes = []
    GetMaxConnectionsPerClient.restype = c_uint

    #void WINAPI SetMaxThreadIdleTimeBeforeSuicide(unsigned int maxThreadIdleTimeBeforeSuicide);
    SetMaxThreadIdleTimeBeforeSuicide = _ussLib_.SetMaxThreadIdleTimeBeforeSuicide
    SetMaxThreadIdleTimeBeforeSuicide.argtypes = [c_uint]
    SetMaxThreadIdleTimeBeforeSuicide.restype = None

    #unsigned int WINAPI GetMaxThreadIdleTimeBeforeSuicide();
    GetMaxThreadIdleTimeBeforeSuicide = _ussLib_.GetMaxThreadIdleTimeBeforeSuicide
    GetMaxThreadIdleTimeBeforeSuicide.argtypes = []
    GetMaxThreadIdleTimeBeforeSuicide.restype = c_uint

    #void WINAPI SetTimerElapse(unsigned int timerElapse);
    SetTimerElapse = _ussLib_.SetTimerElapse
    SetTimerElapse.argtypes = [c_uint]
    SetTimerElapse.restype = None

    #unsigned int WINAPI GetTimerElapse();
    GetTimerElapse = _ussLib_.GetTimerElapse
    GetTimerElapse.argtypes = []
    GetTimerElapse.restype = c_uint

    #unsigned int WINAPI GetSMInterval();
    GetSMInterval = _ussLib_.GetSMInterval
    GetSMInterval.argtypes = []
    GetSMInterval.restype = c_uint

    #void WINAPI SetSMInterval(unsigned int SMInterval);
    SetSMInterval = _ussLib_.SetSMInterval
    SetSMInterval.argtypes = [c_uint]
    SetSMInterval.restype = None

    #void WINAPI SetPingInterval(unsigned int pingInterval);
    SetPingInterval = _ussLib_.SetPingInterval
    SetPingInterval.argtypes = [c_uint]
    SetPingInterval.restype = None

    #unsigned int WINAPI GetPingInterval();
    GetPingInterval = _ussLib_.GetPingInterval
    GetPingInterval.argtypes = []
    GetPingInterval.restype = c_uint

    #void WINAPI SetRecycleGlobalMemoryInterval(unsigned int recycleGlobalMemoryInterval);
    SetRecycleGlobalMemoryInterval = _ussLib_.SetRecycleGlobalMemoryInterval
    SetRecycleGlobalMemoryInterval.argtypes = [c_uint]
    SetRecycleGlobalMemoryInterval.restype = None

    #unsigned int WINAPI GetRecycleGlobalMemoryInterval();
    GetRecycleGlobalMemoryInterval = _ussLib_.GetRecycleGlobalMemoryInterval
    GetRecycleGlobalMemoryInterval.argtypes = []
    GetRecycleGlobalMemoryInterval.restype = c_uint

    #SPA::UINT64 WINAPI GetRequestCount();
    GetRequestCount = _ussLib_.GetRequestCount
    GetRequestCount.argtypes = []
    GetRequestCount.restype = c_uint64

    #unsigned int WINAPI StartHTTPChunkResponse(USocket_Server_Handle h);
    StartHTTPChunkResponse = _ussLib_.StartHTTPChunkResponse
    StartHTTPChunkResponse.argtypes = [c_uint64]
    StartHTTPChunkResponse.restype = c_uint

    #unsigned int WINAPI SendHTTPChunk(USocket_Server_Handle h, const unsigned char *buffer, unsigned int len);
    SendHTTPChunk = _ussLib_.SendHTTPChunk
    SendHTTPChunk.argtypes = [c_uint64, POINTER(c_ubyte), c_uint]
    SendHTTPChunk.restype = c_uint

    #unsigned int WINAPI EndHTTPChunkResponse(USocket_Server_Handle h, const unsigned char *buffer, unsigned int len);
    EndHTTPChunkResponse = _ussLib_.EndHTTPChunkResponse
    EndHTTPChunkResponse.argtypes = [c_uint64, POINTER(c_ubyte), c_uint]
    EndHTTPChunkResponse.restype = c_uint

    #bool WINAPI IsFakeRequest(USocket_Server_Handle h);
    IsFakeRequest = _ussLib_.IsFakeRequest
    IsFakeRequest.argtypes = [c_uint64]
    IsFakeRequest.restype = c_bool

    #bool WINAPI SetZip(USocket_Server_Handle h, bool bZip);
    SetZip = _ussLib_.SetZip
    SetZip.argtypes = [c_uint64, c_bool]
    SetZip.restype = c_bool

    #bool WINAPI GetZip(USocket_Server_Handle h);
    GetZip = _ussLib_.GetZip
    GetZip.argtypes = [c_uint64]
    GetZip.restype = c_bool

    #void WINAPI SetZipLevel(USocket_Server_Handle h, SPA::tagZipLevel zl);
    SetZipLevel = _ussLib_.SetZipLevel
    SetZipLevel.argtypes = [c_uint64, c_int]
    SetZipLevel.restype = None

    #SPA::tagZipLevel WINAPI GetZipLevel(USocket_Server_Handle h);
    GetZipLevel = _ussLib_.GetZipLevel
    GetZipLevel.argtypes = [c_uint64]
    GetZipLevel.restype = c_int

    #unsigned int WINAPI StartQueue(const char *qName, bool dequeueShared, unsigned int ttl);
    StartQueue = _ussLib_.StartQueue
    StartQueue.argtypes = [c_char_p, c_bool, c_uint]
    StartQueue.restype = c_uint

    #unsigned int WINAPI GetMessagesInDequeuing(unsigned int qHandle);
    GetMessagesInDequeuing = _ussLib_.GetMessagesInDequeuing
    GetMessagesInDequeuing.argtypes = [c_uint]
    GetMessagesInDequeuing.restype = c_uint

    #SPA::UINT64 WINAPI Enqueue(unsigned int qHandle, unsigned short reqId, const unsigned char *buffer, unsigned int size);
    Enqueue = _ussLib_.Enqueue
    Enqueue.argtypes = [c_uint, c_ushort, POINTER(c_ubyte), c_uint]
    Enqueue.restype = c_uint64

    #SPA::UINT64 WINAPI GetMessageCount(unsigned int qHandle);
    GetMessageCount = _ussLib_.GetMessageCount
    GetMessageCount.argtypes = [c_uint]
    GetMessageCount.restype = c_uint64

    #bool WINAPI StopQueueByHandle(unsigned int qHandle, bool permanent);
    StopQueueByHandle = _ussLib_.StopQueueByHandle
    StopQueueByHandle.argtypes = [c_uint, c_bool]
    StopQueueByHandle.restype = c_bool

    #bool WINAPI StopQueueByName(const char *qName, bool permanent);
    StopQueueByName = _ussLib_.StopQueueByName
    StopQueueByName.argtypes = [c_char_p, c_bool]
    StopQueueByName.restype = c_bool

    #SPA::UINT64 WINAPI GetQueueSize(unsigned int qHandle);
    GetQueueSize = _ussLib_.GetQueueSize
    GetQueueSize.argtypes = [c_uint]
    GetQueueSize.restype = c_uint64

    #SPA::UINT64 WINAPI Dequeue(unsigned int qHandle, USocket_Server_Handle h, unsigned int messageCount, bool beNotifiedWhenAvailable, unsigned int waitTime);
    Dequeue = _ussLib_.Dequeue
    Dequeue.argtypes = [c_uint, c_uint64, c_uint, c_bool, c_uint]
    Dequeue.restype = c_uint64

    #bool WINAPI IsQueueStartedByName(const char *qName);
    IsQueueStartedByName = _ussLib_.IsQueueStartedByName
    IsQueueStartedByName.argtypes = [c_char_p]
    IsQueueStartedByName.restype = c_bool

    #bool WINAPI IsQueueStartedByHandle(unsigned int qHandle);
    IsQueueStartedByHandle = _ussLib_.IsQueueStartedByHandle
    IsQueueStartedByHandle.argtypes = [c_uint]
    IsQueueStartedByHandle.restype = c_bool

    #bool WINAPI IsQueueSecuredByName(const char *qName);
    IsQueueSecuredByName = _ussLib_.IsQueueSecuredByName
    IsQueueSecuredByName.argtypes = [c_char_p]
    IsQueueSecuredByName.restype = c_bool

    #bool WINAPI IsQueueSecuredByHandle(unsigned int qHandle);
    IsQueueSecuredByHandle = _ussLib_.IsQueueSecuredByHandle
    IsQueueSecuredByHandle.argtypes = [c_uint]
    IsQueueSecuredByHandle.restype = c_bool

    #const char* WINAPI GetQueueName(unsigned int qHandle);
    GetQueueName = _ussLib_.GetQueueName
    GetQueueName.argtypes = [c_uint]
    GetQueueName.restype = c_char_p

    #const char* WINAPI GetQueueFileName(unsigned int qHandle);
    GetQueueFileName = _ussLib_.GetQueueFileName
    GetQueueFileName.argtypes = [c_uint]
    GetQueueFileName.restype = c_char_p

    #SPA::UINT64 WINAPI Dequeue2(unsigned int qHandle, USocket_Server_Handle h, unsigned int maxBytes, bool beNotifiedWhenAvailable, unsigned int waitTime);
    Dequeue2 = _ussLib_.Dequeue2
    Dequeue2.argtypes = [c_uint, c_uint64, c_uint, c_bool, c_uint]
    Dequeue2.restype = c_uint64

    #void WINAPI EnableClientDequeue(USocket_Server_Handle h, bool enable);
    EnableClientDequeue = _ussLib_.EnableClientDequeue
    EnableClientDequeue.argtypes = [c_uint64, c_bool]
    EnableClientDequeue.restype = None

    #bool WINAPI IsDequeueRequest(USocket_Server_Handle h);
    IsDequeueRequest = _ussLib_.IsDequeueRequest
    IsDequeueRequest.argtypes = [c_uint64]
    IsDequeueRequest.restype = c_bool

    #bool WINAPI AbortJob(unsigned int qHandle);
    AbortJob = _ussLib_.AbortJob
    AbortJob.argtypes = [c_uint]
    AbortJob.restype = c_bool

    #bool WINAPI StartJob(unsigned int qHandle);
    StartJob = _ussLib_.StartJob
    StartJob.argtypes = [c_uint]
    StartJob.restype = c_bool

    #bool WINAPI EndJob(unsigned int qHandle);
    EndJob = _ussLib_.EndJob
    EndJob.argtypes = [c_uint]
    EndJob.restype = c_bool

    #SPA::UINT64 WINAPI GetJobSize(unsigned int qHandle);
    GetJobSize = _ussLib_.GetJobSize
    GetJobSize.argtypes = [c_uint]
    GetJobSize.restype = c_uint64

    #bool WINAPI SetRouting(unsigned int serviceId0, SPA::ServerSide::tagRoutingAlgorithm ra0, unsigned int serviceId1, SPA::ServerSide::tagRoutingAlgorithm ra1);
    SetRouting = _ussLib_.SetRouting
    SetRouting.argtypes = [c_uint, c_int, c_uint, c_int]
    SetRouting.restype = c_bool

    #unsigned int WINAPI CheckRouting(unsigned int serviceId);
    CheckRoutin = _ussLib_.CheckRouting
    CheckRoutin.argtypes = [c_uint]
    CheckRoutin.restype = c_uint

    #bool WINAPI AddAlphaRequest(unsigned int serviceId, unsigned short reqId);
    AddAlphaRequest = _ussLib_.AddAlphaRequest
    AddAlphaRequest.argtypes = [c_uint, c_ushort]
    AddAlphaRequest.restype = c_bool

    #unsigned int WINAPI GetAlphaRequestIds(unsigned int serviceId, unsigned short *reqIds, unsigned int count);
    GetAlphaRequestIds = _ussLib_.GetAlphaRequestIds
    GetAlphaRequestIds.argtypes = [c_uint, POINTER(c_ushort), c_uint]
    GetAlphaRequestIds.restype = c_uint

    #SPA::UINT64 WINAPI GetQueueLastIndex(unsigned int qHandle);
    GetQueueLastIndex = _ussLib_.GetQueueLastIndex
    GetQueueLastIndex.argtypes = [c_uint]
    GetQueueLastIndex.restype = c_uint64

    #void WINAPI UseUTF16();
    UseUTF16 = _ussLib_.UseUTF16
    UseUTF16.argtypes = []
    UseUTF16.restype = None

    #SPA::UINT64 WINAPI CancelQueuedRequestsByIndex(unsigned int qHandle, SPA::UINT64 startIndex, SPA::UINT64 endIndex);
    CancelQueuedRequestsByIndex = _ussLib_.CancelQueuedRequestsByIndex
    CancelQueuedRequestsByIndex.argtypes = [c_uint, c_uint64, c_uint64]
    CancelQueuedRequestsByIndex.restype = c_uint64

    #bool WINAPI IsDequeueShared(unsigned int qHandle);
    IsDequeueShared = _ussLib_.IsDequeueShared
    IsDequeueShared.argtypes = [c_uint]
    IsDequeueShared.restype = c_bool

    #SPA::tagQueueStatus WINAPI GetServerQueueStatus(unsigned int qHandle);
    GetServerQueueStatus = _ussLib_.GetServerQueueStatus
    GetServerQueueStatus.argtypes = [c_uint]
    GetServerQueueStatus.restype = c_int

    #bool WINAPI PushQueueTo(unsigned int srcHandle, const unsigned int *targetHandles, unsigned int count);
    PushQueueTo = _ussLib_.PushQueueTo
    PushQueueTo.argtypes = [c_uint, POINTER(c_uint), c_uint]
    PushQueueTo.restype = c_bool

    #unsigned int WINAPI GetTTL(unsigned int qHandle);
    GetTTL = _ussLib_.GetTTL
    GetTTL.argtypes = [c_uint]
    GetTTL.restype = c_uint

    #SPA::UINT64 WINAPI RemoveQueuedRequestsByTTL(unsigned int qHandle);
    RemoveQueuedRequestsByTTL = _ussLib_.RemoveQueuedRequestsByTTL
    RemoveQueuedRequestsByTTL.argtypes = [c_uint]
    RemoveQueuedRequestsByTTL.restype = c_uint64

    #void WINAPI ResetQueue(unsigned int qHandle);
    ResetQueue = _ussLib_.ResetQueue
    ResetQueue.argtypes = [c_uint]
    ResetQueue.restype = None

    #bool WINAPI IsServerQueueIndexPossiblyCrashed();
    IsServerQueueIndexPossiblyCrashed = _ussLib_.IsServerQueueIndexPossiblyCrashed
    IsServerQueueIndexPossiblyCrashed.argtypes = []
    IsServerQueueIndexPossiblyCrashed.restype = c_bool

    #void WINAPI SetServerWorkDirectory(const char *dir);
    SetServerWorkDirectory = _ussLib_.SetServerWorkDirectory
    SetServerWorkDirectory.argtypes = [c_char_p]
    SetServerWorkDirectory.restype = None

    #const char* WINAPI GetServerWorkDirectory();
    GetServerWorkDirectory = _ussLib_.GetServerWorkDirectory
    GetServerWorkDirectory.argtypes = []
    GetServerWorkDirectory.restype = c_char_p

    #SPA::UINT64 WINAPI GetLastQueueMessageTime(unsigned int qHandle);
    GetLastQueueMessageTime = _ussLib_.GetLastQueueMessageTime
    GetLastQueueMessageTime.argtypes = [c_uint]
    GetLastQueueMessageTime.restype = c_uint64

    #void WINAPI AbortDequeuedMessage(USocket_Server_Handle h);
    AbortDequeuedMessage = _ussLib_.GetMessagesInDequeuing
    AbortDequeuedMessage.argtypes = [c_uint64]
    AbortDequeuedMessage.restype = None

    #bool WINAPI IsDequeuedMessageAborted(USocket_Server_Handle h);
    IsDequeuedMessageAborted = _ussLib_.IsDequeuedMessageAborted
    IsDequeuedMessageAborted.argtypes = [c_uint64]
    IsDequeuedMessageAborted.restype = c_bool

    #const char* WINAPI GetUServerSocketVersion();
    GetUServerSocketVersion = _ussLib_.GetUServerSocketVersion
    GetUServerSocketVersion.argtypes = []
    GetUServerSocketVersion.restype = c_char_p

    #void WINAPI SetMessageQueuePassword(const char *pwd);
    SetMessageQueuePassword = _ussLib_.SetMessageQueuePassword
    SetMessageQueuePassword.argtypes = [c_char_p]
    SetMessageQueuePassword.restype = None

    """
    enum tagOptimistic {
        oMemoryCached = 0,
        oSystemMemoryCached = 1,
        oDiskCommitted = 2
    };
    """
    #SPA::tagOptimistic WINAPI GetOptimistic(unsigned int qHandle);
    GetOptimistic = _ussLib_.GetOptimistic
    GetOptimistic.argtypes = [c_uint]
    GetOptimistic.restype = c_int

    # void WINAPI SetOptimistic(unsigned int qHandle, SPA::tagOptimistic optimistic);
    SetOptimistic = _ussLib_.SetOptimistic
    SetOptimistic.argtypes = [c_uint, c_int]
    SetOptimistic.restype = None

    #void WINAPI SetLastCallInfo(const char *str);
    SetLastCallInfo = _ussLib_.SetLastCallInfo
    SetLastCallInfo.argtypes = [c_char_p]
    SetLastCallInfo.restype = None

    #unsigned int WINAPI GetMainThreads();
    GetMainThreads = _ussLib_.GetMainThreads
    GetMainThreads.argtypes = []
    GetMainThreads.restype = c_uint

    #unsigned int WINAPI SendReturnDataIndex(USocket_Server_Handle h, SPA::UINT64 index, unsigned short usReqId, unsigned int ulBufferSize, const unsigned char *pBuffer);
    SendReturnDataIndex = _ussLib_.SendReturnDataIndex
    SendReturnDataIndex.argtypes = [c_uint64, c_uint64, c_ushort, c_uint, POINTER(c_ubyte)]
    SendReturnDataIndex.restype = c_uint

    #unsigned int WINAPI SendExceptionResultIndex(USocket_Server_Handle h, SPA::UINT64 index, const wchar_t* errMessage, const char* errWhere, unsigned short requestId, unsigned int errCode);
    SendExceptionResultIndex = _ussLib_.SendExceptionResultIndex
    SendExceptionResultIndex.argtypes = [c_uint64, c_uint64, c_wchar_p, c_char_p, c_ushort, c_uint]
    SendExceptionResultIndex.restype = c_uint

    #SPA::UINT64 WINAPI GetCurrentRequestIndex(USocket_Server_Handle h);
    GetCurrentRequestIndex = _ussLib_.GetCurrentRequestIndex
    GetCurrentRequestIndex.argtypes = [c_uint64]
    GetCurrentRequestIndex.restype = c_uint64

class CSvsContext(Structure):
    _fields_ = [("m_ta", c_int), #required with a worker thread only on window platforms
                ("m_OnSwitchTo", SCoreLoader.POnSwitchTo),
                ("m_OnRequestArrive", SCoreLoader.POnRequestArrive),
                ("m_OnFastRequestArrive", SCoreLoader.POnFastRequestArrive),
                ("m_OnBaseRequestCame", SCoreLoader.POnBaseRequestCame),
                ("m_OnRequestProcessed", SCoreLoader.POnRequestProcessed), #called when a slow request processed
                ("m_OnClose", SCoreLoader.POnClose),
                #called within worker thread
                ("m_SlowProcess", SCoreLoader.PSLOW_PROCESS), #required with a worker thread
                ("m_OnChatRequestComing", SCoreLoader.POnChatRequestComing),
                ("m_OnChatRequestCame", SCoreLoader.POnChatRequestCame),
                ("m_OnResultsSent", SCoreLoader.POnResultsSent),
                ("m_OnHttpAuthentication", SCoreLoader.POnHttpAuthentication)] #HttpAuthentication

class CConfigImpl(object):
    @property
    def MaxThreadIdleTimeBeforeSuicide(self):
        return SCoreLoader.GetMaxThreadIdleTimeBeforeSuicide()

    @MaxThreadIdleTimeBeforeSuicide.setter
    def MaxThreadIdleTimeBeforeSuicide(self, value):
        SCoreLoader.SetMaxThreadIdleTimeBeforeSuicide(value)

    @property
    def MaxConnectionsPerClient(self):
        return SCoreLoader.GetMaxConnectionsPerClient()

    @MaxConnectionsPerClient.setter
    def MaxConnectionsPerClient(self, value):
        SCoreLoader.SetMaxConnectionsPerClient(value)

    @property
    def TimerElapse(self):
        return SCoreLoader.GetTimerElapse()

    @TimerElapse.setter
    def TimerElapse(self, value):
        SCoreLoader.SetTimerElapse(value)

    @property
    def SMInterval(self):
        return SCoreLoader.GetSMInterval()

    @SMInterval.setter
    def SMInterval(self, value):
        SCoreLoader.SetSMInterval(value)

    @property
    def PingInterval(self):
        return SCoreLoader.GetPingInterval()

    @PingInterval.setter
    def PingInterval(self, value):
        SCoreLoader.SetPingInterval(value)

    @property
    def DefaultZip(self):
        return SCoreLoader.GetDefaultZip()

    @DefaultZip.setter
    def DefaultZip(self, value):
        SCoreLoader.SetDefaultZip(value)

    @property
    def DefaultEncryptionMethod(self):
        return SCoreLoader.GetDefaultEncryptionMethod()

    @DefaultEncryptionMethod.setter
    def DefaultEncryptionMethod(self, value):
        SCoreLoader.SetDefaultEncryptionMethod(value)

    @property
    def SwitchTime(self):
        return SCoreLoader.GetSwitchTime()

    @SwitchTime.setter
    def SwitchTime(self, value):
        SCoreLoader.SetSwitchTime(value)

    @property
    def AuthenticationMethod(self):
        return SCoreLoader.GetAuthenticationMethod()

    @AuthenticationMethod.setter
    def AuthenticationMethod(self, value):
        SCoreLoader.SetAuthenticationMethod(value)

    @property
    def SharedAM(self):
        return SCoreLoader.GetSharedAM()

    @SharedAM.setter
    def SharedAM(self, value):
        SCoreLoader.SetSharedAM(value)

    @property
    def MainThreads(self):
        return SCoreLoader.GetMainThreads()

    def _set_pwd_(self, value):
        if value is None:
            value = u''
        SCoreLoader.SetPKFPassword(value)
    Password = property(None, _set_pwd_)

class CQueueManagerImpl(object):
    @property
    def WorkDirectory(self):
        return SCoreLoader.GetServerWorkDirectory().decode('latin-1')

    @WorkDirectory.setter
    def WorkDirectory(self, value):
        SCoreLoader.SetServerWorkDirectory(value.encode('latin-1'))

    def _set_pwd_(self, value):
        if value is None:
            value = ''
        SCoreLoader.SetMessageQueuePassword(value.encode('latin-1'))
    MessageQueuePassword=property(None, _set_pwd_)

    def StartQueue(self, qName, ttl, dequeueShared=True):
        from spa.serverside.serverqueue import CServerQueue
        return CServerQueue(SCoreLoader.StartQueue(qName.encode('latin-1'), dequeueShared, ttl))

    def StopQueue(self, qName, permanent=False):
        return SCoreLoader.StopQueueByName(qName.encode('latin-1'), permanent)

    def IsQueueStarted(self, qName):
        return SCoreLoader.IsQueueStartedByName(qName)

    def IsQueueSecured(self, qName):
        return SCoreLoader.IsQueueSecuredByName(qName.encode('latin-1'))

    @property
    def IsServerQueueIndexPossiblyCrashed(self):
        return SCoreLoader.IsServerQueueIndexPossiblyCrashed()
