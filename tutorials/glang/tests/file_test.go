package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"gspa/gfile"
	"testing"
)

func print_file(f <-chan interface{}) {
	res := <-f //wait until result comes from remote server
	switch t := res.(type) {
	case *gspa.ErrInfo:
		fmt.Println("Error info", res.(*gspa.ErrInfo))
	case gcs.SocketError: //session error
		fmt.Println(res.(gcs.SocketError))
	case gcs.ServerError: //error from remote server
		fmt.Println(res.(gcs.ServerError))
	default:
		fmt.Println("Unknown data type: ", t)
	}
}

func Test_file_streaming(t *testing.T) {
	sp := gcs.NewPool(gspa.File)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			new(gfile.CStreamingFile).Tie(h)
		}
	})
	ok := sp.StartSocketPool(cc_hw, 1)
	sp.SetQueueName("qme")
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	ah := sp.Seek()
	f := gfile.FindFile(ah)
	f.SetFilesStreamed(5)
	ch0 := f.Transfer(false, "temp0.lib", "jvm.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		fmt.Println(sender.GetFileSize(), transferred)
	})
	ch1 := f.Transfer(false, "temp1.lib", "libboost_coroutine-vc100-mt-s-1_60.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch2 := f.Transfer(false, "temp2.lib", "libboost_math_tr1f-vc100-mt-sgd-1_60.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch3 := f.Transfer(false, "temp3.lib", "libboost_serialization-vc100-mt-s-1_60.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch4 := f.Transfer(false, "temp4.lib", "libboost_wave-vc100-mt-sgd-1_60.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch5 := f.Transfer(true, "temp0.lib", "jvm_copy.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		fmt.Println(sender.GetFileSize(), transferred)
	})
	ch6 := f.Transfer(true, "temp1.lib", "libboost_coroutine-vc100-mt-s-1_60_copy.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch7 := f.Transfer(true, "temp2.lib", "libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch8 := f.Transfer(true, "temp3.lib", "libboost_serialization-vc100-mt-s-1_60_copy.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	ch9 := f.Transfer(true, "temp4.lib", "libboost_wave-vc100-mt-sgd-1_60_copy.lib", func(sender *gfile.CStreamingFile, transferred uint64) {
		//fmt.Println(sender.GetFileSize(), transferred)
	})
	fmt.Println(f.GetFilesStreamed(), f.GetFilesQueued())
	print_file(ch0)
	print_file(ch1)
	print_file(ch2)
	print_file(ch3)
	print_file(ch4)
	print_file(ch5)
	print_file(ch6)
	print_file(ch7)
	print_file(ch8)
	print_file(ch9)
}
