package gcs

import (
	"gspa"
	"syscall"
	"unsafe"
)

func (p *CSocketPool) startPool(maxSocketsPerThread uint32, threads uint16, avg int) bool {
	cbSPE := func(pid PoolId, spe SocketPoolEvent, handle USocket_Client_Handle) uintptr {
		var pos int
		var me *CAsyncHandler
		var event DelPool
		switch spe {
		case SpeStarted:
			p.Mutex.Lock()
			event = p.event
			p.poolId = pid
			p.handlers = make([]*CAsyncHandler, 0)
			p.Mutex.Unlock()
			break
		case SpeShutdown:
			p.Mutex.Lock()
			event = p.event
			p.poolId = 0
			p.Mutex.Unlock()
			break
		case SpeSocketCreated:
			{
				var msg Message
				var h CAsyncHandler
				var cq ClientQueue
				h.msg = &msg
				h.cq = &cq
				h.hSocket = handle
				h.pool = p
				msg.s = &h
				cq.s = &h
				h.setCallbacks()
				h.SetRecvTimeout(p.recvTimeout).SetConnTimeout(p.connTimeout).SetAutoConn(p.autoConn)
				me = &h
				p.Mutex.Lock()
				event = p.event
				p.handlers = append(p.handlers, me)
				p.Mutex.Unlock()
			}
			break
		case SpeSocketKilled:
			{
				p.Mutex.Lock()
				event = p.event
				n := schIndex(p.handlers, handle)
				if n != -1 {
					me = p.handlers[n]
					p.handlers = append(p.handlers[0:n], p.handlers[n+1:]...)
				}
				p.Mutex.Unlock()
			}
			break
		case SpeConnected:
			{
				p.Mutex.Lock()
				event = p.event
				pos = schIndex(p.handlers, handle)
				if pos != -1 {
					me = p.handlers[pos]
					if me.IsOpened() {
						me.SetSockOpt(gspa.RcvBuf, 116800, gspa.Socket)
						me.SetSockOpt(gspa.SndBuf, 116800, gspa.Socket)
						if p.sslAuth != nil && me.GetEncryptionMethod() == gspa.TLSv1 {
							if !p.sslAuth(me) {
								return 0
							}
						}
						text, _ := syscall.UTF16PtrFromString(me.connContext.Password)
						setPassword.Call(uintptr(handle), uintptr(unsafe.Pointer(text)))
						me.StartBatching()
						switchTo.Call(uintptr(handle), uintptr(p.serviceId))
						me.TurnOnZipAtSvr(me.connContext.Zip).SetSockOptAtSvr(gspa.RcvBuf, 116800, gspa.Socket)
						me.SetSockOptAtSvr(gspa.SndBuf, 116800, gspa.Socket)
						me.CommitBatching(false)
					}
				}
				p.Mutex.Unlock()
			}
			break
		default:
			p.Mutex.Lock()
			event = p.event
			if handle != 0 {
				pos = schIndex(p.handlers, handle)
				if pos != -1 {
					me = p.handlers[pos]
				}
			}
			p.Mutex.Unlock()
			if spe == SpeQueueMergedFrom {
				p.fromHandler = me
			} else if spe == SpeQueueMergedTo {
				p.fromHandler.appendTo(me)
				p.fromHandler = nil
			}
			break
		}
		if spe == SpeConnected && me.IsOpened() {
			p.setQueue(pos)
		}
		if event != nil {
			event(spe, me)
		}
		if spe == SpeSocketKilled && me.tie != nil {
			me.tie.Untie()
		}
		return 0
	}
	p.cbPool = syscall.NewCallback(cbSPE)
	id, _, _ := createSocketPool.Call(p.cbPool, uintptr(maxSocketsPerThread), uintptr(threads), uintptr(avg), uintptr(0))
	return id > 0
}

func (p *CSocketPool) ShutdownPool() bool {
	r, _, _ := destroySocketPool.Call(uintptr(p.GetPoolId()))
	return (byte(r) > 0)
}

func (p *CSocketPool) GetQueueAutoMerge() bool {
	r, _, _ := getQueueAutoMergeByPool.Call(uintptr(p.GetPoolId()))
	return (byte(r) > 0)
}

