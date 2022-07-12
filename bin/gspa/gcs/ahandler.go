package gcs

import (
	"gspa"
	"sync"
	"sync/atomic"

	"github.com/gammazero/deque"
)

type RequestEvent func(reqId gspa.ReqId)
type SocketEvent func(ec int32)
type PostProcessingEvent func(hint uint32, data uint64)
type RequestProcessedEvent func(reqId gspa.ReqId, buffer *gspa.CUQueue)
type ServerExceptionEvent func(reqId gspa.ReqId, ec int32, em string, where string)

type CSocket struct {
	cs                 sync.Mutex
	os                 gspa.OperationSystem
	random             bool
	routing            bool
	buffer             gspa.CUQueue
	connContext        CConnectionContext
	hSocket            USocket_Client_Handle
	pool               *CSocketPool
	callBacks          []uintptr
	baseReq            []RequestEvent
	allProcessed       []RequestEvent
	socketClosed       []SocketEvent
	socketConnected    []SocketEvent
	handshakeCompleted []SocketEvent
	postProcessing     []PostProcessingEvent
	requestProcessed   []RequestProcessedEvent
	serverException    []ServerExceptionEvent
	msg                *Message
	cq                 *ClientQueue
	ci                 *CertInfo
}

type DResultHandler func(ar *CAsyncResult)
type DDiscarded func(sender *CAsyncHandler, canceled bool)
type DServerException func(sender *CAsyncHandler, reqId gspa.ReqId, ec int32, em string, where string)
type DMergeTo func(to *CAsyncHandler)
type DClean func()

type CAsyncResult struct {
	Sender         *CAsyncHandler
	ReqId          gspa.ReqId
	Buffer         *gspa.CUQueue
	CurrentHandler DResultHandler
}

type cresultCb struct {
	reqId     gspa.ReqId
	rh        DResultHandler
	discarded DDiscarded
	se        DServerException
}

type CAsyncHandler struct {
	CSocket
	csSend    sync.Mutex
	qCallback deque.Deque
	qBatching deque.Deque
	mergeTo   []DMergeTo
	clean     []DClean
	tie       ITie
}

type ITie interface {
	Tie(ah *CAsyncHandler) bool
	Untie()
	GetHandler() *CAsyncHandler
}

type MessageSender struct {
	UserId    string
	IpAddr    string
	Port      uint16
	ServiceId uint32
	Self      bool
}

type DSubscribe func(ms *MessageSender, groups []uint32)
type DUnsubscribe func(ms *MessageSender, groups []uint32)
type DPublish func(ms *MessageSender, groups []uint32, msg interface{})
type DPublishEx func(ms *MessageSender, groups []uint32, bytes []byte)
type DUserMessage func(ms *MessageSender, msg interface{})
type DUserMessageEx func(ms *MessageSender, bytes []byte)

type Message struct {
	s             *CAsyncHandler
	subscribe     DSubscribe
	unsubscribe   DUnsubscribe
	publish       DPublish
	publishEx     DPublishEx
	userMessage   DUserMessage
	userMessageEx DUserMessageEx
}

type ClientQueue struct {
	s     *CAsyncHandler
	index uint32
}

type CertInfo struct {
	s            *CSocket
	Issuer       string
	Subject      string
	NotBefore    string
	NotAfter     string
	Validity     bool
	SigAlg       string
	CertPem      string
	SessionInfo  string
	PublicKey    []byte
	Algorithm    []byte
	SerialNumber []byte
}

var (
	gIndex uint64
)

type DCertificateVerify func(verified bool, depth int32, ec int32, em string, ci *CertInfo) bool

func GetCallIndex() uint64 {
	return atomic.AddUint64(&gIndex, 1)
}

func (cq *ClientQueue) EnsureAppending(handles []USocket_Client_Handle) bool {
	if cq.IsAvailable() {
		return false
	}
	if cq.GetQueueOpenStatus() != gspa.MergePushing {
		return true
	}
	var ah CAsyncHandler
	var qTemp ClientQueue
	qTemp.s = &ah
	vH := make([]USocket_Client_Handle, 0)
	for _, h := range handles {
		ah.hSocket = h
		if qTemp.GetQueueOpenStatus() != gspa.MergeComplete {
			vH = append(vH, h)
		}
	}
	if len(vH) > 0 {
		return cq.AppendTo(vH)
	}
	cq.Reset()
	return true
}

