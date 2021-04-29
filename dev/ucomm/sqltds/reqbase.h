#ifndef _U_TDS_REQUEST_BASE_H_
#define _U_TDS_REQUEST_BASE_H_

#include "tdsdef.h"

namespace tds {

    class CReqBase {
	private:
		SPA::CScopeUQueue m_sb;
    public:
        CReqBase();
        virtual ~CReqBase();

    public:
        virtual void OnResponse(const unsigned char *data, unsigned int bytes);
        virtual bool IsDone() const;
        const PacketHeader& GetResponseHeader() const;
		bool HasMore() const;
		UINT64 GetCount() const;

    protected:
        virtual void Reset();
		virtual bool ParseDone();
		virtual bool ParseErrorInfo();
		bool ParseCollation(CollationChange &cc);
		void ParseStringChange(tagEnvchangeType type, StringEventChange& sec);
		virtual bool ParseStream() = 0;

    protected:
		SPA::CUQueue &m_buffer;
		tagTokenType m_tt;
        PacketHeader ResponseHeader;
		TokenDone m_Done;
		std::vector<TokenInfo> m_vInfo;
    };

}

#endif