
#ifndef _UDAPARTS_ASYNC_ODBC_SQL_SERVER_H_
#define _UDAPARTS_ASYNC_ODBC_SQL_SERVER_H_

#ifndef __sqlncli_h__

/* defines for MS SQL SERVER */

#define SQL_CA_SS_VARIANT_TYPE          1215

// Driver specific SQL data type defines.
// Microsoft has -150 thru -199 reserved for Microsoft SQL Server Native Client driver usage.
#define SQL_SS_VARIANT                  (-150)
#define SQL_SS_XML                      (-152)

#endif

#ifndef SQL_H_SQLCLI

//IBM DB2 defines

/* SQL extended data types for IBM DB2 */
#define  SQL_GRAPHIC                    -95
#define  SQL_VARGRAPHIC                 -96
#define  SQL_LONGVARGRAPHIC             -97
#define  SQL_BLOB                       -98
#define  SQL_CLOB                       -99
#define  SQL_DBCLOB                     -350
#define  SQL_XML                        -370

#endif

#endif