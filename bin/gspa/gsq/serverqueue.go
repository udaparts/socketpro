package gsq

import (
	"gspa"
	"gspa/gcs"
	"sync"
)

const (
	//use built-in chat service id
	SidQueue = gspa.Chat

	//queue-related request ids
	Enqueue      = gspa.ReservedTwo + 1
	Dequeue      = Enqueue + 1
	StartTrans   = Enqueue + 2
	EndTrans     = Enqueue + 3
	Flush        = Enqueue + 4
	Close        = Enqueue + 5
	GetKeys      = Enqueue + 6
	EnqueueBatch = Enqueue + 7

	//this id is designed for notifying dequeue batch size from server to client
	BatchSizeNotified = Enqueue + 19

	//error code
	QUEUE_OK                            int32 = 0
	QUEUE_TRANS_ALREADY_STARTED         int32 = 1
	QUEUE_TRANS_STARTING_FAILED         int32 = 2
	QUEUE_TRANS_NOT_STARTED_YET         int32 = 3
	QUEUE_TRANS_COMMITTING_FAILED       int32 = 4
	QUEUE_DEQUEUING                     int32 = 5
	QUEUE_OTHER_WORKING_WITH_SAME_QUEUE int32 = 6
	QUEUE_CLOSE_FAILED                  int32 = 7
	QUEUE_ENQUEUING_FAILED              int32 = 8
)

type QueueInfo struct {
	Messages uint64
	FileSize uint64
}

type DeqInfo struct {
	QueueInfo
	DeMessages uint32
	DeBytes    uint32
}

type CAsyncQueue struct {
	ah            *gcs.CAsyncHandler
	cs            sync.Mutex
	batchSize     uint32
	keyDequeue    string
	messageQueued DMessageQueued
	deq           DDequeue
	rr            gcs.RequestProcessedEvent
}

type QueueKeys []*gspa.AStr

type DMessageQueued func(sender *CAsyncQueue)
type DDequeue func(sender *CAsyncQueue, di *DeqInfo)
type DQError func(sender *CAsyncQueue, ec int32)
type DEnqueue func(sender *CAsyncQueue, index uint64)
type DFlushQueue func(sender *CAsyncQueue, qi *QueueInfo)
type DGetKeys func(sender *CAsyncQueue, qk QueueKeys)

func BatchMessage(des *gspa.CUQueue, reqId gspa.ReqId, src *gspa.CUQueue) {
	if des == nil {
		panic("Destination buffer cannot be null!")
	}
	bytes := des.GetSize()
	if bytes == 0 {
		des.SaveUInt(1)
	} else if bytes >= 4 {
		count := *(*uint32)(des.GetBuffer())
		count++
		*(*uint32)(des.GetBuffer()) = count
	} else {
		panic("Invalid destination buffer found")
	}
	des.SaveUShort(uint16(reqId))
	if src != nil {
		bs := src.GetBytes()
		des.SaveBytes(&bs)
	} else {
		des.SaveUInt(0)
	}
}

func (aq *CAsyncQueue) EnqueueBatch(key string, buffer *gspa.CUQueue, de ...DEnqueue) <-chan interface{} {
	if buffer == nil || buffer.GetSize() < 10 {
		// 2 * sizeof (uint32) + sizeof (uint16))
		panic("bad operation because no message batched yet!")
	}
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as)
	buff.PushBytes(buffer.PopBytes())
	defer buff.Recycle()
	res := make(chan interface{})
	res = aq.ah.Send(EnqueueBatch, buff, func(ar *gcs.CAsyncResult) {
		var e DEnqueue
		if len(de) > 0 {
			e = de[0]
		}
		index := ar.Buffer.LoadULong()
		if e != nil {
			e(FindServerQueue(ar.Sender), index)
		}
		res <- index
		close(res)
	})
	buffer.SetSize(0)
	return res
}

func (aq *CAsyncQueue) Enqueue(key string, idMessage gspa.ReqId, buffer *gspa.CUQueue, de ...DEnqueue) <-chan interface{} {
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as).SaveUShort(uint16(idMessage))
	if buffer != nil {
		buff.PushBytes(buffer.PopBytes())
	}
	defer buff.Recycle()
	res := make(chan interface{})
	res = aq.ah.Send(Enqueue, buff, func(ar *gcs.CAsyncResult) {
		var e DEnqueue
		if len(de) > 0 {
			e = de[0]
		}
		index := ar.Buffer.LoadULong()
		if e != nil {
			e(FindServerQueue(ar.Sender), index)
		}
		res <- index
		close(res)
	})
	return res
}

