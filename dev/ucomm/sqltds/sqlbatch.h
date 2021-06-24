#ifndef _U_TDS_SQL_BATCH_H_
#define _U_TDS_SQL_BATCH_H_

#include "reqbase.h"

namespace tds {

    struct SqlCredential {
        CDBString UserId;
        CDBString Password; //SecureString
    };

    struct SqlLogin {

        SqlLogin() {
            char name[128] = {0};
            ::gethostname(name, sizeof (name));
            hostName.assign(name, name + strlen(name)); //client machine name
        }
        SqlAuthenticationMethod authentication = SqlAuthenticationMethod::NotSpecified; // Authentication type
        unsigned int timeout = 30000; // login timeout
        bool userInstance = false; // user instance
        CDBString hostName; // client machine name
        CDBString userName; // user id
        CDBString password; // password
        CDBString applicationName = ApplicationName; // application name
        CDBString serverName; // server name
        CDBString language; // initial language
        CDBString database; // initial database
        CDBString attachDBFilename; // DB filename to be attached
        bool useReplication = false; // user login for replication
        CDBString newPassword; // new password for reset password
        bool useSSPI = false; // use integrated security
        unsigned int packetSize = DEFAULT_PACKET_SIZE; // packet size
        bool readOnlyIntent = false; // read-only intent
        SqlCredential credential; // user id and password in SecureString
        CDBString newSecurePassword;
    };

    class CSqlBatch : public CReqBase {
    private:
        static const unsigned int YUKON_LOG_REC_FIXED_LEN = 0x5e;
        SPA::CScopeUQueue m_sbOut;

        static constexpr unsigned short INVALID_COL = (~0);
        static constexpr unsigned int UINT_NULL_LEN = (~0);
        static constexpr unsigned short USHORT_NULL_LEN = (~0);
        static constexpr unsigned short VAR_MAX = (~0);
        static constexpr UINT64 UNKNOWN_XML_LEN = 0xfffffffffffffffe;
        static constexpr UINT64 BLOB_NULL_LEN = (~0);
        static constexpr unsigned int MAX_IMAGE_TEXT_LEN = 0x7fffffff;
        static constexpr unsigned int MAX_NTEXT_LEN = 0x7ffffffe;
        static constexpr unsigned int PLP_TERMINATOR = 0;
        static constexpr unsigned short PACKET_DATA_SIZE = DEFAULT_PACKET_SIZE - sizeof (PacketHeader);

    public:
        CSqlBatch(CTdsChannel& channel, bool meta = true);

#pragma pack(push,1)

    public:

        struct FeatureExtension {

            FeatureExtension() {
                ::memset(this, 0, sizeof (FeatureExtension));
            }
            unsigned int SessionRecovery : 1;
            unsigned int FedAuth : 2;
            unsigned int Tce : 1;
            unsigned int GlobalTransactions : 4;
            unsigned int AzureSQLSupport : 1;
            unsigned int DataClassification : 1;
            unsigned int UTF8Support : 1;
            unsigned int SQLDNSCaching : 1;

            unsigned int GetValue() {
                return *((unsigned int*) this);
            }
        };

        enum class tagIsolationLevel : unsigned short {
            ilCurrent = 0,
            ilReadUncommitted = 0x01,
            ilReadCommitted = 0x02,
            ilRepeatableRead = 0x03,
            ilSerializable = 0x04,
            ilSnapshot = 0x05
        };

        enum class tagRequestType : unsigned short {
            rtBeginTrans = 0x05,
            rtCommit = 0x07,
            rtRollback = 0x08
        };

    private:

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

        static constexpr int TDS_JDN_OFFSET_1_1_1 = 1721426;
        static constexpr int TDS_JDN_OFFSET_1900_1_1 = 693595;
        static constexpr unsigned int DATETIME_HOUR_TICKET = 60 * 60 * 300;
        static constexpr unsigned int DATETIME_MINUTE_TICKET = 60 * 300;
        static constexpr unsigned int DATETIME_SECOND_TICKET = 300;

        struct TransactionDescriptor {

            TransactionDescriptor(SPA::UINT64 td) : TransDescriptor(td) {
            }
            unsigned int TotalLength = 22;
            unsigned int Length = 18;
            unsigned short Type = 2;
            SPA::UINT64 TransDescriptor;
            unsigned int RequestCount = 1;
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

        struct LoginAck {
            unsigned int Tds_Version = 0;
            CDBString ServerName;
            unsigned int ServerVersion = 0;
        };

        struct OptionalFlags1 {

            OptionalFlags1() {
                ::memset(this, 0, sizeof (OptionalFlags1));
            }
            unsigned char fByteOrder : 1;
            unsigned char fChar : 1;
            unsigned char fFloat : 2;
            unsigned char fDumpLoad : 1;
            unsigned char fUseDB : 1;
            unsigned char fDatabase : 1;
            unsigned char fSetLang : 1;
        };

