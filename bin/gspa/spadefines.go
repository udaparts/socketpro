package gspa

import "strconv"

type ReqId uint16

const (
	UnknownReqId          ReqId = 0
	SwitchTo              ReqId = 1
	RouteeChanged         ReqId = (SwitchTo + 1)
	Encrypted             ReqId = (RouteeChanged + 1)
	BatchZipped           ReqId = (Encrypted + 1)
	Cancel                ReqId = (BatchZipped + 1)
	GetSockOptAtSvr       ReqId = (Cancel + 1)
	SetSockOptAtSvr       ReqId = (GetSockOptAtSvr + 1)
	DoEcho                ReqId = (SetSockOptAtSvr + 1)
	TurnOnZipAtSvr        ReqId = (DoEcho + 1)
	StartBatching         ReqId = (TurnOnZipAtSvr + 1)
	CommitBatching        ReqId = (StartBatching + 1)
	StartMerge            ReqId = (CommitBatching + 1)
	EndMerge              ReqId = (StartMerge + 1)
	Ping                  ReqId = (EndMerge + 1)
	EnableClientDequeue   ReqId = (Ping + 1)
	ServerException       ReqId = (EnableClientDequeue + 1)
	AllMessagesDequeued   ReqId = (ServerException + 1)
	HttpClose             ReqId = (AllMessagesDequeued + 1) //SocketPro HTTP Close
	SetZipLevelAtSvr      ReqId = (HttpClose + 1)
	StartJob              ReqId = (SetZipLevelAtSvr + 1)
	EndJob                ReqId = (StartJob + 1)
	RoutingData           ReqId = (EndJob + 1)
	DequeueConfirmed      ReqId = (RoutingData + 1)
	MessageQueued         ReqId = (DequeueConfirmed + 1)
	StartQueue            ReqId = (MessageQueued + 1)
	StopQueue             ReqId = (StartQueue + 1)
	RoutePeerUnavailable  ReqId = StopQueue + 1
	DequeueBatchConfirmed ReqId = RoutePeerUnavailable + 1
	Interrupt             ReqId = DequeueBatchConfirmed + 1
	Enter                 ReqId = 65
	Speak                 ReqId = 66
	SpeakEx               ReqId = 67
	Exit                  ReqId = 68
	SendUserMessage       ReqId = 69
	SendUserMessageEx     ReqId = 70

	ReservedOne ReqId = 0x100
	ReservedTwo ReqId = 0x2001
)

func (reqId ReqId) String() string {
	switch reqId {
	case SwitchTo:
		return "SwitchTo"
	case RouteeChanged:
		return "RouteeChanged"
	case Encrypted:
		return "Encrypted"
	case BatchZipped:
		return "BatchZipped"
	case Cancel:
		return "Cancel"
	case GetSockOptAtSvr:
		return "GetSockOptAtSvr"
	case SetSockOptAtSvr:
		return "SetSockOptAtSvr"
	case DoEcho:
		return "DoEcho"
	case TurnOnZipAtSvr:
		return "TurnOnZipAtSvr"
	case StartBatching:
		return "StartBatching"
	case CommitBatching:
		return "CommitBatching"
	case StartMerge:
		return "StartMerge"
	case EndMerge:
		return "EndMerge"
	case Ping:
		return "Ping"
	case EnableClientDequeue:
		return "EnableClientDequeue"
	case ServerException:
		return "ServerException"
	case AllMessagesDequeued:
		return "AllMessagesDequeued"
	case HttpClose:
		return "HttpClose"
	case SetZipLevelAtSvr:
		return "SetZipLevelAtSvr"
	case StartJob:
		return "StartJob"
	case EndJob:
		return "EndJob"
	case RoutingData:
		return "RoutingData"
	case DequeueConfirmed:
		return "DequeueConfirmed"
	case MessageQueued:
		return "MessageQueued"
	case StartQueue:
		return "StartQueue"
	case StopQueue:
		return "StopQueue"
	case RoutePeerUnavailable:
		return "RoutePeerUnavailable"
	case DequeueBatchConfirmed:
		return "DequeueBatchConfirmed"
	case Interrupt:
		return "Interrupt"
	case Enter:
		return "Enter"
	case Speak:
		return "Speak"
	case SpeakEx:
		return "SpeakEx"
	case Exit:
		return "Exit"
	case SendUserMessage:
		return "SendUserMessage"
	case SendUserMessageEx:
		return "SendUserMessageEx"
	case ReservedOne:
		return "ReservedOne"
	case ReservedTwo:
		return "ReservedTwo"
	}
	return strconv.FormatInt(int64(reqId), 10)
}