func (aq *CAsyncQueue) EnqueueEx(key string, idMessage gspa.ReqId, buffer *gspa.CUQueue) bool {
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as).SaveUShort(uint16(idMessage))
	if buffer != nil {
		buff.PushBytes(buffer.PopBytes())
	}
	defer buff.Recycle()
	return aq.ah.SendRequest(Enqueue, buff, nil, nil, nil)
}

func (aq *CAsyncQueue) StartQueueTrans(key string, qe ...DQError) <-chan interface{} {
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as)
	defer buff.Recycle()
	cq := aq.ah.GetClientQueue()
	if cq.IsAvailable() {
		cq.StartJob()
	}
	res := make(chan interface{})
	res = aq.ah.Send(StartTrans, buff, func(ar *gcs.CAsyncResult) {
		var err DQError
		ec := ar.Buffer.LoadInt()
		if len(qe) > 0 {
			err = qe[0]
		}
		if err != nil {
			err(FindServerQueue(ar.Sender), ec)
		}
		res <- ec
		close(res)
	})
	return res
}

func (qk QueueKeys) String() string {
	s := "["
	for i, k := range qk {
		if i > 0 {
			s += ","
		}
		if k != nil {
			s += k.String()
		}
	}
	s += "]"
	return s
}

func (aq *CAsyncQueue) GetKeys(gk ...DGetKeys) <-chan interface{} {
	res := make(chan interface{})
	res = aq.ah.Send(GetKeys, nil, func(ar *gcs.CAsyncResult) {
		var k DGetKeys
		if len(gk) > 0 {
			k = gk[0]
		}
		qks := QueueKeys(ar.Buffer.LoadAStringArray())
		if k != nil {
			k(FindServerQueue(ar.Sender), qks)
		}
		res <- qks
		close(res)
	})
	return res
}

func (aq *CAsyncQueue) EndQueueTrans(qe DQError, rollback ...bool) <-chan interface{} {
	rb := false
	if len(rollback) > 0 {
		rb = rollback[0]
	}
	buff := gspa.MakeBuffer().SaveBool(rb)
	defer buff.Recycle()
	res := make(chan interface{})
	res = aq.ah.Send(EndTrans, buff, func(ar *gcs.CAsyncResult) {
		ec := ar.Buffer.LoadInt()
		if qe != nil {
			qe(FindServerQueue(ar.Sender), ec)
		}
		res <- ec
		close(res)
	})
	cq := aq.ah.GetClientQueue()
	if cq.IsAvailable() {
		if rb {
			cq.AbortJob()
		} else {
			cq.EndJob()
		}
	}
	return res
}

func (aq *CAsyncQueue) CloseQueue(key string, qe DQError, permanent ...bool) <-chan interface{} {
	p := false
	if len(permanent) > 0 {
		p = permanent[0]
	}
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as).SaveBool(p)
	defer buff.Recycle()
	res := make(chan interface{})
	res = aq.ah.Send(Close, buff, func(ar *gcs.CAsyncResult) {
		ec := ar.Buffer.LoadInt()
		if qe != nil {
			qe(FindServerQueue(ar.Sender), ec)
		}
		res <- ec
		close(res)
	})
	return res
}

func (aq *CAsyncQueue) FlushQueue(key string, fq DFlushQueue, option ...gspa.Optimistic) <-chan interface{} {
	op := gspa.MemoryCached
	if len(option) > 0 {
		op = option[0]
	}
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as).SaveInt(int32(op))
	defer buff.Recycle()
	res := make(chan interface{})
	res = aq.ah.Send(Flush, buff, func(ar *gcs.CAsyncResult) {
		qi := new(QueueInfo)
		qi.Messages = ar.Buffer.LoadULong()
		qi.FileSize = ar.Buffer.LoadULong()
		if fq != nil {
			fq(FindServerQueue(ar.Sender), qi)
		}
		res <- qi
		close(res)
	})
	return res
}

