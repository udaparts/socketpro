
from spa import IUSerializer, tagVariantDataType

class tagTransactionIsolation(object):
    tiUnspecified = -1
    tiChaos = 0
    tiReadUncommited = 1
    tiBrowse = 2
    tiCursorStability = 3
    tiReadCommited = tiCursorStability
    tiRepeatableRead = 4
    tiSerializable = 5
    tiIsolated = 6

class tagRollbackPlan(object):
    """
    Manual transaction will rollback whenever there is an error by default
    """
    rpDefault = 0

    """
    Manual transaction will rollback whenever there is an error by default
    """
    rpRollbackErrorAny = rpDefault

    """
    Manual transaction will rollback as long as the number of errors is less than the number of ok processing statements
    """
    rpRollbackErrorLess = 1

    """
    Manual transaction will rollback as long as the number of errors is more than the number of ok processing statements
    """
    rpRollbackErrorEqual = 2

    """
    Manual transaction will rollback only if all the processing statements are failed
    """
    rpRollbackErrorAll = 4

    """
    Manual transaction will rollback always no matter what happens
    """
    rpRollbackAlways = 5

class tagUpdateEvent(object):
    ueUnknown = -1

    """
    An event for inserting a record into a table
    """
    ueInsert = 0

    """
    An event for updating a record of a table
    """
    ueUpdate = 1

    """
    An event for deleting a record from a table
    """
    ueDelete = 2

class tagManagementSystem(object):
    msUnknown = -1
    msSqlite = 0
    msMysql = 1
    msODBC = 2
    msMsSQL = 3
    msOracle = 4
    msDB2 = 5
    msPostgreSQL = 6
    msMongoDB = 7

class DB_CONSTS(object):
    """
    Async database client/server just requires the following request identification numbers
    """
    idOpen = 0x7E7F
    idClose = idOpen + 1
    idBeginTrans = idClose + 1
    idEndTrans = idBeginTrans + 1
    idExecute = idEndTrans + 1
    idPrepare = idExecute + 1
    idExecuteParameters = idPrepare + 1

    """
    the request identification numbers used for message push from server to client
    """
    idDBUpdate = idExecuteParameters + 1  # server ==> client only
    idRowsetHeader = idDBUpdate + 1  # server ==> client only
    idOutputParameter = idRowsetHeader + 1  # server ==> client only

    """
    Internal request/response identification numbers used for data communication between client and server
    """
    idBeginRows = idOutputParameter + 1
    idTransferring = idBeginRows + 1
    idStartBLOB = idTransferring + 1
    idChunk = idStartBLOB + 1
    idEndBLOB = idChunk + 1
    idEndRows = idEndBLOB + 1
    idCallReturn = idEndRows + 1
    idGetCachedTables = idCallReturn + 1

    idSqlBatchHeader = idGetCachedTables + 1
    idExecuteBatch = idSqlBatchHeader + 1
    idParameterPosition = idExecuteBatch + 1

    """
    Whenever a data size in bytes is about twice larger than the defined value, the data will be treated in large object and transferred in chunks for reducing memory foot print
    """
    DEFAULT_BIG_FIELD_CHUNK_SIZE = 16 * 1024  # 16k

    """
    A record data size in bytes is approximately equal to or slightly larger than the defined constant
    """
    DEFAULT_RECORD_BATCH_SIZE = 16 * 1024  # 16k

    """
    A flag used with idOpen for tracing database table update events
    """
    ENABLE_TABLE_UPDATE_MESSAGES = 0x1

    """
    A chat group id used at SocketPro server side for notifying database events from server to connected clients
    """
    STREAMING_SQL_CHAT_GROUP_ID = 0x1fffffff

    CACHE_UPDATE_CHAT_GROUP_ID = STREAMING_SQL_CHAT_GROUP_ID + 1

