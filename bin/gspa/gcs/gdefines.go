package gcs

import (
	"gspa"
	"strings"
)

const (
	DEFAULT_QUEUE_TIME_TO_LIVE uint32 = 240 * 3600 //10 days
	DEFAULT_RECV_TIMEOUT       uint32 = 30000
	DEFAULT_CONN_TIMEOUT       uint32 = 30000

	SESSION_CLOSED_AFTER     int32  = -1000
	SESSION_CLOSED_BEFORE    int32  = -1001
	REQUEST_CANCELED         int32  = -1002
	DEFAULT_INTERRUPT_OPTION uint64 = 1

	SESSION_CLOSED_BEFORE_ERR_MSG = "Session already closed before sending the request"
	SESSION_CLOSED_AFTER_ERR_MSG  = "Session closed after sending the request"
	REQUEST_CANCELED_ERR_MSG      = "Request canceled"
)

type ConnectionState int32

const (
	Closed     ConnectionState = 0
	Connecting ConnectionState = 1
	SslShaking ConnectionState = 2
	Closing    ConnectionState = 3
	Connected  ConnectionState = 4
	Switched   ConnectionState = 5
)

func (cs ConnectionState) String() string {
	switch cs {
	case Closed:
		return "Closed"
	case Connecting:
		return "Connecting"
	case SslShaking:
		return "SslShaking"
	case Closing:
		return "Closing"
	case Connected:
		return "Connected"
	case Switched:
		return "Switched"
	}
	return "ConnectionStateUnknown"
}

type SocketPoolEvent int32

const (
	SpeUnknown            SocketPoolEvent = -1
	SpeStarted            SocketPoolEvent = 0
	SpeCreatingThread     SocketPoolEvent = 1
	SpeThreadCreated      SocketPoolEvent = 2
	SpeConnecting         SocketPoolEvent = 3
	SpeConnected          SocketPoolEvent = 4
	SpeKillingThread      SocketPoolEvent = 5
	SpeShutdown           SocketPoolEvent = 6
	SpeSocketCreated      SocketPoolEvent = 7
	SpeHandShakeCompleted SocketPoolEvent = 8
	SpeLocked             SocketPoolEvent = 9
	SpeUnlocked           SocketPoolEvent = 10
	SpeThreadKilled       SocketPoolEvent = 11
	SpeClosingSocket      SocketPoolEvent = 12
	SpeSocketClosed       SocketPoolEvent = 13
	SpeSocketKilled       SocketPoolEvent = 14
	SpeTimer              SocketPoolEvent = 15
	SpeQueueMergedFrom    SocketPoolEvent = 16
	SpeQueueMergedTo      SocketPoolEvent = 17
)

func (spe SocketPoolEvent) String() string {
	switch spe {
	case SpeStarted:
		return "SpeStarted"
	case SpeCreatingThread:
		return "SpeCreatingThread"
	case SpeThreadCreated:
		return "SpeThreadCreated"
	case SpeConnecting:
		return "SpeConnecting"
	case SpeConnected:
		return "SpeConnected"
	case SpeKillingThread:
		return "SpeKillingThread"
	case SpeShutdown:
		return "SpeShutdown"
	case SpeSocketCreated:
		return "SpeSocketCreated"
	case SpeHandShakeCompleted:
		return "SpeHandShakeCompleted"
	case SpeLocked:
		return "SpeLocked"
	case SpeUnlocked:
		return "SpeUnlocked"
	case SpeThreadKilled:
		return "SpeThreadKilled"
	case SpeClosingSocket:
		return "SpeClosingSocket"
	case SpeSocketClosed:
		return "SpeSocketClosed"
	case SpeSocketKilled:
		return "SpeSocketKilled"
	case SpeTimer:
		return "SpeTimer"
	case SpeQueueMergedFrom:
		return "SpeQueueMergedFrom"
	case SpeQueueMergedTo:
		return "SpeQueueMergedTo"
	}
	return "SocketPoolEventUnknown"
}

type USocket_Client_Handle uintptr

type CConnectionContext struct {
	Host             string
	Port             uint32
	UserId           string
	Password         string
	EncryptionMethod gspa.EncryptionMethod
	V6               bool
	Zip              bool
	AnyData          interface{}
}

func (cc *CConnectionContext) Equal(cxt *CConnectionContext) bool {
	if cc == cxt {
		return true
	} else if cc == nil || cxt == nil {
		return false
	}
	return strings.EqualFold(cc.Host, cxt.Host) &&
		cc.Port == cxt.Port &&
		strings.EqualFold(cc.UserId, cxt.UserId) &&
		cc.Password == cxt.Password &&
		cc.EncryptionMethod == cxt.EncryptionMethod &&
		cc.V6 == cxt.V6 &&
		cc.Zip == cxt.Zip &&
		cc.AnyData == cxt.AnyData
}

type SocketError struct {
	gspa.ErrInfo
	ReqId  gspa.ReqId
	Before bool
}

type ServerError struct {
	gspa.ErrInfo
	ReqId    gspa.ReqId
	Location string
}