        struct OptionalFlags2 {

            OptionalFlags2() {
                ::memset(this, 0, sizeof (OptionalFlags2));
            }
            unsigned char fLanguage : 1;
            unsigned char fODBC : 1;
            unsigned char fTransBoundary : 1; //removed in TDS 7.2
            unsigned char fCacheConnect : 1; //removed in TDS 7.2
            unsigned char fUserType : 3;
            unsigned char fIntSecurity : 1;
        };

        struct TypeFlags {

            TypeFlags() {
                ::memset(this, 0, sizeof (TypeFlags));
            }
            unsigned char fSQLType : 4;
            unsigned char fOLEDB : 1; //introduced in TDS 7.2
            unsigned char fReadOnlyIntent : 1; //introduced in TDS 7.4
            unsigned char fReserved : 2;
        };

        struct OptionalFlags3 {

            OptionalFlags3() {
                ::memset(this, 0, sizeof (OptionalFlags3));
            }
            unsigned char fChangePassword : 1; //introduced in TDS 7.2
            unsigned char fUserInstance : 1; //introduced in TDS 7.2
            unsigned char fSendYukonBinaryXML : 1; //introduced in TDS 7.2
            unsigned char fUnknownCollationHandling : 1; //introduced in TDS 7.3
            unsigned char fExtension : 1; //introduced in TDS 7.4
            unsigned char fReserved : 3;
        };

        enum class tagFeatureID : unsigned char {
            fiSessionRecovery = 0x01, //Session recovery (connection resiliency), introduced in TDS 7.4
            fiFederatedAuthentication = 0x02 //Federated authentication, introduced in TDS 7.4
        };

        struct MetaInfoHeader {
            unsigned int UserType = 0;
            ColFlag Flags;
            tagDataType SqlType = tagDataType::SQL_NULL;
        };

        struct RPCOption {

            RPCOption() {
                ::memset(this, 0, sizeof (RPCOption));
            }
            unsigned char fWithRecomp : 1;
            unsigned char fNoMetaData : 1;
            unsigned char fReuseMetaData : 1;
        };

        struct RPCStatus {

            RPCStatus() {
                ::memset(this, 0, sizeof (RPCStatus));
            }
            unsigned char fByRefValue : 1;
            unsigned char fDefaultValue : 1;
            unsigned char fEncrypted : 1;
        };

        struct PLPHeader {

            PLPHeader(SPA::UINT64 h = 0, unsigned int sub_len = 0) : HEADER(h), SUB_LEN(sub_len) {
            }
            SPA::UINT64 HEADER;
            unsigned int SUB_LEN;
        };

#pragma pack(pop)

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

        static_assert(sizeof (OptionalFlags1) == 1, "Wrong OptionalFlags1 size");
        static_assert(sizeof (OptionalFlags2) == 1, "Wrong OptionalFlags2 size");
        static_assert(sizeof (TypeFlags) == 1, "Wrong TypeFlags size");
        static_assert(sizeof (OptionalFlags3) == 1, "Wrong OptionalFlags3 size");
        static_assert(sizeof (FeatureExtension) == 4, "Wrong FeatureExtension size");
        static_assert(sizeof (tagRequestType) == 2, "Wrong tagRequestType size");
        static_assert(sizeof (tagIsolationLevel) == 2, "Wrong tagIsolationLevel size");
        static_assert(sizeof (Collation) == 5, "Wrong Collation size");
        static_assert(sizeof (ColFlag) == 2, "Wrong ColFlag size");
        static_assert(sizeof (TransactionDescriptor) == 22, "Wrong TransactionDescriptor size");

    public:
        /**
         * Send TDS server a login message for login authentication
         * @param rec A structure containing SQL server login info
         * @param requestedFeatures requested extended feature flags
         * @return one of programming error codes (-1981 to -1996) or a positive socket error code
         */
        int SendTDSMessage(const SqlLogin& rec, FeatureExtension requestedFeatures, bool sync = true);

        /**
         * Send TDS server a transaction manager request for starting or ending transaction
         * @param rt one of request types, rtBeginTrans, rtCommit and rtRollback
         * @param il an isolation level
         * @return one of programming error codes (-1981 to -1996) or a positive socket error code
         */
        int SendTDSMessage(tagRequestType rt, tagIsolationLevel il = tagIsolationLevel::ilCurrent, bool sync = true);

        /**
         * Send TDS server a SQL batch message for processing a batch of SQL statements
         * @param sql a batch of SQL statements
         * @param chars the number of SQL string characters
         * @return a positive socket error code
         */
        int SendTDSMessage(const char16_t *sql, unsigned int chars = SPA::UQUEUE_NULL_LENGTH, bool sync = true);

