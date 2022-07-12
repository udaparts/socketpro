package gcs

// #cgo CFLAGS: -g -Wall
// #include <stdint.h>
// #include <stdbool.h>
// #include <wchar.h>
// #include <string.h>
// typedef uint64_t USocket_Client_Handle;
//
// typedef void(*PSocketPoolCallback) (uint32_t pid, int spe, USocket_Client_Handle handle);
// uint32_t CreateSocketPool(PSocketPoolCallback spc, uint32_t maxSocketsPerThread, uint32_t maxThreads, bool bAvg, int ta);
// bool DestroySocketPool(uint32_t poolId);
// bool IsAvg(uint32_t poolId);
// uint32_t GetThreadCount(uint32_t poolId);
// bool DisconnectAll(uint32_t poolId);
// uint32_t GetDisconnectedSockets(uint32_t poolId);
// uint32_t GetSocketsPerThread(uint32_t poolId);
// uint32_t GetLockedSockets(uint32_t poolId);
// uint32_t GetIdleSockets(uint32_t poolId);
// uint32_t GetConnectedSockets(uint32_t poolId);
// bool Connect(USocket_Client_Handle h, const char* host, unsigned int portNumber, bool sync, bool v6);
// void SetPassword(USocket_Client_Handle h, const wchar_t *password);
// bool SwitchTo(USocket_Client_Handle h, unsigned int serviceId);
// void spc_cgo(uint32_t pid, int spe, USocket_Client_Handle handle);
// const char* GetUClientAppName();
// const char* GetUClientSocketVersion();
// bool GetQueueAutoMergeByPool(unsigned int poolId);
// void SetQueueAutoMergeByPool(unsigned int poolId, bool autoMerge);
// USocket_Client_Handle LockASocket(unsigned int poolId, unsigned int timeout, USocket_Client_Handle hSameThread);
// #cgo LDFLAGS: -lusocket
import "C"

import (
	//"fmt"
	"gspa"
	"sync"
	"unsafe"
)

var (
	gLock      sync.Mutex
	gPool      *CSocketPool
	gMapPool   = make(map[PoolId]*CSocketPool)
	gCs        sync.Mutex
	gMapHandle = make(map[C.USocket_Client_Handle]*CAsyncHandler)
)

