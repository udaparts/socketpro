package main

import (
	"fmt"
	"runtime"
)

func main() {
	if runtime.GOOS == "windows" {
		fmt.Print("Make sure that you already copy the whole directory ../socketpro/bin/gspa into C:\\Program Files\\Go\\src")
	} else {
		fmt.Print("Make sure that you already copy the whole directory ../socketpro/bin/gspa into /usr/local/go/src")
	}
	fmt.Println(" before playing unit test files at the sub-directory tests")
}
