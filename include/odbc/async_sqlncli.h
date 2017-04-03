
#ifndef _UDAPARTS_ASYNC_ODBC_SQL_SERVER_H_
#define _UDAPARTS_ASYNC_ODBC_SQL_SERVER_H_

#ifndef __sqlncli_h__

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

#endif