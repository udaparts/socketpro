
from spa.clientside.asyncdbhandler import CAsyncDBHandler
from spa import BaseServiceID, tagBaseRequestID

class CMysql(CAsyncDBHandler):
    sidMysql = BaseServiceID.sidReserved + 0x6FFFFFF1  # asynchronous mysql service id

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

    def __init__(self):
        super(CMysql, self).__init__(CMysql.sidMysql)