#ifndef _U_TDS_LOGIN7_H_
#define _U_TDS_LOGIN7_H_

#include "reqbase.h"
#include "sessionstate.h"

namespace tds {
	class CLogin7 : public CReqBase
	{
	public:
#pragma pack(push,1)
		struct OptionalFlags1 {
			OptionalFlags1() {
				::memset(this, 0, sizeof(OptionalFlags1));
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
				::memset(this, 0, sizeof(OptionalFlags2));
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
				::memset(this, 0, sizeof(TypeFlags));
			}
			unsigned char fSQLType : 4;
			unsigned char fOLEDB : 1; //introduced in TDS 7.2
			unsigned char fReadOnlyIntent : 1; //introduced in TDS 7.4
			unsigned char fReserved : 2;
		};

		struct OptionalFlags3 {
			OptionalFlags3() {
				::memset(this, 0, sizeof(OptionalFlags3));
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
				::memset(this, 0, sizeof(FeatureExtension));
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
				return *((unsigned int*)this);
			}
		};

#pragma pack(pop)
		static_assert(sizeof(OptionalFlags1) == 1, "Wrong OptionalFlags1 size");
		static_assert(sizeof(OptionalFlags2) == 1, "Wrong OptionalFlags2 size");
		static_assert(sizeof(TypeFlags) == 1, "Wrong TypeFlags size");
		static_assert(sizeof(OptionalFlags3) == 1, "Wrong OptionalFlags3 size");
		static_assert(sizeof(FeatureExtension) == 4, "Wrong FeatureExtension size");

		enum class tagFeatureID : unsigned char
		{
			fiSessionRecovery = 0x01, //Session recovery (connection resiliency), introduced in TDS 7.4
			fiFederatedAuthentication = 0x02 //Federated authentication, introduced in TDS 7.4
		};

		class FeatureOptionToken : public ISerialize, public IDeserialize {
		public:
			FeatureOptionToken(tagFeatureID fi) : FeatureID(fi) {}

		public:
			tagFeatureID GetFeatureID() const { return FeatureID; }

		protected:
			void SetFeatureID(tagFeatureID fi) { FeatureID = fi; }

		private:
			tagFeatureID FeatureID;
		};

		class SessionRecoveryOptionToken : public FeatureOptionToken {
		public:
			SessionRecoveryOptionToken() : FeatureOptionToken(tagFeatureID::fiSessionRecovery) {
			}

		public:
			bool SaveTo(SPA::CUQueue &buff) {
				return false;
			}

			bool LoadFrom(const unsigned char *data, unsigned int bytes) {
				return false;
			}

		public:
			
		};

		CLogin7(FeatureExtension requestedFeatures, bool integrated = false, bool dump = false, unsigned int packet_size = 8000, bool readOnlyIntent = false);

	public:
		bool GetClientMessage(unsigned char packet_id, SPA::CUQueue &buffer);
		void OnResponse(const unsigned char *data, unsigned int bytes);
		bool IsDone() const { return false; }

	private:
		FeatureExtension m_fe;
		unsigned int TDSVersion;
		unsigned int PacketSize;
		unsigned int ClientProgVer;
		unsigned int ClientPID;
		unsigned int ConnectionID;
		OptionalFlags1 Option1;
		OptionalFlags2 Option2;
		TypeFlags TypeFlags;
		OptionalFlags3 Option3;
		int ClientTimeZone;
		unsigned int ClientLCID; //LCID ColFlags Version
		CDBString HostName; //Client machine name
		CDBString UserID;
		CDBString Password;
		CDBString AppName;
		CDBString ServerName;
		CDBString LibraryName; //Client library name
		CDBString Language; //User language
		CDBString Database;
		unsigned char ClientID[6];
		unsigned char *SSPI;
		CDBString AttchDBFile; //introduced in TDS 7.2
		CDBString ChangePassword; //introduced in TDS 7.2
		int SSPILong; //introduced in TDS 7.2

	private:
		static const unsigned short FixedPacketLength = sizeof(unsigned int)  // Length
			+ sizeof(unsigned int)  // TDSVersion
			+ sizeof(unsigned int)  // PacketSize
			+ sizeof(unsigned int)  // ClientProgramVersion
			+ sizeof(unsigned int)  // ClientPID
			+ sizeof(unsigned int)  // ConnectionID
			+ sizeof(OptionalFlags1)  // OptionalFlags1
			+ sizeof(OptionalFlags2)  // OptionalFlags2
			+ sizeof(TypeFlags)  // TypeFlags
			+ sizeof(OptionalFlags3)  // OptionalFlags3
			+ sizeof(int)  // ClientTimeZone
			+ sizeof(unsigned int)  // ClientLCID
			+ sizeof(unsigned short) + sizeof(unsigned short)  // HostName
			+ sizeof(unsigned short) + sizeof(unsigned short)  // UserID
			+ sizeof(unsigned short) + sizeof(unsigned short)  // Password
			+ sizeof(unsigned short) + sizeof(unsigned short)  // ApplicationName
			+ sizeof(unsigned short) + sizeof(unsigned short)  // ServerName
			+ sizeof(unsigned short) + sizeof(unsigned short)  // Unused
			+ sizeof(unsigned short) + sizeof(unsigned short)  // LibraryName
			+ sizeof(unsigned short) + sizeof(unsigned short)  // Language
			+ sizeof(unsigned short) + sizeof(unsigned short)  // Database
			+ sizeof(ClientID)  // ClientID
			+ sizeof(unsigned short) + sizeof(unsigned short)  // SSPI
			+ sizeof(unsigned short) + sizeof(unsigned short)  // AttachDatabaseFile
			+ sizeof(unsigned short) + sizeof(unsigned short)  // ChangePassword
			+ sizeof(unsigned int);  // LongSSPI;
	};

}

#endif