package gcs

import (
	"gspa"
	"internal/unsafeheader"
	"syscall"
	"unicode/utf16"
	"unsafe"

	"github.com/gammazero/deque"
)

func utf16ToString(p uintptr) string {
	if p == 0 {
		return ""
	}
	n := 0
	end := unsafe.Pointer(p)
	for *(*uint16)(end) != 0 {
		end = unsafe.Pointer(uintptr(end) + 2)
		n++
	}
	var s []uint16
	hdr := (*unsafeheader.Slice)(unsafe.Pointer(&s))
	hdr.Data = unsafe.Pointer(p)
	hdr.Cap = n
	hdr.Len = n
	return string(utf16.Decode(s))
}

func utf8ToString(p uintptr) string {
	if p == 0 {
		return ""
	}
	n := 0
	end := unsafe.Pointer(p)
	for *(*byte)(end) != 0 {
		end = unsafe.Pointer(uintptr(end) + 1)
		n++
	}
	var s []byte
	hdr := (*unsafeheader.Slice)(unsafe.Pointer(&s))
	hdr.Data = unsafe.Pointer(p)
	hdr.Cap = n
	hdr.Len = n
	return string(s)
}

func toBytes(p uintptr, count uint32) []byte {
	var s []byte
	hdr := (*unsafeheader.Slice)(unsafe.Pointer(&s))
	hdr.Data = unsafe.Pointer(p)
	hdr.Cap = int(count)
	hdr.Len = int(count)
	return s
}

type cMessageSender struct {
	userId      uintptr
	ipAddress   uintptr
	port        uint16
	serviceId   uint32
	selfMessage bool
}

func toMS(pms uintptr) *MessageSender {
	var ms MessageSender
	cms := (*cMessageSender)(unsafe.Pointer(pms))
	ms.UserId = utf16ToString(cms.userId)
	ms.IpAddr = utf8ToString(cms.ipAddress)
	ms.Port = cms.port
	ms.ServiceId = cms.serviceId
	ms.Self = cms.selfMessage
	return &ms
}

func toGroups(groups uintptr, count uint32) []uint32 {
	grps := make([]uint32, count)
	for count > 0 {
		count--
		grps[count] = *(*uint32)(unsafe.Pointer(groups + uintptr(count*4)))
	}
	return grps
}

