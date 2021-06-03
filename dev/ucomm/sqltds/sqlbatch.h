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
        static constexpr unsigned int MAX_IMAGE_TEXT_LEN = 0x7fffffff;
        static constexpr unsigned int MAX_NTEXT_LEN = 0x7ffffffe;

    public:
        CSqlBatch(CTdsChannel& channel, bool meta = true);

#pragma pack(push,1)

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

        enum class tagFeatureID : unsigned char {
            fiSessionRecovery = 0x01, //Session recovery (connection resiliency), introduced in TDS 7.4
            fiFederatedAuthentication = 0x02 //Federated authentication, introduced in TDS 7.4
        };

        enum class tagRequestType : unsigned short {
            rtBeginTrans = 0x05,
            rtCommit = 0x07,
            rtRollback = 0x08
        };

        enum class tagIsolationLevel : unsigned short {
            ilCurrent = 0,
            ilReadUncommitted = 0x01,
            ilReadCommitted = 0x02,
            ilRepeatableRead = 0x03,
            ilSerializable = 0x04,
            ilSnapshot = 0x05
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

#pragma pack(pop)

        static_assert(sizeof (OptionalFlags1) == 1, "Wrong OptionalFlags1 size");
        static_assert(sizeof (OptionalFlags2) == 1, "Wrong OptionalFlags2 size");
        static_assert(sizeof (TypeFlags) == 1, "Wrong TypeFlags size");
        static_assert(sizeof (OptionalFlags3) == 1, "Wrong OptionalFlags3 size");
        static_assert(sizeof (FeatureExtension) == 4, "Wrong FeatureExtension size");
        static_assert(sizeof (tagRequestType) == 2, "Wrong tagRequestType size");
        static_assert(sizeof (tagIsolationLevel) == 2, "Wrong tagIsolationLevel size");

    public:
        int SendTDSMessage(const SqlLogin& rec, FeatureExtension requestedFeatures);
        int SendTDSMessage(tagRequestType rt, tagIsolationLevel il = tagIsolationLevel::ilCurrent);
        int SendTDSMessage(const char16_t *sql);
        int Prepare(const char16_t* sql, CParameterInfoArray& params, unsigned int& parameters);
        int SendTDSMessage(CDBVariantArray &vParam);

    protected:
        void Reset();
        bool ParseStream();

    private:
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
        bool ParseVariant(CDBColumnInfo *cinfo);
        bool ParseDoneInProc();
        bool ParseReturnStatus();
        bool ParseLoginAck();
        bool ParseReturnValue();
        void ParseStringChange(tagEnvchangeType type, StringEventChange& sec);
        void ParseTransChange(tagEnvchangeType type, TransChange& tc);
        static CDBString Prepare(const char16_t* sql, unsigned int& parameters, CDBString& procName, CDBString& catalogSchema);
        static int ToString(const CDBVariantArray& vData, CDBString& s, std::vector<CDBString> &vP);
        static void ToParameter(bool stored, const Collation& collation, const CDBVariant& v, const CDBString& p, SPA::CUQueue& buffer, SPA::UDB::CParameterInfo *pi = nullptr);

    private:
        std::vector<TokenInfo> m_vInfo;
        SPA::CUQueue &m_out;
        StringEventChange m_dbNameChange;
        std::vector<StringEventChange> m_vEventChange;
        CollationChange m_CollationChange;
        LoginAck m_LoginAck;
        TransChange m_tc;
        CDBColumnInfoArray m_vCol;
        unsigned int m_timeout;
        bool m_meta;
        unsigned short m_cols;
        unsigned short m_posCol;
        std::vector<tagDataType> m_vDT;
        Collation m_collation;
        std::vector<unsigned char> m_vNull;
        UINT64 m_lenLarge;
        unsigned int m_endLarge;
        std::vector<unsigned short> m_vOrder;
        DoneInProc m_dip;
        unsigned int m_rs; //ReturnStatus;
        CParameterInfoArray m_vParamInfo;
        CDBString m_sqlPrepare;
        CDBString m_procName;
        CDBString m_catalogSchema;
        unsigned short m_inputs;
        unsigned short m_outputs;

        static CDBString LibraryName; //Client library name
    };

}

#endif