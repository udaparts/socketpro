package gutil

import (
	"unsafe"
)

//Convert a string into an array of int8 (signed char).
func ToUtf8(s string) []int8 {
	as := make([]int8, len(s))
	p := (*[]byte)(unsafe.Pointer(&as))
	copy(*p, s)
	return as
}