func (s *CAsyncHandler) setCallbacks() {
	s.callBacks = make([]uintptr, 14)

	cbEnter := func(handle USocket_Client_Handle, pms uintptr, groups uintptr, count uint32) uintptr {
		s.cs.Lock()
		cb := s.msg.subscribe
		s.cs.Unlock()
		if cb != nil {
			cb(toMS(pms), toGroups(groups, count))
		}
		return 0
	}
	s.callBacks[8] = syscall.NewCallback(cbEnter)
	setOnEnter2.Call(uintptr(s.hSocket), s.callBacks[8])

	cbExit := func(handle USocket_Client_Handle, pms uintptr, groups uintptr, count uint32) uintptr {
		s.cs.Lock()
		cb := s.msg.unsubscribe
		s.cs.Unlock()
		if cb != nil {
			cb(toMS(pms), toGroups(groups, count))
		}
		return 0
	}
	s.callBacks[9] = syscall.NewCallback(cbExit)
	setOnExit2.Call(uintptr(s.hSocket), s.callBacks[9])

	cbSpeak := func(handle USocket_Client_Handle, pms uintptr, groups uintptr, count uint32, msg uintptr, bytes uint32) uintptr {
		s.cs.Lock()
		cb := s.msg.publish
		s.cs.Unlock()
		if cb != nil {
			var gb gspa.CUQueue
			gb.SetBuffer(toBytes(msg, bytes), bytes)
			cb(toMS(pms), toGroups(groups, count), gb.LoadObject())
		}
		return 0
	}
	s.callBacks[10] = syscall.NewCallback(cbSpeak)
	setOnSpeak2.Call(uintptr(s.hSocket), s.callBacks[10])

	cbUserMessage := func(handle USocket_Client_Handle, pms uintptr, msg uintptr, bytes uint32) uintptr {
		s.cs.Lock()
		cb := s.msg.userMessage
		s.cs.Unlock()
		if cb != nil {
			var gb gspa.CUQueue
			gb.SetBuffer(toBytes(msg, bytes), bytes)
			cb(toMS(pms), gb.LoadObject())
		}
		return 0
	}
	s.callBacks[11] = syscall.NewCallback(cbUserMessage)
	setOnSendUserMessage2.Call(uintptr(s.hSocket), s.callBacks[11])

	cbUserMessageEx := func(handle USocket_Client_Handle, pms uintptr, msg uintptr, bytes uint32) uintptr {
		s.cs.Lock()
		cb := s.msg.userMessageEx
		s.cs.Unlock()
		if cb != nil {
			cb(toMS(pms), toBytes(msg, bytes))
		}
		return 0
	}
	s.callBacks[12] = syscall.NewCallback(cbUserMessageEx)
	setOnSendUserMessageEx2.Call(uintptr(s.hSocket), s.callBacks[12])

	cbSpeakEx := func(handle USocket_Client_Handle, pms uintptr, groups uintptr, count uint32, msg uintptr, bytes uint32) uintptr {
		s.cs.Lock()
		cb := s.msg.publishEx
		s.cs.Unlock()
		if cb != nil {
			cb(toMS(pms), toGroups(groups, count), toBytes(msg, bytes))
		}
		return 0
	}
	s.callBacks[13] = syscall.NewCallback(cbSpeakEx)
	setOnSpeakEx2.Call(uintptr(s.hSocket), s.callBacks[13])

	cbSocketClosed := func(handle USocket_Client_Handle, ec int32) uintptr {
		s.cs.Lock()
		cbs := s.socketClosed
		s.ci = nil
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(ec)
		}
		s.CleanCallbacks(false)
		return 0
	}
	s.callBacks[0] = syscall.NewCallback(cbSocketClosed)
	setOnSocketClosed.Call(uintptr(s.hSocket), s.callBacks[0])

	cbConnected := func(handle USocket_Client_Handle, ec int32) uintptr {
		s.cs.Lock()
		cbs := s.socketConnected
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(ec)
		}
		return 0
	}
	s.callBacks[1] = syscall.NewCallback(cbConnected)
	setOnSocketConnected.Call(uintptr(s.hSocket), s.callBacks[1])

	cbSslCompleted := func(handle USocket_Client_Handle, ec int32) uintptr {
		s.cs.Lock()
		cbs := s.handshakeCompleted
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(ec)
		}
		return 0
	}
	s.callBacks[2] = syscall.NewCallback(cbSslCompleted)
	setOnHandShakeCompleted.Call(uintptr(s.hSocket), s.callBacks[2])

	cbRequestProcessed := func(handle USocket_Client_Handle, reqId gspa.ReqId, len uint32) uintptr {
		if len > s.buffer.GetMaxSize() {
			s.buffer.Realloc(len)
		}
		s.buffer.SetSize(0)
		if s.IsRouting() {
			os, _, _ := getPeerOs.Call(uintptr(handle), uintptr(0))
			s.buffer.SetOS(gspa.OperationSystem(os))
		}
		if len > 0 {
			if len > s.buffer.GetMaxSize() {
				s.buffer.Realloc(len)
			}
			ret, _, _ := retrieveResult.Call(uintptr(handle), uintptr(s.buffer.GetBuffer()), uintptr(len))
			s.buffer.SetSize(uint32(ret))
		}
		rcb := s.getARH(reqId)
		if rcb.reqId > 0 && rcb.rh != nil {
			ar := CAsyncResult{s, reqId, &s.buffer, rcb.rh}
			rcb.rh(&ar)
			s.buffer.SetSize(0)
			return 0
		}
		s.cs.Lock()
		cbs := s.requestProcessed
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(reqId, &s.buffer)
		}
		s.buffer.SetSize(0)
		return 0
	}
	s.callBacks[3] = syscall.NewCallback(cbRequestProcessed)
	setOnRequestProcessed.Call(uintptr(s.hSocket), s.callBacks[3])

	cbBaseRequestProcessed := func(handle USocket_Client_Handle, reqId gspa.ReqId) uintptr {
		h := uintptr(handle)
		os, _, _ := getPeerOs.Call(h, uintptr(0))
		random, _, _ := isRandom.Call(h)
		s.cs.Lock()
		cbs := s.baseReq
		s.os = gspa.OperationSystem(os)
		s.buffer.SetOS(s.os)
		s.random = (random > 0)
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(reqId)
		}
		if reqId == gspa.Cancel {
			s.CleanCallbacks(true)
		}
		return 0
	}
	s.callBacks[4] = syscall.NewCallback(cbBaseRequestProcessed)
	setOnBaseRequestProcessed.Call(uintptr(s.hSocket), s.callBacks[4])

	cbAllRequestsProcessed := func(handle USocket_Client_Handle, reqId gspa.ReqId) uintptr {
		s.cs.Lock()
		cbs := s.allProcessed
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(reqId)
		}
		return 0
	}
	s.callBacks[5] = syscall.NewCallback(cbAllRequestsProcessed)
	setOnAllRequestsProcessed.Call(uintptr(s.hSocket), s.callBacks[5])

	cbPostProcessing := func(handle USocket_Client_Handle, hint uint32, data uint64) uintptr {
		s.cs.Lock()
		cbs := s.postProcessing
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(hint, data)
		}

		return 0
	}
	s.callBacks[6] = syscall.NewCallback(cbPostProcessing)
	setOnPostProcessing.Call(uintptr(s.hSocket), s.callBacks[6])

	cbServerException := func(handle USocket_Client_Handle, reqId gspa.ReqId, errMessage uintptr, errWhere uintptr, ec int32) uintptr {
		err := utf16ToString(errMessage)
		where := utf8ToString(errWhere)
		rcb := s.getARH(reqId)
		if rcb.reqId > 0 && rcb.se != nil {
			rcb.se(s, reqId, ec, err, where)
		}
		s.cs.Lock()
		cbs := s.serverException
		s.cs.Unlock()
		for _, cb := range cbs {
			cb(reqId, ec, err, where)
		}
		return 0
	}
	s.callBacks[7] = syscall.NewCallback(cbServerException)
	setOnServerException.Call(uintptr(s.hSocket), s.callBacks[7])
}

