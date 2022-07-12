package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"reflect"
	"testing"
	"time"
)

var cc_hw = gcs.CConnectionContext{Host: "windesk", UserId: "root", Password: "Smash123", Port: 20901}

const (
	sidHelloWorld = gspa.Reserved + 1
	idSayHello    = gspa.ReservedTwo + 1
	idSleep       = idSayHello + 1
	idEcho        = idSleep + 1
)

type CMyStruct struct {
	nullString    *string
	objNull       interface{}
	ADateTime     time.Time
	ADouble       float64
	ABool         bool
	UnicodeString string
	AsciiString   gspa.AStr
	ObjBool       interface{}
	ObjString     interface{}
	ObjArrString  interface{}
	ObjArrInt     interface{}
}

func (ms *CMyStruct) SaveTo(buffer *gspa.CUQueue) *gspa.CUQueue {
	buffer.SaveString(ms.nullString).SaveObject(ms.objNull).SaveDate(ms.ADateTime).Save(ms.ADouble).Save(ms.ABool).SaveString(&ms.UnicodeString)
	buffer.SaveAString(&ms.AsciiString).SaveObject(ms.ObjBool).SaveObject(ms.ObjString).SaveObject(ms.ObjArrString).SaveObject(ms.ObjArrInt)
	return buffer
}

func loadFrom(buffer *gspa.CUQueue) *CMyStruct {
	ms := new(CMyStruct)
	ms.nullString = buffer.LoadString()
	ms.objNull = buffer.LoadObject()
	ms.ADateTime = buffer.LoadDate()
	ms.ADouble = buffer.LoadDouble()
	ms.ABool = buffer.LoadBool()
	ms.UnicodeString = *buffer.LoadString()
	ms.AsciiString = *buffer.LoadAString()
	ms.ObjBool = buffer.LoadObject()
	ms.ObjString = buffer.LoadObject()
	ms.ObjArrString = buffer.LoadObject()
	ms.ObjArrInt = buffer.LoadObject()
	return ms
}

func MakeMyStruct() *CMyStruct {
	ms := new(CMyStruct)
	ms.ADateTime = time.Now()
	ms.ObjBool = true
	ms.UnicodeString = "Unicode"
	ms.ABool = true
	ms.ADouble = 1234.567
	ms.AsciiString = "ASCII"
	ms.ObjString = "test"
	ms.ObjArrInt = []int32{1, 76890}
	var h string = "Hello"
	var w string = "world"
	ms.ObjArrString = []*string{&h, &w}
	return ms
}

func SayHello(h *gcs.CAsyncHandler, fName string, lName string) <-chan interface{} {
	buffer := gspa.MakeBuffer().SaveString(&fName).SaveString(&lName)
	//recycle the buffer for reuse to avoid allocating memories repeatedly
	defer buffer.Recycle()
	res := make(chan interface{})
	res = h.Send(idSayHello, buffer, func(ar *gcs.CAsyncResult) {
		str := ar.Buffer.LoadString()
		if str == nil {
			res <- "null"
		} else {
			res <- *str
		}
		close(res)
	})
	return res
}

func Sleep(h *gcs.CAsyncHandler, timeout uint32) <-chan interface{} {
	buffer := gspa.MakeBuffer().SaveUInt(timeout)
	//recycle the buffer for reuse to avoid allocating memories repeatedly
	defer buffer.Recycle()
	res := make(chan interface{})
	res = h.Send(idSleep, buffer, func(ar *gcs.CAsyncResult) {
		res <- "null"
		close(res)
	})
	return res
}

func Echo(h *gcs.CAsyncHandler, ms *CMyStruct) <-chan interface{} {
	buffer := gspa.MakeBuffer()
	ms.SaveTo(buffer)
	//recycle the buffer for reuse to avoid allocating memories repeatedly
	defer buffer.Recycle()
	res := make(chan interface{})
	res = h.Send(idEcho, buffer, func(ar *gcs.CAsyncResult) {
		res <- loadFrom(ar.Buffer)
		close(res)
	})
	return res
}

func PrintFuture(f <-chan interface{}) {
	res := <-f //wait until result comes from remote server
	switch t := res.(type) {
	case *CMyStruct:
		fmt.Println(res.(*CMyStruct)) //echo
	case string: //normal result
		fmt.Println(res.(string))
	case gcs.SocketError: //session error
		fmt.Println(res.(gcs.SocketError))
	case gcs.ServerError: //error from remote server
		fmt.Println(res.(gcs.ServerError))
	default:
		fmt.Println("Unknown data type: ", t)
	}
}

func Test_hw_1(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		//socket pool events
		fmt.Println("PoolId:", sp.GetPoolId(), "ServiceId:", sp.GetSvsID(), spe)
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
	}
}

func Test_client_queue_1(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	sp.SetQueueName("golang")
	sp.SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Connecting failed")
	}
	queues := sp.GetQueues()
	if sp.GetQueues() != 1 {
		t.Errorf("Received %d; want 1", queues)
	}
}

