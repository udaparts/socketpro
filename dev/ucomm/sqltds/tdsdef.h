#ifndef _H_SQL_TDS_DEFINES_H_
#define _H_SQL_TDS_DEFINES_H_

#include "../include/udatabase.h"

using namespace SPA::UDB;

namespace tds {
    static const unsigned int CLIENT_EXE_VERSION = 0x01000000;
    static const unsigned short BUILD_VERSION = 0x0001; //Little Endian

    static const unsigned int CLIENT_DLL_VERSION = 0x01000001; //1.0.0.1
    static const unsigned int TDS_VERSION = 0x74000004;

    typedef std::u16string CDBString;
    static const CDBString ApplicationName(u"UDAParts Core MSSQL Data Provider");

    static std::vector<unsigned char> TDS_NIC_ADDRESS({0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc});

    enum class tagPacketType : unsigned char {
        ptInitial = 0,
        ptBatch = 1, //SQL batch
        ptPre7Login = 2, //Pre-TDS7 Login
        ptRpc = 3,
        ptResponse = 4, //Tabular result, server-only response type
        ptAttention = 6, //Attention signal
        ptBulk = 7, //Bulk load data
        ptFederated = 8, //Federated Authentication Token
        ptTransaction = 14, //Transaction manager request
        ptLogin7 = 16, //TDS7 Login
        ptSspi = 17,
        ptPrelogin = 18, //Pre-Login
        ptFedAuthInfo = 238
    };

    enum class tagPacketStatus : unsigned char {
        psNormal = 0,
        psEOM = 1,
        psIgnore = 2, //client ==> server + psEOM
        psResetConnection = 8, //client ==> server
        psResetConnectionSkipTran = 16, //client ==> server
    };

    enum class tagTokenType : unsigned char {
        ttZero = 0,
        ttOFFSET = 0x78,
        ttRETURNSTATUS = 0x79,
        ttCOLMETADATA = 0x81,
        ttALTMETADATA = 0x88,
        ttDATACLASSIFICATION = 0xa3,
        ttTABNAME = 0xa4,
        ttCOLINFO = 0xa5,
        ttORDER = 0xa9,
        ttTDS_ERROR = 0xaa,
        ttINFO = 0xab,
        ttRETURNVALUE = 0xac,
        ttLOGINACK = 0xad,
        ttFEATUREEXTACK = 0xae,
        ttROW = 0xd1,
        ttNBCROW = 0xd2,
        ttALTROW = 0xd3,
        ttENVCHANGE = 0xe3,
        ttSESSIONSTATE = 0xe4,
        ttSSPI = 0xed,
        ttFEDAUTHINFO = 0xee,
        ttDONE = 0xfd,
        ttDONEPROC = 0xfe,
        ttDONEINPROC = 0xff
    };

    enum class tagDataType : unsigned char {
        SQL_NULL = 0x1F,
        IMAGE = 0x22,
        TEXT = 0x23,
        UNIQUEIDENTIFIER = 0x24,
        INTN = 0x26, //INTNTYPE
        DATEN = 0x28,
        TIMEN = 0x29,
        DATETIME2N = 0x2A,
        DATETIMEOFFSETN = 0x2B, //timezone offset -840 ~ 840
        TINYINT = 0x30, //INT1
        BIT = 0x32,
        SMALLINT = 0x34,
        INT = 0x38,
        DATETIM4 = 0x3A, //SmallDateTime
        REAL = 0x3B, //Real, float
        MONEY = 0x3C, //8 bytes
        DATETIME = 0x3D,
        FLOAT = 0x3E, //Float, double
        SQL_VARIANT = 0x62, //max length 8009 (8000 for strings)
        NTEXT = 0x63,
        BITN = 0x68,
        DECIMAL = 0x6A,
        NUMERIC = 0x6C,
        FLTN = 0x6D,
        MONEYN = 0x6E,
        DATETIMN = 0x6F, //smalldatetime
        SMALLMONEY = 0x7A, //MONEY4, SmallMoney, 4 bytes
        BIGINT = 0x7F,
        VARBINARY = 0xA5,
        VARCHAR = 0xA7,
        BINARY = 0xAD,
        CHAR = 0xAF,
        NVARCHAR = 0xE7,
        NCHAR = 0xEF,
        UDT = 0xF0,
        XML = 0xF1,
    };

