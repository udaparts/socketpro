package gcs

import (
	"syscall"
	"unsafe"
)

func SetVerifyLocation(ca string) bool {
	var p uintptr
	bytes := ([]byte)(ca)
	if len(ca) > 0 {
		p = uintptr(unsafe.Pointer(&bytes[0]))
	}
	r, _, _ := setVerifyLocation.Call(p)
	return (byte(r) > 0)
}

func SetCertificateVerify(cv DCertificateVerify) {
	var cb uintptr
	if cv == nil {
		setCertificateVerifyCallback.Call(cb)
		return
	}
	cbCV := func(verified bool, depth int32, ec int32, errMsg uintptr, certInfo uintptr) uintptr {
		res := cv(verified, depth, ec, utf8ToString(errMsg), toCI(certInfo))
		if res {
			return 1
		}
		return 0
	}
	cb = syscall.NewCallback(cbCV)
	setCertificateVerifyCallback.Call(cb)
}
