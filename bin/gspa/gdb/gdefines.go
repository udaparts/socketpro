package gdb

import "gspa"

type TransactionIsolation int32

const (
	UnspecifiedTransactionIsolation TransactionIsolation = -1
	Chaos                           TransactionIsolation = 0
	ReadUncommited                  TransactionIsolation = 1
	Browse                          TransactionIsolation = 2
	CursorStability                 TransactionIsolation = 3
	ReadCommited                    TransactionIsolation = CursorStability
	RepeatableRead                  TransactionIsolation = 4
	Serializable                    TransactionIsolation = 5
	Isolated                        TransactionIsolation = 6
)

func (ti TransactionIsolation) String() string {
	switch ti {
	case Chaos:
		return "Chaos"
	case ReadUncommited:
		return "ReadUncommited"
	case Browse:
		return "Browse"
	case ReadCommited:
		return "ReadCommited"
	case RepeatableRead:
		return "RepeatableRead"
	case Serializable:
		return "Serializable"
	case Isolated:
		return "Isolated"
	}
	return "UnspecifiedTransactionIsolation"
}

type RollbackPlan int32

const (
	// Manual transaction will rollback whenever there is an error by default
	Default RollbackPlan = 0

	// Manual transaction will rollback whenever there is an error by default
	RollbackErrorAny RollbackPlan = Default

	// Manual transaction will rollback as long as the number of errors is less than the number of ok processing statements
	RollbackErrorLess RollbackPlan = 1

	// Manual transaction will rollback as long as the number of errors is less or equal than the number of ok processing statements
	RollbackErrorEqual RollbackPlan = 2

	// Manual transaction will rollback as long as the number of errors is more than the number of ok processing statements
	RollbackErrorMore RollbackPlan = 3

	//Manual transaction will rollback only if all the processing statements are failed
	RollbackErrorAll RollbackPlan = 4

	// Manual transaction will rollback always no matter what happens
	RollbackAlways = 5
)

func (rp RollbackPlan) String() string {
	switch rp {
	case Default:
		return "RollbackErrorAny"
	case RollbackErrorLess:
		return "RollbackErrorLess"
	case RollbackErrorEqual:
		return "RollbackErrorEqual"
	case RollbackErrorMore:
		return "RollbackErrorMore"
	case RollbackErrorAll:
		return "RollbackErrorAll"
	case RollbackAlways:
		return "RollbackAlways"
	}
	return "UnknownRollbackPlan"
}

type UpdateEvent int32

const (
	UnknownUpdateEvent UpdateEvent = -1

	// An event for inserting a record into a table
	Insert UpdateEvent = 0

	// An event for updating a record of a table
	Update UpdateEvent = 1

	// An event for deleting a record from a table
	Delete UpdateEvent = 2
)

func (ue UpdateEvent) String() string {
	switch ue {
	case Insert:
		return "Insert"
	case Update:
		return "Update"
	case Delete:
		return "Delete"
	}
	return "UnknownUpdateEvent"
}

type ManagementSystem int32

const (
	UnknownManagementSystem ManagementSystem = -1
	Sqlite                  ManagementSystem = 0
	Mysql                   ManagementSystem = 1
	ODBC                    ManagementSystem = 2
	MsSQL                   ManagementSystem = 3
	Oracle                  ManagementSystem = 4
	DB2                     ManagementSystem = 5
	PostgreSQL              ManagementSystem = 6
	MongoDB                 ManagementSystem = 7
)

func (ms ManagementSystem) String() string {
	switch ms {
	case Sqlite:
		return "Sqlite"
	case Mysql:
		return "Mysql"
	case ODBC:
		return "ODBC"
	case MsSQL:
		return "MsSQL"
	case Oracle:
		return "Oracle"
	case DB2:
		return "DB2"
	case PostgreSQL:
		return "PostgreSQL"
	case MongoDB:
		return "MongoDB"
	}
	return "UnknownManagementSystem"
}

type ParameterDirection int32

const (
	UnknownParameterDirection ParameterDirection = 0
	Input                     ParameterDirection = 1
	Output                    ParameterDirection = 2
	InputOutput               ParameterDirection = 3
	ReturnValue               ParameterDirection = 4
)

func (pd ParameterDirection) String() string {
	switch pd {
	case Input:
		return "Input"
	case Output:
		return "Output"
	case InputOutput:
		return "InputOutput"
	case ReturnValue:
		return "ReturnValue"
	}
	return "UnknownParameterDirection"
}