func (s *CSocket) DoEcho() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := doEcho.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) GetPeerName() (string, uint32) {
	var port uint32
	var chars uint16 = 256
	addr := make([]byte, 256)
	h := uintptr(s.hSocket)
	pa := uintptr(unsafe.Pointer(&addr[0]))
	ret, _, _ := getPeerName.Call(h, uintptr(unsafe.Pointer(&port)), pa, uintptr(chars))
	if byte(ret) > 0 {
		return utf8ToString(pa), port
	}
	return "", 0
}

func (s *CSocket) GetServerPingTime() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getServerPingTime.Call(h)
	return uint32(ret)
}

func (s *CSocket) GetRouteeCount() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getRouteeCount.Call(h)
	return uint32(ret)
}

func (s *CAsyncHandler) IsRouteeRequest() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := isRouteeRequest.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) IgnoreLastRequest(reqId gspa.ReqId) bool {
	h := uintptr(s.hSocket)
	ret, _, _ := ignoreLastRequest.Call(h, uintptr(reqId))
	return byte(ret) > 0
}

func (s *CSocket) Shutdown(st ...gspa.ShutdownType) *CSocket {
	h := uintptr(s.hSocket)
	var t uintptr = uintptr(gspa.Both)
	if len(st) > 0 {
		t = uintptr(st[0])
	}
	shutdown.Call(h, t)
	return s
}

func (s *CSocket) GetZipLevel() gspa.ZipLevel {
	h := uintptr(s.hSocket)
	ret, _, _ := getZipLevel.Call(h)
	return gspa.ZipLevel(ret)
}

func (s *CSocket) SetZipLevel(zl gspa.ZipLevel) *CSocket {
	h := uintptr(s.hSocket)
	setZipLevel.Call(h, uintptr(zl))
	return s
}

func (s *CSocket) SetZipLevelAtSvr(zl gspa.ZipLevel) bool {
	h := uintptr(s.hSocket)
	ret, _, _ := setZipLevelAtSvr.Call(h, uintptr(zl))
	return byte(ret) > 0
}

func (s *CSocket) GetAutoConn() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := getAutoConn.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) IsOpened() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := isOpened.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) SetAutoConn(ac bool) *CSocket {
	h := uintptr(s.hSocket)
	var b uintptr
	if ac {
		b = 1
	}
	setAutoConn.Call(h, b)
	return s
}

