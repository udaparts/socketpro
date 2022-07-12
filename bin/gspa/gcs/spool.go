package gcs

import (
	"gspa"
	"runtime"
	"strconv"
	"strings"
	"sync"
)

type PoolId uint32
type DelPool func(spe SocketPoolEvent, handler *CAsyncHandler)
type DelSslAuth func(handler *CAsyncHandler) bool

type CSocketPool struct {
	Mutex       sync.Mutex
	autoConn    bool
	recvTimeout uint32
	connTimeout uint32
	serviceId   gspa.ServiceID
	poolId      PoolId
	qName       string
	handlers    []*CAsyncHandler
	cbPool      uintptr
	event       DelPool
	sslAuth     DelSslAuth
	fromHandler *CAsyncHandler
}

func NewPool(serviceId gspa.ServiceID) *CSocketPool {
	var p CSocketPool
	p.serviceId = serviceId
	p.autoConn = true
	p.recvTimeout = DEFAULT_RECV_TIMEOUT
	p.connTimeout = DEFAULT_CONN_TIMEOUT
	return &p
}

func schIndex(a []*CAsyncHandler, x USocket_Client_Handle) int {
	for n, e := range a {
		if x == e.hSocket {
			return n
		}
	}
	return -1
}

func (p *CSocketPool) setQueue(pos int) {
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	if len(p.qName) > 0 {
		h := p.handlers[pos]
		cq := h.GetClientQueue()
		if !cq.IsAvailable() {
			qn := p.qName + strconv.Itoa(pos)
			cq.StartQueue(qn, DEFAULT_QUEUE_TIME_TO_LIVE, h.GetEncryptionMethod() != gspa.NoEncryption)
		}
	}
}

func (p *CSocketPool) stopPoolQueue() {
	for _, h := range p.handlers {
		h.GetClientQueue().StopQueue()
	}
}

func (p *CSocketPool) startPoolQueue(qName string) {
	for i, h := range p.handlers {
		qn := qName + strconv.Itoa(i)
		h.GetClientQueue().StartQueue(qn, DEFAULT_QUEUE_TIME_TO_LIVE, h.GetEncryptionMethod() != gspa.NoEncryption)
	}
}

func (p *CSocketPool) Seek() *CAsyncHandler {
	var hRes *CAsyncHandler
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	for _, h := range p.handlers {
		if h.GetConnectionState() < Switched {
			continue
		}
		if hRes == nil {
			hRes = h
		} else {
			var count0 uint64
			var count1 uint64
			cq0 := hRes.GetClientQueue()
			if cq0.IsAvailable() {
				count0 = cq0.GetMessageCount()
			} else {
				count0 = uint64(hRes.GetCountOfRequestsQueued())
			}
			cq := h.GetClientQueue()
			if cq.IsAvailable() {
				count1 = cq.GetMessageCount()
			} else {
				count1 = uint64(h.GetCountOfRequestsQueued())
			}
			if count0 > count1 {
				hRes = h
			}
		}
	}
	return hRes
}

func (p *CSocketPool) SeekByQueue() *CAsyncHandler {
	var hRes *CAsyncHandler
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	for _, h := range p.handlers {
		cq := h.GetClientQueue()
		if (!cq.IsAvailable()) || (cq.GetJobSize() > 0) {
			continue
		}
		if hRes == nil {
			hRes = h
		} else {
			cq0 := hRes.GetClientQueue()
			count0 := cq0.GetMessageCount()
			count1 := cq.GetMessageCount()
			if (count0 > count1) || (h.IsOpened() && !hRes.IsOpened()) {
				hRes = h
			}
		}
	}
	return hRes
}

func (p *CSocketPool) SeekByQueueName(qn string) *CAsyncHandler {
	qn = strings.TrimSpace(qn)
	if len(qn) == 0 {
		return nil
	}
	if runtime.GOOS == "windows" {
		qn = strings.ToLower(qn)
	}
	appName := GetAppName()
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	for _, h := range p.handlers {
		cq := h.GetClientQueue()
		if !cq.IsAvailable() {
			continue
		}
		var qnRaw string
		if cq.IsSecure() {
			qnRaw = qn + "_" + appName + "_1.mqc"
		} else {
			qnRaw = qn + "_" + appName + "_0.mqc"
		}
		qFileName := cq.GetQueueFileName()
		pos := strings.LastIndex(qFileName, qnRaw)
		if 0 == pos {
			return h
		}
		if pos+len(qnRaw) == len(qFileName) {
			return h
		}
	}
	return nil
}

func (p *CSocketPool) GetQueues() uint32 {
	var count uint32
	p.Mutex.Lock()
	for _, h := range p.handlers {
		cq := h.GetClientQueue()
		if cq.IsAvailable() {
			count++
		}
	}
	p.Mutex.Unlock()
	return count
}

func (p *CSocketPool) GetSvsID() gspa.ServiceID {
	return p.serviceId
}

func (p *CSocketPool) GetHandlers() []*CAsyncHandler {
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	return p.handlers
}

