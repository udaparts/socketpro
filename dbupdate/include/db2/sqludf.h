/******************************************************************************
** 
** Source File Name: SQLUDF
** 
** (C) COPYRIGHT International Business Machines Corp. 1987, 2003
** All Rights Reserved
** Licensed Materials - Property of IBM
** 
** US Government Users Restricted Rights - Use, duplication or
** disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
** 
** Function = Include File for use in compiling User Defined Function.
**            Intended to help writers of UDFs deal with the interface
**            between DBMS and UDF.  Contains:
**              common structures
**              "language types" for the SQL types
**              enumerated type declaration for call-type argument
**              SQLSTATE constants
**              macro for prescribed part of the argument list
**              macro for scratchpad oriented part of argument list
**             macros to address prescribed parts of argument list
**             macros to test for argument null and notnull
** 
** Operating System: Common Include File
** 
** Product Names = DB2 Universal Database for z/OS Version 8
**                 DB2 Universal Database Version 8
**                 DB2 Universal Database for iSeries Version 5 Release 3
** 
*******************************************************************************/
#ifndef SQL_H_SQLUDF
#define SQL_H_SQLUDF

#ifdef __cplusplus
extern "C" {
#endif

#if (__OS400_TGTVRM__>=510)
#pragma datamodel(P128)
#endif

#include "sqlsystm.h"                  /* System-Specific Include */

#if (defined(DB2OS2))
#pragma pack(4)
#elif (defined(DB2NT))
#ifdef _WIN64
#pragma pack(8)
#else
#pragma pack(4)
#endif
#elif (defined(DB2WIN))
#pragma pack(2)
#elif (defined(DB2MAC))
#if (defined(DB2PPC))
#pragma align 4
#elif (defined(DB268K))
#pragma align 2
#endif
#elif (defined(DB2390))
#pragma pack(full)

#elif (defined(DB2SUN) && defined(__xlC__))
#pragma options align=natural
#elif (defined(DB2AIX) && defined(__64BIT__))
#pragma options align=natural
#elif (defined(DB2AIX))
#pragma options align=power
#elif (defined(DB2HP) || defined(DB2SNI) || defined(DB2SUN))
#endif

/*==============================================*/
/*==============================================*/
/*== Common #defines, used in this file.      ==*/
/*==============================================*/
/*==============================================*/
/******************************************************************************
** When the following LEN defines are used to define a null-terminated
** string, be sure to add 1 for the null terminating character.
*******************************************************************************/
#define SQLUDF_SQLSTATE_LEN     5      /* length of SQLSTATE                  */
#define SQLUDF_SCRATCHPAD_LEN   100    /* length of scratchpad                */
#define SQLUDF_FQNAME_LEN       517    /* length of fully-qualified name      */
#define SQLUDF_SPECNAME_LEN     128    /* length of function specific name    */
#define SQLUDF_MSGTEXT_LEN      1000   /* length of message text area         */
#define SQLUDF_MAX_IDENT_LEN    128    /* max length of identifier            */
#define SQLUDF_SH_IDENT_LEN     8      /* length of short identifier          */

/******************************************************************************
** LIMIT on total number of columns for a result table of TABLE FUNCTION.
** This may be DIFFERENT than the regular limit on DB2 tables.
*******************************************************************************/
#if (defined(DB2390))
#define SQLUDF_MAXCOL_TABFUN       750
#else
#define SQLUDF_MAXCOL_TABFUN       255
#endif

/******************************************************************************
** Common structures.
** Note:  these are used in the C "language type" definitions which follow.
*******************************************************************************/

#ifndef SQL_SQLDBCHAR_DEFN
#define SQL_SQLDBCHAR_DEFN
  typedef unsigned short sqldbchar;
#endif

/* Structure used for VARCHAR FOR BIT DATA, LONG VARCHAR                      */
SQL_STRUCTURE sqludf_vc_fbd
{
   unsigned short  length;             /* length of data                      */
   char            data[1];            /* first char of data                  */
};

/* Structure used for LONG VARGRAPHIC                                         */
SQL_STRUCTURE sqludf_vg
{
   unsigned short length;         /* length of data */
#ifdef SQL_WCHART_CONVERT
   wchar_t        data[1];        /* first char of data */
#else
   sqldbchar      data[1];        /* first char of data */
#endif
};

/* Structure used for BLOB, CLOB                                              */
SQL_STRUCTURE sqludf_lob
{
   sqluint32       length;             /* length of data                      */
   char            data[1];            /* first char of data                  */
};

/* Structure used for DBCLOB                                                  */
SQL_STRUCTURE  sqludf_lobg
{
   sqluint32      length;         /* length of data */
#ifdef SQL_WCHART_CONVERT
   wchar_t        data[1];        /* first char of data */
#else
   sqldbchar      data[1];        /* first char of data */
#endif
};

#ifndef NO_DATALINK
/* @dlnkdlnkpvt -- begin                                                      */
SQL_STRUCTURE sqludf_datalink
{
   sqluint32       version;
   char            linktype[4];
   sqluint32       url_length;
   sqluint32       comment_length;
   char            reserve2[8];        /* Future                              */
   char            url_plus_comment[230]; /* URL followed by comment          */
};
/* @dlnkdlnkpvt -- end                                                        */
#endif

/******************************************************************************
** Structure used for: the scratchpad.
** Note: This structure as provided by DB2 gives the
**       overall size/structure of the scratchpad argument.
**       Individual UDFs which use a scratchpad may wish to
**       override the "data" variable of this scratchpad
**       with variables that make sense for the particular
**       UDF.
** Note: Remember that the "data" variable is initialized
**       by DB2 to all \0 before the first call.
*******************************************************************************/
SQL_STRUCTURE sqludf_scratchpad
{
   sqluint32       length;             /* length of scratchpad data           */
   char            data[SQLUDF_SCRATCHPAD_LEN]; /* scratchpad data, init. to  */
                                       /* all \0                              */
};

/* -------------------------------------------------------------------------  */
/*                                                                            */
/*  Structure used for: Environment CCSID                                     */
/*                                                                            */
/*   db2_ccsids:                                                              */
/*    An array of triplets of CCSIDs.  There is one set                       */
/*    of 3 CCSIDs for each encoding scheme (ASCII, EBCDIC, and                */
/*    Unicode). It contains the CCSIDs used by default when no                */
/*    explicit CCSID is specified. For example, the CCSID of a host           */
/*    variable when a specific CCSID is not specified.                        */
/*    Since the environment CCSIDs can be explicitly overridden and           */
/*    since a CCSID clause can be specified in a parameter definition         */
/*    in both DB2 UDB for iSeries and DB2 UDB for z/OS, these                 */
/*    CCSIDs cannot generally be used to determine the CCSID of the           */
/*    data passed in the arguments of the function.                           */
/*                                                                            */
/*    On DB2 UDB for z/OS, the environment CCSIDs are the CCSIDs for          */
/*    the table accessed in the containing SQL statement.                     */
/*    On DB2 UDB for iSeries, the environment CCSIDs are the CCSIDs           */
/*    associated with the job.                                                */
/*    On DB2 UDB for multiplatforms, the environment CCSIDs are the           */
/*    CCSIDs for the relational database.                                     */
/*                                                                            */
/*   db2_encoding_scheme:                                                     */
/*    indicates which of the 3 CCSID triplets should be used.                 */
/*    db2_encoding_scheme can be used as an index into the                    */
/*    db2_ccsids array to access the appropriate CCSIDs.                      */
/*                                                                            */
/* -------------------------------------------------------------------------  */
/*                                                                            */
SQL_STRUCTURE db2_ccsids_type
{
   sqluint32       db2_sbcs;           /* DB single byte CS                   */
   sqluint32       db2_dbcs;           /* DB double byte CS                   */
   sqluint32       db2_mixed;          /* DB composite codepage CS            */
};

SQL_STRUCTURE db2_cdpg
{
   struct db2_ccsids_type db2_ccsids_triplet[3]; /* CCSID tripplet            */
   sqluint32        db2_encoding_scheme; /* DB2 ccsid encoding schema         */
   unsigned char   reserved[8];
};

#define db2_ccsids_t  db2_ccsids_triplet  /* support deprecated name */
#define db2_ccsids    db2_ccsids_type  /* support deprecated name */

/* ===============================================================            */
/* == encoding_scheme values for db2_cdpg.                      ==            */
/* == The encoding_scheme field indicates which of the three    ==            */
/* == CCSID triplets should be used as the environment CCSIDs   ==            */
/* ===============================================================            */
#define SQLUDF_ASCII 0                 /* ASCII                               */
#define SQLUDF_EBCDIC 1                /* EBCDIC                              */
#define SQLUDF_UNICODE 2               /* UNICODE                             */

/* Structure used for: Database code page.Deprecated, use db2_cdpg instead.   */
/* DB2/CS code page structure                                                 */
SQL_STRUCTURE db2_cs_cdpg
{
   sqluint32       sbcs;               /* DB single byte CS                   */
   sqluint32       dbcs;               /* DB double byte CS                   */
   sqluint32       comp;               /* DB composite codepage CS            */
   unsigned char   dummy[36];          /* reserved for later use              */
};

/* DB2 MVS code page structure. Deprecated, use db2_cdpg instead.             */
/*  References to db2_mvs_cdpg.encoding_schema will not return                */
/*  the correct value.                                                        */
/* The db2_encoding_scheme field                                              */
/*  defined in db2_cdpg above must be used instead.                           */
SQL_STRUCTURE db2_mvs_cdpg
{
   sqluint32       ascii_sbcs;
   sqluint32       ascii_dbcs;
   sqluint32       ascii_mixed;
   sqluint32       ebcdic_sbcd;
   sqluint32       ebcdic_dbcs;
   sqluint32       ebcdic_mixed;
   sqluint32       encoding_scheme;
   unsigned char   reserved[20];
};

/* CCSID Structure                                                            */
union db_cdpg
{
   struct db2_cs_cdpg cdpg_cs;         /* DB2 UDB UWO structure. Deprecated.  */
   struct db2_mvs_cdpg cdpg_mvs;       /* DB2 for z/OS structure.             */
                                       /* Deprecated.                         */
   struct db2_cdpg cdpg_db2;           /* DB2 UDB common structure            */
};

/******************************************************************************
** Operating System "platform" definitions.
*******************************************************************************/
#define SQLUDF_PLATFORM_UNKNOWN     0       /* Unknown platform               */
#define SQLUDF_PLATFORM_OS2         1       /* OS/2                           */
#define SQLUDF_PLATFORM_WINDOWS     3       /* Windows                        */
#define SQLUDF_PLATFORM_AIX         4       /* AIX                            */
#define SQLUDF_PLATFORM_NT          5       /* NT                             */
#define SQLUDF_PLATFORM_HP          6       /* HP                             */
#define SQLUDF_PLATFORM_SUN         7       /* Sun                            */
#define SQLUDF_PLATFORM_ZOS         8       /* Z/OS                           */
#define SQLUDF_PLATFORM_SNI         13      /* Siemens Nixdorf                */
#define SQLUDF_PLATFORM_WINDOWS95   15      /* Windows 95                     */
#define SQLUDF_PLATFORM_SCO         16      /* SCO                            */
#define SQLUDF_PLATFORM_LINUX       18      /* Linux                          */
#define SQLUDF_PLATFORM_DYNIX       19      /* DYNIX/ptx                      */
#define SQLUDF_PLATFORM_LINUX390    24      /* Linux/390                      */
#define SQLUDF_PLATFORM_LINUXZ64    25      /* LinuxZ64                       */
#define SQLUDF_PLATFORM_LINUXIA64   26      /* Linux/IA64                     */
#define SQLUDF_PLATFORM_LINUXPPC    27      /* Linux/PPC                      */
#define SQLUDF_PLATFORM_LINUXPPC64  28      /* Linux/PPC64                    */
#define SQLUDF_PLATFORM_LINUXX8664  29      /* Linux/X8664                    */
#define SQLUDF_PLATFORM_HP64        30      /* HP-PA64                        */
#define SQLUDF_PLATFORM_HPIA        31      /* HP-IA                          */
#define SQLUDF_PLATFORM_HPIA64      32      /* HP-IA64                        */
#define SQLUDF_PLATFORM_SUNX86      33      /* Sun/X86                        */
#define SQLUDF_PLATFORM_SUNX8664    34      /* Sun/X8664                      */
#define SQLUDF_PLATFORM_DARWIN      35      /* DARWIN                         */
#define SQLUDF_PLATFORM_ISERIES     400     /* AS/400                         */

#define SQLUDF_PLATFORM_AS400       SQLUDF_PLATFORM_ISERIES /* iSeries        */
#define SQLUDF_PLATFORM_MVS         SQLUDF_PLATFORM_ZOS /* z/OS               */
#define SQLUDF_PLATFORM_OS390       SQLUDF_PLATFORM_ZOS /* z/OS               */
#define SQLUDF_PLATFORM_INSPURKUXI64 SQLUDF_PLATFORM_LINUXIA64 /* InspurKUX   */

/******************************************************************************
** Structure used for: dbinfo.
** Note: This structure as provided by DB2 to the UDF
**       contains additional information which may be used to
**       enable (but not require) additional capability or
**       optimizations on the part of the external function.
**       This argument is only present if the DBINFO
**       keyword is specified in the CREATE FUNCTION
**       statement.
** 
** Database version/release.  The field has the format pppvvrrm:
**   ppp -- identifies the product name
**   vv  -- two digit rversion identifier such as '03'
**   rr  -- two digit release identifier such as '01'
**   m   -- one digit modification level identifier such as '0'
** 
** OS Platform information. Please refer to Operating System
** "platform" definitions above for its definition.
*******************************************************************************/
SQL_STRUCTURE sqludf_dbinfo
{
   unsigned short  dbnamelen;          /* database name length                */
   unsigned char   dbname[SQLUDF_MAX_IDENT_LEN]; /* database name             */
   unsigned short  authidlen;          /* authorization ID length             */
   unsigned char   authid[SQLUDF_MAX_IDENT_LEN]; /* appl authorization ID     */
   union db_cdpg   codepg;             /* database code page                  */
   unsigned short  tbschemalen;        /* table schema name length            */
   unsigned char   tbschema[SQLUDF_MAX_IDENT_LEN]; /* table schema name       */
   unsigned short  tbnamelen;          /* table name length                   */
   unsigned char   tbname[SQLUDF_MAX_IDENT_LEN]; /* table name                */
   unsigned short  colnamelen;         /* column name length                  */
   unsigned char   colname[SQLUDF_MAX_IDENT_LEN]; /* column name              */
   unsigned char   ver_rel[SQLUDF_SH_IDENT_LEN]; /* database version/release  */
   unsigned char   resd0[2];           /* alignment                           */
   sqluint32       platform;           /* platform                            */
   unsigned short  numtfcol;           /* number of entries in the TF column  */
                                       /* list array (tfcolumn, below)        */
   unsigned char   resd1[2];           /* Reserved for expansion              */
   sqluint32       procid;             /* ID of procedure being executed      */
   unsigned char   resd2[32];          /* Reserved for expansion              */
   unsigned short  *tfcolumn;          /* tfcolumn will be allocated          */
                                       /* dynamically if TF is defined;       */
                                       /* otherwise, this will be a null      */
                                       /* pointer.                            */
   char            *appl_id;           /* application identifier              */
   sqluint32       dbpartitionnum;     /* Database Partition number of where  */
                                       /* executing                           */
   unsigned short  numdbpartitions;    /* number of entries in dbpartitions   */
                                       /* field                               */
   sqluint32       *dbpartitions;      /* dbpartitions will be allocated      */
                                       /* dynamically for array of            */
                                       /* partitions in partition group when  */
                                       /* TF is processed in parallel;        */
                                       /* otherwise, this will be a null      */
                                       /* pointer                             */
   unsigned char   resd3[4];           /* Reserved for expansion              */
};

/******************************************************************************
** "Language type" definitions, for argument/result values.
** Usage note: define input and output SQL arguments as
**             pointers to these types.  For example:
**             SQLUDF_INTEGER *arg1,   or
**             SQLUDF_NULLIND *arg1_nullind
*******************************************************************************/
#define   SQLUDF_SMALLINT    short                /* for SMALLINT             */
#define   SQLUDF_INTEGER     sqlint32             /* for INTEGER              */
#ifndef NO_BIGINT
#define   SQLUDF_BIGINT      sqlint64             /* for BIGINT (64 bit)      */
#endif
#define   SQLUDF_DOUBLE      double               /* for DOUBLE PRECISION     */
#define   SQLUDF_REAL        float                /* for REAL                 */
#define   SQLUDF_CHAR        char                 /* for CHAR with/without    */
                                                  /*             FOR BIT DATA */
#define   SQLUDF_BINARY      char                 /* for BINARY               */
#define   SQLUDF_VARCHAR     char                 /* for VARCHAR              */
#define   SQLUDF_VARCHAR_FBD SQL_STRUCTURE sqludf_vc_fbd /* VARCHAR           */
                                                         /*      FOR BIT DATA */
#define   SQLUDF_VARBINARY   SQL_STRUCTURE sqludf_vc_fbd /* VARBINARY           */
#define   SQLUDF_LONG        SQL_STRUCTURE sqludf_vc_fbd /* LONG VARCHAR      */
                                                         /* w/wo FOR BIT DATA */

#ifdef SQL_WCHART_CONVERT
#define   SQLUDF_GRAPH       wchar_t              /* for GRAPHIC              */
#else
#define   SQLUDF_GRAPH       sqldbchar            /* for GRAPHIC              */
#endif

#ifdef SQL_WCHART_CONVERT
#define   SQLUDF_VARGRAPH    wchar_t              /* for VARGRAPHIC           */
#else
#define   SQLUDF_VARGRAPH    sqldbchar            /* for VARGRAPHIC           */
#endif

#define   SQLUDF_LONGVARG    SQL_STRUCTURE sqludf_vg /* for LONG VARGRAPHIC   */
#define   SQLUDF_DATE        char                 /* for DATE                 */
#define   SQLUDF_TIME        char                 /* for TIME                 */
#define   SQLUDF_STAMP       char                 /* for TIMESTAMP            */
#define   SQLUDF_CLOB        SQL_STRUCTURE sqludf_lob  /* for CLOB            */
#define   SQLUDF_BLOB        SQL_STRUCTURE sqludf_lob  /* for BLOB            */
#define   SQLUDF_DBCLOB      SQL_STRUCTURE sqludf_lobg /* for DBCLOB          */
#ifndef NO_DATALINK
#define   SQLUDF_DATALINK    SQL_STRUCTURE sqludf_datalink /* for DATALINK    */
#endif

#define   SQLUDF_NULLIND     short                /* for null indicators      */

typedef sqluint32       udf_locator;
#define   SQLUDF_LOCATOR     sqluint32            /* for LOB LOCATOR          */

/******************************************************************************
** "Language type" definitions, for scratchpad and call type
** arguments to a UDF.
** Usage note: define arguments as pointers
**             to these types.  For example:
**             SQLUDF_SCRATCHPAD  *scrpad
*******************************************************************************/
#define   SQLUDF_SCRATCHPAD  SQL_STRUCTURE sqludf_scratchpad /* for scratchpad*/
#define   SQLUDF_CALL_TYPE   enum sqludf_call_type           /* for call type */

/******************************************************************************
** "DBINFO" definitions, for dbinfo arguments to a UDF.
** Usage note: define arguments as pointer. For example:
**             SQLUDF_DBINFO  *dbinfo
*******************************************************************************/
#define   SQLUDF_DBINFO  SQL_STRUCTURE sqludf_dbinfo         /* for dbinfo */

/******************************************************************************
** Enumerated type, for call-type argument.
*******************************************************************************/
/* Contains generic internal values;  right below, we will map to             */
/* these values for scalar and table function call types.                     */
enum sqludf_call_type
{
  SQLUDF_ENUM_CALLTYPE_MINUS2 = -2,     /* Used in table func for FIRST call   */
  SQLUDF_ENUM_CALLTYPE_MINUS1 = -1,     /* Used in table func for OPEN call,   */
                                        /* and scalar func FIRST call          */
  SQLUDF_ENUM_CALLTYPE_ZERO   =  0,     /* Used in table func for FETCH call,  */
                                        /* scalar func NORMAL call.            */
  SQLUDF_ENUM_CALLTYPE_PLUS1  =  1,     /* Used in table func for CLOSE call,  */
                                        /* scalar func FINAL call.             */
  SQLUDF_ENUM_CALLTYPE_PLUS2  =  2,     /* Used in table func for FINAL call.  */
  SQLUDF_ENUM_CALLTYPE_PLUS10 = 10,   /* Used in aggregate func INITialize stage. */
  SQLUDF_ENUM_CALLTYPE_PLUS11 = 11,   /* Used in aggregate func ACCuMulate stage. */
  SQLUDF_ENUM_CALLTYPE_PLUS12 = 12,   /* Used in aggregate func MERGe stage.      */
  SQLUDF_ENUM_CALLTYPE_PLUS13 = 13,   /* Used in aggregate func TERMinate stage.  */
  SQLUDF_ENUM_CALLTYPE_PLUS14 = 14,   /* Used in aggregate func SUBTract stage.  */
  SQLUDF_ENUM_CALLTYPE_PLUS255 = 255,   /* Used in table func FINAL_CRA call   */
                                        /* and scalar func FINAL_CRA call      */
  SQLUDF_ENUM_CALLTYPE_END = 0x7FFFFFFF /* Present only to force 4byte type    */
};

/******************************************************************************
** Call type values to be used for Scalar Functions...
*******************************************************************************/
#define SQLUDF_FIRST_CALL   SQLUDF_ENUM_CALLTYPE_MINUS1  /* First call, only  */
                                  /* made if FINAL CALL spec'd on CREATE.*/
#define SQLUDF_NORMAL_CALL  SQLUDF_ENUM_CALLTYPE_ZERO    /* Fetch next row.   */
#define SQLUDF_FINAL_CALL   SQLUDF_ENUM_CALLTYPE_PLUS1   /* Final call, only  */
                                  /* made if FINAL CALL spec'd on CREATE.*/
#define SQLUDF_FINAL_CRA    SQLUDF_ENUM_CALLTYPE_PLUS255 /* Final call, only  */
                                  /* made if FINAL CALL spec'd, and after*/
                                  /* UDF abend, rollback, or commit.     */

/******************************************************************************
** Call type values to be used for Table Functions...
*******************************************************************************/
#define SQLUDF_TF_FIRST     SQLUDF_ENUM_CALLTYPE_MINUS2  /* First call, only  */
                                  /* made if FINAL CALL spec'd on CREATE.*/
#define SQLUDF_TF_OPEN      SQLUDF_ENUM_CALLTYPE_MINUS1  /* Open table.       */
#define SQLUDF_TF_FETCH     SQLUDF_ENUM_CALLTYPE_ZERO    /* Fetch next row.   */
#define SQLUDF_TF_CLOSE     SQLUDF_ENUM_CALLTYPE_PLUS1   /* Close table.      */
#define SQLUDF_TF_FINAL     SQLUDF_ENUM_CALLTYPE_PLUS2   /* Final call, only  */
                                  /* made if FINAL CALL spec'd on CREATE.*/
#define SQLUDF_TF_FINAL_CRA SQLUDF_ENUM_CALLTYPE_PLUS255 /* Final call, only  */
                                  /* made if FINAL CALL spec'd, and after*/
                                  /* UDF abend, rollback, or commit.     */

/* The following are deprecated; please use the SQLUDF_TF_* forms instead     */
#define   SQL_TF_OPEN   SQLUDF_FIRST_CALL  /* Open table  */
#define   SQL_TF_FETCH  SQLUDF_NORMAL_CALL /* Fetch next row */
#define   SQL_TF_CLOSE  SQLUDF_FINAL_CALL  /* Close table */

/******************************************************************************
** Call type values to be used for Aggregate Functions...
*******************************************************************************/
#define SQLUDF_AGG_INITIALIZE    SQLUDF_ENUM_CALLTYPE_PLUS10/* eq to aggregation INITialize stage  */
#define SQLUDF_AGG_ACCUMULATE    SQLUDF_ENUM_CALLTYPE_PLUS11/* eq to aggregation ACCuMulate stage  */
#define SQLUDF_AGG_MERGE         SQLUDF_ENUM_CALLTYPE_PLUS12/* eq to aggregation MERGe stage       */
#define SQLUDF_AGG_TERMINATE     SQLUDF_ENUM_CALLTYPE_PLUS13/* eq to aggregation TERMinate stage   */
#define SQLUDF_AGG_SUBTRACT      SQLUDF_ENUM_CALLTYPE_PLUS14/* eq to aggregation SUBTract stage    */

/******************************************************************************
** Macros, for defining the always-present (trailing) input arguments:
*******************************************************************************/

/******************************************************************************
** For the case WITHOUT scratchpad / final_call / dbinfo arguments:
** SQLUDF_TRAIL_ARGS
** defines the always present trailing arguments-- pointers to:
**               sqlstate,
**               fully qualified function name,
**               function specific name,
**               message text
** 
** Usage note: when both SCRATCHPAD and FINAL CALL are defined for the
**             function, use SQLUDF_TRAIL_ARGS_ALL immediately below.
** 
**             When SCRATCHPAD is defined and FINAL CALL is not, use
**             SQLUDF_TRAIL_ARGS with an additional argument appended
**             which is a pointer to: SQL_SCRATCHPAD
**             E.G.:  SQL_SCRATCHPAD scratch_ptr
** 
**             When DBINFO is defined, use an additional argument
**             appended at the end of the above argument list.
**             This argument is a pointer to: SQLUDF_DBINFO
*******************************************************************************/
#define SQLUDF_TRAIL_ARGS char sqludf_sqlstate[SQLUDF_SQLSTATE_LEN+1],  \
                          char sqludf_fname[SQLUDF_FQNAME_LEN+1],       \
                          char sqludf_fspecname[SQLUDF_SPECNAME_LEN+1], \
                          char sqludf_msgtext[SQLUDF_MSGTEXT_LEN+1]

/******************************************************************************
** For the case WITH scratchpad / final_call arguments:
** SQLUDF_TRAIL_ARGS_ALL
** defines the trailing arguments which are present-- pointers to:
**               sqlstate,
**               fully qualified function name,
**               function specific name,
**               message text,
**               scratchpad,
**               call type
** 
** Usage note: this can be used when
**             - SCRATCHPAD and FINAL CALL are both defined for UDF.
** 
**             But CANNOT be used for the case where SCRATCHPAD is not
**             defined and FINAL CALL is defined.  In that case the
**             call type argument immediately follows the
**             message text argument.
*******************************************************************************/
#define SQLUDF_TRAIL_ARGS_ALL char   sqludf_sqlstate[SQLUDF_SQLSTATE_LEN+1],  \
                              char   sqludf_fname[SQLUDF_FQNAME_LEN+1],       \
                              char   sqludf_fspecname[SQLUDF_SPECNAME_LEN+1], \
                              char   sqludf_msgtext[SQLUDF_MSGTEXT_LEN+1],    \
                              SQLUDF_SCRATCHPAD *sqludf_scratchpad,           \
                              SQLUDF_CALL_TYPE  *sqludf_call_type

/******************************************************************************
** Macros, for referencing the trailing input arguments.
*******************************************************************************/
/* For the always-present arguments.  Note: all are char arrays.              */
#define   SQLUDF_STATE     sqludf_sqlstate   /* SQLSTATE returned by UDF      */
#define   SQLUDF_FNAME     sqludf_fname      /* fully qual func name          */
#define   SQLUDF_FSPEC     sqludf_fspecname  /* func specific name            */
#define   SQLUDF_MSGTX     sqludf_msgtext    /* message text returned by UDF  */

/* For the scratchpad.  Note: SQLUDF_SCRAT is a pointer.                      */
#define   SQLUDF_SCRAT    (sqludf_scratchpad) /* scratchpad area              */

/* For the call type.   Note: pointer to enum type is dereferenced.           */
#define   SQLUDF_CALLT (*sqludf_call_type) /* call type (used with scr'pad)   */

/******************************************************************************
** Special sqlstate (SQLUDF_STATE) values that UDF can set.
** Usage note: the values "38xxx" and "01Hxx" are included
**             for completeness and for their educational
**             value.  You will want to define and use your
**             own constants with your own values
**             substituted for the generic "x"s.
*******************************************************************************/
#define SQLUDF_STATE_OK       "00000"  /* good return, no problem - SQLUDF    */
                                       /* STATE is init'd to this value by    */
                                       /* DB2 before calling UDF.             */
#define SQLUDF_STATE_SQLCALL  "38001"  /* error- UDF issued SQL call, DB2     */
                                       /* returned error (-487) to UDF.       */
#define SQLUDF_STATE_ERR      "38xxx"  /* error- xxx is UDF-defined, UDF      */
                                       /* should overwrite, but NOTUSE        */
                                       /* values 0xx thru 5xx, as these are   */
                                       /* reserved by IBM.                    */
#define SQLUDF_STATE_WARN     "01Hxx"  /* warning - xx is UDF-defined, UDF    */
                                       /* should overwrite, any values OK.    */

/******************************************************************************
** Macros, for testing for argument nullness:
*******************************************************************************/
/******************************************************************************
** Test for argument NULL.
** pass pointer to null indicator, returns TRUE if NULL
*******************************************************************************/
#define   SQLUDF_NULL(nullptr)      (*nullptr < 0)

/******************************************************************************
** Test for argument NOT NULL.
** pass pointer to null indicator, returns TRUE if NOT NULL
*******************************************************************************/
#define   SQLUDF_NOTNULL(nullptr)      (*nullptr >= 0)

#ifndef NO_LOB_APIs
/******************************************************************************
** Return Codes from LOB Locator APIs...
** (sqludf_length,  sqludf_substr,
**  sqludf_append,  sqludf_create_locator,
**  sqludf_free_locator )
*******************************************************************************/
#define SQLUDF_API_RC_OK           0   /* Successful execution.               */
#define SQLUDF_API_RC_NOT_UNFENCED -2  /* Attempt to use API from FENCED UDF  */
                                       /* - must be from NOT FENCED UDF.      */
#define SQLUDF_API_RC_BAD_INPUT    -3  /* Bad input passed to the API - bad   */
                                       /* pointer or length value.            */
#define SQLUDF_API_RC_INPUT_LOC    -4  /* Attempt to free input locator -     */
                                       /* UDF not permitted to free a         */
                                       /* locator used as input to it.        */

/******************************************************************************
** API definitions, for LOB Locator APIs
** used from within UDFs to manipulate LOB
** locators and the data they represent.
*******************************************************************************/

/******************************************************************************
** sqludf_length() - API to fetch the length of the LOB
** value represented by a LOB locator.
*******************************************************************************/
extern int SQL_API_FN sqludf_length(
    udf_locator        *udfloc_p,       /* in:  Pointer to user-provided LOB */
                                        /*      locator value.               */
    sqlint32           *return_len_p    /* in/out: Pointer to where the LOB  */
                                        /*      length is to be returned.    */
);

/******************************************************************************
** sqludf_substr() - API to fetch n contiguous bytes from
** some byte position within a LOB value represented by
** a LOB locator.
*******************************************************************************/
extern int SQL_API_FN sqludf_substr(
    udf_locator        *udfloc_p,       /* in:  Pointer to user-provided LOB */
                                        /*      locator value.               */
    sqlint32            start,          /* in:  Substring start value, first */
                                        /*      byte is byte 1.              */
    sqlint32            length,         /* in:  Number of bytes to be        */
                                        /*      returned.                    */
    unsigned char      *buffer_p,       /* in/out: Pointer to the buffer     */
                                        /*      into which the bytes are to  */
                                        /*      be placed.                   */
    sqlint32           *return_len_p    /* in/out: Pointer to where the      */
                                        /*      number of bytes actually     */
                                        /*      returned is to be placed     */
                                        /*      (can be smaller than length).*/
);

/******************************************************************************
** sqludf_append() - API to append n bytes to the end of
** the LOB value represented by a LOB locator.
*******************************************************************************/
extern int SQL_API_FN sqludf_append(
    udf_locator        *udfloc_p,       /* in:  Pointer to user-provided LOB */
                                        /*      locator value.               */
    unsigned char      *buffer_p,       /* in:  Pointer to the buffer con-   */
                                        /*      taining the bytes which are  */
                                        /*      to be appended.              */
    sqlint32            length,         /* in:  Number of bytes to be        */
                                        /*      appended.                    */
    sqlint32           *return_len_p    /* in/out: Pointer to where the      */
                                        /*      number of bytes actually     */
                                        /*      appended is to be placed.    */
);

/******************************************************************************
** sqludf_create_locator() - API to create a LOB locator,
** either to return to DB2 from the UDF or for internal
** purposes.
*******************************************************************************/
extern int SQL_API_FN sqludf_create_locator(
    int                 loc_type,       /* in:  Type of LOB;  can be any of  */
                                        /*      the types from sql.h repre-  */
                                        /*      senting LOBs; for example:   */
                                        /*      SQL_TYP_CLOB  for type CLOB, */
                                        /*      SQL_TYP_BLOB  for type BLOB, */
                                        /*      SQL_TYP_DBCLOB for   DBCLOB. */
    udf_locator       **loc_p           /* in/out:  Pointer to a preset      */
                                        /*      pointer to where the locator */
                                        /*      is to be returned.           */
);

/******************************************************************************
** sqludf_free_locator() - API to free a LOB locator
** previously created on this call to the UDF;
** LOB locators returned to DB2 should not be freed by
** the UDF.
*******************************************************************************/
extern int SQL_API_FN sqludf_free_locator(
    udf_locator        *udfloc_p        /* in:  Pointer to user-provided LOB */
                                        /*      locator value.               */
);

#endif                                  /* end if no LOB locator APIs  */

#if (defined(DB2OS2) || defined(DB2NT) || defined(DB2WIN))
#pragma pack()
#elif (defined(DB2MAC))
#pragma align
#elif (defined(DB2AIX) || defined(DB2SUN) && defined(__xlC__))
#pragma options align=reset
#elif (defined(DB2390))
#pragma pack(full)
#elif (defined(DB2HP) || defined(DB2SNI) || defined(DB2SUN))
#endif

#if (__OS400_TGTVRM__>=510)
#pragma datamodel(pop)
#endif

#ifdef __cplusplus 
}
#endif

#endif /* SQL_H_SQLUDF */