func (s *CSocket) SetZip(zip bool) *CSocket {
	h := uintptr(s.hSocket)
	var b uintptr
	if zip {
		b = 1
	}
	setZip.Call(h, b)
	return s
}

func (s *CSocket) TurnOnZipAtSvr(zip bool) *CSocket {
	h := uintptr(s.hSocket)
	var b uintptr
	if zip {
		b = 1
	}
	turnOnZipAtSvr.Call(h, b)
	return s
}

func (s *CSocket) GetZip() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := getZip.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) IsBatching() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := isBatching.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) StartBatching() bool {
	h := uintptr(s.hSocket)
	ret, _, _ := startBatching.Call(h)
	return byte(ret) > 0
}

func (s *CAsyncHandler) CommitBatching(batchingAtServerSide ...bool) bool {
	h := uintptr(s.hSocket)
	var b uintptr
	if len(batchingAtServerSide) > 0 && batchingAtServerSide[0] {
		b = 1
	}
	s.cs.Lock()
	for s.qBatching.Len() > 0 {
		s.qCallback.PushBack(s.qBatching.PopFront())
	}
	s.cs.Unlock()
	ret, _, _ := commitBatching.Call(h, b)
	return byte(ret) > 0
}

func (s *CAsyncHandler) AbortBatching() bool {
	h := uintptr(s.hSocket)
	s.cs.Lock()
	s.qBatching = deque.Deque[cresultCb]{}
	s.cs.Unlock()
	ret, _, _ := abortBatching.Call(h)
	return byte(ret) > 0
}

func (s *CSocket) GetConnTimeout() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getConnTimeout.Call(h)
	return uint32(ret)
}

func (s *CSocket) SetConnTimeout(timeout uint32) *CSocket {
	h := uintptr(s.hSocket)
	setConnTimeout.Call(h, uintptr(timeout))
	return s
}

func (s *CSocket) GetRecvTimeout() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getRecvTimeout.Call(h)
	return uint32(ret)
}

func (s *CSocket) SetRecvTimeout(timeout uint32) *CSocket {
	h := uintptr(s.hSocket)
	setRecvTimeout.Call(h, uintptr(timeout))
	return s
}

func (s *CSocket) Close() *CSocket {
	h := uintptr(s.hSocket)
	close.Call(h)
	return s
}

func (s *CSocket) Cancel(requestsQueued ...uint32) *CSocket {
	var rqs uintptr = 0xffffffff
	if len(requestsQueued) > 0 {
		rqs = uintptr(requestsQueued[0])
	}
	h := uintptr(s.hSocket)
	cancel.Call(h, rqs)
	return s
}

func (s *CSocket) SetSockOpt(so gspa.SocketOption, val int32, sl gspa.SocketLevel) bool {
	h := uintptr(s.hSocket)
	ret, _, _ := setSockOpt.Call(h, uintptr(so), uintptr(val), uintptr(sl))
	return (byte(ret) > 0)
}

func (s *CSocket) SetSockOptAtSvr(so gspa.SocketOption, val int32, sl gspa.SocketLevel) bool {
	h := uintptr(s.hSocket)
	ret, _, _ := setSockOptAtSvr.Call(h, uintptr(so), uintptr(val), uintptr(sl))
	return (byte(ret) > 0)
}

func (s *CSocket) SetUID(uid string) *CSocket {
	h := uintptr(s.hSocket)
	text, _ := syscall.UTF16PtrFromString(uid)
	setUserID.Call(h, uintptr(unsafe.Pointer(text)))
	return s
}

func (s *CSocket) GetUID() string {
	h := uintptr(s.hSocket)
	uid := make([]uint16, 1024)
	ret, _, _ := getUID.Call(h, uintptr(unsafe.Pointer(&uid[0])), uintptr(1024))
	return string(utf16.Decode(uid[0:ret]))
}

func (s *CSocket) GetEncryptionMethod() gspa.EncryptionMethod {
	h := uintptr(s.hSocket)
	ret, _, _ := getEncryptionMethod.Call(h)
	return gspa.EncryptionMethod(ret)
}

func (s *CSocket) SetEncryptionMethod(em gspa.EncryptionMethod) *CSocket {
	h := uintptr(s.hSocket)
	setEncryptionMethod.Call(h, uintptr(em))
	return s
}

