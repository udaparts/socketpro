
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

#define SQL_SOPT_SS_BASE                            1225
#define SQL_SOPT_SS_HIDDEN_COLUMNS                  (SQL_SOPT_SS_BASE+2) // Expose FOR BROWSE hidden columns

#define SQL_CA_SS_BASE                              1200
#define SQL_CA_SS_COLUMN_KEY                        (SQL_CA_SS_BASE+12)  //  Column is key column (FOR BROWSE)

#define SQL_HC_ON                           		1L           //  FOR BROWSE columns are exposed

#endif

#endif