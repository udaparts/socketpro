#ifndef _U_TDS_LOGIN7_H_
#define _U_TDS_LOGIN7_H_

#include "reqbase.h"
#include "sessionstate.h"

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
        unsigned int timeout; // login timeout
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

    class CLogin7 : public CReqBase {
	private:
		SPA::CScopeUQueue m_sb;
        static const unsigned int YUKON_LOG_REC_FIXED_LEN = 0x5e;
    public:
#pragma pack(push,1)

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

#pragma pack(pop)
        static_assert(sizeof (OptionalFlags1) == 1, "Wrong OptionalFlags1 size");
        static_assert(sizeof (OptionalFlags2) == 1, "Wrong OptionalFlags2 size");
        static_assert(sizeof (TypeFlags) == 1, "Wrong TypeFlags size");
        static_assert(sizeof (OptionalFlags3) == 1, "Wrong OptionalFlags3 size");
        static_assert(sizeof (FeatureExtension) == 4, "Wrong FeatureExtension size");

        struct LoginAck {
            unsigned int Tds_Version = 0;
            CDBString ServerName;
            unsigned int ServerVersion = 0;
        };

        enum class tagFeatureID : unsigned char {
            fiSessionRecovery = 0x01, //Session recovery (connection resiliency), introduced in TDS 7.4
            fiFederatedAuthentication = 0x02 //Federated authentication, introduced in TDS 7.4
        };

        CLogin7();

    public:
        bool GetClientMessage(unsigned char packet_id, const SqlLogin &rec, FeatureExtension requestedFeatures, SPA::CUQueue &buffer);
        void OnResponse(const unsigned char *data, unsigned int bytes);

    protected:
        void Reset();

    private:
        static CDBString LibraryName; //Client library name

    private:
		SPA::CUQueue &m_buffer;
        TokenDone m_Done;
        std::vector<TokenEventChange> m_vEventChange;
        std::vector<TokenInfo> m_vInfo;
        CollationChange m_CollationChange;
        LoginAck m_LoginAck;
    };

}

#endif