package gcs

// #include <stdint.h>
// #include <stdbool.h>
// #include <wchar.h>
// #include <string.h>
// typedef uint64_t USocket_Client_Handle;
// typedef struct {wchar_t* UserId; char* IpAddress; unsigned short Port; unsigned int ServiceId; bool SelfMessage;} CMessageSender;
// bool GetAutoConn(USocket_Client_Handle h);
// bool IsOpened(USocket_Client_Handle h);
// void SetAutoConn(USocket_Client_Handle h, bool autoConnecting);
// void SetZip(USocket_Client_Handle h, bool zip);
// bool TurnOnZipAtSvr(USocket_Client_Handle h, bool enableZip);
// bool GetZip(USocket_Client_Handle h);
// bool StartBatching(USocket_Client_Handle h);
// bool CommitBatching(USocket_Client_Handle h, bool batchingAtServerSide);
// bool AbortBatching(USocket_Client_Handle h);
// void SetRecvTimeout(USocket_Client_Handle h, unsigned int timeout);
// unsigned int GetRecvTimeout(USocket_Client_Handle h);
// void SetConnTimeout(USocket_Client_Handle h, unsigned int timeout);
// unsigned int GetConnTimeout(USocket_Client_Handle h);
// void SetUserID(USocket_Client_Handle h, const wchar_t *userId);
// unsigned int GetUID(USocket_Client_Handle h, wchar_t *userId, unsigned int chars);
// bool SetSockOpt(USocket_Client_Handle h, int optName, int optValue, int level);
// bool SetSockOptAtSvr(USocket_Client_Handle h, int optName, int optValue, int level);
// int GetEncryptionMethod(USocket_Client_Handle h);
// void SetEncryptionMethod(USocket_Client_Handle h, int em);
// int GetConnectionState(USocket_Client_Handle h);
// bool WaitAll(USocket_Client_Handle h, unsigned int nTimeout);
// typedef void (*POnSocketClosed) (USocket_Client_Handle handler, int nError);
// typedef void (*POnHandShakeCompleted) (USocket_Client_Handle handler, int nError);
// typedef void (*POnSocketConnected) (USocket_Client_Handle handler, int nError);
// typedef void (*POnRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
// typedef void (*POnBaseRequestProcessed) (USocket_Client_Handle handler, unsigned short requestId);
// typedef void (*POnServerException) (USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, int errCode);
// typedef void (*POnAllRequestsProcessed) (USocket_Client_Handle handler, unsigned short lastRequestId);
// typedef void (*POnPostProcessing) (USocket_Client_Handle handler, unsigned int hint, uint64_t data);
// void SetOnHandShakeCompleted(USocket_Client_Handle h, POnHandShakeCompleted p);
// void SetOnRequestProcessed(USocket_Client_Handle h, POnRequestProcessed p);
// void SetOnSocketClosed(USocket_Client_Handle h, POnSocketClosed p);
// void SetOnSocketConnected(USocket_Client_Handle h, POnSocketConnected p);
// void SetOnBaseRequestProcessed(USocket_Client_Handle h, POnBaseRequestProcessed p);
// void SetOnAllRequestsProcessed(USocket_Client_Handle h, POnAllRequestsProcessed p);
// void SetOnPostProcessing(USocket_Client_Handle h, POnPostProcessing p);
// void SetOnServerException(USocket_Client_Handle h, POnServerException p);
// void socketclosed_cgo(USocket_Client_Handle handle, int ec);
// void socketconnected_cgo(USocket_Client_Handle handle, int ec);
// void handshakecompleted_cgo(USocket_Client_Handle handle, int ec);
// void requestprocessed_cgo(USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
// void baserequestprocessed_cgo(USocket_Client_Handle handler, unsigned short requestId);
// void serverexception_cgo(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, int errCode);
// void allrequestsprocessed_cgo(USocket_Client_Handle handler, unsigned short lastRequestId);
// void postprocessing_cgo(USocket_Client_Handle handler, unsigned int hint, uint64_t data);
// void enter_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
// void exit_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
// void speak_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
// void usermessage_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size);
// void usermessageex_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size);
// void speakex_cgo(USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
// bool IsRandom(USocket_Client_Handle h);
// int GetPeerOs(USocket_Client_Handle h, bool *endian);
// const unsigned char* GetResultBuffer(USocket_Client_Handle h);
// bool IsBatching(USocket_Client_Handle h);
// bool SendRequest(USocket_Client_Handle h, unsigned short reqId, const unsigned char *pBuffer, unsigned int len);
// unsigned int RetrieveResult(USocket_Client_Handle h, unsigned char *pBuffer, unsigned int size);
// int GetErrorCode(USocket_Client_Handle h);
// unsigned int GetErrorMessage(USocket_Client_Handle h, unsigned char *str, unsigned int bufferLen);
// bool SendInterruptRequest(USocket_Client_Handle h, uint64_t options);
// bool UnlockASocket(unsigned int poolId, USocket_Client_Handle h);
// void Close(USocket_Client_Handle h);
// unsigned int GetCountOfRequestsQueued(USocket_Client_Handle h);
// unsigned short GetCurrentRequestID(USocket_Client_Handle h);
// bool Cancel(USocket_Client_Handle h, unsigned int requestsQueued);
// unsigned int GetBytesInSendingBuffer(USocket_Client_Handle h);
// unsigned int GetBytesInReceivingBuffer(USocket_Client_Handle h);
// unsigned int GetBytesBatched(USocket_Client_Handle h);
// uint64_t GetBytesReceived(USocket_Client_Handle h);
// uint64_t GetBytesSent(USocket_Client_Handle h);
// bool DoEcho(USocket_Client_Handle h);
// bool GetPeerName(USocket_Client_Handle h, unsigned int *peerPort, char *ipAddr, unsigned short chars);
// unsigned short GetServerPingTime(USocket_Client_Handle h);
// bool IgnoreLastRequest(USocket_Client_Handle h, unsigned short reqId);
// int GetZipLevel(USocket_Client_Handle h);
// void SetZipLevel(USocket_Client_Handle h, int zl);
// bool SetZipLevelAtSvr(USocket_Client_Handle h, int zipLevel);
// void Shutdown(USocket_Client_Handle h, int how);
// unsigned int GetRouteeCount(USocket_Client_Handle h);
// unsigned short GetServerPingTime(USocket_Client_Handle h);
// bool IsRouteeRequest(USocket_Client_Handle h);
// bool Enter(USocket_Client_Handle h, const unsigned int *pChatGroupId, unsigned int count);
// void Exit(USocket_Client_Handle h);
// bool Speak(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
// bool SpeakEx(USocket_Client_Handle h, const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count);
// bool SendUserMessage(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
// bool SendUserMessageEx(USocket_Client_Handle h, const wchar_t *userId, const unsigned char *message, unsigned int size);
// typedef void (*POnEnter2) (USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
// typedef void (*POnExit2) (USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count);
// typedef void (*POnSpeakEx2) (USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
// typedef void (*POnSpeak2) (USocket_Client_Handle handler, CMessageSender *sender, const unsigned int *pGroup, unsigned int count, const unsigned char *message, unsigned int size);
// typedef void (*POnSendUserMessage2) (USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *message, unsigned int size);
// typedef void (*POnSendUserMessageEx2) (USocket_Client_Handle handler, CMessageSender *sender, const unsigned char *pMessage, unsigned int size);
// void SetOnEnter2(USocket_Client_Handle h, POnEnter2 p);
// void SetOnExit2(USocket_Client_Handle h, POnExit2 p);
// void SetOnSpeakEx2(USocket_Client_Handle h, POnSpeakEx2 p);
// void SetOnSendUserMessageEx2(USocket_Client_Handle h, POnSendUserMessageEx2 p);
// void SetOnSendUserMessage2(USocket_Client_Handle h, POnSendUserMessage2 p);
// void SetOnSpeak2(USocket_Client_Handle h, POnSpeak2 p);
// const char* Verify(USocket_Client_Handle h, int *errCode);
// typedef struct {char* Issuer;char* Subject;char* NotBefore;char* NotAfter;bool Validity;char* SigAlg;char* CertPem;char* SessionInfo;unsigned int PKSize;unsigned char* PublicKey;unsigned int AlgSize;unsigned char* Algorithm;unsigned int SNSize;unsigned char* SerialNumber;} CertInfo;
// CertInfo* GetUCert(USocket_Client_Handle h);
// typedef bool(*PCertificateVerifyCallback) (bool, int, int, const char *errMsg, CertInfo*);
// void SetCertificateVerifyCallback(PCertificateVerifyCallback cvc);
// bool cvc_cgo(bool verified, int depth, int ec, const char *em, CertInfo *ci);
// void PostProcessing(USocket_Client_Handle h, unsigned int hint, uint64_t data);
import "C"

