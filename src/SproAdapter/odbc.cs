
using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public class COdbc : CAsyncDBHandler
        {
            public const uint sidOdbc = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF2; //asynchronous ODBC service id
            public COdbc()
                : base(sidOdbc)
            {
            }

            //meta recordsets
            public const ushort idSQLColumnPrivileges = 0x7f00 + 100;
            public const ushort idSQLColumns = 0x7f00 + 101;
            public const ushort idSQLForeignKeys = 0x7f00 + 102;
            public const ushort idSQLPrimaryKeys = 0x7f00 + 103;
            public const ushort idSQLProcedureColumns = 0x7f00 + 104;
            public const ushort idSQLProcedures = 0x7f00 + 105;
            public const ushort idSQLSpecialColumns = 0x7f00 + 106;
            public const ushort idSQLStatistics = 0x7f00 + 107;
            public const ushort idSQLTablePrivileges = 0x7f00 + 108;
            public const ushort idSQLTables = 0x7f00 + 109;
            public const ushort idSQLGetInfo = 0x7f00 + 110;

            public const int ER_SUCCESS = 0;
            public const int ER_ERROR = -1;

            //error codes from async ODBC server library
            public const int ER_NO_DB_OPENED_YET = -1981;
            public const int ER_BAD_END_TRANSTACTION_PLAN = -1982;
            public const int ER_NO_PARAMETER_SPECIFIED = -1983;
            public const int ER_BAD_PARAMETER_COLUMN_SIZE = -1984;
            public const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985;
            public const int ER_DATA_TYPE_NOT_SUPPORTED = -1986;
            public const int ER_NO_DB_NAME_SPECIFIED = -1987;
            public const int ER_ODBC_ENVIRONMENT_NOT_INITIALIZED = -1988;
            public const int ER_BAD_MANUAL_TRANSACTION_STATE = -1989;
            public const int ER_BAD_INPUT_PARAMETER_DATA_TYPE = -1990;
            public const int ER_BAD_PARAMETER_DIRECTION_TYPE = -1991;
            public const int ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = -1992;

            private Dictionary<ushort, object> m_mapInfo = new Dictionary<ushort, object>();

            public object GetInfo(ushort infoType)
            {
                lock (m_csDB)
                {
                    if (m_mapInfo.ContainsKey(infoType))
                        return m_mapInfo[infoType];
                    return null;
                }
            }
        }
    }
}
