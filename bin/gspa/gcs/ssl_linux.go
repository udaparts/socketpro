package gcs

// #include <stdint.h>
// #include <stdbool.h>
// #include <string.h>
// bool SetVerifyLocation(const char *certFile);
import "C"
import "unsafe"

func SetVerifyLocation(ca string) bool {
	var p *C.char
	bytes := ([]byte)(ca)
	if len(ca) > 0 {
		p = (*C.char)(unsafe.Pointer(&bytes[0]))
	}
	r := C.SetVerifyLocation(p)
	return bool(r)
}