import (
	"gspa"
	"unsafe"

	"github.com/gammazero/deque"
)

//export socketclosed_go
func socketclosed_go(handle C.USocket_Client_Handle, err C.int) {
	ec := int32(err)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cbs := s.socketClosed
	s.cs.Unlock()
	s.CleanCallbacks(false)
	for _, cb := range cbs {
		cb(ec)
	}
}

//export socketconnected_go
func socketconnected_go(handle C.USocket_Client_Handle, err C.int) {
	ec := int32(err)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cbs := s.socketConnected
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(ec)
	}
}

//export handshakecompleted_go
func handshakecompleted_go(handle C.USocket_Client_Handle, err C.int) {
	ec := int32(err)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cbs := s.handshakeCompleted
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(ec)
	}
}

//export allrequestsprocessed_go
func allrequestsprocessed_go(handle C.USocket_Client_Handle, reqId C.uint16_t) {
	rid := gspa.ReqId(reqId)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cbs := s.allProcessed
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(rid)
	}
}

//export postprocessing_go
func postprocessing_go(handle C.USocket_Client_Handle, clue C.uint32_t, data C.uint64_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	hint := uint32(clue)
	d := uint64(data)
	s.cs.Lock()
	cbs := s.postProcessing
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(hint, d)
	}
}