type ServiceID uint32

const (
	Reserved1 ServiceID = 1
	Startup   ServiceID = 0x100
	Chat      ServiceID = (Startup + 1)
	HTTP      ServiceID = (Chat + 1)
	File      ServiceID = (HTTP + 1)
	ODBC      ServiceID = (File + 1)
	Reserved  ServiceID = 0x10000000

	//Your non-db service ids should be between (Reserved + 1) and (DB_RESERVED - 1)

	DB_RESERVED ServiceID = (Reserved + 0x6FFFFF00)

	//Your db streaming service ids must be between DB_RESERVED and (DB_UDAParts_RESERVED - 1)

	DB_UDAParts_RESERVED ServiceID = (Reserved + 0x6FFFFFD0)

	//UDAParts reserved ODBC and db streaming service ids from DB_UDAParts_RESERVED through DB_MAX
	DB_MAX ServiceID = (Reserved + 0x6FFFFFFF)
)

func IsDBService(sid ServiceID) bool {
	return ((sid >= DB_RESERVED && sid <= DB_MAX) || (sid == ODBC))
}

func (sid ServiceID) String() string {
	switch sid {
	case Reserved1:
		return "Reserved1"
	case Startup:
		return "Startup"
	case Chat:
		return "Chat"
	case HTTP:
		return "HTTP"
	case File:
		return "File"
	case ODBC:
		return "ODBC"
	case Reserved:
		return "Reserved"
	case DB_RESERVED:
		return "DB_RESERVED"
	case DB_UDAParts_RESERVED:
		return "DB_UDAParts_RESERVED"
	case DB_MAX:
		return "DB_MAX"
	}
	return strconv.FormatInt(int64(sid), 10)
}

type EncryptionMethod int32

const (
	NoEncryption EncryptionMethod = 0
	TLSv1        EncryptionMethod = 1
)

func (em EncryptionMethod) String() string {
	switch em {
	case NoEncryption:
		return "NoEncryption"
	case TLSv1:
		return "TLSv1"
	}
	return "EncryptionMethodUnknown"
}

type ShutdownType int32

const (
	Receive ShutdownType = 0
	Send    ShutdownType = 1
	Both    ShutdownType = 2
)

type QueueStatus int32

const (
	//everything is fine
	Normal QueueStatus = 0

	MergeComplete QueueStatus = 1

	//merge push not completed yet
	MergePushing QueueStatus = 2

	//merge incomplete (job incomplete or crash)
	MergeIncomplete QueueStatus = 3

	//job incomplete (crash or endjob not found)
	JobIncomplete QueueStatus = 4

	//an incomplete message detected
	Crash QueueStatus = 5

	//file open error
	FileError QueueStatus = 6

	//queue file opened but can't decrypt existing queued messages because of bad password found
	BadPassword QueueStatus = 7

	//duplicate name error
	DuplicateName QueueStatus = 8
)

type Optimistic int32

const (
	MemoryCached       Optimistic = 0
	SystemMemoryCached Optimistic = 1
	DiskCommitted      Optimistic = 2
)

func (o Optimistic) String() string {
	switch o {
	case MemoryCached:
		return "MemoryCached"
	case SystemMemoryCached:
		return "SystemMemoryCached"
	case DiskCommitted:
		return "DiskCommitted"
	}
	return "OptimisticUnknown"
}

func (s ShutdownType) String() string {
	switch s {
	case Receive:
		return "Receive"
	case Send:
		return "Send"
	case Both:
		return "Both"
	}
	return "ShutdownTypeUnknown"
}

func (qs QueueStatus) String() string {
	switch qs {
	case Normal:
		return "Normal"
	case MergeComplete:
		return "MergeComplete"
	case MergePushing:
		return "MergePushing"
	case MergeIncomplete:
		return "MergeIncomplete"
	case JobIncomplete:
		return "JobIncomplete"
	case Crash:
		return "Crash"
	case FileError:
		return "FileError"
	case BadPassword:
		return "BadPassword"
	case DuplicateName:
		return "DuplicateName"
	}
	return "QueueStatusUnknown"
}

type ErrInfo struct {
	ErrCode int32
	ErrMsg  string
}