func (cq *ClientQueue) EnsureOne(newOne ClientQueue) bool {
	handles := make([]USocket_Client_Handle, 1)
	handles[0] = newOne.s.hSocket
	return cq.EnsureAppending(handles)
}

func (cq *ClientQueue) GetHandler() *CAsyncHandler {
	return cq.s
}

func (m *Message) SetOnSubscribe(cb DSubscribe) *Message {
	m.s.cs.Lock()
	m.subscribe = cb
	m.s.cs.Unlock()
	return m
}

func (m *Message) SetOnUnsubscribe(cb DUnsubscribe) *Message {
	m.s.cs.Lock()
	m.unsubscribe = cb
	m.s.cs.Unlock()
	return m
}

func (m *Message) SetOnPublish(cb DPublish) *Message {
	m.s.cs.Lock()
	m.publish = cb
	m.s.cs.Unlock()
	return m
}

func (m *Message) SetOnPublishEx(cb DPublishEx) *Message {
	m.s.cs.Lock()
	m.publishEx = cb
	m.s.cs.Unlock()
	return m
}

func (m *Message) SetOnUserMessage(cb DUserMessage) *Message {
	m.s.cs.Lock()
	m.userMessage = cb
	m.s.cs.Unlock()
	return m
}

func (m *Message) SetOnUserMessageEx(cb DUserMessageEx) *Message {
	m.s.cs.Lock()
	m.userMessageEx = cb
	m.s.cs.Unlock()
	return m
}

func (s *CSocket) GetMsg() *Message {
	return s.msg
}

func (s *CSocket) GetClientQueue() *ClientQueue {
	return s.cq
}

func (m *Message) GetHandler() *CAsyncHandler {
	return m.s
}

func (s *CSocket) GetPool() *CSocketPool {
	return s.pool
}

func (s *CSocket) IsRouting() bool {
	return s.routing
}

func (s *CSocket) IsRandom() bool {
	s.cs.Lock()
	r := s.random
	s.cs.Unlock()
	return r
}

func (s *CSocket) Sendable() bool {
	return (s.IsOpened() || s.cq.IsAvailable())
}

func (s *CSocket) GetSvsID() gspa.ServiceID {
	return s.pool.serviceId
}

func (s *CSocket) IsConnected() bool {
	return s.IsOpened()
}

