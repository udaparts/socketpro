
from spa.clientside.asyncdbhandler import CAsyncDBHandler
from spa import BaseServiceID, tagBaseRequestID

class CMysql(CAsyncDBHandler):
    sidMysql = BaseServiceID.sidReserved + 0x6FFFFFF1  # asynchronous mysql service id

    """
    Use Mysql embedded at SocketPro server side by default.
    Use this const value for the input parameter flags with the method of CAsyncDBHandler::Open at client side
    to open a connection to remote Mysql server at SocketPro server instead of embedded Mysql
    """
    USE_REMOTE_MYSQL = 0x1

    # error codes from async mysql server library
    ER_NO_DB_OPENED_YET = 1981
    ER_BAD_END_TRANSTACTION_PLAN = 1982
    ER_NO_PARAMETER_SPECIFIED = 1983
    ER_BAD_PARAMETER_COLUMN_SIZE = 1984
    ER_BAD_PARAMETER_DATA_ARRAY_SIZE = 1985
    ER_DATA_TYPE_NOT_SUPPORTED = 1986
    ER_BAD_MANUAL_TRANSACTION_STATE = 1987
    ER_UNABLE_TO_SWITCH_TO_DATABASE = 1988
    ER_SERVICE_COMMAND_ERROR = 1989
    ER_MYSQL_LIBRARY_NOT_INITIALIZED = 1990

    def __init__(self, sid=sidMysql):
        super(CMysql, self).__init__(sid)

class CSqlServer(CAsyncDBHandler):
    sidMsSql = BaseServiceID.sidReserved + 0x6FFFFFF2  # asynchronous MS SQL stream service id

    def __init__(self, sid=sidMsSql):
        super(CSqlServer, self).__init__(sid)