        /**
         * Prepare a parameterized SQL statement which may contain multiple sub statements
         * @param sql a parameterized SQL statement
         * @param params an array of parameter information
         * @param parameters the number of input and output parameters, low part for inputs and high part for outputs
         * @return one of programming error codes (-1981 to -1996)
         */
        int Prepare(const char16_t* sql, SPA::UDB::CParameterInfoArray& params, unsigned int& parameters);

        /**
         * Send TDS server a SQL remote procedure call message for processing a batch of SQL parameterized statements
         * @param pVt a pointer to an array of CDBVariant
         * @param count the number of CDBVariants
         * @return one of programming error codes (-1981 to -1996) or a positive socket error code
         */
        int SendTDSMessage(const SPA::UDB::CDBVariant *pVt, unsigned int count, bool sync = true);

        /**
         * Query the number of records affected
         * @return the number of records affected
         */
        SPA::UINT64 GetAffected() const;

        int Cancel(bool completed = true, bool sync = true);

    protected:
        void Reset();

    private:
        bool ParseStream();
        bool ParseInfo();
        bool ParseCollation(CollationChange& cc);
        bool ParseMeta();
        bool ParseEventChange();
        bool ParseRow();
        bool ParseNBCRow();
        bool ParseDone();
        bool ParseData(tagDataType dt, bool max);
        bool ParseOrder();
        bool ParseData(tagDataType dt, unsigned char bytes, unsigned char scale);
        bool ParseVariant(SPA::UDB::CDBColumnInfo *cinfo);
        bool ParseDoneInProc();
        bool ParseReturnStatus();
        bool ParseLoginAck();
        bool ParseReturnValue();
        void ParseStringChange(tagEnvchangeType type, StringEventChange& sec);
        void ParseTransChange(tagEnvchangeType type, TransChange& tc);
        bool ConvertTo(const CDBString &pn);
        bool PopPLP(VARTYPE vt);
        int ToString(const SPA::UDB::CDBVariant* pVt, unsigned int count, CDBString& s) const;
        const SPA::UDB::CParameterInfo* FindParameterInfo(const CDBString& pn) const;
        int SavePLP(const unsigned char* buffer, unsigned int bytes, SPA::CUQueue& q, unsigned char& packet_id);
        int SaveParameter(unsigned char &packet_id, const SPA::UDB::CDBVariant& v, SPA::CUQueue& buffer, const SPA::UDB::CParameterInfo* pi);
        int SendARpcPacket(SPA::CUQueue& buffer, unsigned char& packet_id);
        CDBString Prepare(const char16_t* sql, unsigned int& parameters);
        static inline VARTYPE GetVarType(tagDataType dt, unsigned char money_bytes);
        static inline CDBString GetSqlDeclaredType(tagDataType dt, unsigned char money_bytes);
        static std::vector<unsigned char> ObfuscatePassword(const CDBString& password);
        static unsigned char* ObfuscatePassword(unsigned char* password, unsigned int bytes);
        static int ToTdsJDN(int year, int month, int month_day);
        static void ToDate(Date date, int& year, int& month, int& month_day);
        static void ToDateTime(DateTime dt, int& year, int& month, int& month_day, int& hour, int& minute, int& second, unsigned int& us);
        static void ToTime(SPA::UINT64 time, unsigned char scale, int& hour, int& minute, int& second, unsigned int& us);
        static void ToDateTime(Date dt, SPA::UINT64 time, unsigned char scale, int& year, int& month, int& month_day, int& hour, int& minute, int& second, unsigned int& us);
        static unsigned int ToUDBFlags(ColFlag cf);

    private:
        std::vector<TokenInfo> m_vInfo;
        SPA::CUQueue &m_out;
        StringEventChange m_dbNameChange;
        std::vector<StringEventChange> m_vEventChange;
        CollationChange m_CollationChange;
        LoginAck m_LoginAck;
        TransChange m_tc;
        SPA::UDB::CDBColumnInfoArray m_vCol;
        unsigned int m_timeout;
        bool m_meta;
        unsigned short m_cols;
        unsigned short m_posCol;
        std::vector<tagDataType> m_vDT;
        std::vector<unsigned char> m_vNull;
        UINT64 m_lenLarge;
        unsigned int m_endLarge;
        std::vector<unsigned short> m_vOrder;
        DoneInProc m_dip;
        unsigned int m_rs; //ReturnStatus;
        SPA::UDB::CParameterInfoArray m_vParamInfo;
        CDBString m_sqlPrepare;
        unsigned short m_inputs;
        unsigned short m_outputs;
        UINT64 m_affects;
        static CDBString LibraryName; //Client library name
    };

}

#endif
