#ifndef _U_TDS_REQUEST_BASE_H_
#define _U_TDS_REQUEST_BASE_H_

#include "tdsdef.h"
#include "../include/channelpool.h"
#include <mutex>
#include <condition_variable>

namespace tds {

    class CReqBase {
	private:
		SPA::CScopeUQueue m_sb;

    public:
        CReqBase(SPA::CBaseHandler &channel);
        virtual ~CReqBase();

    public:
        virtual void OnResponse(const unsigned char *data, unsigned int bytes);
        virtual bool IsDone() const;
        const PacketHeader& GetResponseHeader() const;
		bool HasMore() const;
		UINT64 GetCount() const;
		virtual bool Wait(unsigned int ms);

    protected:
        virtual void Reset();
		virtual bool ParseDone();
		virtual bool ParseErrorInfo();
		bool ParseCollation(CollationChange &cc);
		void ParseStringChange(tagEnvchangeType type, StringEventChange& sec);
		virtual bool ParseStream() = 0;

    protected:
		typedef std::unique_lock<std::mutex> CAutoLock;
		SPA::CBaseHandler& m_channel;
		std::mutex m_cs;
		std::condition_variable m_cv;
		SPA::CUQueue &m_buffer;
		tagTokenType m_tt;
        PacketHeader ResponseHeader;
		TokenDone m_Done;
		std::vector<TokenInfo> m_vInfo;
    };

}

#endif