func (p *CSocketPool) SetQueueAutoMerge(am bool) *CSocketPool {
	var b uintptr
	if am {
		b = 1
	}
	setQueueAutoMergeByPool.Call(uintptr(p.GetPoolId()), b)
	return p
}

func (p *CSocketPool) IsAvg() bool {
	r, _, _ := isAvg.Call(uintptr(p.GetPoolId()))
	return (byte(r) > 0)
}

func (p *CSocketPool) IsStarted() bool {
	r, _, _ := getThreadCount.Call(uintptr(p.GetPoolId()))
	return (byte(r) > 0)
}

func (p *CSocketPool) DisconnectAll() bool {
	r, _, _ := disconnectAll.Call(uintptr(p.GetPoolId()))
	return (byte(r) > 0)
}

func (p *CSocketPool) GetThreadsCreated() uint32 {
	r, _, _ := getThreadCount.Call(uintptr(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetDisconnectedSockets() uint32 {
	r, _, _ := getDisconnectedSockets.Call(uintptr(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetSocketsPerThread() uint32 {
	r, _, _ := getSocketsPerThread.Call(uintptr(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetLockedSockets() uint32 {
	r, _, _ := getLockedSockets.Call(uintptr(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetIdleSockets() uint32 {
	r, _, _ := getIdleSockets.Call(uintptr(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetConnectedSockets() uint32 {
	r, _, _ := getConnectedSockets.Call(uintptr(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) Lock(timeout ...uint32) *CAsyncHandler {
	var samethread uintptr
	var forever uint32 = 0xffffffff
	if len(timeout) > 0 {
		forever = timeout[0]
	}
	h, _, _ := lockASocket.Call(uintptr(p.GetPoolId()), uintptr(forever), samethread)
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	for i, _ := range p.handlers {
		if uintptr(p.handlers[i].hSocket) == h {
			return p.handlers[i]
		}
	}
	return nil
}

func (p *CSocketPool) postProcess(acc []CConnectionContext) {
	var first uint16 = 1 //true
	for i, _ := range acc {
		handler := p.handlers[i]
		handler.connContext = acc[i]
		handler.SetUID(acc[i].UserId)
		handler.SetEncryptionMethod(acc[i].EncryptionMethod)
		h := uintptr(handler.hSocket)
		var v6 uintptr
		if acc[i].V6 {
			v6 = 1
		}
		var host uintptr
		var server []byte
		if len(acc[i].Host) > 0 {
			server = []byte(acc[i].Host)
			host = uintptr(unsafe.Pointer(&server[0]))
		}
		port := uintptr(acc[i].Port)
		if first == 1 {
			ok, _, _ := connect.Call(h, host, port, uintptr(first), v6)
			if ok < 1 {
				continue
			}
			if !handler.WaitAll() {
				continue
			}
			if handler.GetConnectionState() < Connected {
				continue
			}
			first = 0
		} else {
			connect.Call(h, host, port, uintptr(0), v6)
		}
	}
}

var (
	usocket                           = syscall.NewLazyDLL("usocket")
	createSocketPool                  = usocket.NewProc("CreateSocketPool")
	destroySocketPool                 = usocket.NewProc("DestroySocketPool")
	getNumberOfSocketPools            = usocket.NewProc("GetNumberOfSocketPoos")
	findAClosedSocket                 = usocket.NewProc("FindAClosedSocket")
	addOneThreadIntoPool              = usocket.NewProc("AddOneThreadIntoPool")
	getLockedSockets                  = usocket.NewProc("GetLockedSockets")
	getIdleSockets                    = usocket.NewProc("GetIdleSockets")
	getConnectedSockets               = usocket.NewProc("GetConnectedSockets")
	disconnectAll                     = usocket.NewProc("DisconnectAll")
	lockASocket                       = usocket.NewProc("LockASocket")
	unlockASocket                     = usocket.NewProc("UnlockASocket")
	getSocketsPerThread               = usocket.NewProc("GetSocketsPerThread")
	isAvg                             = usocket.NewProc("IsAvg")
	isBatching                        = usocket.NewProc("IsBatching")
	getDisconnectedSockets            = usocket.NewProc("GetDisconnectedSockets")
	getThreadCount                    = usocket.NewProc("GetThreadCount")
	isOpened                          = usocket.NewProc("IsOpened")
	setUserID                         = usocket.NewProc("SetUserID")
	getUID                            = usocket.NewProc("GetUID")
	setEncryptionMethod               = usocket.NewProc("SetEncryptionMethod")
	getEncryptionMethod               = usocket.NewProc("GetEncryptionMethod")
	waitAll                           = usocket.NewProc("WaitAll")
	connect                           = usocket.NewProc("Connect")
	getConnectionState                = usocket.NewProc("GetConnectionState")
	setRecvTimeout                    = usocket.NewProc("SetRecvTimeout")
	setConnTimeout                    = usocket.NewProc("SetConnTimeout")
	setAutoConn                       = usocket.NewProc("SetAutoConn")
	getRecvTimeout                    = usocket.NewProc("GetRecvTimeout")
	getConnTimeout                    = usocket.NewProc("GetConnTimeout")
	getAutoConn                       = usocket.NewProc("GetAutoConn")
	setSockOpt                        = usocket.NewProc("SetSockOpt")
	setSockOptAtSvr                   = usocket.NewProc("SetSockOptAtSvr")
	startBatching                     = usocket.NewProc("StartBatching")
	commitBatching                    = usocket.NewProc("CommitBatching")
	abortBatching                     = usocket.NewProc("AbortBatching")
	turnOnZipAtSvr                    = usocket.NewProc("TurnOnZipAtSvr")
	setZip                            = usocket.NewProc("SetZip")
	getZip                            = usocket.NewProc("GetZip")
	switchTo                          = usocket.NewProc("SwitchTo")
	setPassword                       = usocket.NewProc("SetPassword")
	setOnSocketClosed                 = usocket.NewProc("SetOnSocketClosed")
	setOnHandShakeCompleted           = usocket.NewProc("SetOnHandShakeCompleted")
	setOnSocketConnected              = usocket.NewProc("SetOnSocketConnected")
	setOnRequestProcessed             = usocket.NewProc("SetOnRequestProcessed")
	setOnAllRequestsProcessed         = usocket.NewProc("SetOnAllRequestsProcessed")
	setOnBaseRequestProcessed         = usocket.NewProc("SetOnBaseRequestProcessed")
	setOnServerException              = usocket.NewProc("SetOnServerException")
	setOnPostProcessing               = usocket.NewProc("SetOnPostProcessing")
	isRandom                          = usocket.NewProc("IsRandom")
	isRouting                         = usocket.NewProc("IsRouting")
	getPeerOs                         = usocket.NewProc("GetPeerOs")
	sendRequest                       = usocket.NewProc("SendRequest")
	retrieveResult                    = usocket.NewProc("RetrieveResult")
	getErrorCode                      = usocket.NewProc("GetErrorCode")
	getErrorMessage                   = usocket.NewProc("GetErrorMessage")
	getQueueAutoMergeByPool           = usocket.NewProc("GetQueueAutoMergeByPool")
	setQueueAutoMergeByPool           = usocket.NewProc("SetQueueAutoMergeByPool")
	postProcessing                    = usocket.NewProc("PostProcessing")
	sendInterruptRequest              = usocket.NewProc("SendInterruptRequest")
	getUClientAppName                 = usocket.NewProc("GetUClientAppName")
	getUClientSocketVersion           = usocket.NewProc("GetUClientSocketVersion")
	getClientWorkDirectory            = usocket.NewProc("GetClientWorkDirectory")
	setClientWorkDirectory            = usocket.NewProc("SetClientWorkDirectory")
	isClientQueueIndexPossiblyCrashed = usocket.NewProc("IsClientQueueIndexPossiblyCrashed")
	setMessageQueuePassword           = usocket.NewProc("SetMessageQueuePassword")
	setVerifyLocation                 = usocket.NewProc("SetVerifyLocation")
	setCertificateVerifyCallback      = usocket.NewProc("SetCertificateVerifyCallback")
	close                             = usocket.NewProc("Close")
	getCountOfRequestsQueued          = usocket.NewProc("GetCountOfRequestsQueued")
	getCurrentRequestID               = usocket.NewProc("GetCurrentRequestID")
	getCurrentResultSize              = usocket.NewProc("GetCurrentResultSize")
	cancel                            = usocket.NewProc("Cancel")
	getBytesInSendingBuffer           = usocket.NewProc("GetBytesInSendingBuffer")
	getBytesInReceivingBuffer         = usocket.NewProc("GetBytesInReceivingBuffer")
	getBytesBatched                   = usocket.NewProc("GetBytesBatched")
	getBytesReceived                  = usocket.NewProc("GetBytesReceived")
	getBytesSent                      = usocket.NewProc("GetBytesSent")
	enter                             = usocket.NewProc("Enter")
	exit                              = usocket.NewProc("Exit")
	speak                             = usocket.NewProc("Speak")
	speakEx                           = usocket.NewProc("SpeakEx")
	sendUserMessage                   = usocket.NewProc("SendUserMessage")
	sendUserMessageEx                 = usocket.NewProc("SendUserMessageEx")
	doEcho                            = usocket.NewProc("DoEcho")
	getPeerName                       = usocket.NewProc("GetPeerName")
	getServerPingTime                 = usocket.NewProc("GetServerPingTime")
	ignoreLastRequest                 = usocket.NewProc("IgnoreLastRequest")
	isRouteeRequest                   = usocket.NewProc("IsRouteeRequest")
	shutdown                          = usocket.NewProc("Shutdown")
	getZipLevel                       = usocket.NewProc("GetZipLevel")
	setZipLevel                       = usocket.NewProc("SetZipLevel")
	setZipLevelAtSvr                  = usocket.NewProc("SetZipLevelAtSvr")
	getRouteeCount                    = usocket.NewProc("GetRouteeCount")
	setOnEnter2                       = usocket.NewProc("SetOnEnter2")
	setOnExit2                        = usocket.NewProc("SetOnExit2")
	setOnSpeakEx2                     = usocket.NewProc("SetOnSpeakEx2")
	setOnSendUserMessageEx2           = usocket.NewProc("SetOnSendUserMessageEx2")
	setOnSendUserMessage2             = usocket.NewProc("SetOnSendUserMessage2")
	setOnSpeak2                       = usocket.NewProc("SetOnSpeak2")
	startQueue                        = usocket.NewProc("StartQueue")
	stopQueue                         = usocket.NewProc("StopQueue")
	getTTL                            = usocket.NewProc("GetTTL")
	removeQueuedRequestsByTTL         = usocket.NewProc("RemoveQueuedRequestsByTTL")
	getClientQueueStatus              = usocket.NewProc("GetClientQueueStatus")
	getLastQueueMessageTime           = usocket.NewProc("GetLastQueueMessageTime")
	getMessagesInDequeuing            = usocket.NewProc("GetMessagesInDequeuing")
	resetQueue                        = usocket.NewProc("ResetQueue")
	pushQueueTo                       = usocket.NewProc("PushQueueTo")
	abortJob                          = usocket.NewProc("AbortJob")
	startJob                          = usocket.NewProc("StartJob")
	endJob                            = usocket.NewProc("EndJob")
	getJobSize                        = usocket.NewProc("GetJobSize")
	getQueueLastIndex                 = usocket.NewProc("GetQueueLastIndex")
	cancelQueuedRequestsByIndex       = usocket.NewProc("CancelQueuedRequestsByIndex")
	isDequeueEnabled                  = usocket.NewProc("IsDequeueEnabled")
	isQueueStarted                    = usocket.NewProc("IsQueueStarted")
	isDequeueShared                   = usocket.NewProc("IsDequeueShared")
	getQueueFileName                  = usocket.NewProc("GetQueueFileName")
	getQueueName                      = usocket.NewProc("GetQueueName")
	isQueueSecured                    = usocket.NewProc("IsQueueSecured")
	getMessageCount                   = usocket.NewProc("GetMessageCount")
	getQueueSize                      = usocket.NewProc("GetQueueSize")
	enableRoutingQueueIndex           = usocket.NewProc("EnableRoutingQueueIndex")
	isRoutingQueueIndexEnabled        = usocket.NewProc("IsRoutingQueueIndexEnabled")
	getOptimistic                     = usocket.NewProc("GetOptimistic")
	setOptimistic                     = usocket.NewProc("SetOptimistic")
	getUCert						  = usocket.NewProc("GetUCert")
	verify							  = usocket.NewProc("Verify")
)

func GetVersion() string {
	r, _, _ := getUClientSocketVersion.Call()
	return utf8ToString(r)
}

func GetAppName() string {
	r, _, _ := getUClientAppName.Call()
	return utf8ToString(r)
}