//export baserequestprocessed_go
func baserequestprocessed_go(handle C.USocket_Client_Handle, reqId C.uint16_t) {
	rid := gspa.ReqId(reqId)
	os := C.GetPeerOs(handle, (*C.bool)(unsafe.Pointer(nil)))
	random := C.IsRandom(handle)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cbs := s.baseReq
	s.os = gspa.OperationSystem(os)
	s.buffer.SetOS(s.os)
	s.random = bool(random)
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(rid)
	}
	if rid == gspa.Cancel {
		s.CleanCallbacks(true)
	}
}

func toWStr(ws *C.wchar_t) string {
	if ws == nil {
		return ""
	}
	size := C.wcslen(ws)
	if size == 0 {
		return ""
	}
	str := make([]rune, size)
	C.memcpy(unsafe.Pointer(&str[0]), unsafe.Pointer(ws), size<<2)
	return string(str)
}

func toAStr(s *C.char) string {
	if s == nil {
		return ""
	}
	chars := C.strlen(s)
	return C.GoStringN(s, C.int(chars))
}

//export serverexception_go
func serverexception_go(handle C.USocket_Client_Handle, reqId C.uint16_t, errMessage *C.wchar_t, errWhere *C.char, ec C.int) {
	ew := toAStr(errWhere)
	em := toWStr(errMessage)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	rcb := s.getARH(gspa.ReqId(reqId))
	if rcb.reqId > 0 && rcb.se != nil {
		rcb.se(s, gspa.ReqId(reqId), int32(ec), em, ew)
	}
	s.cs.Lock()
	cbs := s.serverException
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(gspa.ReqId(reqId), int32(ec), em, ew)
	}
}

//export requestprocessed_go
func requestprocessed_go(handle C.USocket_Client_Handle, reqId C.uint16_t, bytes C.uint32_t) {
	rid := gspa.ReqId(reqId)
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.buffer.SetSize(0)
	if s.IsRouting() {
		os := C.GetPeerOs(handle, (*C.bool)(unsafe.Pointer(nil)))
		s.buffer.SetOS(gspa.OperationSystem(os))
	}
	if bytes > 0 {
		if uint32(bytes) > s.buffer.GetMaxSize() {
			s.buffer.Realloc(uint32(bytes))
		}
		C.memcpy(s.buffer.GetBuffer(), unsafe.Pointer(C.GetResultBuffer(handle)), C.size_t(bytes))
		s.buffer.SetSize(uint32(bytes))
	}
	rcb := s.getARH(rid)
	if rcb.reqId > 0 && rcb.rh != nil {
		ar := CAsyncResult{s, rid, &s.buffer, rcb.rh}
		rcb.rh(&ar)
		s.buffer.SetSize(0)
		return
	}
	s.cs.Lock()
	cbs := s.requestProcessed
	s.cs.Unlock()
	for _, cb := range cbs {
		cb(rid, &s.buffer)
	}
	s.buffer.SetSize(0)
}