    enum class tagEnvchangeType : unsigned char {
        database = 1,
        language,
        charset,
        packet_size,
        unicode_data_sort_local_id,
        unicode_data_sort_comparison_flags,
        collation,
        begin_trans,
        commit_trans,
        rollback_trans,
        enlist_dist_trans,
        defect_trans,
        log_shipping,
        promote_trans = 15,
        trans_man_address,
        trans_ended,
        reset_completion_acknowledgement,
        user_instance_started,
        routing
    };

    enum class SqlAuthenticationMethod {
        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/NotSpecified/*'/>
        NotSpecified = 0,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/SqlPassword/*'/>
        SqlPassword,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryPassword/*'/>
        ActiveDirectoryPassword,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryIntegrated/*'/>
        ActiveDirectoryIntegrated,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryInteractive/*'/>
        ActiveDirectoryInteractive,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryServicePrincipal/*'/>
        ActiveDirectoryServicePrincipal,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryDeviceCodeFlow/*'/>
        ActiveDirectoryDeviceCodeFlow,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryManagedIdentity/*'/>
        ActiveDirectoryManagedIdentity,

        /// <include file='../../../../../../../doc/snippets/Microsoft.Data.SqlClient/SqlAuthenticationMethod.xml' path='docs/members[@name="SqlAuthenticationMethod"]/ActiveDirectoryMSI/*'/>
        ActiveDirectoryMSI
    };

    typedef unsigned short Packet_Length; //big-endian 512 - 32,767
    typedef unsigned short SPID; //big-endian
    typedef unsigned char Token;


    static const Token TOKEN_TERMINATOR = 0xff;
    static const unsigned short DEFAULT_PACKET_SIZE = 2920;

    static inline unsigned short ChangeEndian(unsigned short s) {
        return ((s & 0xff) << 8) + (s >> 8);
    }

    static inline unsigned int ChangeEndian(unsigned int s) {
        unsigned char *p = (unsigned char *) &s;
        unsigned char b = p[0];
        p[0] = p[3];
        p[3] = b;
        b = p[1];
        p[1] = p[2];
        p[2] = b;
        return s;
    }

    static inline VARTYPE GetVarType(tagDataType dt, unsigned char money_bytes) {
        switch (dt) {
            case tagDataType::IMAGE:
                return VT_ARRAY | VT_UI1;
            case tagDataType::TEXT:
                return VT_ARRAY | VT_I1;
            case tagDataType::UNIQUEIDENTIFIER:
                return VT_CLSID;
            case tagDataType::INTN:
            case tagDataType::TINYINT:
                return VT_UI1;
            case tagDataType::BIT:
                return VT_BOOL;
            case tagDataType::SMALLINT:
                return VT_I2;
            case tagDataType::INT:
                return VT_I4;
            case tagDataType::REAL:
                return VT_R4;
            case tagDataType::MONEYN:
                return (money_bytes > 4) ? VT_I8 : VT_I4;
            case tagDataType::MONEY:
                return VT_I8;
            case tagDataType::DATETIMEOFFSETN:
                return VT_ARRAY | VT_I1;
            case tagDataType::TIMEN:
            case tagDataType::DATEN:
            case tagDataType::DATETIMN: //smalldatetime
            case tagDataType::DATETIME:
            case tagDataType::DATETIME2N: //datetime2
                return VT_DATE;
            case tagDataType::FLOAT:
                return VT_R8;
            case tagDataType::SQL_VARIANT:
                return VT_VARIANT;
            case tagDataType::NTEXT:
                return VT_BSTR;
            case tagDataType::DECIMAL:
            case tagDataType::NUMERIC:
                return VT_DECIMAL;
            case tagDataType::SMALLMONEY:
                return VT_I4;
            case tagDataType::BIGINT:
                return VT_I8;
            case tagDataType::CHAR:
            case tagDataType::VARCHAR:
                return (VT_ARRAY | VT_I1);
            case tagDataType::VARBINARY:
            case tagDataType::BINARY:
                return (VT_ARRAY | VT_UI1);
            case tagDataType::NVARCHAR:
            case tagDataType::NCHAR:
                return VT_BSTR;
            case tagDataType::UDT:
                return (VT_ARRAY | VT_UI1);
            case tagDataType::XML:
                return VT_BSTR;
            default:
                assert(false);
                break;
        }
        return VT_VARIANT;
    }

    static inline CDBString GetSqlDeclaredType(tagDataType dt, unsigned char money_bytes) {
        switch (dt) {
            case tagDataType::IMAGE:
                return u"image";
            case tagDataType::TEXT:
                return u"text";
            case tagDataType::UNIQUEIDENTIFIER:
                return u"uniqueidentifier";
            case tagDataType::INTN:
            case tagDataType::TINYINT:
                return u"tinyint";
            case tagDataType::BIT:
                return u"bit";
            case tagDataType::SMALLINT:
                return u"smallint";
            case tagDataType::INT:
                return u"int";
            case tagDataType::REAL:
                return u"real";
            case tagDataType::MONEYN:
                return ((money_bytes > 4) ? u"money" : u"smallmoney");
            case tagDataType::MONEY:
                return u"money";
            case tagDataType::DATETIMEOFFSETN:
                return u"datetimeoffset";
            case tagDataType::TIMEN:
                return u"time";
            case tagDataType::DATEN:
                return u"date";
            case tagDataType::DATETIMN:
                return ((money_bytes > 4) ? u"datetime" : u"smalldatetime");
            case tagDataType::DATETIME:
                return u"datetime";
            case tagDataType::DATETIME2N: //datetime2
                return u"datetime2";
            case tagDataType::FLOAT:
                return u"float";
            case tagDataType::SQL_VARIANT:
                return u"sql_variant";
            case tagDataType::NTEXT:
                return u"ntext";
            case tagDataType::DECIMAL:
                return u"decimal";
            case tagDataType::NUMERIC:
                return u"numeric";
            case tagDataType::SMALLMONEY:
                return u"smallmoney";
            case tagDataType::BIGINT:
                return u"bigint";
            case tagDataType::CHAR:
                return u"char";
            case tagDataType::VARCHAR:
                return u"varchar";
            case tagDataType::VARBINARY:
                return u"varbinary";
            case tagDataType::BINARY:
                return u"binary";
            case tagDataType::NVARCHAR:
                return u"nvarchar";
            case tagDataType::NCHAR:
                return u"nchar";
            case tagDataType::UDT:
                return u"udt";
            case tagDataType::XML:
                return u"xml";
            default:
                assert(false);
                break;
        }
        return u"sql_variant";
    }

    static inline int ChangeEndian(int s) {
        unsigned char *p = (unsigned char *) &s;
        unsigned char b = p[0];
        p[0] = p[3];
        p[3] = b;
        b = p[1];
        p[1] = p[2];
        p[2] = b;
        return s;
    }

    /*
    static inline tagTokenType GetTokenType(Token token) {
            token &= 12;
            switch (token)
            {
            case 0:
                    //COLMETADATA and ALTMETADATA both use a 2-byte count
                    return tagTokenType::ttVariableCount;
            case 4:
                    return tagTokenType::ttVariableLength;
            case 8:
                    //no length
                    return tagTokenType::ttZero;
            case 12:
                    // followed by 1, 2, 4, or 8 bytes of data
                    return tagTokenType::ttFixedLength;
            default:
                    break;
            }
            return tagTokenType::ttUnknown;
    }
     */
    static inline unsigned char GetFixLen(Token token) {
        assert((token & 12) == 12);
        token <<= 4;
        token &= 3;
        switch (token) {
            case 0:
                return 1;
            case 1:
                return 4;
            case 2:
                return 2;
            case 3:
                return 8;
            default:
                assert(false); //shouldn't come here
                break;
        }
        return 255;
    }