class CDBColumnInfo(IUSerializer):
    FLAG_NOT_NULL = 0x1
    FLAG_UNIQUE = 0x2
    FLAG_PRIMARY_KEY = 0x4
    FLAG_AUTOINCREMENT = 0x8
    FLAG_NOT_WRITABLE = 0x10
    FLAG_ROWID = 0x20
    FLAG_XML = 0x40
    FLAG_JSON = 0x80
    FLAG_CASE_SENSITIVE = 0x100
    FLAG_IS_ENUM = 0x200
    FLAG_IS_SET = 0x400
    FLAG_IS_UNSIGNED = 0x800
    FLAG_IS_BIT = 0x1000

    def __init__(self, DisplayName = u'', TablePath = u'', DBPath = u'', OriginalName = u'', DeclaredType = u''):
        self.DBPath = DBPath
        self.TablePath = TablePath
        self.DisplayName = DisplayName
        self.OriginalName = u''
        self.DeclaredType = u''
        self.Collation = u''
        self.ColumnSize = 0
        self.Flags = 0
        self.DataType = tagVariantDataType.sdVT_NULL
        self.Precision = 0
        self.Scale = 0

    def LoadFrom(self, q):
        self.DBPath = q.LoadString()
        self.TablePath = q.LoadString()
        self.DisplayName = q.LoadString()
        self.OriginalName = q.LoadString()
        self.DeclaredType = q.LoadString()
        self.Collation = q.LoadString()
        self.ColumnSize = q.LoadUInt()
        self.Flags = q.LoadUInt()
        self.DataType = q.LoadUShort()
        self.Precision = q.LoadByte()
        self.Scale = q.LoadByte()

    def SaveTo(self, q):
        q.SaveString(self.DBPath)
        q.SaveString(self.TablePath)
        q.SaveString(self.DisplayName)
        q.SaveString(self.OriginalName)
        q.SaveString(self.DeclaredType)
        q.SaveString(self.Collation)
        q.SaveUInt(self.ColumnSize)
        q.SaveUInt(self.Flags)
        q.SaveUShort(self.DataType)
        q.SaveByte(self.Precision)
        q.SaveByte(self.Scale)

class CDBColumnInfoArray(IUSerializer):
    def __init__(self, lstColumnInfo = []):
        self.__list__ = lstColumnInfo

    def LoadFrom(self, q):
        self.__list__ = []
        size = q.LoadUInt()
        while size > 0:
            colInfo = CDBColumnInfo()
            colInfo.LoadFrom(q)
            self.__list__.append(colInfo)
            size -= 1

    def SaveTo(self, q):
        size = len(self.__list__)
        q.SaveUInt(size)
        for obj in self.__list__:
            obj.SaveTo(q)

    def __iter__(self):
        return self.__list__.__iter__()

    def append(self, col):
        self.__list__.append(col)

    def __len__(self):
        return len(self.list)

    def __getitem__(self, item):
        return self.__list__.__getitem__(item)

    @property
    def list(self):
        return self.__list__

class tagParameterDirection(object):
    pdUnknown = 0
    pdInput = 1
    pdOutput = 2
    pdInputOutput = 3
    pdReturnValue = 4

class CParameterInfo(IUSerializer):
    def __init__(self, dt = tagVariantDataType.sdVT_NULL, paramterType = tagParameterDirection.pdInput):
        self.Direction = paramterType
        self.DataType = dt
        self.ColumnSize = 0
        self.Precision = 0
        self.Scale = 0
        self.ParameterName = u''

    def LoadFrom(self, q):
        self.Direction = q.LoadInt()
        self.DataType = q.LoadUShort()
        self.ColumnSize = q.LoadUInt()
        self.Precision = q.LoadByte()
        self.Scale = q.LoadByte()
        self.ParameterName = q.LoadString()

    def SaveTo(self, q):
        q.SaveInt(self.Direction)
        q.SaveUShort(self.DataType)
        q.SaveUInt(self.ColumnSize)
        q.SaveByte(self.Precision)
        q.SaveByte(self.Scale)
        q.SaveString(self.ParameterName)