func toGroups(groups *C.uint32_t, count C.uint32_t) []uint32 {
	grps := make([]uint32, int(count))
	if count > 0 {
		C.memcpy(unsafe.Pointer(&grps[0]), unsafe.Pointer(groups), C.size_t(count<<2))
	}
	return grps
}

func toMS(sender *C.CMessageSender) *MessageSender {
	var ms MessageSender
	ms.UserId = toWStr(sender.UserId)
	ms.IpAddr = toAStr(sender.IpAddress)
	ms.Port = uint16(sender.Port)
	ms.ServiceId = uint32(sender.ServiceId)
	ms.Self = bool(sender.SelfMessage)
	return &ms
}

//export enter_go
func enter_go(handle C.USocket_Client_Handle, sender *C.CMessageSender, groups *C.uint32_t, count C.uint32_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cb := s.msg.subscribe
	s.cs.Unlock()
	if cb != nil {
		cb(toMS(sender), toGroups(groups, count))
	}
}

//export exit_go
func exit_go(handle C.USocket_Client_Handle, sender *C.CMessageSender, groups *C.uint32_t, count C.uint32_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cb := s.msg.unsubscribe
	s.cs.Unlock()
	if cb != nil {
		cb(toMS(sender), toGroups(groups, count))
	}
}

func toBytes(msg *C.uchar, bytes C.uint32_t) []byte {
	ba := make([]byte, int(bytes))
	if bytes > 0 {
		C.memcpy(unsafe.Pointer(&ba[0]), unsafe.Pointer(msg), C.size_t(bytes))
	}
	return ba
}

//export speak_go
func speak_go(handle C.USocket_Client_Handle, sender *C.CMessageSender, groups *C.uint32_t, count C.uint32_t, msg *C.uchar, bytes C.uint32_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cb := s.msg.publish
	s.cs.Unlock()
	if cb != nil {
		var gb gspa.CUQueue
		gb.SetBuffer(toBytes(msg, bytes), uint32(bytes))
		cb(toMS(sender), toGroups(groups, count), gb.LoadObject())
	}
}

//export speakex_go
func speakex_go(handle C.USocket_Client_Handle, sender *C.CMessageSender, groups *C.uint32_t, count C.uint32_t, msg *C.uchar, bytes C.uint32_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cb := s.msg.publishEx
	s.cs.Unlock()
	if cb != nil {
		cb(toMS(sender), toGroups(groups, count), toBytes(msg, bytes))
	}
}

//export usermessage_go
func usermessage_go(handle C.USocket_Client_Handle, sender *C.CMessageSender, msg *C.uchar, bytes C.uint32_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cb := s.msg.userMessage
	s.cs.Unlock()
	if cb != nil {
		var gb gspa.CUQueue
		gb.SetBuffer(toBytes(msg, bytes), uint32(bytes))
		cb(toMS(sender), gb.LoadObject())
	}
}

//export usermessageex_go
func usermessageex_go(handle C.USocket_Client_Handle, sender *C.CMessageSender, msg *C.uchar, bytes C.uint32_t) {
	gCs.Lock()
	s := gMapHandle[handle]
	gCs.Unlock()
	s.cs.Lock()
	cb := s.msg.userMessage
	s.cs.Unlock()
	if cb != nil {
		cb(toMS(sender), toBytes(msg, bytes))
	}
}

