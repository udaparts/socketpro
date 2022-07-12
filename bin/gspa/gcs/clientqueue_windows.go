package gcs

import (
	"gspa"
	"unsafe"
)

func GetWorkDirectory() string {
	r, _, _ := getClientWorkDirectory.Call()
	return utf8ToString(r)
}

func SetWorkDirectory(dir string) {
	var p uintptr
	bytes := ([]byte)(dir)
	if len(dir) > 0 {
		p = uintptr(unsafe.Pointer(&bytes[0]))
	}
	setClientWorkDirectory.Call(p)
}

func IsClientQueueIndexPossiblyCrashed() bool {
	r, _, _ := isClientQueueIndexPossiblyCrashed.Call()
	return (byte(r) > 0)
}

func SetMessageQueuePassword(pwd string) bool {
	var p uintptr
	bytes := ([]byte)(pwd)
	if len(pwd) > 0 {
		p = uintptr(unsafe.Pointer(&bytes[0]))
	}
	r, _, _ := setMessageQueuePassword.Call(p)
	return (byte(r) > 0)
}

func (cq *ClientQueue) StartQueue(qName string, ttl uint32, secure bool) bool {
	h := uintptr(cq.s.hSocket)
	var qn uintptr
	if len(qName) > 0 {
		bytes := ([]byte)(qName)
		qn = uintptr(unsafe.Pointer(&bytes[0]))
	}
	var sec uintptr
	if secure {
		sec = 1
	}
	var shared uintptr
	r, _, _ := startQueue.Call(h, qn, sec, shared, uintptr(ttl))
	return (byte(r) > 0)
}

func (cq *ClientQueue) StopQueue(permanent ...bool) *ClientQueue {
	var del uintptr
	h := uintptr(cq.s.hSocket)
	if len(permanent) > 0 && permanent[0] {
		del = 1
	}
	stopQueue.Call(h, del)
	return cq
}

func (cq *ClientQueue) GetTTL() uint32 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getTTL.Call(h)
	return uint32(r)
}

func (cq *ClientQueue) RemoveByTTL() uint64 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := removeQueuedRequestsByTTL.Call(h)
	return uint64(r)
}

func (cq *ClientQueue) GetQueueOpenStatus() gspa.QueueStatus {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getClientQueueStatus.Call(h)
	return gspa.QueueStatus(r)
}

func (cq *ClientQueue) GetMessagesInDequeuing() uint32 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getMessagesInDequeuing.Call(h)
	return uint32(r)
}

func (cq *ClientQueue) Reset() *ClientQueue {
	h := uintptr(cq.s.hSocket)
	resetQueue.Call(h)
	return cq
}

func (cq *ClientQueue) AppendTo(handles []USocket_Client_Handle) bool {
	count := len(handles)
	if count == 0 {
		return true
	}
	h := uintptr(cq.s.hSocket)
	pHandles := uintptr(unsafe.Pointer(&handles[0]))
	r, _, _ := pushQueueTo.Call(h, pHandles, uintptr(count))
	return (byte(r) > 0)
}

func (cq *ClientQueue) AppendOne(newOne ClientQueue) bool {
	handles := make([]USocket_Client_Handle, 1)
	handles[0] = newOne.s.hSocket
	return cq.AppendTo(handles)
}

func (cq *ClientQueue) EndJob() bool {
	h := uintptr(cq.s.hSocket)
	r, _, _ := endJob.Call(h)
	return (byte(r) > 0)
}

func (cq *ClientQueue) StartJob() bool {
	h := uintptr(cq.s.hSocket)
	cq.s.cs.Lock()
	cq.index = uint32(cq.s.qCallback.Len())
	r, _, _ := startJob.Call(h)
	cq.s.cs.Unlock()
	return (byte(r) > 0)
}

func (cq *ClientQueue) AbortJob() bool {
	h := uintptr(cq.s.hSocket)
	cq.s.cs.Lock()
	aborted := uint32(cq.s.qCallback.Len()) - cq.index
	r, _, _ := abortJob.Call(h)
	ok := (byte(r) > 0)
	if ok {
		for aborted > 0 && cq.s.qCallback.Len() > 0 {
			cq.s.qCallback.PopBack()
			aborted--
		}
	}
	cq.s.cs.Unlock()
	return ok
}

func (cq *ClientQueue) GetJobSize() uint64 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getJobSize.Call(h)
	return uint64(r)
}

func (cq *ClientQueue) GetLastIndex() uint64 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getQueueLastIndex.Call(h)
	return uint64(r)
}

func (cq *ClientQueue) CancelQueuedRequests(startIndex uint64, endIndex uint64) uint64 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := cancelQueuedRequestsByIndex.Call(h, uintptr(startIndex), uintptr(endIndex))
	return uint64(r)
}

func (cq *ClientQueue) IsDequeueEnabled() bool {
	h := uintptr(cq.s.hSocket)
	r, _, _ := isDequeueEnabled.Call(h)
	return (byte(r) > 0)
}

func (cq *ClientQueue) IsAvailable() bool {
	h := uintptr(cq.s.hSocket)
	r, _, _ := isQueueStarted.Call(h)
	return (byte(r) > 0)
}

func (cq *ClientQueue) IsDequeueShared() bool {
	h := uintptr(cq.s.hSocket)
	r, _, _ := isDequeueShared.Call(h)
	return (byte(r) > 0)
}

func (cq *ClientQueue) GetQueueFileName() string {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getQueueFileName.Call(h)
	return utf8ToString(r)
}

func (cq *ClientQueue) GetQueueName() string {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getQueueName.Call(h)
	return utf8ToString(r)
}

func (cq *ClientQueue) IsSecure() bool {
	h := uintptr(cq.s.hSocket)
	r, _, _ := isQueueSecured.Call(h)
	return (byte(r) > 0)
}

func (cq *ClientQueue) GetMessageCount() uint64 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getMessageCount.Call(h)
	return uint64(r)
}

func (cq *ClientQueue) GetQueueSize() uint64 {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getQueueSize.Call(h)
	return uint64(r)
}

func (cq *ClientQueue) EnableRoutingQueueIndex(enable bool) *ClientQueue {
	var ok uintptr
	h := uintptr(cq.s.hSocket)
	if enable {
		ok = 1
	}
	enableRoutingQueueIndex.Call(h, ok)
	return cq
}

func (cq *ClientQueue) IsRoutingQueueIndexEnabled() bool {
	h := uintptr(cq.s.hSocket)
	r, _, _ := isRoutingQueueIndexEnabled.Call(h)
	return (byte(r) > 0)
}

func (cq *ClientQueue) GetOptimistic() gspa.Optimistic {
	h := uintptr(cq.s.hSocket)
	r, _, _ := getOptimistic.Call(h)
	return gspa.Optimistic(r)
}

func (cq *ClientQueue) SetOptimistic(optimistic gspa.Optimistic) *ClientQueue {
	opt := uintptr(optimistic)
	h := uintptr(cq.s.hSocket)
	setOptimistic.Call(h, opt)
	return cq
}
