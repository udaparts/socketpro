package gspa

type ZipLevel int32

const (
	Default         ZipLevel = 0
	BestSpeed       ZipLevel = 1
	BestCompression ZipLevel = 2
)

func (zl ZipLevel) String() string {
	switch zl {
	case Default:
		return "Default"
	case BestSpeed:
		return "BestSpeed"
	case BestCompression:
		return "BestCompression"
	}
	return "ZipLevelUnknown"
}

type SocketOption int32

const (
	TcpNoDelay SocketOption = 1
	ReuseAddr  SocketOption = 4
	KeepAlive  SocketOption = 8
	//send buffer size
	SndBuf SocketOption = 0x1001
	//receive buffer size
	RcvBuf SocketOption = 0x1002
)

func (so SocketOption) String() string {
	switch so {
	case TcpNoDelay:
		return "TcpNoDelay"
	case ReuseAddr:
		return "ReuseAddr"
	case KeepAlive:
		return "KeepAlive"
	case SndBuf:
		return "SndBuf"
	case RcvBuf:
		return "RcvBuf"
	}
	return "SocketOptionUnknown"
}

type SocketLevel int32

const (
	Tcp    SocketLevel = 6
	Socket SocketLevel = 0xFFFF
)

func (sl SocketLevel) String() string {
	switch sl {
	case Tcp:
		return "Tcp"
	case Socket:
		return "Socket"
	}
	return "SocketLevelUnknown"
}

type OperationSystem int32

//supported operation system
const (
	Win     OperationSystem = 0
	Apple   OperationSystem = 1
	Mac     OperationSystem = Apple
	IPhone  OperationSystem = Apple
	Unix    OperationSystem = 2
	Linux   OperationSystem = Unix
	BSD     OperationSystem = Unix
	Android OperationSystem = 3
	WinCE   OperationSystem = 4
)

func (os OperationSystem) String() string {
	switch os {
	case Win:
		return "Win"
	case Apple:
		return "Apple"
	case Linux:
		return "Linux"
	case Android:
		return "Android"
	case WinCE:
		return "WinCE"
	}
	return "OperationSystemUnknown"
}

type ThreadApartment int32

const (
	//No COM apartment involved
	None ThreadApartment = 0

	//STA apartment
	Apartment ThreadApartment = 1

	//MTA (free) or neutral apartments
	Free ThreadApartment = 2
)