func (aq *CAsyncQueue) Dequeue(key string, d DDequeue, timeout ...uint32) <-chan interface{} {
	var max_time uint32
	if len(timeout) > 0 {
		max_time = timeout[0]
	}
	as := gspa.AStr(key)
	buff := gspa.MakeBuffer().SaveAString(&as).SaveUInt(max_time)
	defer buff.Recycle()
	res := make(chan interface{})
	aq.cs.Lock()
	aq.keyDequeue = key
	aq.deq = d
	aq.cs.Unlock()
	res = aq.ah.Send(Dequeue, buff, func(ar *gcs.CAsyncResult) {
		di := new(DeqInfo)
		di.Messages = ar.Buffer.LoadULong()
		di.FileSize = ar.Buffer.LoadULong()
		ret := ar.Buffer.LoadULong()
		di.DeMessages = uint32(ret)
		di.DeBytes = uint32(ret >> 32)
		if d != nil {
			d(FindServerQueue(ar.Sender), di)
		}
		res <- di
		close(res)
	})
	return res
}

func (aq *CAsyncQueue) SetMessageQueued(cb DMessageQueued) *CAsyncQueue {
	aq.cs.Lock()
	aq.messageQueued = cb
	aq.cs.Unlock()
	return aq
}

func (aq *CAsyncQueue) SetResultReturned(cb gcs.RequestProcessedEvent) *CAsyncQueue {
	aq.cs.Lock()
	aq.rr = cb
	aq.cs.Unlock()
	return aq
}

func (aq *CAsyncQueue) GetLastDequeueCallback() DDequeue {
	aq.cs.Lock()
	d := aq.deq
	aq.cs.Unlock()
	return d
}

func (aq *CAsyncQueue) requestProcessed(reqId gspa.ReqId, buffer *gspa.CUQueue) {
	switch reqId {
	case Close, Enqueue:
		buffer.SetSize(0)
		break
	case BatchSizeNotified:
		aq.cs.Lock()
		aq.batchSize = buffer.LoadUInt()
		aq.cs.Unlock()
		break
	}
	aq.cs.Lock()
	if aq.rr != nil {
		aq.rr(reqId, buffer)
	}
	aq.cs.Unlock()
}

func (aq *CAsyncQueue) baseReq(reqId gspa.ReqId) {
	if reqId == gspa.MessageQueued {
		aq.cs.Lock()
		key := aq.keyDequeue
		d := aq.deq
		mq := aq.messageQueued
		aq.cs.Unlock()
		if mq != nil {
			mq(aq)
		}
		if d != nil {
			//we automatically send a request to dequeue messages after a notification message arrives
			//because a new message is enqueued at server side
			aq.Dequeue(key, d)
		}
	}
}

//implementation of interface gcs.ITie::GetHandler
func (aq *CAsyncQueue) GetHandler() *gcs.CAsyncHandler {
	return aq.ah
}

var (
	csAQ   sync.Mutex
	gMapAQ = make(map[*gcs.CAsyncHandler]*CAsyncQueue)
)

//implementation of interface gcs.ITie::Tie
//tie an instnace of CAsyncQueue aq with a given instence of base handler gcs.CAsyncHandler ah
func (aq *CAsyncQueue) Tie(ah *gcs.CAsyncHandler) bool {
	if ah != nil && aq != nil {
		ah.SetTie(aq) //Tie aq with ah
		aq.cs.Lock()
		aq.ah = ah //for gcs.ITie::GetHandler()
		//subscribe two events, BaseRequestProcessed and RequestProcessed
		ah.SetOnBaseRequestProcessed(aq.baseReq)
		ah.SetOnRequestProcessed(aq.requestProcessed)
		aq.cs.Unlock()
		csAQ.Lock()
		//register aq so that we can use FindServerQueue to find an instance of CAsyncQueue later
		gMapAQ[ah] = aq
		csAQ.Unlock()
		return true
	}
	return false
}

//implementation of interface gcs.ITie::Untie
//this required method will be automatically called by a socket pool when the pool is about to be shutdown
func (aq *CAsyncQueue) Untie() {
	if aq != nil {
		ah := aq.ah
		if ah != nil {
			csAQ.Lock()
			//unreqister aq
			delete(gMapAQ, ah)
			csAQ.Unlock()
			ah.SetTie(nil)
		}
	}
}

//find an instance of CAsyncQueue from a tied/given base handler ah
func FindServerQueue(ah *gcs.CAsyncHandler) *CAsyncQueue {
	csAQ.Lock()
	defer csAQ.Unlock()
	if val, ok := gMapAQ[ah]; ok {
		return val
	}
	return nil
}
