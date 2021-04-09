#ifndef _H_SQL_TDS_DEFINES_H_
#define _H_SQL_TDS_DEFINES_H_

namespace tds {

	enum class tagPacketType : unsigned char {
		ptBatch = 1, //SQL batch
		ptPre7Login = 2, //Pre-TDS7 Login
		ptRpc = 3,
		ptTabular = 4, //Tabular result, server-only response type
		ptAttention = 6, //Attention signal
		ptBulk = 7, //Bulk load data
		ptFederated = 8, //Federated Authentication Token
		ptTransaction = 14, //Transaction manager request
		rLogin7 = 16, //TDS7 Login
		rSspi = 17,
		rPrelogin = 18 //Pre-Login
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

	typedef unsigned short Packet_Length; //big-endian 512 - 32,767
	typedef unsigned short SPID; //big-endian
	typedef unsigned char Token;

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

	struct PacketHeader {
		tagPacketType Type;
		tagPacketStatus Status = tagPacketStatus::psEOM;
		Packet_Length Length;
		SPID Spid = GetSPID();
		unsigned char PacketID;
		unsigned char Window = 0; //ignored
	};

	static_assert(sizeof(PacketHeader) == 8, "Wrong PacketHeader size");
};


#endif