//ids for DB requests
const (
	// Async database client/server just requires the following request identification numbers
	Open              gspa.ReqId = 0x7E7F
	Close                        = Open + 1
	BeginTrans                   = Close + 1
	EndTrans                     = BeginTrans + 1
	Execute                      = EndTrans + 1
	Prepare                      = Execute + 1
	ExecuteParameters            = Prepare + 1

	// Request identification numbers used for message push from server to client
	DBUpdate        = ExecuteParameters + 1 //server ==> client only
	RowsetHeader    = DBUpdate + 1          //server ==> client only
	OutputParameter = RowsetHeader + 1      //server ==> client only

	//Internal request/response identification numbers used for data communication between client and server
	BeginRows    = OutputParameter + 1
	Transferring = BeginRows + 1
	StartBLOB    = Transferring + 1
	Chunk        = StartBLOB + 1
	EndBLOB      = Chunk + 1
	EndRows      = EndBLOB + 1
	CallReturn   = EndRows + 1

	GetCachedTables = CallReturn + 1

	SqlBatchHeader    = GetCachedTables + 1
	ExecuteBatch      = SqlBatchHeader + 1
	ParameterPosition = ExecuteBatch + 1
	ExecuteEx         = ParameterPosition + 1
)

const (
	ODBC_SQLColumnPrivileges gspa.ReqId = 0x7f00 + 100
	ODBC_SQLColumns          gspa.ReqId = 0x7f00 + 101
	ODBC_SQLForeignKeys      gspa.ReqId = 0x7f00 + 102
	ODBC_SQLPrimaryKeys      gspa.ReqId = 0x7f00 + 103
	ODBC_SQLProcedureColumns gspa.ReqId = 0x7f00 + 104
	ODBC_SQLProcedures       gspa.ReqId = 0x7f00 + 105
	ODBC_SQLSpecialColumns   gspa.ReqId = 0x7f00 + 106
	ODBC_SQLStatistics       gspa.ReqId = 0x7f00 + 107
	ODBC_SQLTablePrivileges  gspa.ReqId = 0x7f00 + 108
	ODBC_SQLTables           gspa.ReqId = 0x7f00 + 109
	ODBC_SQLGetInfo          gspa.ReqId = 0x7f00 + 110

	ODBC_SUCCESS int32 = 0
	ODBC_ERROR   int32 = -1

	//error codes from async ODBC server library
	ODBC_ER_NO_DB_OPENED_YET                        int32 = -1981
	ODBC_ER_BAD_END_TRANSTACTION_PLAN               int32 = -1982
	ODBC_ER_NO_PARAMETER_SPECIFIED                  int32 = -1983
	ODBC_ER_BAD_PARAMETER_COLUMN_SIZE               int32 = -1984
	ODBC_ER_BAD_PARAMETER_DATA_ARRAY_SIZE           int32 = -1985
	ODBC_ER_DATA_TYPE_NOT_SUPPORTED                 int32 = -1986
	ODBC_ER_NO_DB_NAME_SPECIFIED                    int32 = -1987
	ODBC_ER_ODBC_ENVIRONMENT_NOT_INITIALIZED        int32 = -1988
	ODBC_ER_BAD_MANUAL_TRANSACTION_STATE            int32 = -1989
	ODBC_ER_BAD_INPUT_PARAMETER_DATA_TYPE           int32 = -1990
	ODBC_ER_BAD_PARAMETER_DIRECTION_TYPE            int32 = -1991
	ODBC_ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET int32 = -1992
)

const (
	BLOB_LENGTH_NOT_AVAILABLE uint32 = 0xffffffe0
	/**
	 * Whenever a data size in bytes is about twice larger than the defined value,
	 * the data will be treated in large object and transferred in chunks for reducing memory foot print
	 */
	DEFAULT_BIG_FIELD_CHUNK_SIZE uint32 = 16 * 1024

	/**
	 * A record data size in bytes is approximately equal to or slightly larger than the defined constant
	 */
	DEFAULT_RECORD_BATCH_SIZE uint32 = 16 * 1024

	/**
	 * A flag used with idOpen for tracing database table update events
	 */
	ENABLE_TABLE_UPDATE_MESSAGES uint32 = 0x1

	/**
	 * A flag used with idOpen to enable in-line batching query statements for better performance
	 */
	USE_QUERY_BATCHING uint32 = 0x2

	/**
	 * A chat group id used at SocketPro server side for notifying database events from server to connected clients
	 */
	STREAMING_SQL_CHAT_GROUP_ID uint32 = 0x1fffffff

	CACHE_UPDATE_CHAT_GROUP_ID uint32 = STREAMING_SQL_CHAT_GROUP_ID + 1
)