    static SPID GetSPID() {
#ifdef WIN32_64
        UTHREAD_ID tid = ::GetCurrentThreadId();
#else
        UTHREAD_ID tid = pthread_self();
#endif
        return ChangeEndian((SPID) tid);
    }

    static unsigned int GetThreadId() {
#ifdef WIN32_64
        UTHREAD_ID tid = ::GetCurrentThreadId();
#else
        UTHREAD_ID tid = pthread_self();
#endif
        return (unsigned int) tid;
    }

#pragma pack(push,1)

    struct PacketHeader {

        PacketHeader(tagPacketType type, unsigned char packetId) : Type(type), PacketID(packetId) {
        }
        tagPacketType Type;
        tagPacketStatus Status = tagPacketStatus::psEOM;
        Packet_Length Length = 0;
        SPID Spid = 0;
        unsigned char PacketID;
        unsigned char Window = 0; //ignored
    };

    enum tagDoneStatus : unsigned short {
        dsFinal = 0,
        dsMore = 0x01,
        dsError = 0x02,
        dsInTrans = 0x04,
        dsCount = 0x10,
        dsAttention = 0x20,
        dsSrvError = 0x100,
        dsInitial = 0xffff
    };

    struct TokenDone {
        tagDoneStatus Status = tagDoneStatus::dsInitial;
        unsigned short Operation = 0;
        UINT64 RowCount = 0;
    };

