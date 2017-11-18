
from spa import *
from spa.udb import *
from spa.clientside.asyncdbhandler import CAsyncDBHandler
import copy
from functools import total_ordering
import threading


class CTable(object):
    BAD_ORDINAL = -1
    BAD_DATA_TYPE = -2
    OPERATION_NOT_SUPPORTED = -3
    COMPARISON_NOT_SUPPORTED = -4
    NO_TABLE_NAME_GIVEN = -5
    NO_TABLE_FOUND = -6
    BAD_INPUT_PARAMETER = -7

    INVALID_ORDINAL = -1

    @total_ordering
    class MinType(object):
        def __le__(self, other):
            return True
        def __eq__(self, other):
            return self is other
    _MIN_ = MinType()

    class Operator(object):
        equal = 0
        great = 1
        less = 2
        great_equal = 3
        less_equal = 4
        not_equal = 5
        is_null = 6

    def __init__(self, meta, bFieldCaseSensitive = False, bDataCaseSensitive = False):
        self._meta_ = CDBColumnInfoArray()
        if meta and isinstance(meta, CDBColumnInfoArray):
           self._meta_ = copy.deepcopy(meta)
        self._bFieldCaseSensitive_ = bFieldCaseSensitive
        self._bDataCaseSensitive_ = bDataCaseSensitive
        self._vRow_ = []

    @property
    def Meta(self):
        return self._meta_

    @property
    def DataMatrix(self):
        return self._vRow_

    @property
    def Keys(self):
        index = 0
        keys = {}
        for col in self._meta_:
            if (col.Flags & CDBColumnInfo.FLAG_AUTOINCREMENT) == CDBColumnInfo.FLAG_AUTOINCREMENT or (col.Flags & CDBColumnInfo.FLAG_PRIMARY_KEY) == CDBColumnInfo.FLAG_PRIMARY_KEY:
                keys[index] = col
            index += 1
        return keys

    def FindARow(self, key0, key1=None):
        if key0 is None:
            return None
        v_col = []
        keys = self.Keys
        for (c, col) in keys:
            v_col.append(c)
        for r in self._vRow_:
            if self._eq_(key0, r[v_col[0]]) > 0:
                if key1 is None:
                    return r
                if len(v_col) == 2 and self._eq_(key1, r[v_col[1]]) > 0:
                    return r
        return None

    def Copy(self):
        tbl = CTable(self._meta_, self._bFieldCaseSensitive_, self._bDataCaseSensitive_)
        tbl._vRow_ = copy.deepcopy(self._vRow_)
        return tbl

    def Append(self, tbl):
        if tbl and isinstance(tbl, CTable):
            if len(tbl._meta_) != len(self._meta_):
                return CTable.OPERATION_NOT_SUPPORTED
            index = 0
            for col in self._meta_:
                if col.DataType != tbl._meta_[index].DataType:
                    return CTable.BAD_DATA_TYPE
                index += 1
            for v in tbl._vRow_:
                self._vRow_.append(v)
        return 1

    def Sort(self, ordinal, desc = False):
        if ordinal < 0 or ordinal >= len(self._meta_):
            return CTable.BAD_ORDINAL
        dt = self._meta_[ordinal].DataType

        def key(a):
            if a[ordinal] is None:
                return CTable._MIN_
            return a[ordinal]

        def str_key_nocase(a):
            if a[ordinal] is None:
                return CTable._MIN_
            return a[ordinal].lower()
        key_func = key
        if not self._bDataCaseSensitive_ and (dt == tagVariantDataType.sdVT_BSTR or dt == (tagVariantDataType.sdVT_ARRAY | tagVariantDataType.sdVT_I1)):
            key_func = str_key_nocase
        self._vRow_.sort(key=key_func, reverse=desc)

    def FindOrdinal(self, colName):
        if not colName or not isinstance(colName, str):
            return CTable.INVALID_ORDINAL
        index = 0
        for col in self._meta_:
            s0 = self._meta_[index].DisplayName
            if not self._bFieldCaseSensitive_:
                s0 = s0.lower()
                colName = colName.lower()
            if s0 == colName:
                return index
            index += 1

    def _gt_(self, vt0, vt1):
        if vt0 is None:
            return 0
        if isinstance(vt0, str) and not self._bDataCaseSensitive_:
            vt0 = vt0.lower()
            vt1 = vt1.lower()
        if vt0 > vt1:
            return 1
        return 0

    def _ge_(self, vt0, vt1):
        if vt0 is None:
            return 0
        if isinstance(vt0, str) and not self._bDataCaseSensitive_:
            vt0 = vt0.lower()
            vt1 = vt1.lower()
        if vt0 >= vt1:
            return 1
        return 0

    def _lt_(self, vt0, vt1):
        if vt0 is None:
            return 0
        if isinstance(vt0, str) and not self._bDataCaseSensitive_:
            vt0 = vt0.lower()
            vt1 = vt1.lower()
        if vt0 < vt1:
            return 1
        return 0

    def _le_(self, vt0, vt1):
        if vt0 is None:
            return 0
        if isinstance(vt0, str) and not self._bDataCaseSensitive_:
            vt0 = vt0.lower()
            vt1 = vt1.lower()
        if vt0 <= vt1:
            return 1
        return 0

    def _eq_(self, vt0, vt1):
        if vt0 is None:
            return 0
        if isinstance(vt0, str) and not self._bDataCaseSensitive_:
            vt0 = vt0.lower()
            vt1 = vt1.lower()
        if vt0 == vt1:
            return 1
        return 0

    def _neq_(self, vt0, vt1):
        if vt0 is None:
            return 0
        if isinstance(vt0, str) and not self._bDataCaseSensitive_:
            vt0 = vt0.lower()
            vt1 = vt1.lower()
        if vt0 != vt1:
            return 1
        return 0

    def Find(self, ordinal, op, vt, tbl, copy_data=False):
        if not isinstance(tbl, CTable):
            return CTable.BAD_INPUT_PARAMETER
        tbl._meta_ = copy.deepcopy(self._meta_)
        tbl._vRow_ = []
        tbl._bDataCaseSensitive_ = self._bDataCaseSensitive_
        tbl._bFieldCaseSensitive_ = self._bFieldCaseSensitive_
        if ordinal >= len(self._meta_):
            return CTable.BAD_ORDINAL
        if vt is None and op != CTable.Operator.is_null:
            return CTable.COMPARISON_NOT_SUPPORTED
        for r in self._vRow_:
            ok = False
            if op == CTable.Operator.is_null:
                ok = r[ordinal] is None
            else:
                v0 = r[ordinal]
                if op == CTable.Operator.equal:
                    ok = self._eq_(v0, vt) > 0
                elif op == CTable.Operator.not_equal:
                    ok = self._neq_(v0, vt) > 0
                elif op == CTable.Operator.great:
                    ok = self._gt_(v0, vt) > 0
                elif op == CTable.Operator.great_equal:
                    ok = self._ge_(v0, vt) > 0
                elif op == CTable.Operator.less:
                    ok = self._lt_(v0, vt) > 0
                elif op == CTable.Operator.less_equal:
                    ok = self._le_(v0, vt) > 0
                else:
                    return CTable.OPERATION_NOT_SUPPORTED
            if ok:
                if copy_data:
                    tbl._vRow_.append(copy.deepcopy(r))
                else:
                    tbl._vRow_.append(r)
        return 1

    def FindNull(self, ordinal, tbl, copy_data=False):
        return self.Find(ordinal, CTable.Operator.is_null, None, tbl, copy_data)

    def Between(self, ordinal, vt0, vt1, tbl, copy_data=False):
        if not isinstance(tbl, CTable):
            return CTable.BAD_INPUT_PARAMETER
        tbl._meta_ = copy.deepcopy(self._meta_)
        tbl._vRow_ = []
        tbl._bDataCaseSensitive_ = self._bDataCaseSensitive_
        tbl._bFieldCaseSensitive_ = self._bFieldCaseSensitive_
        if ordinal >= len(self._meta_):
            return CTable.BAD_ORDINAL
        if vt0 is None or vt1 is None:
            return CTable.COMPARISON_NOT_SUPPORTED
        small_vt = vt0
        large_vt = vt1
        if self._gt_(small_vt, large_vt) > 0:
            small_vt = vt1
            large_vt = vt0
        for r in self._vRow_:
            vt = r[ordinal]
            if self._ge_(vt, small_vt) == 0:
                continue
            elif self._le_(vt, large_vt) == 0:
                continue
            if copy_data:
                tbl._vRow_.append(copy.deepcopy(r))
            else:
                tbl._vRow_.append(r)
        return 1

    def _In_(self, arr, v0):
        for a in arr:
            if self._eq_(a, v0) > 0:
                return True
        return False

    def In(self, ordinal, v, tbl, copy_data=False):
        if not isinstance(tbl, CTable):
            return CTable.BAD_INPUT_PARAMETER
        tbl._meta_ = copy.deepcopy(self._meta_)
        tbl._vRow_ = []
        tbl._bDataCaseSensitive_ = self._bDataCaseSensitive_
        tbl._bFieldCaseSensitive_ = self._bFieldCaseSensitive_
        if ordinal >= len(self._meta_):
            return CTable.BAD_ORDINAL
        for r in self._vRow_:
            v0 = r[ordinal]
            if self._In_(v, v0):
                if copy_data:
                    tbl._vRow_.append(copy.deepcopy(r))
                else:
                    tbl._vRow_.append(r)
        return 1

    def NotIn(self, ordinal, v, tbl, copy_data=False):
        if not isinstance(tbl, CTable):
            return CTable.BAD_INPUT_PARAMETER
        tbl._meta_ = copy.deepcopy(self._meta_)
        tbl._vRow_ = []
        tbl._bDataCaseSensitive_ = self._bDataCaseSensitive_
        tbl._bFieldCaseSensitive_ = self._bFieldCaseSensitive_
        if ordinal >= len(self._meta_):
            return CTable.BAD_ORDINAL
        for r in self._vRow_:
            v0 = r[ordinal]
            if not v0 is None and not self._In_(v, v0):
                if copy_data:
                    tbl._vRow_.append(copy.deepcopy(r))
                else:
                    tbl._vRow_.append(r)
        return 1


