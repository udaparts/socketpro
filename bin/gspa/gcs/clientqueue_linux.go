package gcs

// #include <stdint.h>
// #include <stdbool.h>
// #include <string.h>
// typedef uint64_t USocket_Client_Handle;
// bool IsClientQueueIndexPossiblyCrashed();
// const char* GetClientWorkDirectory();
// void SetClientWorkDirectory(const char *dir);
// const char* GetClientWorkDirectory();
// bool SetMessageQueuePassword(const char *pwd);
// bool StartQueue(USocket_Client_Handle h, const char *qName, bool secure, bool dequeueShared, unsigned int ttl);
// void StopQueue(USocket_Client_Handle h, bool permanent);
// bool DequeuedResult(USocket_Client_Handle h);
// unsigned int GetMessagesInDequeuing(USocket_Client_Handle h);
// uint64_t GetMessageCount(USocket_Client_Handle h);
// uint64_t GetQueueSize(USocket_Client_Handle h);
// bool IsQueueSecured(USocket_Client_Handle h);
// bool IsQueueStarted(USocket_Client_Handle h);
// const char* GetQueueName(USocket_Client_Handle h);
// const char* GetQueueFileName(USocket_Client_Handle h);
// bool IsDequeueEnabled(USocket_Client_Handle h);
// bool AbortJob(USocket_Client_Handle h);
// bool StartJob(USocket_Client_Handle h);
// bool EndJob(USocket_Client_Handle h);
// uint64_t GetJobSize(USocket_Client_Handle h);
// uint64_t GetQueueLastIndex(USocket_Client_Handle h);
// uint64_t CancelQueuedRequestsByIndex(USocket_Client_Handle h, uint64_t startIndex, uint64_t endIndex);
// bool IsDequeueShared(USocket_Client_Handle h);
// int GetClientQueueStatus(USocket_Client_Handle h);
// bool PushQueueTo(USocket_Client_Handle src, const USocket_Client_Handle *targets, unsigned int count);
// unsigned int GetTTL(USocket_Client_Handle h);
// uint64_t RemoveQueuedRequestsByTTL(USocket_Client_Handle h);
// void ResetQueue(USocket_Client_Handle h);
// uint64_t GetLastQueueMessageTime(USocket_Client_Handle h);
// void AbortDequeuedMessage(USocket_Client_Handle h);
// bool IsDequeuedMessageAborted(USocket_Client_Handle h);
// void EnableRoutingQueueIndex(USocket_Client_Handle h, bool enable);
// bool IsRoutingQueueIndexEnabled(USocket_Client_Handle h);
// int GetOptimistic(USocket_Client_Handle h);
// void SetOptimistic(USocket_Client_Handle h, int optimistic);
import "C"

import "gspa"
import "unsafe"

func GetWorkDirectory() string {
	str := C.GetClientWorkDirectory()
	chars := C.strlen(str)
	return C.GoStringN(str, C.int(chars))
}

func SetWorkDirectory(dir string) {
	var p unsafe.Pointer
	bytes := ([]byte)(dir)
	if len(dir) > 0 {
		p = unsafe.Pointer(&bytes[0])
	}
	C.SetClientWorkDirectory((*C.char)(p))
}

func IsClientQueueIndexPossiblyCrashed() bool {
	r := C.IsClientQueueIndexPossiblyCrashed()
	return bool(r)
}

func SetMessageQueuePassword(pwd string) bool {
	var p unsafe.Pointer
	bytes := ([]byte)(pwd)
	if len(pwd) > 0 {
		p = unsafe.Pointer(&bytes[0])
	}
	r := C.SetMessageQueuePassword((*C.char)(p))
	return bool(r)
}

func (cq *ClientQueue) StartQueue(qName string, ttl uint32, secure bool) bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	var qn *C.char
	if len(qName) > 0 {
		bytes := ([]byte)(qName)
		qn = (*C.char)(unsafe.Pointer(&bytes[0]))
	}
	sec := C.bool(secure)
	var shared C.bool = false
	r := C.StartQueue(h, qn, sec, shared, C.uint32_t(ttl))
	return bool(r)
}

func (cq *ClientQueue) StopQueue(permanent ...bool) *ClientQueue {
	var del C.bool = false
	h := C.USocket_Client_Handle(cq.s.hSocket)
	if len(permanent) > 0 && permanent[0] {
		del = true
	}
	C.StopQueue(h, del)
	return cq
}