func (s *CSocket) DoEcho() bool {
	ret := C.DoEcho(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) GetPeerName() (string, uint32) {
	var port uint32
	var chars uint16 = 256
	addr := make([]byte, 256)
	pa := (*C.char)(unsafe.Pointer(&addr[0]))
	ret := C.GetPeerName(C.USocket_Client_Handle(s.hSocket), (*C.uint32_t)(unsafe.Pointer(&port)), pa, C.uint16_t(chars))
	if bool(ret) {
		size := C.strlen(pa)
		return C.GoStringN(pa, C.int(size)), port
	}
	return "", 0
}

func (ci *CertInfo) Verify() (int32, string) {
	var ec int32
	h := C.USocket_Client_Handle(ci.s.hSocket)
	ret := C.Verify(h, (*C.int)(unsafe.Pointer(&ec)))
	return ec, toAStr(ret)
}

func toCI(certInfo *C.CertInfo) *CertInfo {
	ci := new(CertInfo)
	cci := (*C.CertInfo)(certInfo)
	ci.Issuer = toAStr(cci.Issuer)
	ci.Subject = toAStr(cci.Subject)
	ci.NotBefore = toAStr(cci.NotBefore)
	ci.NotAfter = toAStr(cci.NotAfter)
	ci.Validity = bool(cci.Validity)
	ci.SigAlg = toAStr(cci.SigAlg)
	ci.CertPem = toAStr(cci.CertPem)
	ci.SessionInfo = toAStr(cci.SessionInfo)
	ci.PublicKey = toBytes(cci.PublicKey, cci.PKSize)
	ci.Algorithm = toBytes(cci.Algorithm, cci.AlgSize)
	ci.SerialNumber = toBytes(cci.SerialNumber, cci.SNSize)
	return ci
}

var g_cv DCertificateVerify

//export cvc_go
func cvc_go(verified C.bool, depth C.int32_t, ec C.int32_t, errMsg *C.char, ci *C.CertInfo) C.bool {
	gCs.Lock()
	cv := g_cv
	gCs.Unlock()
	if cv != nil {
		return C.bool(cv(bool(verified), int32(depth), int32(ec), toAStr(errMsg), toCI(ci)))
	}
	return verified
}

func SetCertificateVerify(cv DCertificateVerify) {
	gCs.Lock()
	g_cv = cv
	gCs.Unlock()
	if cv == nil {
		C.SetCertificateVerifyCallback((C.PCertificateVerifyCallback)(nil))
		return
	}
	C.SetCertificateVerifyCallback((C.PCertificateVerifyCallback)(unsafe.Pointer(C.cvc_cgo)))
}

func (s *CSocket) GetCert() *CertInfo {
	h := C.USocket_Client_Handle(s.hSocket)
	ret := C.GetUCert(h)
	if ret == nil {
		s.ci = nil
	} else {
		ci := toCI(ret)
		ci.s = s
		s.ci = ci
	}
	return s.ci
}

func (s *CSocket) GetServerPingTime() uint32 {
	ret := C.GetServerPingTime(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) GetRouteeCount() uint32 {
	ret := C.GetRouteeCount(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CAsyncHandler) IsRouteeRequest() bool {
	ret := C.IsRouteeRequest(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) IgnoreLastRequest(reqId gspa.ReqId) bool {
	ret := C.IgnoreLastRequest(C.USocket_Client_Handle(s.hSocket), C.uint16_t(reqId))
	return bool(ret)
}

func (s *CSocket) Shutdown(st ...gspa.ShutdownType) *CSocket {
	var t C.int = C.int(gspa.Both)
	if len(st) > 0 {
		t = C.int(st[0])
	}
	C.Shutdown(C.USocket_Client_Handle(s.hSocket), t)
	return s
}

func (s *CSocket) GetZipLevel() gspa.ZipLevel {
	ret := C.GetZipLevel(C.USocket_Client_Handle(s.hSocket))
	return gspa.ZipLevel(ret)
}

func (s *CSocket) SetZipLevel(zl gspa.ZipLevel) *CSocket {
	C.SetZipLevel(C.USocket_Client_Handle(s.hSocket), C.int(zl))
	return s
}

func (s *CSocket) SetZipLevelAtSvr(zl gspa.ZipLevel) bool {
	ret := C.SetZipLevelAtSvr(C.USocket_Client_Handle(s.hSocket), C.int(zl))
	return bool(ret)
}

func (s *CSocket) GetAutoConn() bool {
	ret := C.GetAutoConn(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) IsOpened() bool {
	ret := C.IsOpened(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) SetAutoConn(ac bool) *CSocket {
	C.SetAutoConn(C.USocket_Client_Handle(s.hSocket), C.bool(ac))
	return s
}

func (s *CSocket) SetZip(zip bool) *CSocket {
	C.SetZip(C.USocket_Client_Handle(s.hSocket), C.bool(zip))
	return s
}

func (s *CSocket) TurnOnZipAtSvr(zip bool) *CSocket {
	C.TurnOnZipAtSvr(C.USocket_Client_Handle(s.hSocket), C.bool(zip))
	return s
}

func (s *CSocket) GetZip() bool {
	ret := C.GetZip(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) StartBatching() bool {
	ret := C.StartBatching(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CAsyncHandler) CommitBatching(batchingAtServerSide ...bool) bool {
	var b bool
	if len(batchingAtServerSide) > 0 {
		b = batchingAtServerSide[0]
	}
	s.cs.Lock()
	for s.qBatching.Len() > 0 {
		s.qCallback.PushBack(s.qBatching.PopFront())
	}
	s.cs.Unlock()
	ret := C.CommitBatching(C.USocket_Client_Handle(s.hSocket), C.bool(b))
	return bool(ret)
}

func (s *CAsyncHandler) AbortBatching() bool {
	s.cs.Lock()
	s.qBatching = deque.Deque[cresultCb]{}
	s.cs.Unlock()
	ret := C.AbortBatching(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) GetConnTimeout() uint32 {
	ret := C.GetConnTimeout(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) SetConnTimeout(timeout uint32) *CSocket {
	C.SetConnTimeout(C.USocket_Client_Handle(s.hSocket), C.uint32_t(timeout))
	return s
}

func (s *CSocket) GetRecvTimeout() uint32 {
	ret := C.GetRecvTimeout(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) SetRecvTimeout(timeout uint32) *CSocket {
	C.SetRecvTimeout(C.USocket_Client_Handle(s.hSocket), C.uint32_t(timeout))
	return s
}

func (s *CSocket) IsBatching() bool {
	ret := C.IsBatching(C.USocket_Client_Handle(s.hSocket))
	return bool(ret)
}

func (s *CSocket) SetUID(uid string) *CSocket {
	if len(uid) > 0 {
		text := []rune(uid)
		C.SetUserID(C.USocket_Client_Handle(s.hSocket), (*C.wchar_t)(&text[0]))
	} else {
		C.SetUserID(C.USocket_Client_Handle(s.hSocket), (*C.wchar_t)(unsafe.Pointer(nil)))
	}
	return s
}

func (s *CSocket) GetUID() string {
	uid := make([]rune, 1024)
	ret := C.GetUID(C.USocket_Client_Handle(s.hSocket), (*C.wchar_t)(&uid[0]), 1024)
	return string(uid[0:ret])
}

func (s *CSocket) GetErrorCode() int32 {
	ret := C.GetErrorCode(C.USocket_Client_Handle(s.hSocket))
	return int32(ret)
}

func (s *CSocket) GetErrorMsg() string {
	msg := make([]byte, 4096)
	ret := C.GetErrorMessage(C.USocket_Client_Handle(s.hSocket), (*C.uchar)(&msg[0]), 4096)
	return string(msg[0:ret])
}

func (s *CSocket) SetSockOpt(so gspa.SocketOption, val int32, sl gspa.SocketLevel) bool {
	ret := C.SetSockOpt(C.USocket_Client_Handle(s.hSocket), C.int(so), C.int(val), C.int(sl))
	return bool(ret)
}

func (s *CSocket) SetSockOptAtSvr(so gspa.SocketOption, val int32, sl gspa.SocketLevel) bool {
	ret := C.SetSockOptAtSvr(C.USocket_Client_Handle(s.hSocket), C.int(so), C.int(val), C.int(sl))
	return bool(ret)
}

func (s *CSocket) Close() *CSocket {
	C.Close(C.USocket_Client_Handle(s.hSocket))
	return s
}

func (s *CSocket) Cancel(requestsQueued ...uint32) *CSocket {
	var rqs C.uint32_t = 0xffffffff
	if len(requestsQueued) > 0 {
		rqs = C.uint32_t(requestsQueued[0])
	}
	C.Cancel(C.USocket_Client_Handle(s.hSocket), rqs)
	return s
}

func (s *CSocket) GetCountOfRequestsQueued() uint32 {
	ret := C.GetCountOfRequestsQueued(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) GetBytesInSendingBuffer() uint32 {
	ret := C.GetBytesInSendingBuffer(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) GetBytesInReceivingBuffer() uint32 {
	ret := C.GetBytesInReceivingBuffer(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) GetBytesBatched() uint32 {
	ret := C.GetBytesBatched(C.USocket_Client_Handle(s.hSocket))
	return uint32(ret)
}

func (s *CSocket) GetBytesReceived() uint64 {
	ret := C.GetBytesReceived(C.USocket_Client_Handle(s.hSocket))
	return uint64(ret)
}

func (s *CSocket) GetBytesSent() uint64 {
	ret := C.GetBytesSent(C.USocket_Client_Handle(s.hSocket))
	return uint64(ret)
}

func (s *CSocket) GetCurrentRequestID() gspa.ReqId {
	ret := C.GetCurrentRequestID(C.USocket_Client_Handle(s.hSocket))
	return gspa.ReqId(ret)
}

func (s *CSocket) GetEncryptionMethod() gspa.EncryptionMethod {
	ret := C.GetEncryptionMethod(C.USocket_Client_Handle(s.hSocket))
	return gspa.EncryptionMethod(ret)
}

func (s *CSocket) SetEncryptionMethod(em gspa.EncryptionMethod) *CSocket {
	C.SetEncryptionMethod(C.USocket_Client_Handle(s.hSocket), C.int(em))
	return s
}

func (s *CSocket) GetConnectionState() ConnectionState {
	ret := C.GetConnectionState(C.USocket_Client_Handle(s.hSocket))
	return ConnectionState(ret)
}

func (s *CSocket) WaitAll(timeout ...uint32) bool {
	var forever uint32 = 0xffffffff
	if len(timeout) > 0 {
		forever = timeout[0]
	}
	ret := C.WaitAll(C.USocket_Client_Handle(s.hSocket), C.uint32_t(forever))
	return bool(ret)
}

func (s *CSocket) setCallbacks() {
	C.SetOnSocketClosed(C.USocket_Client_Handle(s.hSocket), (C.POnSocketClosed)(unsafe.Pointer(C.socketclosed_cgo)))
	C.SetOnSocketConnected(C.USocket_Client_Handle(s.hSocket), (C.POnSocketConnected)(unsafe.Pointer(C.socketconnected_cgo)))
	C.SetOnHandShakeCompleted(C.USocket_Client_Handle(s.hSocket), (C.POnHandShakeCompleted)(unsafe.Pointer(C.handshakecompleted_cgo)))
	C.SetOnBaseRequestProcessed(C.USocket_Client_Handle(s.hSocket), (C.POnBaseRequestProcessed)(unsafe.Pointer(C.baserequestprocessed_cgo)))
	C.SetOnAllRequestsProcessed(C.USocket_Client_Handle(s.hSocket), (C.POnAllRequestsProcessed)(unsafe.Pointer(C.allrequestsprocessed_cgo)))
	C.SetOnRequestProcessed(C.USocket_Client_Handle(s.hSocket), (C.POnRequestProcessed)(unsafe.Pointer(C.requestprocessed_cgo)))
	C.SetOnPostProcessing(C.USocket_Client_Handle(s.hSocket), (C.POnPostProcessing)(unsafe.Pointer(C.postprocessing_cgo)))
	C.SetOnServerException(C.USocket_Client_Handle(s.hSocket), (C.POnServerException)(unsafe.Pointer(C.serverexception_cgo)))
	C.SetOnEnter2(C.USocket_Client_Handle(s.hSocket), (C.POnEnter2)(unsafe.Pointer(C.enter_cgo)))
	C.SetOnExit2(C.USocket_Client_Handle(s.hSocket), (C.POnExit2)(unsafe.Pointer(C.exit_cgo)))
	C.SetOnSpeak2(C.USocket_Client_Handle(s.hSocket), (C.POnSpeak2)(unsafe.Pointer(C.speak_cgo)))
	C.SetOnSendUserMessage2(C.USocket_Client_Handle(s.hSocket), (C.POnSendUserMessage2)(unsafe.Pointer(C.usermessage_cgo)))
	C.SetOnSendUserMessageEx2(C.USocket_Client_Handle(s.hSocket), (C.POnSendUserMessageEx2)(unsafe.Pointer(C.usermessageex_cgo)))
	C.SetOnSpeakEx2(C.USocket_Client_Handle(s.hSocket), (C.POnSpeakEx2)(unsafe.Pointer(C.speakex_cgo)))
}

func (s *CSocket) Interrupt(hint uint64) bool {
	ret := C.SendInterruptRequest(C.USocket_Client_Handle(s.hSocket), C.uint64_t(hint))
	return bool(ret)
}

func (s *CAsyncHandler) PostProcessing(hint uint32, data uint64) *CAsyncHandler {
	C.PostProcessing(C.USocket_Client_Handle(s.hSocket), C.uint32_t(hint), C.uint64_t(hint))
	return s
}

func (ah *CAsyncHandler) SendRequest(reqId gspa.ReqId, buffer *gspa.CUQueue, rh DResultHandler, discarded DDiscarded, se DServerException) bool {
	var p *C.uchar
	var bytes C.uint32_t
	if buffer != nil {
		bytes = C.uint32_t(buffer.GetSize())
		p = (*C.uchar)(buffer.GetBuffer())
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
	ret := C.SendRequest(C.USocket_Client_Handle(ah.hSocket), C.uint16_t(reqId), p, bytes)
	ah.csSend.Unlock()
	ok := bool(ret)
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
	C.UnlockASocket(C.uint32_t(p.GetPoolId()), C.USocket_Client_Handle(ah.hSocket))
}

func (m *Message) Unsubscribe() *Message {
	C.Exit(C.USocket_Client_Handle(m.s.hSocket))
	return m
}

func (m *Message) Subscribe(groups []uint32) bool {
	var p *C.uint32_t
	var count C.uint32_t
	if len(groups) > 0 {
		count = C.uint32_t(len(groups))
		p = (*C.uint32_t)(unsafe.Pointer(&groups[0]))
	}
	r := C.Enter(C.USocket_Client_Handle(m.s.hSocket), p, count)
	return bool(r)
}

func (m *Message) Publish(msg interface{}, groups []uint32) bool {
	var p *C.uint32_t
	var count C.uint32_t
	if len(groups) > 0 {
		count = C.uint32_t(len(groups))
		p = (*C.uint32_t)(unsafe.Pointer(&groups[0]))
	}
	b := gspa.MakeBuffer().SaveObject(msg)
	defer b.Recycle()
	buffer := (*C.uchar)(b.GetBuffer())
	bytes := b.GetSize()
	r := C.Speak(C.USocket_Client_Handle(m.s.hSocket), buffer, C.uint32_t(bytes), p, count)
	return bool(r)
}

func (m *Message) PublishEx(msg []byte, groups []uint32) bool {
	var p *C.uint32_t
	var count C.uint32_t
	var buffer *C.uchar
	if len(groups) > 0 {
		count = (C.uint32_t)(len(groups))
		p = (*C.uint32_t)(unsafe.Pointer(&groups[0]))
	}
	bytes := len(msg)
	if bytes > 0 {
		buffer = (*C.uchar)(unsafe.Pointer(&msg[0]))
	}
	r := C.SpeakEx(C.USocket_Client_Handle(m.s.hSocket), buffer, (C.uint32_t)(bytes), p, count)
	return bool(r)
}

func (m *Message) SendUserMessage(msg interface{}, user string) bool {
	var userId *C.wchar_t
	uid := []rune(user)
	if len(uid) > 0 {
		userId = (*C.wchar_t)(unsafe.Pointer(&uid[0]))
	}
	b := gspa.MakeBuffer().SaveObject(msg)
	defer b.Recycle()
	buffer := (*C.uchar)(b.GetBuffer())
	bytes := b.GetSize()
	r := C.SendUserMessage(C.USocket_Client_Handle(m.s.hSocket), userId, buffer, (C.uint32_t)(bytes))
	return bool(r)
}

func (m *Message) SendUserMessageEx(user string, msg []byte) bool {
	var userId *C.wchar_t
	var buffer *C.uchar
	uid := []rune(user)
	if len(uid) > 0 {
		userId = (*C.wchar_t)(unsafe.Pointer(&uid[0]))
	}
	bytes := len(msg)
	if bytes > 0 {
		buffer = (*C.uchar)(unsafe.Pointer(&msg[0]))
	}
	r := C.SendUserMessageEx(C.USocket_Client_Handle(m.s.hSocket), userId, buffer, (C.uint32_t)(bytes))
	return bool(r)
}