func (s *CAsyncHandler) SetOnServerException(cb ServerExceptionEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.serverException = append(s.serverException, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnRequestProcessed(cb RequestProcessedEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.requestProcessed = append(s.requestProcessed, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnPostProcessing(cb PostProcessingEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.postProcessing = append(s.postProcessing, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnHandShakeCompleted(cb SocketEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.handshakeCompleted = append(s.handshakeCompleted, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnSocketConnected(cb SocketEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.socketConnected = append(s.socketConnected, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnSocketClosed(cb SocketEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.socketClosed = append(s.socketClosed, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnBaseRequestProcessed(cb RequestEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.baseReq = append(s.baseReq, cb)
		s.cs.Unlock()
	}
	return s
}

func (s *CAsyncHandler) SetOnAllRequestsProcessed(cb RequestEvent) *CAsyncHandler {
	if cb != nil {
		s.cs.Lock()
		s.allProcessed = append(s.allProcessed, cb)
		s.cs.Unlock()
	}
	return s
}

func (ah *CAsyncHandler) appendTo(to *CAsyncHandler) {
	to.cs.Lock()
	ah.cs.Lock()
	cbs := ah.mergeTo
	for ah.qCallback.Len() > 0 {
		to.qCallback.PushBack(ah.qCallback.PopFront())
	}
	ah.cs.Unlock()
	for _, cb := range cbs {
		cb(to)
	}
	to.cs.Unlock()
}

func (ah *CAsyncHandler) SetOnMergeTo(cb DMergeTo) *CAsyncHandler {
	if cb != nil {
		ah.cs.Lock()
		ah.mergeTo = append(ah.mergeTo, cb)
		ah.cs.Unlock()
	}
	return ah
}

func (ah *CAsyncHandler) SetOnClean(cb DClean) *CAsyncHandler {
	if cb != nil {
		ah.cs.Lock()
		ah.clean = append(ah.clean, cb)
		ah.cs.Unlock()
	}
	return ah
}

func (ah *CAsyncHandler) GetTie() ITie {
	ah.cs.Lock()
	tie := ah.tie
	ah.cs.Unlock()
	return tie
}

func (ah *CAsyncHandler) SetTie(tie ITie) *CAsyncHandler {
	ah.cs.Lock()
	ah.tie = tie
	ah.cs.Unlock()
	return ah
}

func (ah *CAsyncHandler) getARH(reqId gspa.ReqId) cresultCb {
	ah.cs.Lock()
	ah.cs.Unlock()
	if ah.qCallback.Len() == 0 {
		return cresultCb{}
	}
	if ah.random {
		index := ah.qCallback.Index(func(rcb interface{}) bool {
			return (rcb.(cresultCb).reqId == reqId)
		})
		if index == -1 {
			return cresultCb{}
		}
		return ah.qCallback.Remove(index).(cresultCb)
	}
	if ah.qCallback.Front().(cresultCb).reqId == reqId {
		return ah.qCallback.PopFront().(cresultCb)
	}
	return cresultCb{}
}

func (ah *CAsyncHandler) CleanCallbacks(canceled bool) uint32 {
	ah.cs.Lock()
	cb := ah.qCallback
	b := ah.qBatching
	ah.qCallback = deque.Deque{}
	ah.qBatching = deque.Deque{}
	vClean := ah.clean
	ah.cs.Unlock()
	for _, cb := range vClean {
		cb()
	}
	total := b.Len() + cb.Len()
	for b.Len() > 0 {
		rcb := b.PopFront().(cresultCb)
		if rcb.discarded != nil {
			rcb.discarded(ah, canceled)
		}
	}
	for cb.Len() > 0 {
		rcb := cb.PopFront().(cresultCb)
		if rcb.discarded != nil {
			rcb.discarded(ah, canceled)
		}
	}
	return uint32(total)
}

func (ah *CAsyncHandler) setSocketErrorBefore(ch chan interface{}, reqId gspa.ReqId) *CAsyncHandler {
	var err SocketError
	err.ReqId = reqId
	err.Before = true
	err.ErrCode = ah.GetErrorCode()
	if err.ErrCode != 0 {
		err.ErrMsg = ah.GetErrorMsg()
	} else {
		err.ErrCode = SESSION_CLOSED_BEFORE
		err.ErrMsg = SESSION_CLOSED_BEFORE_ERR_MSG
	}
	go func() {
		ch <- err
	}()
	return ah
}

func (ah *CAsyncHandler) setCancelError(ch chan interface{}, reqId gspa.ReqId, canceled bool) *CAsyncHandler {
	var err SocketError
	err.ReqId = reqId
	err.Before = false
	if canceled {
		err.ErrCode = REQUEST_CANCELED
		err.ErrMsg = REQUEST_CANCELED_ERR_MSG
	} else {
		err.ErrCode = ah.GetErrorCode()
		if err.ErrCode != 0 {
			err.ErrMsg = ah.GetErrorMsg()
		} else {
			err.ErrCode = SESSION_CLOSED_AFTER
			err.ErrMsg = SESSION_CLOSED_AFTER_ERR_MSG
		}
	}
	ch <- err
	return ah
}

func (ah *CAsyncHandler) Send(reqId gspa.ReqId, buffer *gspa.CUQueue, rh DResultHandler) chan interface{} {
	res := make(chan interface{})
	ok := ah.SendRequest(reqId, buffer, rh, func(sender *CAsyncHandler, canceled bool) {
		sender.setCancelError(res, reqId, canceled)
	}, func(sender *CAsyncHandler, reqId gspa.ReqId, ec int32, em string, where string) {
		var err ServerError
		err.ReqId = reqId
		err.ErrCode = ec
		err.ErrMsg = em
		err.Location = where
		res <- err
	})
	if !ok {
		ah.setSocketErrorBefore(res, reqId)
	}
	return res
}

func (ah *CAsyncHandler) SendEx(reqId gspa.ReqId, buffer *gspa.CUQueue, rh DResultHandler, discarded DDiscarded) chan interface{} {
	res := make(chan interface{})
	ok := ah.SendRequest(reqId, buffer, rh, discarded, func(sender *CAsyncHandler, reqId gspa.ReqId, ec int32, em string, where string) {
		var err ServerError
		err.ReqId = reqId
		err.ErrCode = ec
		err.ErrMsg = em
		err.Location = where
		res <- err
	})
	if !ok {
		ah.setSocketErrorBefore(res, reqId)
	}
	return res
}