func (cq *ClientQueue) GetTTL() uint32 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetTTL(h)
	return uint32(r)
}

func (cq *ClientQueue) RemoveByTTL() uint64 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.RemoveQueuedRequestsByTTL(h)
	return uint64(r)
}

func (cq *ClientQueue)GetQueueOpenStatus() gspa.QueueStatus {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetClientQueueStatus(h)
	return gspa.QueueStatus(r)
}

func (cq *ClientQueue) GetMessagesInDequeuing() uint32 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetMessagesInDequeuing(h)
	return uint32(r)
}

func (cq *ClientQueue) Reset() *ClientQueue {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	C.ResetQueue(h)
	return cq
}

func (cq *ClientQueue) AppendTo(handles []USocket_Client_Handle) bool {
	count := len(handles)
	if count == 0 {
		return true
	}
	h := C.USocket_Client_Handle(cq.s.hSocket)
	pHandles := (*C.USocket_Client_Handle)(unsafe.Pointer(&handles[0]))
	r := C.PushQueueTo(h, pHandles, C.uint32_t(count))
	return bool(r)
}

func (cq *ClientQueue) AppendOne(newOne ClientQueue) bool {
	handles := make([]USocket_Client_Handle, 1)
	handles[0] = newOne.s.hSocket
	return cq.AppendTo(handles)
}

func (cq *ClientQueue) EndJob() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.EndJob(h)
	return bool(r)
}

func (cq *ClientQueue) StartJob() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	cq.s.cs.Lock()
	cq.index = uint32(cq.s.qCallback.Len())
	r := C.StartJob(h)
	cq.s.cs.Unlock()
	return bool(r)
}

func (cq *ClientQueue) AbortJob() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	cq.s.cs.Lock()
	aborted := uint32(cq.s.qCallback.Len()) - cq.index
	r := C.AbortJob(h)
	ok := bool(r)
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
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetJobSize(h)
	return uint64(r)
}

func (cq *ClientQueue) GetLastIndex() uint64 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetQueueLastIndex(h)
	return uint64(r)
}

func (cq *ClientQueue) CancelQueuedRequests(startIndex uint64, endIndex uint64) uint64 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.CancelQueuedRequestsByIndex(h, C.uint64_t(startIndex), C.uint64_t(endIndex))
	return uint64(r)
}

func (cq *ClientQueue) IsDequeueEnabled() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.IsDequeueEnabled(h)
	return bool(r)
}

func (cq *ClientQueue) IsAvailable() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.IsQueueStarted(h)
	return bool(r)
}

func (cq *ClientQueue) IsDequeueShared() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.IsDequeueShared(h)
	return bool(r)
}

func (cq *ClientQueue) GetQueueFileName() string {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetQueueFileName(h)
	return toAStr(r)
}

func (cq *ClientQueue) GetQueueName() string {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetQueueName(h)
	return toAStr(r)
}

func (cq *ClientQueue) IsSecure() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.IsQueueSecured(h)
	return bool(r)
}

func (cq *ClientQueue) GetMessageCount() uint64 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetMessageCount(h)
	return uint64(r)
}

func (cq *ClientQueue) GetQueueSize() uint64 {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetQueueSize(h)
	return uint64(r)
}

func (cq *ClientQueue) EnableRoutingQueueIndex(enable bool) *ClientQueue {
	ok := C.bool(enable)
	h := C.USocket_Client_Handle(cq.s.hSocket)
	C.EnableRoutingQueueIndex(h, ok)
	return cq
}

func (cq *ClientQueue) IsRoutingQueueIndexEnabled() bool {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.IsRoutingQueueIndexEnabled(h)
	return bool(r)
}

func (cq *ClientQueue) GetOptimistic() gspa.Optimistic {
	h := C.USocket_Client_Handle(cq.s.hSocket)
	r := C.GetOptimistic(h)
	return gspa.Optimistic(r)
}

func (cq *ClientQueue) SetOptimistic(optimistic gspa.Optimistic) *ClientQueue {
	opt := C.int(optimistic)
	h := C.USocket_Client_Handle(cq.s.hSocket)
	C.SetOptimistic(h, opt)
	return cq
}