//export spc_go
func spc_go(pid C.uint32_t, spe C.int, handle C.USocket_Client_Handle) {
	var pos int
	var me *CAsyncHandler
	var event DelPool
	poolId := PoolId(pid)
	pe := SocketPoolEvent(spe)

	//fmt.Println("PoolId:", poolId, pe, "Handle:", handle)
	var p *CSocketPool
	if pe == SpeStarted {
		p = gPool
		event = p.event
		p.poolId = poolId
		p.handlers = make([]*CAsyncHandler, 0)
		gMapPool[poolId] = p
	} else if pe == SpeShutdown {
		p = gMapPool[poolId]
		p.Mutex.Lock()
		event = p.event
		p.poolId = 0
		p.Mutex.Unlock()
	} else if pe == SpeSocketCreated {
		var h CAsyncHandler
		var msg Message
		var cq ClientQueue
		h.hSocket = USocket_Client_Handle(handle)
		h.cq = &cq
		h.msg = &msg
		msg.s = &h
		cq.s = &h
		me = &h
		gCs.Lock()
		gMapHandle[handle] = me
		gCs.Unlock()
		p = gMapPool[poolId]
		h.pool = p
		h.SetRecvTimeout(p.recvTimeout).SetConnTimeout(p.connTimeout).SetAutoConn(p.autoConn)
		event = p.event
		h.setCallbacks()
		p.Mutex.Lock()
		p.handlers = append(p.handlers, me)
		p.Mutex.Unlock()
	} else if pe == SpeCreatingThread {
		p = gMapPool[poolId]
		p.Mutex.Lock()
		event = p.event
		p.Mutex.Unlock()
	} else if pe == SpeThreadCreated {
		p = gMapPool[poolId]
		p.Mutex.Lock()
		event = p.event
		p.Mutex.Unlock()
	} else {
		gLock.Lock()
		p = gMapPool[poolId]
		gLock.Unlock()
		switch pe {
		case SpeSocketKilled:
			{
				p.Mutex.Lock()
				event = p.event
				pos = schIndex(p.handlers, USocket_Client_Handle(handle))
				if pos != -1 {
					me = p.handlers[pos]
					p.handlers = append(p.handlers[0:pos], p.handlers[pos+1:]...)
				}
				p.Mutex.Unlock()
				gCs.Lock()
				delete(gMapHandle, handle)
				gCs.Unlock()
			}
			break
		case SpeConnected:
			{
				p.Mutex.Lock()
				event = p.event
				pos = schIndex(p.handlers, USocket_Client_Handle(handle))
				if pos != -1 {
					me = p.handlers[pos]
					if me.IsOpened() {
						me.SetSockOpt(gspa.RcvBuf, 116800, gspa.Socket)
						me.SetSockOpt(gspa.SndBuf, 116800, gspa.Socket)
						if p.sslAuth != nil && me.GetEncryptionMethod() == gspa.TLSv1 {
							if !p.sslAuth(me) {
								p.Mutex.Unlock()
								return
							}
						}
						if len(me.connContext.Password) > 0 {
							text := []rune(me.connContext.Password)
							C.SetPassword(handle, (*C.wchar_t)(&text[0]))
						} else {
							C.SetPassword(handle, (*C.wchar_t)(unsafe.Pointer(nil)))
						}
						me.StartBatching()
						C.SwitchTo(handle, C.uint32_t(p.serviceId))
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
				pos = schIndex(p.handlers, USocket_Client_Handle(handle))
				if pos != -1 {
					me = p.handlers[pos]
				}
			}
			p.Mutex.Unlock()
			if pe == SpeQueueMergedFrom {
				p.fromHandler = me
			} else if pe == SpeQueueMergedTo {
				p.fromHandler.appendTo(me)
				p.fromHandler = nil
			}
			break
		}
	}
	if pe == SpeConnected && me.IsOpened() {
		p.setQueue(pos)
	}
	if event != nil {
		event(pe, me)
	}
	if pe == SpeSocketKilled && me.tie != nil {
		me.tie.Untie()
	}
}

func (p *CSocketPool) startPool(maxSocketsPerThread uint32, threads uint16, avg int) bool {
	b := (avg != 0)
	gLock.Lock()
	gPool = p
	id := C.CreateSocketPool((C.PSocketPoolCallback)(unsafe.Pointer(C.spc_cgo)), C.uint32_t(maxSocketsPerThread), C.uint32_t(threads), C.bool(b), C.int(0))
	gLock.Unlock()
	p.poolId = PoolId(id)
	return id > 0
}

func (p *CSocketPool) ShutdownPool() bool {
	pid := p.GetPoolId()
	ok := bool(C.DestroySocketPool(C.uint32_t(pid)))
	gLock.Lock()
	delete(gMapPool, pid)
	gLock.Unlock()
	return ok
}

func (p *CSocketPool) GetQueueAutoMerge() bool {
	r := C.GetQueueAutoMergeByPool(C.uint32_t(p.GetPoolId()))
	return bool(r)
}

func (p *CSocketPool) SetQueueAutoMerge(am bool) *CSocketPool {
	var b C.bool
	if am {
		b = true
	}
	C.SetQueueAutoMergeByPool(C.uint32_t(p.GetPoolId()), b)
	return p
}

func (p *CSocketPool) IsAvg() bool {
	return bool(C.IsAvg(C.uint32_t(p.GetPoolId())))
}

func (p *CSocketPool) IsStarted() bool {
	r := C.GetThreadCount(C.uint32_t(p.GetPoolId()))
	return (r > 0)
}

func (p *CSocketPool) GetThreadsCreated() uint32 {
	r := C.GetThreadCount(C.uint32_t(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) DisconnectAll() bool {
	return bool(C.DisconnectAll(C.uint32_t(p.GetPoolId())))
}

func (p *CSocketPool) GetDisconnectedSockets() uint32 {
	r := C.GetDisconnectedSockets(C.uint32_t(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetSocketsPerThread() uint32 {
	r := C.GetSocketsPerThread(C.uint32_t(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetLockedSockets() uint32 {
	r := C.GetLockedSockets(C.uint32_t(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetIdleSockets() uint32 {
	r := C.GetIdleSockets(C.uint32_t(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) GetConnectedSockets() uint32 {
	r := C.GetConnectedSockets(C.uint32_t(p.GetPoolId()))
	return uint32(r)
}

func (p *CSocketPool) Lock(timeout ...uint32) *CAsyncHandler {
	var samethread C.USocket_Client_Handle
	var forever uint32 = 0xffffffff
	if len(timeout) > 0 {
		forever = timeout[0]
	}
	h := C.LockASocket(C.uint32_t(p.GetPoolId()), C.uint32_t(forever), samethread)
	p.Mutex.Lock()
	defer p.Mutex.Unlock()
	for i, _ := range p.handlers {
		if C.USocket_Client_Handle(p.handlers[i].hSocket) == h {
			return p.handlers[i]
		}
	}
	return nil
}

func (p *CSocketPool) postProcess(acc []CConnectionContext) {
	var first C.bool = true
	for i, _ := range acc {
		handler := p.handlers[i]
		handler.connContext = acc[i]
		handler.SetUID(acc[i].UserId)
		handler.SetEncryptionMethod(acc[i].EncryptionMethod)
		h := C.USocket_Client_Handle(handler.hSocket)
		var host *C.char
		var server []byte
		if len(acc[i].Host) > 0 {
			server = []byte(acc[i].Host)
			host = (*C.char)(unsafe.Pointer(&server[0]))
		}
		if first {
			ok := C.Connect(h, host, C.uint32_t(acc[i].Port), first, C.bool(acc[i].V6))
			if !ok {
				continue
			}
			if !handler.WaitAll() {
				continue
			}
			if handler.GetConnectionState() < Connected {
				continue
			}
			first = false
		} else {
			C.Connect(h, host, C.uint32_t(acc[i].Port), first, C.bool(acc[i].V6))
		}
	}
}

func GetVersion() string {
	str := C.GetUClientSocketVersion()
	chars := C.strlen(str)
	return C.GoStringN(str, C.int(chars))
}

func GetAppName() string {
	str := C.GetUClientAppName()
	chars := C.strlen(str)
	return C.GoStringN(str, C.int(chars))
}
