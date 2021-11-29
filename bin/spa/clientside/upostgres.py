from spa.clientside.asyncdbhandler import CAsyncDBHandler
from spa import BaseServiceID, tagBaseRequestID

class CPostgres(CAsyncDBHandler):
    # Asynchronous and SQL streaming postgreSQL service id
    sidPostgres = BaseServiceID.sidReserved + 0x6FFFFFF4

    """
    An Open flag option, which is specific to PostgreSQL plugin.
    It is noted that this flag option is not implemented within SocketPro plugin yet.
    """
    ROWSET_META_FLAGS_REQUIRED = 0x40000000

    """
    An Open flag option, which is specific to PostgreSQL plugin.
    When the flag option is used with the method Open or open,
    it forces fetching data from remote PostgreSQL server to SocketPro plugin row-by-row instead of all.
    The flag option should be used if there is a large number of data within a rowset.
    """
    USE_SINGLE_ROW_MODE = 0x20000000

    # error codes for unexpected programming errors
    ER_NO_DB_OPENED_YET = -1981
    ER_BAD_END_TRANSTACTION_PLAN = -1982
    ER_NO_PARAMETER_SPECIFIED = -1983
    ER_BAD_PARAMETER_COLUMN_SIZE = -1984
    ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985
    ER_DATA_TYPE_NOT_SUPPORTED = -1986
    ER_BAD_TRANSTACTION_STAGE = -1987

    def __init__(self, sid=sidPostgres):
        super(CPostgres, self).__init__(sid)
