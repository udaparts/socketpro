
from spa.memqueue import CUQueue, CScopeUQueue
from spa.clientside.asyncdbhandler import CAsyncDBHandler
from spa import BaseServiceID, tagBaseRequestID, Pair

class COdbc(CAsyncDBHandler):
    sidOdbc = BaseServiceID.sidODBC # asynchronous ODBC service id

    # meta recordsets
    idSQLColumnPrivileges = 0x7f00 + 100
    idSQLColumns = 0x7f00 + 101
    idSQLForeignKeys = 0x7f00 + 102
    idSQLPrimaryKeys = 0x7f00 + 103
    idSQLProcedureColumns = 0x7f00 + 104
    idSQLProcedures = 0x7f00 + 105
    idSQLSpecialColumns = 0x7f00 + 106
    idSQLStatistics = 0x7f00 + 107
    idSQLTablePrivileges = 0x7f00 + 108
    idSQLTables = 0x7f00 + 109
    idSQLGetInfo = 0x7f00 + 110;

    ER_SUCCESS = 0
    ER_ERROR = -1

    # error codes from async ODBC server library
    ER_NO_DB_OPENED_YET = -1981
    ER_BAD_END_TRANSTACTION_PLAN = -1982
    ER_NO_PARAMETER_SPECIFIED = -1983
    ER_BAD_PARAMETER_COLUMN_SIZE = -1984
    ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985
    ER_DATA_TYPE_NOT_SUPPORTED = -1986
    ER_NO_DB_NAME_SPECIFIED = -1987
    ER_ODBC_ENVIRONMENT_NOT_INITIALIZED = -1988
    ER_BAD_MANUAL_TRANSACTION_STATE = -1989
    ER_BAD_INPUT_PARAMETER_DATA_TYPE = -1990
    ER_BAD_PARAMETER_DIRECTION_TYPE = -1991
    ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = -1992

    def ColumnPrivileges(self, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, canceled = None):
        return self._DoMeta4(COdbc.idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, canceled)

    def Columns(self, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, canceled = None):
        return self._DoMeta4(COdbc.idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, canceled)

    def ProcedureColumns(self, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, canceled = None):
        return self._DoMeta4(COdbc.idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, canceled)

    def PrimaryKeys(self, CatalogName, SchemaName, TableName, handler, row, rh, canceled = None):
        return self._DoMeta3(COdbc.idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh, canceled)

    def Procedures(self, CatalogName, SchemaName, ProcName, handler, row, rh, canceled = None):
        return self._DoMeta3(COdbc.idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh, canceled)

    def TablePrivileges(self, CatalogName, SchemaName, TableName, handler, row, rh, canceled = None):
        return self._DoMeta3(COdbc.idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh, canceled)

    def Tables(self, CatalogName, SchemaName, TableName, TableType, handler, row, rh, canceled = None):
        return self._DoMeta4(COdbc.idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh, canceled)

    def _ProcessODBC_(self, handler, ar, reqId, index):
        mc = ar.UQueue;
        res = mc.LoadInt()
        errMsg = mc.LoadString()
        fail_ok = mc.LoadULong()
        with self._csDB:
            self._dbError = res
            self._dbErrorMsg = errMsg
            self._lastReqId = reqId
            self._affected = 0
            if index in self._mapRowset:
                self._mapRowset.pop(index)
        if handler:
            handler(self, res, errMsg, 0, fail_ok, None)

    def Statistics(self, CatalogName, SchemaName, TableName, unique, reserved, handler, row, rh, canceled = None):
        q = CScopeUQueue.Lock().SaveString(CatalogName).SaveString(SchemaName).SaveString(TableName).SaveUShort(unique).SaveUShort(reserved)
        ok = True
        with self._csOneSending:
            index = self.GetCallIndex()
            q.SaveULong(index)
            with self._csDB:
                self._mapRowset[index] = Pair(rh, row)
            ok = self.SendRequest(COdbc.idSQLStatistics, q, lambda ar: self._ProcessODBC_(handler, ar, COdbc.idSQLStatistics, index), canceled)
            if not ok:
                with self._csDB:
                    self._mapRowset.pop(index)
        CScopeUQueue.Unlock(q)
        return ok

    def SpecialColumns(self, identifierType, CatalogName, SchemaName, TableName, scope, nullable, handler, row, rh, canceled = None):
        q = CScopeUQueue.Lock().SaveShort(identifierType).SaveString(CatalogName).SaveString(SchemaName).SaveString(TableName).SaveShort(scope).SaveShort(nullable)
        ok = True
        with self._csOneSending:
            index = self.GetCallIndex()
            q.SaveULong(index)
            with self._csDB:
                self._mapRowset[index] = Pair(rh, row)
            ok = self.SendRequest(COdbc.idSQLSpecialColumns, q, lambda ar: self._ProcessODBC_(handler, ar, COdbc.idSQLSpecialColumns, index), canceled)
            if not ok:
                with self._csDB:
                    self._mapRowset.pop(index)
        CScopeUQueue.Unlock(q)
        return ok

    def ForeignKeys(self, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, handler, row, rh, canceled = None):
        q = CScopeUQueue.Lock().SaveString(PKCatalogName).SaveString(PKSchemaName).SaveString(PKTableName).SaveString(FKCatalogName).SaveString(FKSchemaName).SaveString(FKTableName)
        ok = True
        with self._csOneSending:
            index = self.GetCallIndex()
            q.SaveULong(index)
            with self._csDB:
                self._mapRowset[index] = Pair(rh, row)
            ok = self.SendRequest(COdbc.idSQLForeignKeys, q, lambda ar: self._ProcessODBC_(handler, ar, COdbc.idSQLForeignKeys, index), canceled)
            if not ok:
                with self._csDB:
                    self._mapRowset.pop(index)
        CScopeUQueue.Unlock(q)
        return ok

    def _DoMeta3(self, id, s0, s1, s2, handler, row, rh, canceled):
        q = CScopeUQueue.Lock().SaveString(s0).SaveString(s1).SaveString(s2)
        ok = True
        with self._csOneSending:
            index = self.GetCallIndex()
            q.SaveULong(index)
            with self._csDB:
                self._mapRowset[index] = Pair(rh, row)
            ok = self.SendRequest(id, q, lambda ar: self._ProcessODBC_(handler, ar, id, index), canceled)
            if not ok:
                with self._csDB:
                    self._mapRowset.pop(index)
        CScopeUQueue.Unlock(q)
        return ok

    def _DoMeta4(self, id, s0, s1, s2, s3, handler, row, rh, canceled):
        q = CScopeUQueue.Lock().SaveString(s0).SaveString(s1).SaveString(s2).SaveString(s3)
        ok = True
        with self._csOneSending:
            index = self.GetCallIndex()
            q.SaveULong(index)
            with self._csDB:
                self._mapRowset[index] = Pair(rh, row)
            ok = self.SendRequest(id, q, lambda ar: self._ProcessODBC_(handler, ar, id, index), canceled)
            if not ok:
                with self._csDB:
                    self._mapRowset.pop(index)
        CScopeUQueue.Unlock(q)
        return ok

    def __init__(self, sid=sidOdbc):
        super(COdbc, self).__init__(sid)
        self._mapInfo = {}

    def GetInfo(self, infoType):
        with self._csDB:
            if infoType in self._mapInfo:
                return self._mapInfo.get(infoType)

    def OnResultReturned(self, reqId, mc):
        if reqId == COdbc.idSQLGetInfo:
            with self._csDB:
                self._mapInfo = {}
                while mc.GetSize() > 0:
                    infoType = mc.LoadUShort()
                    vt = mc.LoadObject()
                    self._mapInfo[infoType] = vt
        else:
            super(COdbc, self).OnResultReturned(reqId, mc)