    typedef TokenDone DoneInProc;

    struct TransactionDescriptor {

        TransactionDescriptor(SPA::UINT64 td) : TransDescriptor(td) {
        }
        unsigned int TotalLength = 22;
        unsigned int Length = 18;
        unsigned short Type = 2;
        SPA::UINT64 TransDescriptor;
        unsigned int RequestCount = 1;
    };

    struct CollationFlag {

        CollationFlag() {
            ::memset(this, 0, sizeof (CollationFlag));
        }
        unsigned short Reserved : 4;
        unsigned short fIgnoreCase : 1;
        unsigned short fIgnoreAccent : 1;
        unsigned short fIgnoreWidth : 1;
        unsigned short fIgnoreKana : 1;
        unsigned short fBinary : 1;
        unsigned short fBinary2 : 1;
        unsigned short fUTF8 : 1;
        unsigned short fReserved : 1;
        unsigned short Version : 4;

        unsigned short GetValue() const {
            return *(unsigned short*) this;
        }
    };

    struct Collation {
        unsigned short CodePage = 0; //LCID
        CollationFlag Flags;
        unsigned char CharsetId = 0;

        inline bool operator==(const Collation & c) const noexcept {
            return (!::memcmp(this, &c, sizeof (c)));
        }

        inline bool operator!=(const Collation & c) const noexcept {
            return ::memcmp(this, &c, sizeof (c));
        }

        CDBString GetString() const {
            char str[16];
#ifdef WIN32_64
            sprintf_s(str, "%x|%x|%x", CodePage, Flags.GetValue(), CharsetId);
#else
            sprintf(str, "%x|%x|%x", CodePage, Flags.GetValue(), CharsetId);
#endif
            size_t len = ::strlen(str);
            return CDBString(str, str + len);
        }
    };

    struct ColFlag {

        ColFlag() {
            ::memset(this, 0, sizeof (ColFlag));
        }
        unsigned short Nullable : 1;
        unsigned short CaseSensitivity : 1;
        unsigned short Updateable : 2;
        unsigned short Identity : 1;
        unsigned short Computed : 1;
        unsigned short ReservedODBC : 2;
        unsigned short FixedLenCLRType : 1;
        unsigned short SparseColumnSet : 1;
        unsigned short Encrypted : 1;
        unsigned short Hidden : 1;
        unsigned short Key : 1;
        unsigned short NullableUnknown : 1;
    };

    struct SmallDateTime {
        //days since January 1, 1900
        unsigned short Date;
        unsigned short Minute;
    };

