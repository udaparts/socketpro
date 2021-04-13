#ifndef _U_TDS_PRELOGIN_H_
#define _U_TDS_PRELOGIN_H_

#include "reqbase.h"

namespace tds {

	class Prelogin : public CReqBase
	{
	public:
		enum tagOptionToken : unsigned char {
			VERSION = 0,
			ENCRYPTION = 1,
			INSTOPT = 2,
			THREADID = 3,
			MARS = 4,
			TRACEID = 5,
			FEDAUTHREQUIRED = 6,
			NONCEOPT = 7
		};

#pragma pack(push,1)
		struct Option {
			tagOptionToken Token;
			unsigned short Offset; //big endian
			unsigned short Len; //Big endian
		};
#pragma pack(pop)
		static_assert(sizeof(Option) == 5, "Wrong Option size");

	public:
		Prelogin(bool mars_enabled = false, bool fed_auth_required = false);
		bool GetClientMessage(unsigned char packet_id, SPA::CUQueue &buffer);
		void OnResponse(const unsigned char *data, unsigned int bytes);

	private:
		unsigned char m_bInst;
		unsigned char m_bMars;
		unsigned char m_bFed;
		unsigned int Version;
		unsigned short SubBuild;
		unsigned char m_bEncryption;
		unsigned int m_nThreadId;
		unsigned char ClientTraceID[16];
		unsigned char ActivityID[20];
		unsigned char Nonce[32];
		std::vector<tds::Prelogin::Option> Options;
	};


}

#endif