func Test_client_queue_2(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	sp.SetQueueName("golang")
	sp.SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 2)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Connecting failed")
	}
	time.Sleep(15 * time.Second)
	queues := sp.GetQueues()
	if sp.GetQueues() != 2 {
		t.Errorf("queues = %d; want 2", queues)
	} else {
		cq := sp.SeekByQueue().GetClientQueue()
		fmt.Println(cq.IsAvailable(), cq.GetJobSize(), cq.GetLastIndex(), cq.GetOptimistic(), cq.GetQueueFileName(), cq.GetQueueName(),
			cq.GetQueueOpenStatus(), cq.GetQueueSize(), cq.GetTTL(), cq.IsDequeueShared(), cq.IsDequeueEnabled(),
			cq.IsRoutingQueueIndexEnabled(), cq.IsSecure(), cq.GetMessageCount(), cq.GetMessagesInDequeuing(), sp.GetLockedSockets())
	}
}
func Test_sayhello(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Connecting failed")
	} else {
		ah := sp.Seek()
		f := SayHello(ah, "Jone", "Don")
		res := (<-f).(string) //wait until result comes from remote server
		if res != "Hello Jone Don" {
			t.Errorf("res = %s; want Hello Jone Don", res)
		}
	}
}

func Test_sleep(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Connecting failed")
	} else {
		ah := sp.Seek()
		f := Sleep(ah, 4000)
		res := (<-f).(string) //wait until result comes from remote server
		if res != "null" {
			t.Errorf("res = %s; want 'null'", res)
		}
	}
}

func equal(ms *CMyStruct, ms0 *CMyStruct) bool {
	if ms == nil && ms0 == nil {
		return true
	} else if ms == nil && ms0 != nil {
		return false
	} else if ms != nil && ms0 == nil {
		return false
	} else if ms.ADateTime.UnixNano()/100 != ms0.ADateTime.UnixNano()/100 {
		return false
	}
	eq := ms.ABool == ms0.ABool && ms.nullString == ms0.nullString &&
		ms.objNull == ms0.objNull && ms.ADouble == ms0.ADouble &&
		ms.AsciiString == ms0.AsciiString && ms.ObjBool == ms0.ObjBool &&
		ms.UnicodeString == ms0.UnicodeString && ms.ObjString == ms0.ObjString
	if !eq {
		return false
	}
	eq = reflect.DeepEqual(ms.ObjArrString, ms0.ObjArrString) && reflect.DeepEqual(ms.ObjArrInt, ms0.ObjArrInt)
	return eq
}

func Test_struc(t *testing.T) {
	ms := MakeMyStruct()
	ms0 := loadFrom(gspa.MakeBuffer().Save(ms))
	if !equal(ms, ms0) {
		t.Errorf("ms0 = %+v; want %+v", ms0, ms)
	}
}

func Test_echo(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Connecting failed")
	} else {
		ah := sp.Seek()
		ms := MakeMyStruct()
		f := Echo(ah, ms)
		res := (<-f).(*CMyStruct) //wait until result comes from remote server
		fmt.Println(ms)
		fmt.Println(res)
	}
}

func Test_all(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Connecting failed")
	} else {
		ah := sp.Seek()
		ms := MakeMyStruct()
		fmt.Println(ms)
		vf := []<-chan interface{}{SayHello(ah, "John", "Don"), Sleep(ah, 5000),
			SayHello(ah, "您正在访问", "域名"), SayHello(ah, "Donald", "Trump"),
			SayHello(ah, "Joe", "Biden"), Echo(ah, ms)}
		for _, f := range vf {
			PrintFuture(f)
		}
	}
}

func Test_online_messages(t *testing.T) {
	sp := gcs.NewPool(sidHelloWorld)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		t.Errorf("Test_online_messages/Connecting failed")
	} else {
		ah := sp.Lock()
		message := ah.GetMsg()
		message.SetOnSubscribe(func(ms *gcs.MessageSender, groups []uint32) {
			fmt.Println("Subscribe", *ms, groups)
		}).SetOnUnsubscribe(func(ms *gcs.MessageSender, groups []uint32) {
			fmt.Println("Unsubscribe", *ms, groups)
		}).SetOnUserMessage(func(ms *gcs.MessageSender, msg interface{}) {
			fmt.Println("UserMessage", *ms, msg)
		}).SetOnPublish(func(ms *gcs.MessageSender, groups []uint32, msg interface{}) {
			fmt.Println("Publish", *ms, groups, msg)
		}).SetOnPublishEx(func(ms *gcs.MessageSender, groups []uint32, bytes []byte) {
			fmt.Println("PublishEx", *ms, groups, bytes)
		}).SetOnUserMessageEx(func(ms *gcs.MessageSender, bytes []byte) {
			fmt.Println("UserMessageEx", *ms, bytes)
		})
		groups := []uint32{1, 2, 3}
		message.Unsubscribe().Subscribe(groups)
		message.SendUserMessage("golang message", "UDAPARTS")
		message.Publish("A sample publish message", groups)
		buffer := gspa.MakeBuffer().Save("普京出动装甲车队进攻乌克兰首都基辅重点清除的目标之", "他下一秒就要你的命")
		message.PublishEx(buffer.GetBytes(), groups)
		message.SendUserMessageEx("UDAPARTS", buffer.GetBytes())
		addr, port := ah.GetPeerName()
		fmt.Println(addr, port, ah.DoEcho(), ah.GetServerPingTime(), ah.GetRouteeCount(), ah.IsRouteeRequest(),
			ah.GetZipLevel(), ah.Sendable())
		ah.SetZipLevel(gspa.BestSpeed)
		fmt.Println(ah.SetZipLevelAtSvr(gspa.BestSpeed))
		time.Sleep(5 * time.Second)
	}
}