func (s *CSocket) GetConnectionState() ConnectionState {
	h := uintptr(s.hSocket)
	ret, _, _ := getConnectionState.Call(h)
	return ConnectionState(ret)
}

func (s *CSocket) GetCountOfRequestsQueued() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getCountOfRequestsQueued.Call(h)
	return uint32(ret)
}

func (s *CSocket) GetBytesInSendingBuffer() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getBytesInSendingBuffer.Call(h)
	return uint32(ret)
}

func (s *CSocket) GetBytesInReceivingBuffer() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getBytesInReceivingBuffer.Call(h)
	return uint32(ret)
}

func (s *CSocket) GetBytesBatched() uint32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getBytesBatched.Call(h)
	return uint32(ret)
}

func (s *CSocket) GetBytesReceived() uint64 {
	h := uintptr(s.hSocket)
	ret, _, _ := getBytesReceived.Call(h)
	return uint64(ret)
}

func (s *CSocket) GetBytesSent() uint64 {
	h := uintptr(s.hSocket)
	ret, _, _ := getBytesSent.Call(h)
	return uint64(ret)
}

func (s *CSocket) GetCurrentRequestID() gspa.ReqId {
	h := uintptr(s.hSocket)
	ret, _, _ := getCurrentRequestID.Call(h)
	return gspa.ReqId(ret)
}

func (s *CSocket) GetErrorCode() int32 {
	h := uintptr(s.hSocket)
	ret, _, _ := getErrorCode.Call(h)
	return int32(ret)
}

func (s *CSocket) GetErrorMsg() string {
	h := uintptr(s.hSocket)
	uid := make([]byte, 4096)
	ret, _, _ := getErrorMessage.Call(h, uintptr(unsafe.Pointer(&uid[0])), uintptr(4096))
	return string((uid[0:ret]))
}

func (s *CSocket) WaitAll(timeout ...uint32) bool {
	var forever uint32 = 0xffffffff
	if len(timeout) > 0 {
		forever = timeout[0]
	}
	h := uintptr(s.hSocket)
	ret, _, _ := waitAll.Call(h, uintptr(forever))
	return byte(ret) > 0
}

func (s *CSocket) Interrupt(hint uint64) bool {
	h := uintptr(s.hSocket)
	ret, _, _ := sendInterruptRequest.Call(h, uintptr(hint))
	return byte(ret) > 0
}

func (s *CAsyncHandler) PostProcessing(hint uint32, data uint64) *CAsyncHandler {
	h := uintptr(s.hSocket)
	postProcessing.Call(h, uintptr(hint), uintptr(data))
	return s
}

type cCertInfo struct {
	issuer       uintptr
	subject      uintptr
	notBefore    uintptr
	notAfter     uintptr
	validity     bool
	sigAlg       uintptr
	certPem      uintptr
	sessionInfo  uintptr
	pKSize       uint32
	publicKey    uintptr
	algSize      uint32
	algorithm    uintptr
	sNSize       uint32
	serialNumber uintptr
}

func (ci *CertInfo) Verify() (int32, string) {
	var ec int32
	h := uintptr(ci.s.hSocket)
	ret, _, _ := verify.Call(h, uintptr(unsafe.Pointer(&ec)))
	return ec, utf8ToString(ret)
}

func toCI(certInfo uintptr) *CertInfo {
	ci := new(CertInfo)
	cci := (*cCertInfo)(unsafe.Pointer(certInfo))
	ci.Issuer = utf8ToString(cci.issuer)
	ci.Subject = utf8ToString(cci.subject)
	ci.NotBefore = utf8ToString(cci.notBefore)
	ci.NotAfter = utf8ToString(cci.notAfter)
	ci.Validity = cci.validity
	ci.SigAlg = utf8ToString(cci.sigAlg)
	ci.CertPem = utf8ToString(cci.certPem)
	ci.SessionInfo = utf8ToString(cci.sessionInfo)
	ci.PublicKey = toBytes(cci.publicKey, cci.pKSize)
	ci.Algorithm = toBytes(cci.algorithm, cci.algSize)
	ci.SerialNumber = toBytes(cci.serialNumber, cci.sNSize)
	return ci
}

func (s *CSocket) GetCert() *CertInfo {
	h := uintptr(s.hSocket)
	ret, _, _ := getUCert.Call(h)
	if ret == 0 {
		s.ci = nil
	} else {
		ci := toCI(ret)
		ci.s = s
		s.ci = ci
	}
	return s.ci
}

