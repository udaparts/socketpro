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

	static std::vector<unsigned char> TDS_NIC_ADDRESS({ 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc });

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
		ttFixedLength,
		ttVariableLength,
		ttVariableCount,
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
		DATETIMN = 0x6F,
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

	enum class SqlAuthenticationMethod
	{
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
		unsigned char *p = (unsigned char *)&s;
		unsigned char b = p[0];
		p[0] = p[3];
		p[3] = b;
		b = p[1];
		p[1] = p[2];
		p[2] = b;
		return s;
	}

	static inline int ChangeEndian(int s) {
		unsigned char *p = (unsigned char *)&s;
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
		switch (token)
		{
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
		return ChangeEndian((SPID)tid);
	}

	static unsigned int GetThreadId() {
#ifdef WIN32_64
		UTHREAD_ID tid = ::GetCurrentThreadId();
#else
		UTHREAD_ID tid = pthread_self();
#endif
		return (unsigned int)tid;
	}
#pragma pack(push,1)
	struct PacketHeader {
		PacketHeader(tagPacketType type, unsigned char packetId) : Type(type), PacketID(packetId) {
		}
		tagPacketType Type;
		tagPacketStatus Status = tagPacketStatus::psEOM;
		Packet_Length Length;
		SPID Spid;
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

	struct Collation {
		unsigned short CodePage = 0; //LCID
		unsigned short Flags = 0;
		unsigned char CharsetId = 0;
	};
#pragma pack(pop)
	static_assert(sizeof(PacketHeader) == 8, "Wrong PacketHeader size");
	static_assert(sizeof(TokenDone) == 12, "Wrong TokenDone size");
	static_assert(sizeof(Collation) == 5, "Wrong Collation size");

	struct TokenEventChange {
		tagEnvchangeType Type;
		CDBString NewValue;
		CDBString OldValue;
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
		unsigned int LineNumber = 0;
	};

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
			bLo = (unsigned char)(c16 & 0xff);
			bHi = (unsigned char)((c16 >> 8) & 0xff);
			bObfuscated[n << 1] = (unsigned char)((((bLo & 0x0f) << 4) | (bLo >> 4)) ^ 0xa5);
			bObfuscated[(n << 1) + 1] = (unsigned char)((((bHi & 0x0f) << 4) | (bHi >> 4)) ^ 0xa5);
		}
#else
		const unsigned char *start = (const unsigned char *)password.data();
		std::vector<unsigned char> v(start, start + (password.size() << 1));
#endif
		return v;
	}

	static unsigned char* ObfuscatePassword(unsigned char* password, unsigned int bytes)
	{
		unsigned char bLo, bHi;
		for (unsigned int i = 0; i < bytes; i++)
		{
			bLo = (password[i] & 0x0f);
			bHi = (password[i] & 0xf0);
			password[i] = (((bHi >> 4) | (bLo << 4)) ^ 0xa5);
		}
		return password;
	}
};


#endif
