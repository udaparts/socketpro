#ifndef _H_SQL_TDS_DEFINES_H_
#define _H_SQL_TDS_DEFINES_H_

#include "../include/commutil.h"

namespace tds {
	static const unsigned short BUILD_VERSION = 0x0100;
	static const unsigned int TDS_VERSION = 0x04000074;

	enum class tagPacketType : unsigned char {
		ptBatch = 1, //SQL batch
		ptPre7Login = 2, //Pre-TDS7 Login
		ptRpc = 3,
		ptTabular = 4, //Tabular result, server-only response type
		ptAttention = 6, //Attention signal
		ptBulk = 7, //Bulk load data
		ptFederated = 8, //Federated Authentication Token
		ptTransaction = 14, //Transaction manager request
		ptLogin7 = 16, //TDS7 Login
		ptSspi = 17,
		ptPrelogin = 18 //Pre-Login
	};

	enum class tagPacketStatus : unsigned char {
		psNormal = 0,
		psEOM = 1,
		psIgnore = 2, //client ==> server + psEOM
		psResetConnection = 8, //client ==> server
		psResetConnectionSkipTran = 16, //client ==> server
	};

	enum class tagTokenType : char {
		ttUnknown = -1,
		ttZero = 0,
		ttFixedLength,
		ttVariableLength,
		ttVariableCount
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

	typedef unsigned short Packet_Length; //big-endian 512 - 32,767
	typedef unsigned short SPID; //big-endian
	typedef unsigned char Token;

	static const Token TOKEN_TERMINATOR = 0xff;

	static inline unsigned short ChangeEndian(unsigned short s) {
		return ((s & 0xff) << 8) + (s >> 8);
	}

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
			assert(false); //shouldn't come here
			break;
		}
		return tagTokenType::ttUnknown;
	}

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

	static_assert(sizeof(PacketHeader) == 8, "Wrong PacketHeader size");
};


#endif
