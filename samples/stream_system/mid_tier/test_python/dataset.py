
from spa import *
from spa.udb import *
import copy
from functools import total_ordering


class CTable(object):
    BAD_ORDINAL = -1
    BAD_DATA_TYPE = -2
    OPERATION_NOT_SUPPORTED = -3
    COMPARISON_NOT_SUPPORTED = -4
    NO_TABLE_NAME_GIVEN = -5
    NO_TABLE_FOUND = -6

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
                if (col.DataType != tbl._meta_[index].DataType):
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