func (p *CSocketPool) SetPoolEvent(del DelPool) *CSocketPool {
	p.Mutex.Lock()
	p.event = del
	p.Mutex.Unlock()
	return p
}

func (p *CSocketPool) SetSslAuth(del DelSslAuth) *CSocketPool {
	p.Mutex.Lock()
	p.sslAuth = del
	p.Mutex.Unlock()
	return p
}

func (p *CSocketPool) GetAutoConn() bool {
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	return p.autoConn
}

func (p *CSocketPool) SetAutoConn(autoConn bool) *CSocketPool {
	p.Mutex.Lock()
	p.autoConn = autoConn
	for i, _ := range p.handlers {
		p.handlers[i].SetAutoConn(autoConn)
	}
	p.Mutex.Unlock()
	return p
}

func (p *CSocketPool) GetRecvTimeout() uint32 {
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	return p.recvTimeout
}

func (p *CSocketPool) SetRecvTimeout(timeout uint32) *CSocketPool {
	p.Mutex.Lock()
	if timeout < 1000 {
		timeout = 1000
	}
	p.recvTimeout = timeout
	for i, _ := range p.handlers {
		p.handlers[i].SetRecvTimeout(timeout)
	}
	p.Mutex.Unlock()
	return p
}

func (p *CSocketPool) GetConnTimeout() uint32 {
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	return p.connTimeout
}

func (p *CSocketPool) SetConnTimeout(timeout uint32) *CSocketPool {
	p.Mutex.Lock()
	if timeout < 1000 {
		timeout = 1000
	}
	p.connTimeout = timeout
	for i, _ := range p.handlers {
		p.handlers[i].SetConnTimeout(timeout)
	}
	p.Mutex.Unlock()
	return p
}

func (p *CSocketPool) GetPoolId() PoolId {
	p.Mutex.Lock()
	pid := p.poolId
	p.Mutex.Unlock()
	return pid
}

func (p *CSocketPool) GetQueueName() string {
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	return p.qName
}

func (p *CSocketPool) SetQueueName(qn string) *CSocketPool {
	qn = strings.TrimSpace(qn)
	if runtime.GOOS == "windows" {
		qn = strings.ToLower(qn)
	}
	p.Mutex.Lock()
	if p.qName != qn {
		p.stopPoolQueue()
		p.qName = qn
		if len(qn) > 0 {
			p.startPoolQueue(qn)
		}
	}
	p.Mutex.Unlock()

	return p
}

// Start a pool of sockets and handlers with one given connection context
// cc: A given connection context
// maxSocketsPerThread: The number of sockets per worker thread
// Example 0: p.StartSocketPool(cc, 2) for 2 sessions & 1 worker thread (2 * 1)
// Example 1: p.StartSocketPool(cc, 3, 2) for 6 sessions & 2 worker threads (3 * 2)
func (p *CSocketPool) StartSocketPool(cc CConnectionContext, maxSocketsPerThread uint32, args ...uint16) bool {
	if 0 == maxSocketsPerThread {
		maxSocketsPerThread = 1
	}
	var threads uint16 = 1
	size := len(args)
	if size > 0 && args[0] > 1 {
		threads = args[0]
	}
	avg := uint16(1)
	if size > 1 && 0 == args[1] {
		avg = 0
	}
	count := maxSocketsPerThread * uint32(threads)
	aCC := make([]CConnectionContext, count)
	for i, _ := range aCC {
		aCC[i] = cc
	}
	return p.StartSocketPoolEx(aCC, maxSocketsPerThread, avg)
}

// Start a pool of sockets and handlers with a given array of connection contexts
// maxSocketsPerThread: The number of sockets per worker thread
// Example 0: 	aCC := []CConnectionContext{{Host: "localhost", UserId: "root", Password: "Smash123", Port: 20901}, {Host: "windesk", UserId:
//		"MyUserId",Password: "MyPassword", Port: 20901}, {Host: "domain.com", UserId: "johndon", Password: "MySecret", Port: 20901}}
//		p.StartSocketPoolEx(aCC, 2)
func (p *CSocketPool) StartSocketPoolEx(aCC []CConnectionContext, maxSocketsPerThread uint32, args ...uint16) bool {
	if p.IsStarted() {
		panic("Cannot restart this pool")
	}
	avg := 1 //true
	size := len(args)
	if size > 0 && 0 == args[0] {
		avg = 0
	}
	size = len(aCC)
	if 0 == size {
		panic("Connection context array cannot be empty")
	}
	if 0 == maxSocketsPerThread {
		maxSocketsPerThread = 1
	}
	threads := size / int(maxSocketsPerThread)
	actual := threads * int(maxSocketsPerThread)
	if actual < size {
		actual += int(maxSocketsPerThread)
		size = actual
		threads += 1
		diff := size - len(aCC)
		ad := make([]CConnectionContext, diff)
		for i, _ := range ad {
			ad[i] = aCC[0]
		}
		aCC = append(aCC, ad...)
	}
	ok := p.startPool(maxSocketsPerThread, uint16(threads), avg)
	if !ok {
		return false
	}
	p.postProcess(aCC)
	return (p.GetConnectedSockets() > 0)
}