    struct DateTime {
        //days January 1, 1900
        int Day;
        unsigned int SecCount; //300 counts per second
    };

    //days since January 1, year 1

    struct Date {
        unsigned short Low;
        char High;
    };

#pragma pack(pop)
    static_assert(sizeof (PacketHeader) == 8, "Wrong PacketHeader size");
    static_assert(sizeof (TokenDone) == 12, "Wrong TokenDone size");
    static_assert(sizeof (Collation) == 5, "Wrong Collation size");
    static_assert(sizeof (ColFlag) == 2, "Wrong ColFlag size");
    static_assert(sizeof (TransactionDescriptor) == 22, "Wrong TransactionDescriptor size");

    static unsigned int ToUDBFlags(ColFlag cf) {
        unsigned int flags = 0;
        if (cf.CaseSensitivity) {
            flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
        }
        if (!cf.Nullable) {
            flags |= CDBColumnInfo::FLAG_NOT_NULL;
        }
        if (cf.Identity) {
            flags |= CDBColumnInfo::FLAG_NOT_NULL;
            flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
            flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
            flags |= CDBColumnInfo::FLAG_UNIQUE;
        }
        if (cf.Updateable == 0) {
            flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
        }
        return flags;
    }

    struct StringEventChange {
        tagEnvchangeType Type;
        CDBString NewValue;
        CDBString OldValue;
    };

    struct TransChange {
        tagEnvchangeType Type;
        UINT64 NewValue = 0;
        UINT64 OldValue = 0;
    };

    struct CollationChange {
        Collation NewValue;
        Collation OldValue;
    };

    struct TokenInfo {
        unsigned int SQLErrorNumber = 0;
        unsigned char State = 0;
        unsigned char Class = 0;
        CDBString ErrorMessage;
        CDBString ServerName;
        unsigned char ProcessNameLength = 0;
        CDBString ProcName;
        unsigned int LineNumber = 0;

        void Reset() {
            SQLErrorNumber = 0;
            State = 0;
            Class = 0;
            ErrorMessage.clear();
            ProcessNameLength = 0;
            ProcName.clear();
            LineNumber = 0;
        }
    };

    static constexpr int TDS_JDN_OFFSET_1_1_1 = 1721426;
    static constexpr int TDS_JDN_OFFSET_1900_1_1 = 693595;
    static constexpr unsigned int DATETIME_HOUR_TICKET = 60 * 60 * 300;
    static constexpr unsigned int DATETIME_MINUTE_TICKET = 60 * 300;
    static constexpr unsigned int DATETIME_SECOND_TICKET = 300;

    //https://en.wikipedia.org/wiki/Julian_day#Gregorian_calendar_from_Julian_day_number

    static int ToTdsJDN(int year, int month, int month_day) {
        return (1461 * (year + 4800 + (month - 14) / 12)) / 4 + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4 + month_day - 32075 - TDS_JDN_OFFSET_1_1_1;
    }

    static void ToDate(Date date, int &year, int &month, int &month_day) {
        int J = date.High;
        J <<= 16;
        J += date.Low;
        if (!J) {
            year = 0;
            month = 0;
            month_day = 0;
            return;
        }

        J += TDS_JDN_OFFSET_1_1_1; //offset 01/01/01

        int f = J + 1401 + (((4 * J + 274277) / 146097) * 3) / 4 - 38;
        int e = 4 * f + 3;
        int g = (e % 1461) / 4;
        int h = 5 * g + 2;
        month_day = (h % 153) / 5 + 1;
        month = ((h / 153 + 2) % 12) + 1;
        year = (e / 1461) - 4716 + (14 - month) / 12;
    }

