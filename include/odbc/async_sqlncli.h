
#ifndef _UDAPARTS_ASYNC_ODBC_SQL_SERVER_H_
#define _UDAPARTS_ASYNC_ODBC_SQL_SERVER_H_

#ifndef __sqlncli_h__

/* defines for MS SQL SERVER */

#define SQL_CA_SS_VARIANT_TYPE	1215

// Driver specific SQL data type defines.
// Microsoft has -150 thru -199 reserved for Microsoft SQL Server Native Client driver usage.
#define SQL_SS_VARIANT                      (-150)
#define SQL_SS_UDT                          (-151)
#define SQL_SS_XML                          (-152)
#define SQL_SS_TABLE                        (-153)
#define SQL_SS_TIME2                        (-154)
#define SQL_SS_TIMESTAMPOFFSET              (-155)

#endif

#ifndef SQL_H_SQLCLI

//IBM DB2 defines

/* SQL extended data types for IBM DB2 */
#define  SQL_GRAPHIC            -95
#define  SQL_VARGRAPHIC         -96
#define  SQL_LONGVARGRAPHIC     -97
#define  SQL_BLOB               -98
#define  SQL_CLOB               -99
#define  SQL_DBCLOB             -350
#define  SQL_XML                -370
#define  SQL_CURSORHANDLE       -380
#define  SQL_DATALINK           -400
#define  SQL_USER_DEFINED_TYPE  -450

/* C data type to SQL data type mapping */
#define  SQL_C_DBCHAR         SQL_DBCLOB
#define  SQL_C_DECIMAL_IBM    SQL_DECIMAL
#define  SQL_C_DATALINK       SQL_C_CHAR
#define  SQL_C_PTR            2463
#define  SQL_C_DECIMAL_OLEDB  2514
#define  SQL_C_DECIMAL64      SQL_DECFLOAT
#define  SQL_C_DECIMAL128     -361
#define  SQL_C_TIMESTAMP_EXT  -362
#define  SQL_C_TYPE_TIMESTAMP_EXT SQL_C_TIMESTAMP_EXT
#define  SQL_C_BINARYXML      -363
#define  SQL_C_TIMESTAMP_EXT_TZ         -364
#define  SQL_C_TYPE_TIMESTAMP_EXT_TZ    SQL_C_TIMESTAMP_EXT_TZ
#define  SQL_C_CURSORHANDLE   -365

#endif

#endif