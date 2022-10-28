package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"gspa/gsq"
	"strconv"
	"sync"
	"testing"

	"github.com/gammazero/deque"
)

var (
	Queue_Key          = "queue_name_0"
	ENCQUEUE_CYCLE int = 1024
	idMessage0         = gspa.ReservedTwo + 100
	idMessage1         = idMessage0 + 1
	idMessage2         = idMessage0 + 2
	idMessage3         = idMessage0 + 3
	idMessage4         = idMessage0 + 4
)

func print_sq(f <-chan interface{}) {
	res := <-f //wait until result comes from remote server
	switch t := res.(type) {
	case gsq.QueueKeys:
		fmt.Println("Queue keys", res.(gsq.QueueKeys))
	case uint64:
		fmt.Println("message index", res.(uint64))
	case int32:
		fmt.Println("error code", res.(int32))
	case *gsq.QueueInfo:
		fmt.Println("Queue info", res.(*gsq.QueueInfo))
	case *gsq.DeqInfo:
		fmt.Println("Dequeue info", res.(*gsq.DeqInfo))
	case gcs.SocketError: //session error
		fmt.Println(res.(gcs.SocketError))
	case gcs.ServerError: //error from remote server
		fmt.Println(res.(gcs.ServerError))
	default:
		fmt.Println("Unknown data type: ", t)
	}
}

func Test_sq_1(t *testing.T) {
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
	}
}

func Test_sq_2(t *testing.T) {
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	aq := gsq.FindServerQueue(sp.GetHandlers()[0])
	fs := aq.StartQueueTrans(Queue_Key)
	fe := aq.EndQueueTrans(nil)
	ff := aq.FlushQueue(Queue_Key, nil)
	fk := aq.GetKeys()
	fc := aq.CloseQueue(Queue_Key, nil)
	print_sq(fs)
	print_sq(fe)
	print_sq(ff)
	print_sq(fk)
	print_sq(fc)
}

func Test_batchenqueue(t *testing.T) {
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	sb := gspa.MakeBuffer()
	defer sb.Recycle()
	aq := gsq.FindServerQueue(sp.GetHandlers()[0])
	fs := aq.StartQueueTrans(Queue_Key)
	b := gspa.MakeBuffer().Save("Hello").Save("World")
	gsq.BatchMessage(sb, idMessage3, b)
	b.SetSize(0).Save(true, float64(234.456), "MyTestWhatever")
	gsq.BatchMessage(sb, idMessage4, b)
	fb := aq.EnqueueBatch(Queue_Key, sb)
	b.SetSize(0).Save("Charlie", "Ye")
	feq := aq.Enqueue(Queue_Key, idMessage3, b)
	fe := aq.EndQueueTrans(nil)
	print_sq(fs)
	print_sq(fb)
	print_sq(feq)
	print_sq(fe)
}