    static void ToDateTime(DateTime dt, int &year, int &month, int &month_day, int &hour, int &minute, int &second, unsigned int &us) {
        int dmy = dt.Day + TDS_JDN_OFFSET_1900_1_1;
        Date date;
        date.Low = (dmy & 0xffff);
        date.High = (dmy >> 16) & 0xff;
        ToDate(date, year, month, month_day);

        hour = dt.SecCount / DATETIME_HOUR_TICKET;
        unsigned int remain = (dt.SecCount % DATETIME_HOUR_TICKET);

        minute = remain / DATETIME_MINUTE_TICKET;
        remain = (remain % DATETIME_MINUTE_TICKET);

        second = remain / DATETIME_SECOND_TICKET;
        us = (remain % DATETIME_SECOND_TICKET);
        us *= 10;
        double d = us;
        d = d / 3 + 0.5;
        us = (unsigned int) d;
        us *= 1000;
    }

    static void ToTime(SPA::UINT64 time, unsigned char scale, int &hour, int &minute, int &second, unsigned int &us) {
        assert(scale <= 7);
        unsigned int p = (unsigned int) pow(10, scale);
        unsigned int fraction = (unsigned int) (time % p);
        unsigned int day_seconds = (unsigned int) (time / p);
        hour = (int) (day_seconds / 3600);
        day_seconds = (day_seconds % 3600);
        minute = (int) (day_seconds / 60);
        second = (day_seconds % 60);
        double d = fraction / pow(10, (char) scale - 6) + 0.5;
        us = (unsigned int) d;
    }

    static void ToDateTime(Date dt, SPA::UINT64 time, unsigned char scale, int &year, int &month, int &month_day, int &hour, int &minute, int &second, unsigned int &us) {
        ToDate(dt, year, month, month_day);
        ToTime(time, scale, hour, minute, second, us);
    }

    struct ISerialize {
        virtual bool SaveTo(SPA::CUQueue &buff) = 0;
    };

    struct IDeserialize {
        virtual bool LoadFrom(SPA::CUQueue &buff) = 0;
    };

    // Obfuscate password to be sent to SQL Server
    // Blurb from the TDS spec at https://msdn.microsoft.com/en-us/library/dd304523.aspx
    // "Before submitting a password from the client to the server, for every byte in the password buffer 
    // starting with the position pointed to by IbPassword, the client SHOULD first swap the four high bits 
    // with the four low bits and then do a bit-XOR with 0xA5 (10100101). After reading a submitted password, 
    // for every byte in the password buffer starting with the position pointed to by IbPassword, the server SHOULD 
    // first do a bit-XOR with 0xA5 (10100101) and then swap the four high bits with the four low bits."
    // The password exchange during Login phase happens over a secure channel i.e. SSL/TLS 
    // Note: The same logic is used in SNIPacketSetData (SniManagedWrapper) to encrypt passwords stored in SecureString
    //       If this logic changed, SNIPacketSetData needs to be changed as well

    static std::vector<unsigned char> ObfuscatePassword(const CDBString &password) {
#if 1
        std::vector<unsigned char> v(password.size() << 1);
        unsigned char *bObfuscated = &v.front();
        unsigned char bLo, bHi;
        for (size_t n = 0, len = password.size(); n < len; ++n) {
            char16_t c16 = password[n];
            bLo = (unsigned char) (c16 & 0xff);
            bHi = (unsigned char) ((c16 >> 8) & 0xff);
            bObfuscated[n << 1] = (unsigned char) ((((bLo & 0x0f) << 4) | (bLo >> 4)) ^ 0xa5);
            bObfuscated[(n << 1) + 1] = (unsigned char) ((((bHi & 0x0f) << 4) | (bHi >> 4)) ^ 0xa5);
        }
#else
        const unsigned char *start = (const unsigned char *) password.data();
        std::vector<unsigned char> v(start, start + (password.size() << 1));
#endif
        return v;
    }

    static unsigned char* ObfuscatePassword(unsigned char* password, unsigned int bytes) {
        unsigned char bLo, bHi;
        for (unsigned int i = 0; i < bytes; i++) {
            bLo = (password[i] & 0x0f);
            bHi = (password[i] & 0xf0);
            password[i] = (((bHi >> 4) | (bLo << 4)) ^ 0xa5);
        }
        return password;
    }
};


#endif