class CDataSet(object):
    INVALID_VALUE = -1

    def __init__(self):
        self._cs_ = threading.Lock()
        self._ds_ = []
        self._Ip_ = ''
        self._HostName_ = ''
        self._Updater_ = ''
        self._ms_ = tagManagementSystem.msUnknown
        self._DBNameCase_ = False
        self._TableNameCase_ = False
        self._FieldNameCase_ = False
        self._DataCase_ = False

    @property
    def IsEmpty(self):
        with self._cs_:
            return len(self._ds_) == 0

    @property
    def DBNameCase(self):
        with self._cs_:
            return self._DBNameCase_

    @DBNameCase.setter
    def DBNameCase(self, value):
        with self._cs_:
            self._DBNameCase_ = not not value

    @property
    def TableNameCase(self):
        with self._cs_:
            return self._TableNameCase_

    @TableNameCase.setter
    def TableNameCase(self, value):
        with self._cs_:
            self._TableNameCase_ = not not value

    @property
    def FieldNameCase(self):
        with self._cs_:
            return self._FieldNameCase_

    @FieldNameCase.setter
    def FieldNameCase(self, value):
        with self._cs_:
            self._FieldNameCase_ = not not value

    @property
    def DataCase(self):
        with self._cs_:
            return self._DataCase_

    @DataCase.setter
    def DataCase(self, value):
        with self._cs_:
            self._DataCase_ = not not value

    def Empty(self):
        with self._cs_:
            self._ds_ = []

    @property
    def DBManagementSystem(self):
        with self._cs_:
            return self._ms_

    @property
    def DBServerIp(self):
        with self._cs_:
            return self._Ip_

    @property
    def DBServerName(self):
        with self._cs_:
            return self._HostName_

    @DBServerName.setter
    def DBServerName(self, value):
        with self._cs_:
            if isinstance(value, str):
                self._HostName_ = value

    @property
    def Updater(self):
        with self._cs_:
            return self._Updater_

    @Updater.setter
    def Updater(self, value):
        with self._cs_:
            if isinstance(value, str):
                self._Updater_ = value

    def Set(self, ip, ms):
        if not isinstance(ip, str):
            ip = ''
        with self._cs_:
            self._Ip_ = ip
            self._ms_ = ms

    def Swap(self, ds):
        if not isinstance(ds, CDataSet):
            return
        with self._cs_:
            temp = self._ds_
            self._ds_ = ds._ds_
            ds._ds_ = temp
            temp = self._Ip_
            self._Ip_ = ds._Ip_
            ds._Ip_ = temp
            temp = self._HostName_
            self._HostName_ = ds._HostName_
            ds._HostName_ = temp
            temp = self._Updater_
            self._Updater_ = ds._Updater_
            ds._Updater_ = temp
            temp = self._ms_
            self._ms_ = ds._ms_
            ds._ms_ = temp

    def AddEmptyRowset(self, meta):
        if meta is None or len(meta) == 0:
            return
        if len(meta[0].DBPath) == 0 or len(meta[0].TablePath) == 0:
            raise Exception('The first column meta must contain database name and table name')
        tbl = CTable(meta, self._FieldNameCase_, self._DataCase_)
        keys = tbl.Keys
        if len(keys) > 2 or len(keys) == 0:
            raise Exception('Table meta must contain one or two keys')
        with self._cs_:
            self._ds_.append(tbl)

    @property
    def DBTablePair(self):
        pair = []
        with self._cs_:
            for tbl in self._ds_:
                p = (tbl._meta_[0].DBPath, tbl._meta_[0].TablePath)
                pair.append(p)
        return pair

    def _Is_(self, tbl, dbName, tblName):
        s0 = tbl._meta_[0].DBPath
        if not self._DBNameCase_:
            s0 = s0.lower()
            dbName = dbName.lower()
        if s0 != dbName:
            return False
        s0 = tbl._meta_[0].TablePath
        if not self._TableNameCase_:
            s0 = s0.lower()
            tblName = tblName.lower()
        return s0 == tblName

    def GetColumMeta(self, dbName, tblName):
        if not isinstance(tblName, str):
            return CTable.NO_TABLE_NAME_GIVEN
        if not isinstance(dbName, str):
            dbName = ''
        with self._cs_:
            for tbl in self._ds_:
                if self._Is_(tbl, dbName, tblName):
                    return tbl._meta_
        return CDBColumnInfoArray()

    def GetRowCount(self, dbName, tblName):
        if not isinstance(tblName, str):
            return CTable.NO_TABLE_NAME_GIVEN
        if not isinstance(dbName, str):
            dbName = ''
        with self._cs_:
            for tbl in self._ds_:
                if self._Is_(tbl, dbName, tblName):
                    return len(tbl._vRow_)
        return CDataSet.INVALID_VALUE

    def FindKeys(self, dbName, tblName):
        if not isinstance(tblName, str):
            return CTable.NO_TABLE_NAME_GIVEN
        if not isinstance(dbName, str):
            dbName = ''
        with self._cs_:
            for tbl in self._ds_:
                if self._Is_(tbl, dbName, tblName):
                    return tbl.Keys
        return ()

    def FindOrdinal(self, dbName, tblName, colName):
        if not isinstance(tblName, str):
            return CTable.NO_TABLE_NAME_GIVEN
        if not isinstance(dbName, str):
            dbName = ''
        with self._cs_:
            for tbl in self._ds_:
                if self._Is_(tbl, dbName, tblName):
                    return tbl.FindOrdinal(colName)
        return CDataSet.INVALID_VALUE

    def Find(self, dbName, tblName, ordinal, op, vt, tbl):
        if not isinstance(dbName, str):
            dbName = ''
        if not isinstance(tblName, str) or len(tblName) == 0:
            return CTable.NO_TABLE_NAME_GIVEN
        with self._cs_:
            for t in self._ds_:
                if self._Is_(t, dbName, tblName):
                    return t.Find(ordinal, op, vt, tbl, True)
        return CTable.NO_TABLE_FOUND

    def FindNull(self, dbName, tblName, ordinal, tbl):
        return self.Find(dbName, tblName, ordinal, CTable.Operator.is_null, None, tbl)

    def In(self, dbName, tblName, ordinal, v, tbl):
        if not isinstance(dbName, str):
            dbName = ''
        if not isinstance(tblName, str) or len(tblName) == 0:
            return CTable.NO_TABLE_NAME_GIVEN
        with self._cs_:
            for t in self._ds_:
                if self._Is_(t, dbName, tblName):
                    return t.In(ordinal, v, tbl, True)
        return CTable.NO_TABLE_FOUND

    def NotIn(self, dbName, tblName, ordinal, v, tbl):
        if not isinstance(dbName, str):
            dbName = ''
        if not isinstance(tblName, str) or len(tblName) == 0:
            return CTable.NO_TABLE_NAME_GIVEN
        with self._cs_:
            for t in self._ds_:
                if self._Is_(t, dbName, tblName):
                    return t.NotIn(ordinal, v, tbl, True)
        return CTable.NO_TABLE_FOUND

    def Between(self, dbName, tblName, ordinal, v0, v1, tbl):
        if not isinstance(dbName, str):
            dbName = ''
        if not isinstance(tblName, str) or len(tblName) == 0:
            return CTable.NO_TABLE_NAME_GIVEN
        with self._cs_:
            for t in self._ds_:
                if self._Is_(t, dbName, tblName):
                    return t.Between(ordinal, v0, v1, tbl, True)
        return CTable.NO_TABLE_FOUND

    def AddRows(self, dbName, tblName, vData):
        size = len(vData)
        if size == 0:
            return 0
        if not isinstance(dbName, str) or not isinstance(tblName, str):
            return CDataSet.INVALID_VALUE
        with self._cs_:
            for t in self._ds_:
                if self._Is_(t, dbName, tblName):
                    col_count = len(t._meta_)
                    if size % col_count > 0:
                        return CDataSet.INVALID_VALUE
                    r = []
                    index = 0
                    for d in vData:
                        if index % col_count == 0:
                            r = []
                            t._vRow_.append(r)
                        r.append(d)
                        index += 1
                    return size / col_count
        return CTable.NO_TABLE_FOUND

    def UpdateARow(self, dbName, tblName, vData):
        size = len(vData)
        if size == 0:
            return 0
        if not isinstance(dbName, str) or not isinstance(tblName, str):
            return CDataSet.INVALID_VALUE
        with self._cs_:
            for tbl in self._ds_:
                if self._Is_(tbl, dbName, tblName):
                    col_count = len(tbl._meta_)
                    if 2 * col_count != size:
                        return CDataSet.INVALID_VALUE
                    keys = tbl.Keys
                    v = []
                    for (c, col) in keys:
                        v.append(vData[2 * c])
                    key1 = None
                    if len(v) == 2:
                        key1 = v[1]
                    r = tbl.FindARow(v[0], key1)
                    if not isinstance(list):
                        return 0
                    index = 0
                    while index < col_count:
                        r[index] = vData[2 * index + 1]
                        index += 1
                    return 1
        return 0

    def DeleteARow(self, dbName, tblName, key0, key1):
        deleted = 0
        if not isinstance(dbName, str) or not isinstance(tblName, str):
            return CDataSet.INVALID_VALUE
        with self._cs_:
            for tbl in self._ds_:
                if self._Is_(tbl, dbName, tblName):
                    r = tbl.FindARow(key0, key1)
                    if not isinstance(list):
                        return 0
                    tbl._vRow_.remove(r)
                    return 1
        return 0