func Test_enqueue(t *testing.T) {
	var idMessage gspa.ReqId
	buff := gspa.MakeBuffer()
	defer buff.Recycle()
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	ah := sp.GetHandlers()[0]
	if !ok {
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	aq := gsq.FindServerQueue(sp.GetHandlers()[0])
	af := make([]<-chan interface{}, ENCQUEUE_CYCLE)
	fmt.Printf("Going to enqueue %d messages ......", ENCQUEUE_CYCLE)
	for n, _ := range af {
		str := strconv.Itoa(n) + " Object test"
		switch n % 3 {
		case 0:
			idMessage = idMessage0
		case 1:
			idMessage = idMessage1
		default:
			idMessage = idMessage2
		}
		buff.Save("SampleName", str, int32(n))
		af[n] = aq.Enqueue(Queue_Key, idMessage, buff)
		buff.SetSize(0)
	}
	for _, f := range af {
		print_sq(f)
	}
}

func Test_dequeue_low_performance(t *testing.T) {
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	aq := gsq.FindServerQueue(sp.GetHandlers()[0])
	aq.SetResultReturned(func(reqId gspa.ReqId, buffer *gspa.CUQueue) {
		switch reqId {
		case idMessage0, idMessage1, idMessage2:
			//process dequeued messages here
			name := *buffer.LoadString()
			str := *buffer.LoadString()
			index := buffer.LoadInt()
			fmt.Println(reqId, name, str, index)
		case idMessage3:
			s0 := *buffer.LoadString()
			s1 := *buffer.LoadString()
			fmt.Println(reqId, s0, s1)
		case idMessage4:
			b := buffer.LoadBool()
			d := buffer.LoadDouble()
			s := *buffer.LoadString()
			fmt.Println(reqId, b, d, s)
		default:
			break
		}
	})
	done := false
	for !done {
		res := <-aq.Dequeue(Queue_Key, nil)
		done = true
		switch ty := res.(type) {
		case *gsq.DeqInfo:
			di := res.(*gsq.DeqInfo)
			fmt.Println(di)
			if di.Messages != 0 {
				done = false
			}
		case gcs.SocketError: //session error
			se := res.(gcs.SocketError)
			t.Errorf("error code: %d, error message: %s", se.ErrCode, se.ErrMsg)
		case gcs.ServerError: //error from remote server
			se := res.(gcs.ServerError)
			t.Errorf("error code: %d, error message: %s", se.ErrCode, se.ErrMsg)
		default:
			fmt.Println("Unknown data type: ", ty)
		}
	}
	//wait until all dequeue message confirmations are completed before shutting down pool
	<-aq.GetKeys()
}

func Test_enqueue_ex(t *testing.T) {
	var n int
	var idMessage gspa.ReqId
	buff := gspa.MakeBuffer()
	defer buff.Recycle()
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	ah := sp.GetHandlers()[0]
	if !ok {
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	aq := gsq.FindServerQueue(sp.GetHandlers()[0])
	fmt.Printf("Going to enqueue %d messages ......", ENCQUEUE_CYCLE)
	for n < ENCQUEUE_CYCLE {
		str := strconv.Itoa(n) + " Object test"
		switch n % 3 {
		case 0:
			idMessage = idMessage0
		case 1:
			idMessage = idMessage1
		default:
			idMessage = idMessage2
		}
		buff.Save("SampleName", str, int32(n))
		if !aq.EnqueueEx(Queue_Key, idMessage, buff) {
			t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
			break
		} else {
			n++
			buff.SetSize(0)
		}
	}
	//wait until all messages are enqueued before shutting down pool
	aq.GetHandler().WaitAll()
}

func Test_dequeue_high_performance(t *testing.T) {
	sp := gcs.NewPool(gspa.Chat)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			aq := new(gsq.CAsyncQueue)
			aq.Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	aq := gsq.FindServerQueue(sp.GetHandlers()[0])
	aq.SetResultReturned(func(reqId gspa.ReqId, buffer *gspa.CUQueue) {
		switch reqId {
		case idMessage0, idMessage1, idMessage2:
			//process dequeued messages here
			name := *buffer.LoadString()
			str := *buffer.LoadString()
			index := buffer.LoadInt()
			fmt.Println(reqId, name, str, index)
		case idMessage3:
			s0 := *buffer.LoadString()
			s1 := *buffer.LoadString()
			fmt.Println(reqId, s0, s1)
		case idMessage4:
			b := buffer.LoadBool()
			d := buffer.LoadDouble()
			s := *buffer.LoadString()
			fmt.Println(reqId, b, d, s)
		default:
			break
		}
	})
	//high dequeue performance
	var cs sync.Mutex
	//protected by cs as it will be accessed from different threads
	var qf deque.Deque[<-chan interface{}]

	var d gsq.DDequeue
	d = func(sq *gsq.CAsyncQueue, di *gsq.DeqInfo) {
		fmt.Println(di)
		if di.Messages != 0 {
			cs.Lock()
			//auto dequeue messages until all messages at server side are dequeued
			qf.PushBack(aq.Dequeue(Queue_Key, d))
			cs.Unlock()
		}
	}
	cs.Lock()
	qf.PushBack(aq.Dequeue(Queue_Key, d))
	//optionally, add one extra to improve processing concurrency
	//at both client and server sides for better performance and through-output
	qf.PushBack(aq.Dequeue(Queue_Key, d))
	cs.Unlock()
	for {
		cs.Lock()
		if qf.Len() == 0 {
			cs.Unlock()
			break
		}
		i := qf.PopFront()
		cs.Unlock()
		print_sq(i)
	}
	//wait until all dequeue message confirmations are completed before shutting down pool
	<-aq.GetKeys()
}