const (
	FLAG_NOT_NULL       uint32 = 0x1
	FLAG_UNIQUE         uint32 = 0x2
	FLAG_PRIMARY_KEY    uint32 = 0x4
	FLAG_AUTOINCREMENT  uint32 = 0x8
	FLAG_NOT_WRITABLE   uint32 = 0x10
	FLAG_ROWID          uint32 = 0x20
	FLAG_XML            uint32 = 0x40
	FLAG_JSON           uint32 = 0x80
	FLAG_CASE_SENSITIVE uint32 = 0x100
	FLAG_IS_ENUM        uint32 = 0x200
	FLAG_IS_SET         uint32 = 0x400
	FLAG_IS_UNSIGNED    uint32 = 0x800
	FLAG_IS_BIT         uint32 = 0x1000
	FLAG_FIXED_LENGTH   uint32 = 0x2000
)

type CDBColumnInfo struct {
	DBPath       string
	TablePath    string
	DisplayName  string
	OriginalName string
	DeclaredType string
	Collation    string
	ColumnSize   uint32
	Flags        uint32
	DataType     gspa.VarType
	Precision    byte
	Scale        byte
}

func (info *CDBColumnInfo) SaveTo(buffer *gspa.CUQueue) *gspa.CUQueue {
	buffer.Save(info.DBPath, info.TablePath, info.DisplayName, info.OriginalName)
	buffer.Save(info.DeclaredType, info.Collation, info.ColumnSize, info.Flags)
	buffer.Save(uint16(info.DataType), info.Precision, info.Scale)
	return buffer
}

func (info *CDBColumnInfo) LoadFrom(buffer *gspa.CUQueue) *gspa.CUQueue {
	info.DBPath = *buffer.LoadString()
	info.TablePath = *buffer.LoadString()
	info.DisplayName = *buffer.LoadString()
	info.OriginalName = *buffer.LoadString()
	info.DeclaredType = *buffer.LoadString()
	info.Collation = *buffer.LoadString()
	info.ColumnSize = buffer.LoadUInt()
	info.Flags = buffer.LoadUInt()
	info.DataType = gspa.VarType(buffer.LoadUShort())
	info.Precision = buffer.LoadByte()
	info.Scale = buffer.LoadByte()
	return buffer
}

type CDBColumnInfoArray []*CDBColumnInfo

func (arr *CDBColumnInfoArray) SaveTo(buffer *gspa.CUQueue) *gspa.CUQueue {
	buffer.SaveUInt(uint32(len(*arr)))
	for _, info := range *arr {
		info.SaveTo(buffer)
	}
	return buffer
}

func (arr *CDBColumnInfoArray) LoadFrom(buffer *gspa.CUQueue) *gspa.CUQueue {
	size := buffer.LoadUInt()
	a := make(CDBColumnInfoArray, size)
	for i, _ := range a {
		info := new(CDBColumnInfo)
		info.LoadFrom(buffer)
		a[i] = info
	}
	*arr = a
	return buffer
}

type CParameterInfo struct {
	Direction     ParameterDirection //required
	DataType      gspa.VarType       //required
	ColumnSize    uint32             //-1 BLOB/text, string length or binary bytes; ignored for other data types
	Precision     byte               //datetime, decimal or numeric only
	Scale         byte               //datetime, decimal or numeric only
	ParameterName string             //may be optional, which depends on remote database system
}

func (info *CParameterInfo) SaveTo(buffer *gspa.CUQueue) *gspa.CUQueue {
	buffer.Save(int32(info.Direction), uint16(info.DataType), info.ColumnSize, info.Precision, info.Scale, info.ParameterName)
	return buffer
}

func (info *CParameterInfo) LoadFrom(buffer *gspa.CUQueue) *gspa.CUQueue {
	info.Direction = ParameterDirection(buffer.LoadInt())
	info.DataType = gspa.VarType(buffer.LoadUShort())
	info.ColumnSize = buffer.LoadUInt()
	info.Precision = buffer.LoadByte()
	info.Scale = buffer.LoadByte()
	info.ParameterName = *buffer.LoadString()
	return buffer
}

const (
	SidSqlite   = gspa.Reserved + 0x6FFFFFF0
	SidMysql    = gspa.Reserved + 0x6FFFFFF1
	SidMssql    = gspa.Reserved + 0x6FFFFFF2
	SidDB2      = gspa.Reserved + 0x6FFFFFF3
	SidPostgres = gspa.Reserved + 0x6FFFFFF4
)