func (ah *CAsyncHandler) SendRequest(reqId gspa.ReqId, buffer *gspa.CUQueue, rh DResultHandler, discarded DDiscarded, se DServerException) bool {
	h := uintptr(ah.hSocket)
	var p uintptr
	var bytes uintptr
	if buffer != nil {
		bytes = uintptr(buffer.GetSize())
		p = uintptr(buffer.GetBuffer())
	}
	batching := ah.IsBatching()
	ah.csSend.Lock()
	have_cb := (rh != nil || discarded != nil || se != nil)
	if have_cb {
		cb := cresultCb{reqId: reqId, rh: rh, discarded: discarded, se: se}
		ah.cs.Lock()
		if batching {
			ah.qBatching.PushBack(cb)
		} else {
			ah.qCallback.PushBack(cb)
		}
		ah.cs.Unlock()
	}
	ret, _, _ := sendRequest.Call(h, uintptr(reqId), p, bytes)
	ah.csSend.Unlock()
	ok := (byte(ret) > 0)
	if !ok && have_cb {
		ah.cs.Lock()
		if batching {
			ah.qBatching.PopBack()
		} else {
			ah.qCallback.PopBack()
		}
		ah.cs.Unlock()
	}
	return ok
}

func (ah *CAsyncHandler) Unlock() {
	p := ah.pool
	unlockASocket.Call(uintptr(p.GetPoolId()), uintptr(ah.hSocket))
}

func (m *Message) Unsubscribe() *Message {
	exit.Call(uintptr(m.s.hSocket))
	return m
}

func (m *Message) Subscribe(groups []uint32) bool {
	var p uintptr
	var count uintptr
	if len(groups) > 0 {
		count = uintptr(len(groups))
		p = uintptr(unsafe.Pointer(&groups[0]))
	}
	r, _, _ := enter.Call(uintptr(m.s.hSocket), p, count)
	return (byte(r) > 0)
}

func (m *Message) Publish(msg interface{}, groups []uint32) bool {
	var p uintptr
	var count uintptr
	if len(groups) > 0 {
		count = uintptr(len(groups))
		p = uintptr(unsafe.Pointer(&groups[0]))
	}
	b := gspa.MakeBuffer().SaveObject(msg)
	defer b.Recycle()
	buffer := uintptr(b.GetBuffer())
	bytes := b.GetSize()
	r, _, _ := speak.Call(uintptr(m.s.hSocket), buffer, uintptr(bytes), p, count)
	return (byte(r) > 0)
}

func (m *Message) PublishEx(msg []byte, groups []uint32) bool {
	var p uintptr
	var count uintptr
	var buffer uintptr
	if len(groups) > 0 {
		count = uintptr(len(groups))
		p = uintptr(unsafe.Pointer(&groups[0]))
	}
	bytes := len(msg)
	if bytes > 0 {
		buffer = uintptr(unsafe.Pointer(&msg[0]))
	}
	r, _, _ := speakEx.Call(uintptr(m.s.hSocket), buffer, uintptr(bytes), p, count)
	return (byte(r) > 0)
}

func (m *Message) SendUserMessage(msg interface{}, user string) bool {
	var userId uintptr
	uid := utf16.Encode([]rune(user))
	if len(uid) > 0 {
		userId = uintptr(unsafe.Pointer(&uid[0]))
	}
	b := gspa.MakeBuffer().SaveObject(msg)
	defer b.Recycle()
	buffer := uintptr(b.GetBuffer())
	bytes := b.GetSize()
	r, _, _ := sendUserMessage.Call(uintptr(m.s.hSocket), userId, buffer, uintptr(bytes))
	return (byte(r) > 0)
}

func (m *Message) SendUserMessageEx(user string, msg []byte) bool {
	var userId uintptr
	var buffer uintptr
	uid := utf16.Encode([]rune(user))
	if len(uid) > 0 {
		userId = uintptr(unsafe.Pointer(&uid[0]))
	}
	bytes := len(msg)
	if bytes > 0 {
		buffer = uintptr(unsafe.Pointer(&msg[0]))
	}
	r, _, _ := sendUserMessageEx.Call(uintptr(m.s.hSocket), userId, buffer, uintptr(bytes))
	return (byte(r) > 0)
}
